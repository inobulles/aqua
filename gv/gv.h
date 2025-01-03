// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2024 Aymeric Wibo

#pragma once

#include "../kos/kos.h"

#include <stdint.h>
#include <sys/types.h>

#define GV_NODES_PATH "/tmp/gv.nodes"
#define GV_LOCK_PATH "/tmp/gv.lock"

ssize_t gv_query_vdevs(kos_vdev_descr_t** vdevs_out);
