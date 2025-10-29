// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#pragma once

#include "root.h"

/**
 * Test library component context.
 */
typedef struct test_ctx_t* test_ctx_t;

/**
 * Initialize the test library component.
 *
 * @param ctx The AQUA library context.
 * @return The test library component handle.
 */
aqua_component_t test_init(aqua_ctx_t ctx);

/**
 * Connect to a test VDEV.
 *
 * @param vdev The descriptor of the test VDEV to connect to.
 * @return The test library component context or `NULL` if allocation failed.
 */
test_ctx_t test_conn(kos_vdev_descr_t const* vdev);

/**
 * Disconnect from a test VDEV.
 *
 * @param ctx The test library component context.
 */
void test_disconn(test_ctx_t ctx);

/**
 * Add 69 to a number.
 *
 * @param ctx The test library component context.
 * @param x Number.
 * @return Come on man.
 */
uint64_t test_add(test_ctx_t ctx, uint64_t x);
