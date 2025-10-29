// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2024-2025 Aymeric Wibo

#include "conn.h"
#include "query.h"

#include <aqua/gv_proto.h>
#include <aqua/vdriver_loader.h>

#include <umber.h>

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>

static umber_class_t const* cls = NULL;

static __attribute__((constructor)) void init(void) {
	cls = umber_class_new("aqua.gv.conn", UMBER_LVL_INFO, "GrapeVine daemon connection handling.");
}

static void conn_vdev(conn_t* conn, uint64_t vdev_id) {
	// TODO If this fails, we are responsible for sending a CONN_FAIL (or whatever).

	LOG_V(cls, "Looking for VDRIVER associated to VID %" PRIu64 ".", vdev_id);

	vdriver_t* const vdriver = vdriver_loader_find_loaded_by_vid(vdev_id);

	if (vdriver == NULL) {
		LOG_E(cls, "Could not find associated VDRIVER.");
		goto done;
	}

	// Spawn KOS agent process.
	// TODO Note that if you're stuck on an issue here, it might be that gv-kos-agent failed to start; it will fail silently if so!

	LOG_V(cls, "Spawning KOS agent process (spec=%s).", vdriver->spec);

	char vid_str[16];
	snprintf(vid_str, sizeof vid_str, "%" PRIu64, vdev_id);

	char* const path = "gv-kos-agent";
	char* const argv[] = {path, "-s", vdriver->spec, "-v", vid_str, NULL};

	posix_spawn_file_actions_t actions;
	posix_spawn_file_actions_init(&actions);

	posix_spawn_file_actions_adddup2(&actions, conn->sock, 3); // Make socket available as fd 3 in child.

	extern char** environ;
	pid_t pid;

	if (posix_spawnp(&pid, path, &actions, NULL, argv, environ) < 0) {
		LOG_E(cls, "posix_spawnp(\"%s\"): %s", path, strerror(errno));
	}

	posix_spawn_file_actions_destroy(&actions);

done:

	close(conn->sock);

	// TODO We should keep track of all our KOS agents so that we can ask them to terminate all connections when we go down.
}

void* conn_thread(void* arg) {
	conn_t* const conn = arg;

	packet_t buf;
	int len;

	while ((len = recv(conn->sock, &buf, sizeof buf, 0)) > 0) {
		switch (buf.header.type) {
		case ELP:
			LOG_E(
				cls,
				"Received ELP from %s:0x%x (host %" PRIx64 ") on TCP. This should not happen!",
				inet_ntoa(conn->addr.sin_addr),
				ntohs(conn->addr.sin_port),
				buf.elp.host_id
			);
			break;
		case QUERY:
			if (query_res(conn) < 0) {
				goto stop;
			}

			break;
		case QUERY_RES:
			LOG_E(
				cls,
				"Received QUERY_RES from %s:0x%x (host %" PRIx64 "). This should not happen, as this connection isn't ever used to send out QUERY packets!",
				inet_ntoa(conn->addr.sin_addr),
				ntohs(conn->addr.sin_port),
				buf.elp.host_id
			);
			break;
		case CONN_VDEV:
			if (recv(conn->sock, &buf.conn_vdev, sizeof buf.conn_vdev, 0) != sizeof buf.conn_vdev) {
				LOG_E(cls, "recv: %s", strerror(errno));
				goto stop;
			}

			conn_vdev(conn, buf.conn_vdev.vdev_id);
			break;
		case CONN_VDEV_RES:
			LOG_E(
				cls,
				"Received CONN_VDEV_RES from %s:0x%x (host %" PRIx64 "). This should not happen, as this connection isn't ever used to send out CONN_VDEV packets!",
				inet_ntoa(conn->addr.sin_addr),
				ntohs(conn->addr.sin_port),
				buf.elp.host_id
			);
			break;
		}
	}

stop:

	close(conn->sock);
	conn->slot_used = false;

	return NULL;
}
