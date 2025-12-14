// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#pragma once

#if !defined(__AQUA_LIB_COMPONENT__)
# error "This file should only be included by AQUA library component source files."
#endif

#include "ui.h"

typedef struct {
	uint32_t init;
	uint32_t render;
} backend_fns_t;

struct ui_ctx_t {
	// TODO This is pretty repetitive between library components - maybe we should inherit from a base struct?

	uint64_t hid;
	uint64_t vid;

	kos_cookie_t last_cookie;
	kos_val_t last_ret;
	bool last_success;

	bool is_conn;
	uint64_t conn_id;

	struct {
		uint32_t ELEM_KIND_DIV;
		uint32_t ELEM_KIND_TEXT;
	} consts;

	struct {
		uint32_t create;
		uint32_t destroy;
		uint32_t get_root;
		uint32_t add_div;
		uint32_t add_text;
		uint32_t set_attr;
	} fns;

	// Backend-specific functions.

	ui_supported_backends_t supported_backends;
	backend_fns_t backend_wgpu_fns;
};

struct ui_t {
	ui_ctx_t ctx;
	kos_opaque_ptr_t opaque_ptr;
};

struct ui_elem_t {
	ui_t ui;
	kos_opaque_ptr_t opaque_ptr;
};
