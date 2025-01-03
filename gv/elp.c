// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2024 Aymeric Wibo

#include "elp.h"
#include "internal.h"
#include "query.h"

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>

static void write_nodes(state_t* state) {
	FILE* const f = fopen(GV_NODES_PATH, "w");

	if (f == NULL) {
		fprintf(stderr, "fopen: %s\n", strerror(errno));
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
}

static void* elp_sender(void* arg) {
	state_t* const state = arg;

	// Build ELP packet and address info.

	uint64_t const mac = sockaddr_to_mac(state->found_ether->ifa_addr) | 0x11AD; // Approximation of Link-Local ADdress.
	srand(time(NULL));

	packet_t const packet = {
		.header.type = ELP,
		.elp.unique = rand(),
		.elp.vers = ELP_VERS,
		.elp.host = mac,
	};

	size_t const packet_size = sizeof packet.header + sizeof packet.elp;

	struct sockaddr_in const addr = {
		.sin_family = AF_INET,
		.sin_port = htons(ELP_PORT),
		.sin_addr.s_addr = sockaddr_to_in_addr(state->found_ipv4->ifa_broadaddr),
	};

	for (;;) {
		// Send ELP packet.

		if (sendto(state->elp_sock, &packet, packet_size, 0, (struct sockaddr*) &addr, sizeof addr) != sizeof packet) {
			fprintf(stderr, "sendto: %s\n", strerror(errno));
			exit(EXIT_FAILURE); // XXX
		}

		// Decrement TTL of all known nodes.

		pthread_mutex_lock(&state->nodes_mutex);

		for (size_t i = 0; i < state->node_count; i++) {
			node_t* const node = &state->nodes[i];

			if (!node->slot_used) {
				continue;
			}

			node->ttl -= ELP_DELAY;

			if (node->ttl <= 0) {
				printf("Node with host ID %" PRIx64 " is considered dead.\n", node->host);
				node->slot_used = false;
				write_nodes(state);
			}
		}

		pthread_mutex_unlock(&state->nodes_mutex);

		// Wait for $ELP_DELAY seconds.

		sleep(ELP_DELAY);
	}

	return NULL;
}

static void* elp_listener(void* arg) {
	state_t* const state = arg;

	in_addr_t const my_in_addr = sockaddr_to_in_addr(state->found_ipv4->ifa_addr);

	for (;;) {
		struct sockaddr_in recv_addr;
		socklen_t recv_addr_len = sizeof recv_addr;

		packet_t buf;
		ssize_t const len = recvfrom(state->elp_sock, &buf, sizeof buf, 0, (struct sockaddr*) &recv_addr, &recv_addr_len);

		if (len < 0) {
			fprintf(stderr, "recvfrom: %s\n", strerror(errno));
			exit(EXIT_FAILURE); // XXX
		}

		if (recv_addr.sin_addr.s_addr == my_in_addr) {
			continue;
		}

		if (buf.header.type != ELP) {
			fprintf(stderr, "Received unknown packet type %d of length %zu.\n", buf.header.type, len);
			continue;
		}

		if (len != sizeof buf.header + sizeof buf.elp) {
			fprintf(stderr, "Received packet of unexpected length %zu.\n", len);
			continue;
		}

		if (buf.elp.vers != ELP_VERS) {
			fprintf(stderr, "Received ELP packet with unknown version %d.\n", buf.elp.vers);
			continue;
		}

		// Look for the host ID in the list of known nodes.
		// If we already have a node with this host ID, just refresh its TTL.
		// If it's unique has changed since last time we saw it, discard the previous one and start over.

		pthread_mutex_lock(&state->nodes_mutex);
		node_t* found = NULL;
		bool unique_changed = false;

		for (size_t i = 0; i < state->node_count; i++) {
			node_t* const node = &state->nodes[i];

			if (!node->slot_used) {
				found = node;
				continue;
			}

			if (node->host == buf.elp.host) {
				node->ttl = NODE_TTL;

				if (node->unique == buf.elp.unique) {
					goto nodes_mutex_unlock;
				}

				unique_changed = true;
				found = node;

				break;
			}
		}

		// We know we're going to have to fill in the node data by this point.
		// Before we do anything, query a list of VDEV's from the node.

		size_t vdev_count;
		kos_vdev_descr_t* vdevs;

		if (query(recv_addr.sin_addr.s_addr, &vdev_count, &vdevs) < 0) {
			goto nodes_mutex_unlock;
		}

		// If we didn't find an empty slot or a matching host ID, create a new node.

		if (found == NULL) {
			state->nodes = realloc(state->nodes, ++state->node_count * sizeof *state->nodes);
			assert(state->nodes != NULL);
			found = &state->nodes[state->node_count - 1];
		}

		// Fill in the node information.

		char const* const verb = unique_changed ? "Updated" : "Found new";
		printf("%s node with host ID %" PRIx64 " (at %s) with %zu VDEVs.\n", verb, buf.elp.host, inet_ntoa(recv_addr.sin_addr), vdev_count);

		found->slot_used = true;
		found->unique = buf.elp.unique;
		found->host = buf.elp.host;
		found->addr = recv_addr;
		found->ttl = NODE_TTL;

		// Create node entry for writing out to file (for IPC).

		size_t const vdevs_bytes = vdev_count * sizeof *vdevs;
		found->ent_bytes = sizeof(node_ent_t) + vdevs_bytes;
		found->ent = malloc(found->ent_bytes);
		assert(found->ent != NULL);

		found->ent->host = found->host;
		found->ent->ipv4 = found->addr.sin_addr.s_addr;
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

	// Create socket, set options, and bind it.

	state->elp_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (state->elp_sock < 0) {
		fprintf(stderr, "socket: %s\n", strerror(errno));
		return -1;
	}

	setsockopt(state->elp_sock, SOL_SOCKET, SO_REUSEADDR, &(int) {1}, sizeof(int));

	if (setsockopt(state->elp_sock, SOL_SOCKET, SO_BROADCAST, &(int) {1}, sizeof(int)) < 0) {
		fprintf(stderr, "setsockopt(SO_BROADCAST): %s\n", strerror(errno));
		return -1;
	}

	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_port = htons(ELP_PORT),
		.sin_addr.s_addr = htonl(INADDR_ANY),
	};

	if (bind(state->elp_sock, (struct sockaddr*) &addr, sizeof addr) < 0) {
		fprintf(stderr, "bind: %s\n", strerror(errno));
		return -1;
	}

	// Start ELP sender and listener threads.

	pthread_mutex_init(&state->nodes_mutex, NULL);

	pthread_create(&state->elp_sender_thread, NULL, elp_sender, state);
	pthread_create(&state->elp_listener_thread, NULL, elp_listener, state);

	state->elp_threads_started = true;

	return 0;
}

void elp_free(state_t* state) {
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
}
