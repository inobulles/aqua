// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2024 Aymeric Wibo

#pragma once

#include "../kos/kos.h"

#include <stdint.h>
#include <sys/types.h>

#define GV_NODES_PATH "/tmp/gv.nodes"
#define GV_LOCK_PATH "/tmp/gv.lock"

/**
 * A VDEV connection object.
 */
typedef struct {
	int sock;
	pthread_t thread;
} gv_vdev_conn_t;

/**
 * Get all the VDEVs found by the GrapeVine daemon.
 *
 * This function reads the GrapeVine nodes file which should've been populated by the daemon for node entries and their reported VDEVs.
 *
 * @param vdevs_out The VDEVs found by the daemon.
 * @return The number of VDEVs found, 0 if the daemon is not running, and -1 if some unexpected error occurred.
 */
ssize_t gv_query_vdevs(kos_vdev_descr_t** vdevs_out);

/**
 * Request a connection to a VDEV.
 *
 * This function will first attempt to find a node with the given host ID. If one is found, it will send a VDEV connection request to that node for the given VDEV ID and initialize a new connection object.
 *
 * TODO Define what happens next.
 *
 * @param conn The connection object to initialize.
 * @param host_id The host ID of the node to connect to.
 * @param vdev_id The VDEV ID to connect to.
 * @return 0 on success, -1 if something went wrong.
 */
int gv_vdev_conn(gv_vdev_conn_t* conn, uint64_t host_id, uint64_t vdev_id);
