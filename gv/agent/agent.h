// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

/**
 * Interface for GrapeVine KOS agents.
 *
 * This is used by the CLI, of which a process is spawned by the GrapeVine daemon when a new connection is made to it.
 * It is a standalone library to allow other processes to become GrapeVine KOS agents themselves (e.g. if a VDRIVER must be loaded by a specific process in order to work).
 * These other processes must handle the connection themselves however, which usually involves receiving one already established by gvd through a UDS.
 */

#pragma once

#include <stdint.h>

/**
 * GrapeVine KOS agent handle.
 */
typedef struct gv_agent_t gv_agent_t;

/**
 * Create a GrapeVine KOS agent.
 *
 * A connection should already have been established with the KOS issuing the commands.
 *
 * @param sock Socket connection has been established on.
 * @param spec The spec of the VDRIVER to look for the requested VDEV ID in.
 * @param vdev_id The VDEV ID of the VDEV we should send commands to.
 * @return The GrapeVine KOS agent handle.
 */
gv_agent_t* gv_agent_create(int sock, char const* spec, uint64_t vdev_id);

/**
 * Loop the agent and listen for packets over the connection.
 *
 * @param agent The agent to loop for.
 */
void gv_agent_loop(gv_agent_t* agent);

/**
 * Destroy a GrapeVine KOS agent.
 *
 * This also disconnects from the VDEV and the KOS.
 *
 * @param agent The agent to destroy.
 */
void gv_agent_destroy(gv_agent_t* agent);
