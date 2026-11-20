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
 * @param x_res The image's X resolution.
 * @param y_res The image's Y resolution.
 * @param data User-defined data passed to the callback. This is set when registering the callback with {@link wm_register_redraw_cb}.
 */
typedef void (*wm_redraw_cb_t)(
	wm_t wm,
	void* raw_vk_image,
	uint32_t x_res,
	uint32_t y_res,
	void* data
);

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
 * WM window surface.
 *
 * A window is made up of one or more of these, which must be drawn back to front.
 */
typedef struct {
	/**
	 * X position of the surface relative to the window's geometry (can be negative, e.g. for CSD shadows).
	 */
	int32_t x;

	/**
	 * Y position of the surface relative to the window's geometry (can be negative, e.g. for CSD shadows).
	 */
	int32_t y;

	/**
	 * X resolution of the surface's image.
	 */
	uint32_t x_res;

	/**
	 * Y resolution of the surface's image.
	 */
	uint32_t y_res;

	/**
	 * Raw image containing the surface contents.
	 */
	void* raw_image;

	/**
	 * WebGPU texture format (WGPUTextureFormat) the image should be imported as.
	 */
	uint32_t format;

	/**
	 * Whether the image's alpha channel should be ignored, i.e. the surface is fully opaque.
	 *
	 * This is the case for formats with padding instead of an alpha channel (e.g. XRGB8888), which share their WebGPU format with their alpha counterparts.
	 */
	bool opaque;
} wm_surf_t;

/**
 * WM window redraw event callback.
 *
 * @param wm The WM object.
 * @param win The WM window handle.
 * @param x_res The new X resolution of the window (can change between calls).
 * @param y_res The new Y resolution of the window (can change between calls).
 * @param surf_count Number of surfaces in {@link surfs}.
 * @param surfs Surfaces making up the window, ordered back to front. Only valid for the duration of the callback.
 * @param data User-defined data passed to the callback. This is set when registering the callback with {@link wm_register_redraw_win_cb}.
 */
typedef void (*wm_redraw_win_cb_t)(
	wm_t wm,
	wm_win_t win,
	uint32_t x_res,
	uint32_t y_res,
	size_t surf_count,
	wm_surf_t const* surfs,
	void* data
);

/**
 * WM image destroy event callback.
 *
 * @param wm The WM object.
 * @param raw_image Raw image whcih is about to be destroyed.
 * @param data User-defined data passed to the callback. This is set when registering the callback with {@link wm_register_destroy_image_cb}.
 */
typedef void (*wm_destroy_image_cb_t)(wm_t wm, void* raw_image, void* data);

/**
 * Modifier key bitmask.
 *
 * Passed to input event callbacks to indicate which modifiers were held when the event happened.
 */
typedef enum {
	// clang-format off
// MODS:BEGIN
	WM_MOD_SHIFT = 1 << 0,
	WM_MOD_CAPS = 1 << 1,
	WM_MOD_CTRL = 1 << 2,
	WM_MOD_ALT = 1 << 3,
	WM_MOD_MOD2 = 1 << 4,
	WM_MOD_MOD3 = 1 << 5,
	WM_MOD_META = 1 << 6,
	WM_MOD_MOD5 = 1 << 7,
// MODS:END
	// clang-format on
} wm_mod_t;

/**
 * WM backend.
 *
 * This is the kind of backend the WM VDEV is running on.
 * {@link WM_BACKEND_WAYLAND} and {@link WM_BACKEND_X11} mean the WM is nested within another compositor or X server.
 */
typedef enum {
	// clang-format off
// BACKENDS:BEGIN
	WM_BACKEND_DRM,
	WM_BACKEND_WAYLAND,
	WM_BACKEND_X11,
	WM_BACKEND_HEADLESS,
// BACKENDS:END
	// clang-format on
	WM_BACKEND_UNKNOWN,
} wm_backend_t;

/**
 * WM mouse motion event callback.
 *
 * This is called whenever the pointer moves.
 *
 * If {@link is_abs} is true, {@link x} and {@link y} contain absolute
 * coordinates. Otherwise, {@link dx} and {@link dy} contain relative motion.
 *
 * @param wm The WM object.
 * @param time Time at which event happened. Should only really be used when passing event on with {@link wm_toplevel_notify_mouse_motion}.
 * @param is_abs Non-zero if the motion is absolute, zero if relative.
 * @param dx Relative X motion (valid if is_abs is zero).
 * @param dy Relative Y motion (valid if is_abs is zero).
 * @param x Absolute X coordinate (valid if is_abs is non-zero).
 * @param y Absolute Y coordinate (valid if is_abs is non-zero).
 * @param unaccel_dx Unaccelerated relative X motion (valid if is_abs is non-zero).
 * @param unaccel_dy Unaccelerated relative Y motion (valid if is_abs is non-zero).
 * @param mods Modifiers held when the event happened (see {@link wm_mod_t}).
 * @param data User-defined data passed to the callback. This is set when registering the callback with {@link wm_register_mouse_motion_cb}.
 */
