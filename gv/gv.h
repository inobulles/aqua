// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2024-2025 Aymeric Wibo

#pragma once

#include "conn.h"

#include <aqua/gv_ipc.h>
#include <aqua/kos.h>

#include <umber.h>

#include <pthread.h>
#include <stdbool.h>

#include <ifaddrs.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define ELP_DELAY 1 // In seconds.
#define NODE_TTL 5  // In seconds.

_Static_assert(sizeof(in_addr_t) == sizeof(uint32_t), "in_addr_t is not 32 bits long.");

typedef struct {
	bool slot_used;
	uint64_t unique;
	uint64_t host;
	struct sockaddr_in addr;
	int32_t ttl; // In seconds.

	size_t ent_bytes;
	gv_node_ent_t* ent;
} node_t;

typedef struct state_t state_t;

struct state_t {
	struct ifaddrs* found_ether;
	struct ifaddrs* found_ipv4;

	uint64_t host_id;

	int sock;
	int elp_sock;

	bool elp_threads_started;
	pthread_t elp_sender_thread;
	pthread_t elp_listener_thread;

	kos_vdev_descr_t* vdevs;
	size_t vdev_count;

	pthread_mutex_t nodes_mutex;
	size_t node_count;
	node_t* nodes;

	size_t connection_count;
	conn_t* connections;

	// Logging classes.

	umber_class_t const* init_cls;
	umber_class_t const* listener_cls;
	umber_class_t const* elp_cls;
	umber_class_t const* query_cls;
};

static inline in_addr_t sockaddr_to_in_addr(struct sockaddr* addr) {
	return *(in_addr_t*) ((uint8_t*) addr->sa_data + 2);
}

static inline uint64_t sockaddr_to_mac(struct sockaddr* addr) {
	return *(uint64_t*) ((uint8_t*) addr->sa_data + 9) & 0xFFFFFFFFFFFF;
}
