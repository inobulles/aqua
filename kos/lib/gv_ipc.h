// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

/**
 * This header is used both by the GrapeVine daemon and the KOS.
 *
 * It defines the paths and structs used for IPC between the GrapeVine daemon and the KOS.
 */

#pragma once

#include <aqua/kos.h>

#include <netinet/in.h>
#include <stdlib.h>

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

/**
 * Get the host ID of the current machine.
 *
 * This value can be set with the GV_HOST_ID_PATH environment variable.
 *
 * @return Host ID. This is either allocated in the environment or is a constant, so don't free this.
 */
static inline char const* gv_get_host_id_path(void) {
	char const* const env = getenv("GV_HOST_ID_PATH");

	if (env == NULL) {
		return "/tmp/gv.host_id";
	}

	return env;
}

/**
 * Get the path to the discovered GrapeVine nodes file.
 *
 * This is the file where the discovered nodes and their VDEVs are stored.
 * This value can be set with the GV_NODES_PATH environment variable.
 *
 * @return Nodes file path. This is either allocated in the environment or is a constant, so don't free this.
 */
static inline char const* gv_get_nodes_path(void) {
	char const* const env = getenv("GV_NODES_PATH");

	if (env == NULL) {
		return "/tmp/gv.nodes";
	}

	return env;
}

/**
 * Get the path to the GrapeVine lock file.
 *
 * The lock file is used to determine if the GrapeVine daemon is running.
 * If this file exists and is locked, then the GrapeVine daemon is running.
 * This value can be set with the GV_LOCK_PATH environment variable.
 *
 * @return Lock file path. This is either allocated in the environment or is a constant, so don't free this.
 */
static inline char const* gv_get_lock_path(void) {
	char const* const env = getenv("GV_LOCK_PATH");

	if (env == NULL) {
		return "/tmp/gv.lock";
	}

	return env;
}