typedef void (*wm_mouse_motion_cb_t)(
	wm_t wm,
	uint32_t time,
	bool is_abs,
	double dx,
	double dy,
	double x,
	double y,
	double unaccel_dx,
	double unaccel_dy,
	uint32_t mods,
	void* data
);

/**
 * WM mouse button event callback.
 *
 * This is called whenever a mouse button is pressed or released.
 *
 * @param wm The WM object.
 * @param time Time at which event happened. Should only really be used when passing event on with {@link wm_toplevel_notify_mouse_button}.
 * @param pressed True if the button was pressed, false if it was released.
 * @param button The button identifier.
 * @param mods Modifiers held when the event happened (see {@link wm_mod_t}).
 * @param data User-defined data passed to the callback. This is set when registering the callback with {@link wm_register_mouse_button_cb}.
 */
typedef void (*wm_mouse_button_cb_t)(wm_t wm, uint32_t time, bool pressed, uint32_t button, uint32_t mods, void* data);

/**
 * WM keyboard key event callback.
 *
 * This is called whenever a key is pressed or released.
 *
 * Key events are not passed on to any window automatically; it is up to the WM to do so with {@link wm_win_notify_key}.
 *
 * @param wm The WM object.
 * @param time Time at which event happened. Should only really be used when passing event on with {@link wm_win_notify_key}.
 * @param pressed True if the key was pressed, false if it was released.
 * @param keycode Raw keycode of the key. This is what should be passed on with {@link wm_win_notify_key}.
 * @param keysym XKB keysym of the key, resolved with the current keyboard layout and modifiers (see `XKB_KEY_*` in `<aqua/xkbcommon-keysyms.h>`). This is `XKB_KEY_NoSymbol` (0) if the key doesn't resolve to a single keysym.
 * @param mods Modifiers held when the event happened (see {@link wm_mod_t}).
 * @param data User-defined data passed to the callback. This is set when registering the callback with {@link wm_register_key_cb}.
 */
typedef void (*wm_key_cb_t)(wm_t wm, uint32_t time, bool pressed, uint32_t keycode, uint32_t keysym, uint32_t mods, void* data);

/**
 * Source of a mouse axis (scroll) event.
 *
 * Clients behave differently depending on the source, e.g. only finger (touchpad) scrolling gets kinetic scrolling and swipe-to-navigate.
 */
typedef enum {
	WM_AXIS_SOURCE_WHEEL = 0,
	WM_AXIS_SOURCE_FINGER = 1,
	WM_AXIS_SOURCE_CONTINUOUS = 2,
	WM_AXIS_SOURCE_WHEEL_TILT = 3,
} wm_axis_source_t;

/**
 * Orientation of a mouse axis (scroll) event.
 */
typedef enum {
	WM_AXIS_ORIENTATION_VERTICAL = 0,
	WM_AXIS_ORIENTATION_HORIZONTAL = 1,
} wm_axis_orientation_t;

/**
 * Relative direction of a mouse axis (scroll) event to the physical motion.
 *
 * This is {@link WM_AXIS_DIRECTION_INVERTED} when natural scrolling is enabled.
 */
typedef enum {
	WM_AXIS_DIRECTION_IDENTICAL = 0,
	WM_AXIS_DIRECTION_INVERTED = 1,
} wm_axis_direction_t;

/**
 * WM mouse axis (scroll) event callback.
 *
 * This is called whenever a scroll wheel is turned or the user scrolls on a touchpad.
 * When the user lifts their fingers off the touchpad, an event with a {@link delta} of zero is sent with source {@link WM_AXIS_SOURCE_FINGER}.
 * It's important to pass this on too, as clients use it to start kinetic scrolling.
 *
 * @param wm The WM object.
 * @param time Time at which event happened. Should only really be used when passing event on with {@link wm_win_notify_mouse_axis}.
 * @param source Source of the scroll event (see {@link wm_axis_source_t}).
 * @param orientation Whether this is a vertical or horizontal scroll (see {@link wm_axis_orientation_t}).
 * @param direction Relative direction of the scroll compared to the physical motion (see {@link wm_axis_direction_t}).
 * @param delta Scroll distance, in the same coordinate space as mouse motion.
 * @param delta_discrete Scroll distance in fractions of 120 wheel clicks (zero for non-wheel sources).
 * @param mods Modifiers held when the event happened (see {@link wm_mod_t}).
 * @param data User-defined data passed to the callback. This is set when registering the callback with {@link wm_register_mouse_axis_cb}.
 */
