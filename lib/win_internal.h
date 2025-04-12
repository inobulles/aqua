// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2025 Aymeric Wibo

#pragma once

#if !defined(__AQUA_LIB_COMPONENT__)
# error "This file should only be included by AQUA library component source files."
#endif

#include "win.h"

struct win_t {
	win_ctx_t ctx;
	void* opaque_ptr;
	kos_ino_t ino;

	void* redraw_data;
	win_redraw_cb_t redraw;
};
