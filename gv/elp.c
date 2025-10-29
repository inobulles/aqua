// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2024 Aymeric Wibo

#include "elp.h"
#include "gv.h"
#include "query.h"

#include <aqua/gv_ipc.h>
#include <aqua/gv_proto.h>

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>

static void write_nodes(state_t* state) {
	LOG_V(state->elp_cls, "Writing nodes to %s.", gv_get_nodes_path());
	FILE* const f = fopen(gv_get_nodes_path(), "w");

	if (f == NULL) {
		LOG_E(state->elp_cls, "fopen: %s", strerror(errno));
		exit(EXIT_FAILURE); // XXX
	}

	for (size_t i = 0; i < state->node_count; i++) {
		node_t* const node = &state->nodes[i];

		if (!node->slot_used) {
			continue;
		}

		fwrite(node->ent, node->ent_bytes, 1, f);
	}

	fclose(f);
	LOG_V(state->elp_cls, "Wrote %zu nodes to %s.", state->node_count, gv_get_nodes_path());
}

static void* elp_sender(void* arg) {
	state_t* const state = arg;

	LOG_V(state->elp_cls, "Building ELP packet and address info.");

	srand(time(NULL));

	gv_packet_t const packet = {
		.header.type = GV_PACKET_TYPE_ELP,
		.elp.unique = rand(),
		.elp.vers = GV_ELP_VERS,
		.elp.host_id = state->host_id,
		.elp.name = "TODO",
	};

	size_t const packet_size = sizeof packet.header + sizeof packet.elp;

	struct sockaddr_in const addr = {
		.sin_family = AF_INET,
		.sin_port = htons(GV_ELP_PORT),
		.sin_addr.s_addr = sockaddr_to_in_addr(state->found_ipv4->ifa_broadaddr),
	};

	for (;;) {
		LOG_V(state->elp_cls, "Broadcasting ELP packet.");

		if (sendto(state->elp_sock, &packet, packet_size, 0, (struct sockaddr*) &addr, sizeof addr) != sizeof packet) {
			LOG_E(state->elp_cls, "sendto: %s", strerror(errno));
			exit(EXIT_FAILURE); // XXX
		}

		LOG_V(state->elp_cls, "Decrementing TTL of all known nodes.");
		pthread_mutex_lock(&state->nodes_mutex);

		for (size_t i = 0; i < state->node_count; i++) {
			node_t* const node = &state->nodes[i];

			if (!node->slot_used) {
				continue;
			}

			node->ttl -= ELP_DELAY;

			if (node->ttl <= 0) {
				LOG_I(state->elp_cls, "Node with host ID 0x%" PRIx64 " is considered dead.", node->host);
				node->slot_used = false;
				write_nodes(state);
			}
		}

		pthread_mutex_unlock(&state->nodes_mutex);

		LOG_V(state->elp_cls, "Waiting for %d seconds before sending the next ELP packet.", ELP_DELAY);
		sleep(ELP_DELAY);
	}

	return NULL;
}