typedef void (*wm_mouse_axis_cb_t)(
	wm_t wm,
	uint32_t time,
	wm_axis_source_t source,
	wm_axis_orientation_t orientation,
	wm_axis_direction_t direction,
	double delta,
	int32_t delta_discrete,
	uint32_t mods,
	void* data
);

/**
 * Kind of touchpad gesture.
 */
typedef enum {
	WM_GESTURE_SWIPE = 0,
	WM_GESTURE_PINCH = 1,
	WM_GESTURE_HOLD = 2,
} wm_gesture_kind_t;

/**
 * Phase of a touchpad gesture.
 *
 * Every gesture starts with a begin and ends with an end phase.
 * Swipe and pinch gestures have any number of update phases in between; hold gestures have none.
 */
typedef enum {
	WM_GESTURE_PHASE_BEGIN = 0,
	WM_GESTURE_PHASE_UPDATE = 1,
	WM_GESTURE_PHASE_END = 2,
} wm_gesture_phase_t;

/**
 * WM touchpad gesture event callback.
 *
 * This is called on each phase of a touchpad gesture.
 *
 * @param wm The WM object.
 * @param time Time at which event happened. Should only really be used when passing event on with {@link wm_win_notify_gesture}.
 * @param kind Kind of gesture (see {@link wm_gesture_kind_t}).
 * @param phase Phase of gesture (see {@link wm_gesture_phase_t}).
 * @param fingers Number of fingers (valid for begin phase).
 * @param dx Relative X motion of the gesture's center since the last event (valid for update phase).
 * @param dy Relative Y motion of the gesture's center since the last event (valid for update phase).
 * @param scale Absolute scale compared to the begin phase (valid for update phase of pinch gestures).
 * @param rotation Relative clockwise rotation in degrees since the last event (valid for update phase of pinch gestures).
 * @param cancelled Whether the gesture was cancelled (valid for end phase).
 * @param mods Modifiers held when the event happened (see {@link wm_mod_t}).
 * @param data User-defined data passed to the callback. This is set when registering the callback with {@link wm_register_gesture_cb}.
 */
typedef void (*wm_gesture_cb_t)(
	wm_t wm,
	uint32_t time,
	wm_gesture_kind_t kind,
	wm_gesture_phase_t phase,
	uint32_t fingers,
	double dx,
	double dy,
	double scale,
	double rotation,
	bool cancelled,
	uint32_t mods,
	void* data
);

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
 * These will be called when a window needs to be redrawn.
 *
 * @param wm The WM to register the callback for.
 * @param cb The callback to register.
 * @param data User-defined data passed to the callback.
 */
void wm_register_redraw_win_cb(wm_t wm, wm_redraw_win_cb_t cb, void* data);

/**
 * Register an image destruction callback.
 *
 * These will be called when an image is about to be destroyed, alerting us we shouldn't be referencing this image anymore.
 *
 * @param wm The WM to register the callback for.
 * @param cb The callback to register.
 * @param data User-defined data passed to the callback.
 */
void wm_register_destroy_image_cb(wm_t wm, wm_destroy_image_cb_t cb, void* data);

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
 * Register a mouse axis (scroll) callback.
 *
 * These will be called whenever a scroll wheel is turned or the user scrolls on a touchpad.
 *
 * @param wm The WM to register the callback for.
 * @param cb The callback to register.
 * @param data User-defined data passed to the callback.
 */
void wm_register_mouse_axis_cb(wm_t wm, wm_mouse_axis_cb_t cb, void* data);

/**
 * Register a touchpad gesture callback.
 *
 * These will be called on each phase of a touchpad swipe, pinch, or hold gesture.
 *
 * @param wm The WM to register the callback for.
 * @param cb The callback to register.
 * @param data User-defined data passed to the callback.
 */
void wm_register_gesture_cb(wm_t wm, wm_gesture_cb_t cb, void* data);

/**
 * Register a keyboard key callback.
 *
 * These will be called whenever a key is pressed or released.
 *
 * @param wm The WM to register the callback for.
 * @param cb The callback to register.
 * @param data User-defined data passed to the callback.
 */
void wm_register_key_cb(wm_t wm, wm_key_cb_t cb, void* data);

/**
 * Enter the WM event loop.
 *
 * This will block until the WM is closed.
 *
 * @param wm The WM object to run the loop on.
 */
void wm_loop(wm_t wm);

/**
 * Get the backend the WM is running on.
 *
 * This is known as soon as the WM is created and doesn't change over its lifetime.
 *
 * @param wm The WM object.
 * @return The WM's backend, or {@link WM_BACKEND_UNKNOWN} if the call failed or the VDEV returned a backend we don't know about.
 */
wm_backend_t wm_get_backend(wm_t wm);

/**
 * Ask for a new frame to be rendered.
 *
 * The WM only redraws when something asks it to or when window contents change, so the redraw callback (see {@link wm_register_redraw_cb}) is only called after this.
 * This can be called from within the redraw callback (e.g. to keep animating), in which case another frame is rendered after the current one.
 *
 * @param wm The WM object.
 */
