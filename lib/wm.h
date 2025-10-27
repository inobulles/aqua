// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#pragma once

#include "root.h"

#include <aqua/kos.h>

/**
 * WM library component context.
 */
typedef struct wm_ctx_t* wm_ctx_t;

/**
 * WM object.
 *
 * This is an individual WM created with {@link wm_create}.
 */
typedef struct wm_t* wm_t;

/**
 * Initialize the WM library component.
 *
 * @param ctx The AQUA library context.
 * @return The WM library component handle.
 */
aqua_component_t wm_init(aqua_ctx_t ctx);

/**
 * Connect to a WM VDEV.
 *
 * {@link wm_disconn} must be called to disconnect from the VDEV and to free the context.
 *
 * @param vdev The descriptor of the WM VDEV to connect to.
 * @return The WM library component context or `NULL` if allocation failed.
 */
wm_ctx_t wm_conn(kos_vdev_descr_t const* vdev);

/**
 * Disconnect from a WM VDEV.
 *
 * This function disconnects from the WM VDEV and frees the context.
 *
 * @param ctx The WM library component context.
 */
void wm_disconn(wm_ctx_t ctx);

/**
 * Create a WM.
 *
 * @param ctx The WM library component context.
 * @return A WM object.
 */
wm_t wm_create(wm_ctx_t ctx);

/**
 * Destroy a WM.
 *
 * @param wm The WM to destroy.
 */
void wm_destroy(wm_t wm);

/**
 * Enter the WM event loop.
 *
 * This will block until the WM is closed.
 *
 * @param wm The WM object to run the loop on.
 */
void wm_loop(wm_t wm);
