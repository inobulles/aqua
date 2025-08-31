// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

/**
 * This header is used both by the GrapeVine daemon and the KOS.
 *
 * It defines the paths and structs used for IPC between the GrapeVine daemon and the KOS.
 */

#pragma once

#include "kos.h"

#include <netinet/in.h>

/**
 * Where the host ID of the current machine is stored.
 */
#define GV_HOST_ID_PATH "/tmp/gv.host_id"

/**
 * Where the discovered GrapeVine nodes and their VDEVs are stored.
 */
#define GV_NODES_PATH "/tmp/gv.nodes"

/**
 * The path to the lock file used to determine if the GrapeVine daemon is running.
 *
 * If this file exists and is locked, then the GrapeVine daemon is running.
 */
#define GV_LOCK_PATH "/tmp/gv.lock"

_Static_assert(sizeof(in_addr_t) == sizeof(uint32_t), "in_addr_t is not 32 bits long.");
_Static_assert(sizeof(struct in6_addr) == sizeof(uint8_t) * 16, "in6_addr_t is not 128 bits long.");

/**
 * Node entry in the GrapeVine nodes file.
 *
 * One entry represents a node's host information and its VDEVs.
 */
typedef struct __attribute__((packed)) {
	/**
	 * The host ID of the node.
	 *
	 * This is meant to be a unique identifier for the node.
	 * In practice this is usually the MAC address.
	 */
	uint64_t host_id;

	/**
	 * The IP address of the node.
	 */
	union {
		/**
		 * IPv4 address.
		 */
		in_addr_t v4;
		/**
		 * IPv6 address.
		 */
		struct in6_addr v6;
	} ip;

	/**
	 * The number of VDEVs exposed by this node.
	 */
	uint16_t vdev_count;
	/**
	 * The VDEV descriptor structs for each VDEV exposed by this node.
	 */
	kos_vdev_descr_t vdevs[];
} gv_node_ent_t;