void wm_schedule_frame(wm_t wm);

/**
 * Notify WM window of a mouse motion event.
 *
 * @param wm The WM object which owns the window.
 * @param win Window handle to send the notification to.
 * @param time The time at which the notification occurred. This should've been gotten from {@link wm_mouse_motion_cb_t}.
 * @param x New X position of mouse relative to this window.
 * @param y New Y position of mouse relative to this window.
 */
void wm_win_notify_mouse_motion(wm_t wm, wm_win_t win, uint32_t time, uint32_t x, uint32_t y);

/**
 * Notify WM window of a mouse button press/release event.
 *
 * @param wm The WM object which owns the window.
 * @param win Window handle to send the notification to.
 * @param time The time at which the notification occurred. This should've been gotten from {@link wm_mouse_button_cb_t}.
 * @param pressed True if the button was pressed, false if it was released.
 * @param button The button identifier.
 */
void wm_win_notify_mouse_button(wm_t wm, wm_win_t win, uint32_t time, bool pressed, uint32_t button);

/**
 * Notify WM window of a keyboard key press/release event.
 *
 * This gives the window keyboard focus if it didn't already have it.
 *
 * @param wm The WM object which owns the window.
 * @param win Window handle to send the notification to.
 * @param time The time at which the notification occurred. This should've been gotten from {@link wm_key_cb_t}.
 * @param pressed True if the key was pressed, false if it was released.
 * @param keycode The raw keycode of the key. This should've been gotten from {@link wm_key_cb_t}.
 */
void wm_win_notify_key(wm_t wm, wm_win_t win, uint32_t time, bool pressed, uint32_t keycode);

/**
 * Notify WM window of a mouse axis (scroll) event.
 *
 * The window must already have pointer focus, i.e. it must be the last one {@link wm_win_notify_mouse_motion} was called on.
 *
 * @param wm The WM object which owns the window.
 * @param win Window handle to send the notification to.
 * @param time The time at which the notification occurred. This should've been gotten from {@link wm_mouse_axis_cb_t}.
 * @param source Source of the scroll event.
 * @param orientation Whether this is a vertical or horizontal scroll.
 * @param direction Relative direction of the scroll compared to the physical motion.
 * @param delta Scroll distance.
 * @param delta_discrete Scroll distance in fractions of 120 wheel clicks.
 */
void wm_win_notify_mouse_axis(
	wm_t wm,
	wm_win_t win,
	uint32_t time,
	wm_axis_source_t source,
	wm_axis_orientation_t orientation,
	wm_axis_direction_t direction,
	double delta,
	int32_t delta_discrete
);

/**
 * Notify WM window of a touchpad gesture event.
 *
 * The window must already have pointer focus, i.e. it must be the last one {@link wm_win_notify_mouse_motion} was called on.
 * All phases of a gesture should be sent to the same window.
 *
 * @param wm The WM object which owns the window.
 * @param win Window handle to send the notification to.
 * @param time The time at which the notification occurred. This should've been gotten from {@link wm_gesture_cb_t}.
 * @param kind Kind of gesture.
 * @param phase Phase of gesture.
 * @param fingers Number of fingers.
 * @param dx Relative X motion of the gesture's center.
 * @param dy Relative Y motion of the gesture's center.
 * @param scale Absolute scale compared to the begin phase.
 * @param rotation Relative clockwise rotation in degrees.
 * @param cancelled Whether the gesture was cancelled.
 */
void wm_win_notify_gesture(
	wm_t wm,
	wm_win_t win,
	uint32_t time,
	wm_gesture_kind_t kind,
	wm_gesture_phase_t phase,
	uint32_t fingers,
	double dx,
	double dy,
	double scale,
	double rotation,
	bool cancelled
);

/**
 * Ask a WM window to resize itself.
 *
 * This is only a request: the window is free to pick a different size (e.g. to respect its minimum size).
 * Its actual size is only known once it is redrawn, through {@link wm_redraw_win_cb_t}.
 *
 * @param wm The WM object which owns the window.
 * @param win Window handle to resize.
 * @param x_res Requested width of the window's geometry.
 * @param y_res Requested height of the window's geometry.
 */
void wm_win_set_size(wm_t wm, wm_win_t win, uint32_t x_res, uint32_t y_res);

/**
 * Tell a WM window whether it is being interactively resized.
 *
 * This should be set while the user is dragging to resize the window, and unset when they're done.
 * Windows may use this information to avoid doing expensive work on each resize.
 *
 * @param wm The WM object which owns the window.
 * @param win Window handle.
 * @param resizing Whether the window is being interactively resized.
 */
void wm_win_set_resizing(wm_t wm, wm_win_t win, bool resizing);
