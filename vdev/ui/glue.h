// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2026 Aymeric Wibo

// Types shared between the C glue and the Go side of the VDRIVER.

#pragma once

#include <stdint.h>

typedef enum : uint32_t {
	RENDER_FLAG_NEEDS_FRAME = 1 << 0,
} render_flags_t;
