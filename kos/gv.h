// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2025 Aymeric Wibo

#pragma once

#include "kos.h"

#include <unistd.h>

ssize_t query_gv_vdevs(kos_vdev_descr_t** vdevs_out);
