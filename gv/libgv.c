// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2024 Aymeric Wibo

#include "../kos/kos.h"
#include "gv.h"
#include "internal.h"

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/file.h>

static size_t node_count = 0;
static node_ent_t nodes[256];

static void unlock(FILE* f) {
	flock(fileno(f), LOCK_UN);
	fclose(f);
}

static bool is_gvd_running(void) {
	FILE* const lock_file = fopen(GV_LOCK_PATH, "r");

	if (lock_file == NULL) {
		return false;
	}

	int const rv = flock(fileno(lock_file), LOCK_EX | LOCK_NB);

	if (rv == 0) { // If we got the lock, gvd is not running.
		unlock(lock_file);
		return false;
	}

	if (rv < 0 && errno != EWOULDBLOCK) { // Some other issue occurred.
		unlock(lock_file);
		return false;
	}

	unlock(lock_file);
	return true;
}

ssize_t gv_query_vdevs(kos_vdev_descr_t** vdevs_out) {
	*vdevs_out = NULL;

	if (!is_gvd_running()) {
		return 0;
	}

	// Actually read.

	FILE* const f = fopen(GV_NODES_PATH, "r");

	if (f == NULL) {
		return 0;
	}

	ssize_t vdev_count = 0;
	kos_vdev_descr_t* vdevs = NULL;

	node_count = 0;

	while (!feof(f)) {
		node_ent_t header;

		if (fread(&header, 1, sizeof header, f) != sizeof header) {
			goto done;
		}

		nodes[node_count++] = header;

		if (node_count >= sizeof nodes / sizeof *nodes) {
			fprintf(stderr, "Too many nodes in GrapeVine nodes file.\n");
			goto done;
		}

		size_t const vdevs_bytes = header.vdev_count * sizeof *vdevs;
		size_t const ent_size = sizeof header + vdevs_bytes;

		node_ent_t* const ent = malloc(ent_size);
		assert(ent != NULL);
		memcpy(ent, &header, sizeof header);

		if (fread(ent + sizeof header, 1, vdevs_bytes, f) != vdevs_bytes) {
			free(ent);
			fclose(f);

			return -1;
		}

		vdevs = realloc(vdevs, (vdev_count + header.vdev_count) * sizeof *vdevs);
		assert(vdevs != NULL);
		memcpy(vdevs + vdev_count, ent + sizeof header, vdevs_bytes);

		// Set the host ID of each reported VDEV to the host ID of the node.

		for (size_t i = 0; i < header.vdev_count; i++) {
			vdevs[vdev_count + i].host_id = header.host;
		}

		vdev_count += header.vdev_count;
		free(ent);
	}

done:

	fclose(f);
	*vdevs_out = vdevs;

	return vdev_count;
}

static void* vdev_conn_thread(void* arg) {
	for (;;) {
	}

	return NULL;
}

int gv_conn(gv_vdev_conn_t* conn, uint64_t host_id, uint64_t vdev_id) {
	// First, find the node with the host ID.

	if (!is_gvd_running()) {
		return -1;
	}

	node_ent_t* found = NULL;

	for (size_t i = 0; i < node_count; i++) {
		if (nodes[i].host == host_id) {
			if (found != NULL) {
				fprintf(stderr, "Found multiple nodes with host ID %" PRIx64 "\n", host_id);
				return -1;
			}

			found = &nodes[i];
		}
	}

	if (found == NULL) {
		fprintf(stderr, "Could not find a node with host ID %" PRIx64 "\n", host_id);
		return -1;
	}

	// Now, establish a TCP connection to that node.

	int const sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (sock < 0) {
		fprintf(stderr, "socket: %s\n", strerror(errno));
		return -1;
	}

	// Connect to the node.

	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_port = htons(GV_PORT),
		.sin_addr.s_addr = found->ipv4,
	};

	if (connect(sock, (struct sockaddr*) &addr, sizeof addr) < 0) {
		fprintf(stderr, "connect: %s\n", strerror(errno));
		return -1;
	}

	// Send CONN_VDEV packet.

	packet_t const packet = {
		.header.type = CONN_VDEV,
		.conn_vdev = {
			.vdev_id = vdev_id,
		},
	};

	size_t const size = sizeof packet.header + sizeof packet.conn_vdev;

	if (sendto(sock, &packet, size, 0, (struct sockaddr*) &addr, sizeof addr) != size) {
		fprintf(stderr, "sendto: %s\n", strerror(errno));
		return -1;
	}

	conn->sock = sock;
	pthread_create(&conn->thread, NULL, vdev_conn_thread, conn);

	return 0;
}
