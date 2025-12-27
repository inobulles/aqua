// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#pragma once

#include <stdint.h>

typedef struct {
	// Vulkan stuff.

	void* vk_instance;
	void* vk_phys_dev;
	void* vk_dev;
	uint32_t vk_queue_family;
} aqua_wm_t;
