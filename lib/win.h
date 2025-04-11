// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2025 Aymeric Wibo

#pragma once

#include "aqua.h"

#include <aqua/kos.h>

/**
 * Window library component context.
 */
typedef struct win_ctx_t* win_ctx_t;

/**
 * Window object.
 *
 * This is an individual window created with `win_create`.
 */
typedef struct win_t* win_t;

/**
 * Initialize the window driver.
 *
 * This requests the window VDEV from the KOS. It should be called once at the beginning of the program to signal to the KOS that we want to request and receive attach notifications from window VDEVs.
 *
 * @param ctx The AQUA library context.
 * @return The window library component handle.
 */
aqua_component_t win_init(aqua_ctx_t ctx);

/**
 * Connect to a window VDEV.
 *
 * Once we've chosen the attached window VDEV we'd like to connect to and we've called `win_probe` on it to ensure it is supported, this function may be called to create a library component context and connect to it.
 *
 * Since the caller probably wants to store the context before it gets a connection notification, this function doesn't flush the KOS. Instead, the caller must do this through `kos_flush(true)` after calling this function.
 *
 * `win_disconn` must be called to disconnect from the VDEV and to free the context.
 *
 * @param vdev The descriptor of the window VDEV to connect to.
 * @return The window library component context or `NULL` if allocation failed.
 */
win_ctx_t win_conn(kos_vdev_descr_t const* vdev);

/**
 * Disconnect from a window VDEV.
 *
 * This function disconnects from the window VDEV and frees the context.
 *
 * @param ctx The window library component context.
 */
void win_disconn(win_ctx_t ctx);

/**
 * Create a window.
 *
 * Create a window object. This also registers an interrupt for window events. The caller must call `win_interrupt` on each `KOS_NOTIF_INTERRUPT` notification it receives for these window event interrupts to be processed.
 *
 * @param ctx The window library component context.
 * @return A window object.
 */
win_t win_create(win_ctx_t ctx);

/**
 * Destroy a window.
 *
 * @param win The window to destroy.
 */
void win_destroy(win_t win);

/**
 * Enter the window event loop.
 *
 * This will block until the window is closed. Any registered callbacks will be called when the corresponding events they were registered for occur.
 *
 * @param win The window to start and enter the event loop of.
 */
void win_loop(win_t win);