static void* elp_listener(void* arg) {
	state_t* const state = arg;

	in_addr_t const my_in_addr = sockaddr_to_in_addr(state->found_ipv4->ifa_addr);

	LOG_I(state->elp_cls, "Listening for ELP packets on %s.", inet_ntoa(*(struct in_addr*) &my_in_addr));

	for (;;) {
		LOG_V(state->elp_cls, "Waiting for ELP packet.");

		struct sockaddr_in recv_addr;
		socklen_t recv_addr_len = sizeof recv_addr;

		gv_packet_t buf;
		ssize_t const len = recvfrom(state->elp_sock, &buf, sizeof buf, 0, (struct sockaddr*) &recv_addr, &recv_addr_len);

		if (len < 0) {
			LOG_E(state->elp_cls, "recvfrom: %s", strerror(errno));
			exit(EXIT_FAILURE); // XXX
		}

		if (recv_addr.sin_addr.s_addr == my_in_addr) {
			continue;
		}

		if (buf.header.type != GV_PACKET_TYPE_ELP) {
			LOG_W(state->elp_cls, "Received unknown packet type %d of length %zu. Ignoring.", buf.header.type, len);
			continue;
		}

		if (len != sizeof buf.header + sizeof buf.elp) {
			LOG_W(state->elp_cls, "Received packet of unexpected length %zu. Ignoring.", len);
			continue;
		}

		if (buf.elp.vers != GV_ELP_VERS) {
			LOG_W(state->elp_cls, "Received ELP packet with unsupported version %d. Ignoring.", buf.elp.vers);
			continue;
		}

		// Look for the host ID in the list of known nodes.
		// If we already have a node with this host ID, just refresh its TTL.
		// If it's unique has changed since last time we saw it, discard the previous one and start over.

		LOG_V(
			state->elp_cls,
			"Received ELP packet from %s:0x%x (host_id=0x%" PRIx64 ").",
			inet_ntoa(recv_addr.sin_addr),
			ntohs(recv_addr.sin_port),
			buf.elp.host_id
		);

		pthread_mutex_lock(&state->nodes_mutex);
		node_t* found = NULL;
		bool unique_changed = false;

		for (size_t i = 0; i < state->node_count; i++) {
			node_t* const node = &state->nodes[i];

			if (!node->slot_used) {
				found = node;
				continue;
			}

			if (node->host == buf.elp.host_id) {
				node->ttl = NODE_TTL;

				if (node->unique == buf.elp.unique) {
					LOG_V(state->elp_cls, "Is existing node and unique has not changed.", node->host);
					goto nodes_mutex_unlock;
				}

				unique_changed = true;
				found = node;

				break;
			}
		}

		// We know we're going to have to fill in the node data by this point.
		// Before we do anything, query a list of VDEV's from the node.

		LOG_V(state->elp_cls, "Querying VDEVs from node.");

		size_t vdev_count;
		kos_vdev_descr_t* vdevs;

		if (query(recv_addr.sin_addr.s_addr, &vdev_count, &vdevs) < 0) {
			LOG_E(state->elp_cls, "Failed to query VDEVs.");
			goto nodes_mutex_unlock;
		}

		// If we didn't find an empty slot or a matching host ID, create a new node.

		if (found == NULL) {
			LOG_V(state->elp_cls, "No matching node found, creating a new one.");

			state->nodes = realloc(state->nodes, ++state->node_count * sizeof *state->nodes);
			assert(state->nodes != NULL);
			found = &state->nodes[state->node_count - 1];
		}

		// Fill in the node information.

		char const* const verb = unique_changed ? "Updated" : "Found new";

		LOG_I(
			state->elp_cls,
			"%s node with host ID 0x%" PRIx64 " (at %s) with %zu VDEVs.",
			verb,
			buf.elp.host_id,
			inet_ntoa(recv_addr.sin_addr),
			vdev_count
		);

		found->slot_used = true;
		found->unique = buf.elp.unique;
		found->host = buf.elp.host_id;
		found->addr = recv_addr;
		found->ttl = NODE_TTL;

		// Create node entry for writing out to file (for IPC).

		size_t const vdevs_bytes = vdev_count * sizeof *vdevs;
		found->ent_bytes = sizeof(gv_node_ent_t) + vdevs_bytes;
		found->ent = malloc(found->ent_bytes);
		assert(found->ent != NULL);

		found->ent->host_id = found->host;
		found->ent->ip.v4 = found->addr.sin_addr.s_addr;
		found->ent->vdev_count = vdev_count;
		memcpy(found->ent->vdevs, vdevs, vdevs_bytes);
		free(vdevs);

		write_nodes(state);

nodes_mutex_unlock:

		pthread_mutex_unlock(&state->nodes_mutex);
	}

	return NULL;
}

int elp(state_t* state) {
	state->nodes = NULL;
	state->node_count = 0;

	state->elp_threads_started = false;

	LOG_V(state->elp_cls, "Creating & binding ELP socket.");

	state->elp_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (state->elp_sock < 0) {
		LOG_F(state->elp_cls, "socket: %s", strerror(errno));
		return -1;
	}

	setsockopt(state->elp_sock, SOL_SOCKET, SO_REUSEADDR, &(int) {1}, sizeof(int));

	if (setsockopt(state->elp_sock, SOL_SOCKET, SO_BROADCAST, &(int) {1}, sizeof(int)) < 0) {
		LOG_F(state->elp_cls, "setsockopt(SO_BROADCAST): %s", strerror(errno));
		return -1;
	}

	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_port = htons(GV_ELP_PORT),
		.sin_addr.s_addr = htonl(INADDR_ANY),
	};

	if (bind(state->elp_sock, (struct sockaddr*) &addr, sizeof addr) < 0) {
		LOG_F(state->elp_cls, "bind: %s", strerror(errno));
		return -1;
	}

	LOG_I(state->elp_cls, "ELP socket bound to port 0x%x.", GV_ELP_PORT);
	LOG_V(state->elp_cls, "Starting ELP sender and listener threads.");

	pthread_mutex_init(&state->nodes_mutex, NULL);

	pthread_create(&state->elp_sender_thread, NULL, elp_sender, state);
	pthread_create(&state->elp_listener_thread, NULL, elp_listener, state);

	state->elp_threads_started = true;

	LOG_V(state->elp_cls, "ELP subsystem started successfully.");

	return 0;
}

void elp_free(state_t* state) {
	LOG_V(state->elp_cls, "Stopping ELP subsystem.");

	if (state->elp_threads_started) {
		pthread_join(state->elp_sender_thread, NULL);
		pthread_join(state->elp_listener_thread, NULL);
	}

	if (state->elp_sock >= 0) {
		close(state->elp_sock);
	}

	if (state->nodes != NULL) {
		for (size_t i = 0; i < state->node_count; i++) {
			free(state->nodes[i].ent);
		}

		free(state->nodes);
	}

	pthread_mutex_destroy(&state->nodes_mutex);

	LOG_V(state->elp_cls, "ELP subsystem stopped successfully.");
}
