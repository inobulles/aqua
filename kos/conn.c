// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2024 Aymeric Wibo

#include "conn.h"

#include <assert.h>
#include <stdlib.h>

conn_t* conns = NULL;
uint64_t conn_count = 0;

uint64_t conn_new(vdriver_t* vdriver) {
	conns = realloc(conns, (conn_count + 1) * sizeof *conns);
	assert(conns != NULL);

	conn_t* const conn = &conns[conn_count];

	conn->alive = true;
	conn->vdriver = vdriver;

	return conn_count++;
}
