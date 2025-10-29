// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2024-2025 Aymeric Wibo

#include "conn.h"
#include "query.h"

#include <aqua/gv_proto.h>
#include <aqua/vdriver_loader.h>

#include <umber.h>

#include <errno.h>
#include <inttypes.h>
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
	state_t* const state = conn->state;
	(void) state;

	// TODO Here we're going to need to actually connect to the VDEV. On response, we send out a CONN_VDEV_RES packet.
	// We're probably going to need a libvdev library, this is in common with conn_local in the KOS.

	// Actually, we need to find the vdriver with this first, as we need to call the connection on it.

	kos_vdev_descr_t* found = NULL;

	for (size_t i = 0; i < conn->state->vdev_count; i++) {
		kos_vdev_descr_t* const vdev = &conn->state->vdevs[i];

		if (vdev->vdev_id == vdev_id) {
			if (found != NULL) {
				LOG_E(cls, "Found multiple VDEVs with ID %" PRIx64 "", vdev_id);
			}

			found = vdev;
		}
	}

	if (found == NULL) {
		LOG_E(cls, "Could not find a VDEV with ID %" PRIx64 "", vdev_id);

		// TODO Send back failure in CONN_VDEV_RES packet.

		return;
	}
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
