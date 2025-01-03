// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2024 Aymeric Wibo

#pragma once

#include <stdint.h>
#include <stdlib.h>

#include "kos.h"

typedef uint64_t vid_t;

typedef struct {
	vid_t id;
	char human[256];
} vdev_t;

typedef struct {
	char spec[64];
	char human[256];
	uint32_t vers;

	vid_t vdev_id_lo;
	vid_t vdev_id_hi;

	kos_vdev_notif_cb_t vdev_notif_cb;
	void* vdev_notif_data;

	void (*init)(void);
	void (*probe)(void);
	void* (*conn)(uint64_t vdev_id, kos_vdev_notif_cb_t cb, void* data);
} vdriver_t;

extern vdriver_t VDRIVER;
