// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2024 Aymeric Wibo

#include "gv.h"

#include <aqua/gv_proto.h>

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void close_sock(int const* sock) {
	if (*sock >= 0) {
		close(*sock);
	}
}

int query(state_t* s, in_addr_t in_addr, size_t* vdev_count_ref, kos_vdev_descr_t** vdevs_ref) {
	LOG_V(s->query_cls, "Create a new socket for sending to this node.");

	int const __attribute__((cleanup(close_sock))) sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (sock < 0) {
		LOG_E(s->query_cls, "socket: %s", strerror(errno));
		return -1;
	}

	LOG_V(s->query_cls, "Connect to the node.");

	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_port = htons(GV_PORT),
		.sin_addr.s_addr = in_addr,
	};

	if (connect(sock, (struct sockaddr*) &addr, sizeof addr) < 0) {
		LOG_E(s->query_cls, "connect: %s", strerror(errno));
		return -1;
	}

	LOG_V(s->query_cls, "Send QUERY packet.");

	gv_packet_t packet = {
		.header.type = GV_PACKET_TYPE_QUERY,
	};

	if (sendto(sock, &packet, sizeof packet.header, 0, (struct sockaddr*) &addr, sizeof addr) < 0) {
		LOG_E(s->query_cls, "sendto: %s", strerror(errno));
		return -1;
	}

	LOG_V(s->query_cls, "Receive QUERY_RES header.");

	size_t const query_res_size = sizeof packet.header + sizeof packet.query_res;

	if (recv(sock, &packet, query_res_size, MSG_WAITALL) != (ssize_t) query_res_size) {
		LOG_E(s->query_cls, "recv failed.");
		return -1;
	}

	if (packet.header.type != GV_PACKET_TYPE_QUERY_RES) {
		LOG_E(s->query_cls, "Unexpected packet type for QUERY response: %d", packet.header.type);
		return -1;
	}

	LOG_V(s->query_cls, "Receive VDEV descriptors themselves.");

	size_t const vdev_count = packet.query_res.vdev_count;
	size_t const vdev_bytes = vdev_count * sizeof(kos_vdev_descr_t);

	kos_vdev_descr_t* const vdevs = malloc(vdev_bytes);
	assert(vdevs != NULL);

	if (recv(sock, vdevs, vdev_bytes, MSG_WAITALL) != (ssize_t) vdev_bytes) {
		LOG_E(s->query_cls, "recv failed.");
		free(vdevs);
		return -1;
	}

	LOG_V(s->query_cls, "Queried.");

	*vdev_count_ref = vdev_count;
	*vdevs_ref = vdevs;

	return 0;
}

int query_res(conn_t* conn) {
	state_t* const state = conn->state;
	gv_packet_t* __attribute__((cleanup(gv_packet_free))) packet = NULL;

	size_t const vdevs_size = state->vdev_count * sizeof *packet->query_res.vdevs;
	size_t const packet_size = sizeof packet->header + sizeof packet->query_res + vdevs_size;

	packet = malloc(packet_size);
	assert(packet != NULL);

	packet->header.type = GV_PACKET_TYPE_QUERY_RES;
	packet->query_res.vdev_count = state->vdev_count;
	memcpy(packet->query_res.vdevs, state->vdevs, vdevs_size);

	LOG_V(state->query_cls, "Sending QUERY_RES packet.");

	if (send(conn->sock, packet, packet_size, 0) != (ssize_t) packet_size) {
		LOG_E(state->query_cls, "sendto: %s", strerror(errno));
		return -1;
	}

	LOG_V(state->query_cls, "QUERY_RES packet sent.");
	return 0;
}
