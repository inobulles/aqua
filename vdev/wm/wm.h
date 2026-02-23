// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

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
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>

#include <webgpu/webgpu.h>
#include <webgpu/wgpu.h>

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
	struct wlr_renderer* wlr_renderer;
	struct wlr_allocator* allocator;

	// Output layouts.

	struct wlr_output_layout* output_layout;
	struct wl_list outputs;
	struct wl_listener new_output;

	// Input methods.

	struct wl_listener new_input;
	struct wl_list keyboards;

	// Scene graph.

	struct wlr_scene* scene;
	struct wlr_scene_output_layout* scene_layout;

	// XDG-shell stuff.

	struct wlr_xdg_shell* xdg_shell;
	struct wl_listener new_xdg_toplevel;
	struct wl_listener new_xdg_popup;
	struct wl_listener new_xdg_surface;
	struct wl_list toplevels;

	// Cursor stuff.

	struct wlr_cursor* cursor;
	struct wl_listener cursor_motion;
	struct wl_listener cursor_motion_absolute;
	struct wl_listener cursor_button;
	struct wl_listener cursor_axis;
	struct wl_listener cursor_frame;

	// Seat stuff.

	struct wlr_seat* seat;

	// Backend stuff.

	void* cur_dummy_cmd_buf;
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

typedef struct {
	struct wl_list link;

	wm_t* wm;
	struct wlr_xdg_surface* xdg_surface;

	struct wl_listener config;
	struct wl_listener destroy;
} surf_t;

typedef enum : uint8_t {
	INTR_REDRAW,
	INTR_NEW_WIN,
	INTR_DESTROY_WIN,
	INTR_REDRAW_WIN,
} intr_t;

void wm_vdev_init(umber_class_t const* cls, umber_class_t const* cls_wlr);

wm_t* wm_vdev_create(void);
void wm_vdev_destroy(wm_t* wm);

void wm_vdev_loop(wm_t* wm);
void wm_vdev_get_fb(toplevel_t* toplevel, void* buf);
