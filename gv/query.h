// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2024 Aymeric Wibo

#pragma once

#include "gv.h"

int query(in_addr_t in_addr, size_t* vdev_count_ref, kos_vdev_descr_t** vdevs_ref);
int query_res(conn_t* conn);
