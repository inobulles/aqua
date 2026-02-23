// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025-2026 Aymeric Wibo

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
 * The raw image we have to draw to is acquired by the WM VDEV from its internal swapchain.
 * You can turn this into e.g. a WebGPU texture with the {@link wgpuRenderTextureFromVkImage} extension.
 *
 * @param wm The WM object.
 * @param raw_vk_image Raw Vulkan image which we have to draw to.
 * @param raw_vk_cmd_pool Raw Vulkan command pool the {@link raw_vk_cmd_buf} command buffer was created on.
 * @param raw_vk_cmd_buf Raw Vulkan command buffer we must use for rendering.
 * @param data User-defined data passed to the callback. This is set when registering the callback with {@link wm_register_redraw_cb}.
 */
typedef void (*wm_redraw_cb_t)(wm_t wm, void* raw_vk_image, void* raw_vk_cmd_pool, void* raw_vk_cmd_buf, void* data);

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
 * WM destroy window event callback.
 *
 * @param wm The WM object.
 * @param win The WM window handle.
 * @param data User-defined data passed to the callback. This is set when registering the callback with {@link wm_register_destroy_win_cb}.
 */
typedef void (*wm_destroy_win_cb_t)(wm_t wm, wm_win_t win, void* data);

/**
 * WM window redraw event callback.
 *
 * @param wm The WM object.
 * @param win The WM window handle.
 * @param x_res The new X resolution of the window (can change between calls).
 * @param y_res The new Y resolution of the window (can change between calls).
 * @param raw_image Raw image containing the window contents.
 * @param data User-defined data passed to the callback. This is set when registering the callback with {@link wm_register_new_win_cb}.
 */
typedef void (*wm_redraw_win_cb_t)(wm_t wm, wm_win_t win, uint32_t x_res, uint32_t y_res, void* raw_image, void* data);

/**
 * WM mouse motion event callback.
 *
 * This is called whenever the pointer moves.
 *
 * If {@link is_abs} is true, {@link x} and {@link y} contain absolute
 * coordinates. Otherwise, {@link dx} and {@link dy} contain relative motion.
 *
 * @param wm The WM object.
 * @param is_abs Non-zero if the motion is absolute, zero if relative.
 * @param dx Relative X motion (valid if is_abs is zero).
 * @param dy Relative Y motion (valid if is_abs is zero).
 * @param x Absolute X coordinate (valid if is_abs is non-zero).
 * @param y Absolute Y coordinate (valid if is_abs is non-zero).
 * @param unaccel_dx Unaccelerated relative X motion (valid if is_abs is non-zero).
 * @param unaccel_dy Unaccelerated relative Y motion (valid if is_abs is non-zero).
 * @param data User-defined data passed to the callback. This is set when registering the callback with {@link wm_register_mouse_motion_cb}.
 */
typedef void (*wm_mouse_motion_cb_t)(
	wm_t wm,
	bool is_abs,
	double dx,
	double dy,
	double x,
	double y,
	double unaccel_dx,
	double unaccel_dy,
	void* data
);

/**
 * WM mouse button event callback.
 *
 * This is called whenever a mouse button is pressed or released.
 *
 * @param wm The WM object.
 * @param press True if the button was pressed, false if it was released.
 * @param button The button identifier.
 * @param data User-defined data passed to the callback. This is set when registering the callback with {@link wm_register_mouse_button_cb}.
 */
typedef void (*wm_mouse_button_cb_t)(wm_t wm, bool press, uint32_t button, void* data);

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
 * Register a destroy window callback.
 *
 * These will be called when a window that's being destroyed has been unmapped on the WM.
 *
 * @param wm The WM to register the callback for.
 * @param cb The callback to register.
 * @param data User-defined data passed to the callback.
 */
void wm_register_destroy_win_cb(wm_t wm, wm_destroy_win_cb_t cb, void* data);

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
 * Register a mouse motion callback.
 *
 * These will be called whenever the pointer moves.
 *
 * @param wm The WM to register the callback for.
 * @param cb The callback to register.
 * @param data User-defined data passed to the callback.
 */
void wm_register_mouse_motion_cb(wm_t wm, wm_mouse_motion_cb_t cb, void* data);

/**
 * Register a mouse button callback.
 *
 * These will be called whenever a mouse button is pressed or released.
 *
 * @param wm The WM to register the callback for.
 * @param cb The callback to register.
 * @param data User-defined data passed to the callback.
 */
void wm_register_mouse_button_cb(wm_t wm, wm_mouse_button_cb_t cb, void* data);

/**
 * Enter the WM event loop.
 *
 * This will block until the WM is closed.
 *
 * @param wm The WM object to run the loop on.
 */
void wm_loop(wm_t wm);

/**
 * Copy window contents to buffer.
 *
 * The output buffer must have the last resolution reported by the window redraw callback.
 * WARNING This API is very much subject to change and will probably disappear in favour of something better!
 *
 * @param wm The WM object which owns the window.
 * @param win Window handle to copy the contents of.
 * @param buf Buffer to copy the window contents to.
 */
void wm_get_win_fb(wm_t wm, wm_win_t win, void* buf);
