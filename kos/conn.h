// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#pragma once

#include "lib/vdriver.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

typedef enum {
	CONN_TYPE_LOCAL,
	CONN_TYPE_GV,
} conn_type_t;

/**
 * A connection to a VDEV.
 *
 * TODO Talk more about the properties of a connection here.
 */
typedef struct {
	/**
	 * Whether the connection is still active.
	 *
	 * A connection is marked as unalive after disconnection.
	 */
	bool alive;

	/**
	 * Whether the connection is to a local VDRIVER or on the GrapeVine.
	 */
	conn_type_t type;

	union {
		/**
		 * For local VDEVs, the VDRIVER pointer.
		 */
		vdriver_t* vdriver;
		
		struct {
			/**
			 * For GrapeVine VDEVs, the socket.
			 */
			int sock;
			
			/**
			 * For GrapeVine VDEVs, the connection ID on the remote KOS agent the GrapeVine daemon spawned for us.
			 */
			uint64_t remote_cid;
		};
	};

	/**
	 * The number of functions the VDEV this connection is for supports.
	 */
	size_t fn_count;

	/**
	 * The functions the VDEV this connection is for supports.
	 */
	kos_fn_t const* fns;
} conn_t;

static conn_t* conns = NULL;
static uint64_t conn_count = 0;

/**
 * Create new generic connection, i.e. neither local or GV.
 *
 * @returns Connection ID of new connection.
 */
static uint64_t conn_new(void) {
	conns = realloc(conns, (conn_count + 1) * sizeof *conns);
	assert(conns != NULL);

	conn_t* const conn = &conns[conn_count];
	conn->alive = false;

	return conn_count++;
}

/**
 * Create new local connection.
 *
 * @param vdriver VDRIVER connection is to.
 * @returns Connection ID of new connection.
 */
static uint64_t conn_new_local(vdriver_t* vdriver) {
	uint64_t const cid = conn_new();

	conns[cid].type = CONN_TYPE_LOCAL;
	conns[cid].vdriver = vdriver;

	return cid;
}

/**
 * Create new GrapeVine connection.
 *
 * @param sock Socket the connection is happening over.
 * @param sock Remote connection ID.
 * @returns Connection ID of new connection.
 */
static uint64_t conn_new_gv(int sock, uint64_t remote_cid) {
	uint64_t const cid = conn_new();

	conns[cid].type = CONN_TYPE_GV;
	conns[cid].remote_cid = remote_cid;
	conns[cid].sock = sock;

	return cid;
}
