// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2024 Aymeric Wibo

#pragma once

#include "vdev.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
	bool alive;
	vdriver_t* vdriver;

	size_t fn_count;
	kos_vdev_fn_t const* fns;
} conn_t;

static conn_t* conns = NULL;
static uint64_t conn_count = 0;

static uint64_t conn_new(vdriver_t* vdriver) {
	conns = realloc(conns, (conn_count + 1) * sizeof *conns);
	assert(conns != NULL);

	conn_t* const conn = &conns[conn_count];

	conn->alive = false;
	conn->vdriver = vdriver;

	return conn_count++;
}
