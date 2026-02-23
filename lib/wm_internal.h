// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025-2026 Aymeric Wibo

#pragma once

#if !defined(__AQUA_LIB_COMPONENT__)
# error "This file should only be included by AQUA library component source files."
#endif

#include "wm.h"

struct wm_t {
	wm_ctx_t ctx;
	kos_opaque_ptr_t opaque_ptr;
	kos_ino_t ino;

	void* redraw_data;
	wm_redraw_cb_t redraw;

	void* new_win_data;
	wm_new_win_cb_t new_win;

	void* destroy_win_data;
	wm_destroy_win_cb_t destroy_win;

	void* redraw_win_data;
	wm_redraw_win_cb_t redraw_win;

	void* mouse_motion_data;
	wm_mouse_motion_cb_t mouse_motion;

	void* mouse_button_data;
	wm_mouse_button_cb_t mouse_button;
};
