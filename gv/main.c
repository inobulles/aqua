// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2024 Aymeric Wibo

#include "conn.h"
#include "elp.h"
#include "internal.h"
#include "vdev.h"

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <sys/file.h>
#include <sys/socket.h>

// TODO Spawn new processes out of each connection instead of just threads?

int main(int argc, char* argv[]) {
	int rv = EXIT_FAILURE;
	state_t state;

	// Parse options.

	char* interface_name = NULL;

	int c;

	while ((c = getopt(argc, argv, "i:")) != -1) {
		switch (c) {
		case 'i':
			interface_name = optarg;
			break;
		default:
			fprintf(stderr, "Unknown option: %c\n", c);
			goto err_getopt;
		}
	}

	argc -= optind;
	argv += optind;

	if (argc != 0) {
		fprintf(stderr, "More arguments than expected.\n");
		goto err_getopt;
	}

	if (interface_name == NULL) {
		fprintf(stderr, "Interface name (-i) is required.\n");
		goto err_getopt;
	}

	// Acquire the lock file.
	// https://0pointer.de/blog/projects/locking.html

	FILE* const lock_file = fopen(GV_LOCK_PATH, "w");

	if (lock_file == NULL) {
		fprintf(stderr, "fopen: %s\n", strerror(errno));
		goto err_lock;
	}

	if (flock(fileno(lock_file), LOCK_EX | LOCK_NB) < 0) {
		if (errno == EWOULDBLOCK) {
			fprintf(stderr, "Another instance of the GrapeVine daemon is already running.\n");
		}

		else {
			fprintf(stderr, "flock: %s\n", strerror(errno));
		}

		goto err_lock;
	}

	fprintf(lock_file, "%d\n", getpid());

	// Make an inventory of the local VDEV's we can make available.

	state.vdev_count = 0;
	state.vdevs = NULL;

	if (vdev_inventory(&state) < 0) {
		goto err_vdev_inventory;
	}

	printf("Found and inventoried %zu VDEVs.\n", state.vdev_count);

	// Find the interface's IPv4 address.

	struct ifaddrs* ifap;

	if (getifaddrs(&ifap) < 0) {
		fprintf(stderr, "getifaddrs: %s\n", strerror(errno));
		goto err_getifaddrs;
	}

	bool iface_found = false;

	state.found_ether = NULL;
	state.found_ipv4 = NULL;

	for (struct ifaddrs* ifa = ifap; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL) {
			continue;
		}

		if (strcmp(ifa->ifa_name, interface_name) != 0) {
			continue;
		}

		iface_found = true;

		switch (ifa->ifa_addr->sa_family) {
		case AF_LINK:
			state.found_ether = ifa;
			break;
		case AF_INET:
			state.found_ipv4 = ifa;
			break;
		default:
			break;
		}
	}

	if (state.found_ipv4 == NULL || state.found_ether == NULL) {
		fprintf(stderr, iface_found ? "Interface %s does not have required addresses.\n" : "Interface %s not found.\n", interface_name);
		goto err_no_iface;
	}

	// Create socket for TCP connections.

	state.sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (state.sock < 0) {
		fprintf(stderr, "socket: %s\n", strerror(errno));
		goto err_socket;
	}

	setsockopt(state.sock, SOL_SOCKET, SO_REUSEADDR, &(int) {1}, sizeof(int));

	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_port = htons(GV_PORT),
		.sin_addr.s_addr = htonl(INADDR_ANY),
	};

	if (bind(state.sock, (struct sockaddr*) &addr, sizeof addr) < 0) {
		fprintf(stderr, "bind: %s\n", strerror(errno));
		goto err_bind;
	}

	// Run the ELP stuff.

	if (elp(&state) < 0) {
		goto err_elp;
	}

	// Start listening for connections.

	if (listen(state.sock, 5) < 0) { // What's a sane default for the backlog number? Should this be configurable?
		fprintf(stderr, "listen: %s\n", strerror(errno));
		goto err_listen;
	}

	// Wait for connections.

	printf("GrapeVine daemon bound to port 0x%x and listening for connections.\n", GV_PORT);

	state.connection_count = 0;
	state.connections = NULL;

	for (;;) {
		conn_t conn;

		conn.state = &state;
		conn.sock = accept(state.sock, (struct sockaddr*) &conn.addr, &conn.addr_len);

		if (conn.sock < 0) {
			fprintf(stderr, "accept: %s\n", strerror(errno));
			goto err_accept;
		}

		// Connection started, find a slot for it.

		conn_t* slot = NULL;

		for (size_t i = 0; i < state.connection_count; i++) {
			slot = &state.connections[i];

			if (!slot->slot_used) {
				break;
			}
		}

		if (slot == NULL) {
			state.connections = realloc(state.connections, ++state.connection_count * sizeof *state.connections);
			assert(state.connections != NULL);
			slot = &state.connections[state.connection_count - 1];
		}

		*slot = conn;

		slot->slot_used = true;
		pthread_create(&slot->thread, NULL, conn_thread, slot);
	}

	rv = EXIT_SUCCESS;

err_accept:

	if (state.connections != NULL) {
		for (size_t i = 0; i < state.connection_count; i++) {
			conn_t* const conn = &state.connections[i];

			if (conn->slot_used) {
				pthread_join(conn->thread, NULL);
			}
		}

		free(state.connections);
	}

err_listen:
err_bind:

	close(state.sock);

err_socket:
err_elp:

	elp_free(&state);

err_no_iface:

	freeifaddrs(ifap);

err_getifaddrs:

	if (state.vdevs != NULL) {
		free(state.vdevs);
	}

err_vdev_inventory:

	flock(fileno(lock_file), LOCK_UN); // XXX Shouldn't be necessary, but just in case.
	fclose(lock_file);

err_lock:
err_getopt:

	return rv;
}
