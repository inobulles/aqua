// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2024 Aymeric Wibo

#pragma once

#include "vdev.h"
#include <stdint.h>

typedef struct {
	vdriver_t* vdriver;
} conn_t;

extern conn_t* conns;
extern uint64_t conn_count;

uint64_t conn_new(vdriver_t* vdriver);
