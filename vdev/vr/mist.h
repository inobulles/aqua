// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct {
	bool set;
	void (*send_win)(uint32_t id, uint32_t x_res, uint32_t y_res, void const* fb_data);
} mist_ops_t;
