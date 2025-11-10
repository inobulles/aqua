// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
	bool set;

	void (*send_win)(
		uint32_t id,
		uint32_t x_res,
		uint32_t y_res,
		uint32_t tiles_x,
		uint32_t tiles_y,
		uint64_t const* tile_update_bitmap,
		void const* tile_data
	);

	void (*destroy_win)(uint32_t id);
} mist_ops_t;
