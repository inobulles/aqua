// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2024 Aymeric Wibo

#pragma once

#include "../kos/kos.h"

#include <stdint.h>
#include <sys/types.h>

#define GV_NODES_PATH "/tmp/gv.nodes"
#define GV_LOCK_PATH "/tmp/gv.lock"

/**
 * Get all the VDEVs found by the GrapeVine daemon.
 *
 * This function reads the GrapeVine nodes file which should've been populated by the daemon for node entries and their reported VDEVs.
 *
 * @param vdevs_out The VDEVs found by the daemon.
 * @return The number of VDEVs found, 0 if the daemon is not running, and -1 if some unexpected error occurred.
 */
ssize_t gv_query_vdevs(kos_vdev_descr_t** vdevs_out);
