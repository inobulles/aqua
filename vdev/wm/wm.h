// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025-2026 Aymeric Wibo

#pragma once

#include <umber.h>

#include "wm_public.h"

#include <wayland-client.h>
#include <wayland-server-core.h>

#define WLR_USE_UNSTABLE 1 // TODO why does the header force me to define this?

#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_pointer_constraints_v1.h>
#include <wlr/types/wlr_pointer_gestures_v1.h>
#include <wlr/types/wlr_relative_pointer_v1.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_server_decoration.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_xdg_decoration_v1.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>

#include <webgpu/webgpu.h>
#include <webgpu/wgpu.h>

typedef enum : uint8_t {
	BACKEND_DRM,
	BACKEND_WAYLAND,
	BACKEND_X11,
	BACKEND_HEADLESS,
} backend_t;

typedef struct {
	aqua_wm_t public;

	// AQUA stuff.

	bool has_ino;
	uint32_t ino;

	// Wayland stuff.

	struct wl_display* display;
	struct wl_event_loop* event_loop;
	struct wl_compositor* compositor;

	// Surface stuff.

	struct wl_listener new_surf;

	// wlroots stuff.

	struct wlr_backend* backend;
	backend_t backend_kind;
	struct wlr_renderer* wlr_renderer;
	struct wlr_allocator* allocator;

	// Output layouts.

	struct wlr_output_layout* output_layout;
	struct wl_list outputs;
	struct wl_listener new_output;

	// Input methods.

	struct wl_listener new_input;
	struct wl_list keyboards;
	struct wl_list mice;

	// Touchpad gestures, which have their own protocol (pointer-gestures-unstable-v1).

	struct wlr_pointer_gestures_v1* pointer_gestures;

	// Relative pointer motion and pointer constraints (relative-pointer-unstable-v1 and pointer-constraints-unstable-v1).
	// Only locks are supported for now; confines are never activated.

	struct wlr_relative_pointer_manager_v1* relative_pointer_mgr;
	struct wlr_pointer_constraints_v1* pointer_constraints;
	struct wl_listener new_pointer_constraint;

	struct wlr_pointer_constraint_v1* active_constraint;
	struct wl_listener active_constraint_destroy;

	// Scene graph.

	struct wlr_scene* scene;
	struct wlr_scene_output_layout* scene_layout;

	// XDG-shell stuff.

	struct wlr_xdg_shell* xdg_shell;
	struct wl_listener new_xdg_toplevel;
	struct wl_listener new_xdg_popup;
	struct wl_listener new_xdg_surface;
	struct wl_list toplevels;

	// Decoration stuff.

	struct wlr_xdg_decoration_manager_v1* xdg_deco_mgr;
	struct wl_listener new_xdg_deco;

	// Cursor stuff.

	struct wlr_cursor* cursor;
	struct wl_listener cursor_motion;
	struct wl_listener cursor_motion_absolute;
	struct wl_listener cursor_button;
	struct wl_listener cursor_axis;
	struct wl_listener cursor_frame;

	// Seat stuff.

	struct wlr_seat* seat;
	uint32_t seat_caps;
	struct wl_listener request_set_selection;

	// Tracking stuff.

	struct wl_list images;
} wm_t;

typedef struct {
	struct wl_list link;

	wm_t* wm;
	struct wlr_xdg_toplevel* xdg_toplevel;
	struct wlr_scene_tree* scene_tree;

	struct wl_listener map;
	struct wl_listener unmap;
	struct wl_listener commit;
	struct wl_listener destroy;
} toplevel_t;

typedef enum : uint8_t {
	INTR_REDRAW,
	INTR_NEW_WIN,
	INTR_DESTROY_WIN,
	INTR_REDRAW_WIN,
	INTR_DESTROY_IMAGE,
	INTR_MOUSE_MOTION,
	INTR_MOUSE_BUTTON,
	INTR_MOUSE_AXIS,
	INTR_GESTURE,
} intr_t;

typedef enum : uint8_t {
	AXIS_SOURCE_WHEEL,
	AXIS_SOURCE_FINGER,
	AXIS_SOURCE_CONTINUOUS,
	AXIS_SOURCE_WHEEL_TILT,
} axis_source_t;

typedef enum : uint8_t {
	AXIS_ORIENTATION_VERTICAL,
	AXIS_ORIENTATION_HORIZONTAL,
} axis_orientation_t;

typedef enum : uint8_t {
	AXIS_DIRECTION_IDENTICAL,
	AXIS_DIRECTION_INVERTED,
} axis_direction_t;

typedef enum : uint8_t {
	GESTURE_SWIPE,
	GESTURE_PINCH,
	GESTURE_HOLD,
} gesture_kind_t;

typedef enum : uint8_t {
	GESTURE_PHASE_BEGIN,
	GESTURE_PHASE_UPDATE,
	GESTURE_PHASE_END,
} gesture_phase_t;

void wm_vdev_init(umber_class_t const* cls, umber_class_t const* cls_wlr);

wm_t* wm_vdev_create(void);
void wm_vdev_destroy(wm_t* wm);

void wm_vdev_loop(wm_t* wm);
backend_t wm_vdev_get_backend(wm_t* wm);

void wm_vdev_interrupt(wm_t* wm, size_t data_size, void const* data);
void wm_vdev_focus_toplevel(toplevel_t* toplevel);

// Input stuff (input.c).

void wm_vdev_input_init(umber_class_t const* cls);
void wm_vdev_new_input(struct wl_listener* listener, void* data);

void wm_vdev_new_pointer_constraint(struct wl_listener* listener, void* data);
void wm_vdev_update_pointer_constraint(wm_t* wm);

void wm_vdev_toplevel_notify_mouse_motion(toplevel_t* toplevel, uint32_t time, uint32_t x, uint32_t y);
void wm_vdev_toplevel_notify_mouse_button(toplevel_t* toplevel, uint32_t time, bool pressed, uint32_t button);
void wm_vdev_toplevel_notify_mouse_axis(
	toplevel_t* toplevel,
	uint32_t time,
	axis_source_t source,
	axis_orientation_t orientation,
	axis_direction_t direction,
	double delta,
	int32_t delta_discrete
);
void wm_vdev_toplevel_notify_gesture(
	toplevel_t* toplevel,
	uint32_t time,
	gesture_kind_t kind,
	gesture_phase_t phase,
	uint32_t fingers,
	double dx,
	double dy,
	double scale,
	double rotation,
	bool cancelled
);
void wm_vdev_toplevel_set_size(toplevel_t* toplevel, uint32_t x_res, uint32_t y_res);
void wm_vdev_toplevel_set_resizing(toplevel_t* toplevel, bool resizing);
