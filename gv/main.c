// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2024 Aymeric Wibo

#include "conn.h"
#include "elp.h"
#include "gv.h"
#include "vdev.h"

#include <aqua/gv_ipc.h>

#include <umber.h>

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
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

int main(int argc, char* argv[]) {
	int rv = EXIT_FAILURE;
	state_t state;

	umber_class_t const* const init_cls = umber_class_new("aqua.gvd.init", UMBER_LVL_VERBOSE, "GrapeVine daemon initialization.");
	umber_class_t const* const conn_listener = umber_class_new("aqua.gvd.listener", UMBER_LVL_VERBOSE, "GrapeVine daemon connection listener.");

	LOG_V(init_cls, "Parsing options.");

	char* interface_name = NULL;

	int c;

	while ((c = getopt(argc, argv, "i:")) != -1) {
		switch (c) {
		case 'i':
			LOG_V(init_cls, "Setting interface name to %s.", optarg);
			interface_name = optarg;
			break;
		default:
			LOG_F(init_cls, "Unknown option: %c", c);
			goto err_getopt;
		}
	}

	argc -= optind;
	argv += optind;

	if (argc != 0) {
		LOG_F(init_cls, "More arguments than expected.");
		goto err_getopt;
	}

	if (interface_name == NULL) {
		LOG_F(init_cls, "Interface name (-i) is required.");
		goto err_getopt;
	}

	LOG_V(init_cls, "Acquiring lock file %s.", GV_LOCK_PATH);

	// See https://0pointer.de/blog/projects/locking.html

	FILE* const lock_file = fopen(GV_LOCK_PATH, "w");

	if (lock_file == NULL) {
		LOG_F(init_cls, "fopen: %s", strerror(errno));
		goto err_lock;
	}

	if (flock(fileno(lock_file), LOCK_EX | LOCK_NB) < 0) {
		if (errno == EWOULDBLOCK) {
			LOG_F(init_cls, "Another instance of the GrapeVine daemon is already running.");
		}

		else {
			LOG_F(init_cls, "flock: %s", strerror(errno));
		}

		goto err_lock;
	}

	fprintf(lock_file, "%d\n", getpid());

	LOG_V(init_cls, "Making an inventory of local VDEVs we can make available.");

	state.vdev_count = 0;
	state.vdevs = NULL;

	if (vdev_inventory(&state) < 0) {
		LOG_F(init_cls, "vdev_inventory failed.");
		goto err_vdev_inventory;
	}

	LOG_I(init_cls, "Found and inventoried %zu VDEVs.", state.vdev_count);
	LOG_V(init_cls, "Find the interface's IPv4 address.");

	struct ifaddrs* ifap;

	if (getifaddrs(&ifap) < 0) {
		LOG_F(init_cls, "getifaddrs: %s", strerror(errno));
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
		case AF_INET6:
			LOG_W(init_cls, "Interface %s has an IPv6 address, but GrapeVine only supports IPv4 for now (I know, shame, shame). Skipping.", interface_name);
			break;
		default:
			break;
		}
	}

	if (state.found_ipv4 == NULL || state.found_ether == NULL) {
		LOG_F(init_cls, iface_found ? "Interface %s does not have required addresses.\n" : "Interface %s not found.\n", interface_name);
		goto err_no_iface;
	}

	LOG_I(
		init_cls,
		"Found interface %s with IPv4 address %s.",
		interface_name,
		inet_ntoa(((struct sockaddr_in*) state.found_ipv4->ifa_addr)->sin_addr)
	);

	LOG_V(init_cls, "Creating socket for TCP connections (binding to port 0x%x).", GV_PORT);

	state.sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (state.sock < 0) {
		LOG_F(init_cls, "socket: %s", strerror(errno));
		goto err_socket;
	}

	setsockopt(state.sock, SOL_SOCKET, SO_REUSEADDR, &(int) {1}, sizeof(int));

	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_port = htons(GV_PORT),
		.sin_addr.s_addr = htonl(INADDR_ANY),
	};

	if (bind(state.sock, (struct sockaddr*) &addr, sizeof addr) < 0) {
		LOG_F(init_cls, "bind: %s", strerror(errno));
		goto err_bind;
	}

	LOG_V(init_cls, "Start the echolocation (ELP) subsystem.");

	if (elp(&state) < 0) {
		LOG_F(init_cls, "Starting echolocation (ELP) subsystem failed.");
		goto err_elp;
	}

	LOG_V(init_cls, "Starting to listen for connections.");

	if (listen(state.sock, 5) < 0) { // TODO What's a sane default for the backlog number? Should this be configurable?
		LOG_F(init_cls, "listen: %s", strerror(errno));
		goto err_listen;
	}

	// Wait for connections.

	LOG_I(init_cls, "GrapeVine daemon bound to port 0x%x and listening for connections.", GV_PORT);

	state.connection_count = 0;
	state.connections = NULL;

	for (;;) {
		conn_t conn;

		LOG_V(conn_listener, "Waiting for new connection.");

		conn.state = &state;
		conn.sock = accept(state.sock, (struct sockaddr*) &conn.addr, &conn.addr_len);

		if (conn.sock < 0) {
			LOG_F(conn_listener, "accept: %s", strerror(errno));
			goto err_accept;
		}

		LOG_I(
			conn_listener,
			"Accepted connection from %s:0x%x (host %" PRIx64 ").",
			inet_ntoa(conn.addr.sin_addr),
			ntohs(conn.addr.sin_port),
			sockaddr_to_mac((struct sockaddr*) &conn.addr)
		);

		LOG_V(conn_listener, "Finding/creating slot for connection.");

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

		LOG_V(conn_listener, "Starting connection thread.");

		pthread_create(&slot->thread, NULL, conn_thread, slot);
	}

	rv = EXIT_SUCCESS;
	LOG_I(init_cls, "GrapeVine daemon is shutting down gracefully.");

err_accept:

	LOG_V(init_cls, "Joining all connection threads.");

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

	LOG_V(init_cls, "Releasing lock file %s.", GV_LOCK_PATH);

	flock(fileno(lock_file), LOCK_UN); // XXX Shouldn't be necessary, but just in case.
	fclose(lock_file);

err_lock:
err_getopt:

	LOG_V(init_cls, "GrapeVine daemon finished with status %d.", rv);

	return rv;
}
