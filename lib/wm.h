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
 * WM window handle.
 */
typedef uint64_t wm_win_t;

/**
 * WM redraw event callback.
 *
 * @param wm The WM object.
 * @param data User-defined data passed to the callback. This is set when registering the callback with {@link wm_register_redraw_cb}.
 */
typedef void (*wm_redraw_cb_t)(wm_t wm, void* data);

/**
 * WM new window event callback.
 *
 * @param wm The WM object.
 * @param win The WM window handle.
 * @param app_id The WM window app ID.
 * @param data User-defined data passed to the callback. This is set when registering the callback with {@link wm_register_new_win_cb}.
 */
typedef void (*wm_new_win_cb_t)(wm_t wm, wm_win_t win, char const* app_id, void* data);

/**
 * WM window redraw event callback.
 *
 * @param wm The WM object.
 * @param win The WM window handle.
 * @param x_res The new X resolution of the window (can change between calls).
 * @param y_res The new Y resolution of the window (can change between calls).
 * @param data User-defined data passed to the callback. This is set when registering the callback with {@link wm_register_new_win_cb}.
 */
typedef void (*wm_redraw_win_cb_t)(wm_t wm, wm_win_t win, uint32_t x_res, uint32_t y_res, void* data);

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
 * Register a redraw callback.
 *
 * These will be called when the WM needs to be redrawn, only once the WM's loop has been entered (see {@link wm_loop}).
 *
 * @param wm The WM to register the callback for.
 * @param cb The callback to register.
 * @param data User-defined data passed to the callback.
 */
void wm_register_redraw_cb(wm_t wm, wm_redraw_cb_t cb, void* data);

/**
 * Register a new window callback.
 *
 * These will be called when a new window has been mapped on the WM.
 *
 * @param wm The WM to register the callback for.
 * @param cb The callback to register.
 * @param data User-defined data passed to the callback.
 */
void wm_register_new_win_cb(wm_t wm, wm_new_win_cb_t cb, void* data);

/**
 * Register a window redraw callback.
 *
 * These will be called when a window needs to be redrawn..
 *
 * @param wm The WM to register the callback for.
 * @param cb The callback to register.
 * @param data User-defined data passed to the callback.
 */
void wm_register_redraw_win_cb(wm_t wm, wm_redraw_win_cb_t cb, void* data);

/**
 * Enter the WM event loop.
 *
 * This will block until the WM is closed.
 *
 * @param wm The WM object to run the loop on.
 */
void wm_loop(wm_t wm);
