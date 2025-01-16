// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#pragma once

#include <stdint.h>

typedef enum {
	AQUA_WIN_KIND_NONE,
	AQUA_WIN_KIND_WAYLAND,
	AQUA_WIN_KIND_XCB,
	AQUA_WIN_KIND_XLIB,
	AQUA_WIN_KIND_APPKIT,
} aqua_win_kind_t;

// This struct is used for communicating a cross-platform window handle between VDEV's.
// This is similar to Rust's RawDisplayHandle/RawWindowHandle.

typedef struct {
	aqua_win_kind_t kind;

	union {
		struct {
		} none;

		struct {
			void* display; // Pointer to wl_display.
			void* surface; // Pointer to wl_surface.
		} wayland;

		struct {
			void* connection; // Pointer to xcb_connection_t.
			int32_t screen;
			uint32_t window;
			uint32_t visual_id;
		} xcb;

		struct {
			void* display; // Pointer to Xlib Display.
			int32_t screen;
			uint64_t window;
			uint64_t visual_id;
		} xlib;

		struct {
			void* ns_view; // Pointer to NSView.
		} appkit;
	} detail;

	char priv[];
} aqua_win_t;
