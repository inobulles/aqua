// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#pragma once

#include "root.h"

/**
 * VR library component context.
 */
typedef struct vr_ctx_t* vr_ctx_t;

/**
 * Initialize the VR library component.
 *
 * @param ctx The AQUA library context.
 * @return The VR library component handle.
 */
aqua_component_t vr_init(aqua_ctx_t ctx);

/**
 * Connect to a VR VDEV.
 *
 * @param vdev The descriptor of the VR VDEV to connect to.
 * @return The VR library component context or `NULL` if allocation failed.
 */
vr_ctx_t vr_conn(kos_vdev_descr_t const* vdev);

/**
 * Disconnect from a VR VDEV.
 *
 * @param ctx The VR library component context.
 */
void vr_disconn(vr_ctx_t ctx);

/**
 * TODO
 */
void vr_send_win(
	vr_ctx_t ctx,
	uint32_t id,
	uint32_t x_res,
	uint32_t y_res,
	uint32_t tiles_x,
	uint32_t tiles_y,
	uint64_t* tile_update_bitmap,
	size_t tile_data_size,
	void* tile_data
);

/**
 * TODO
 */
void vr_destroy_win(vr_ctx_t ctx, uint32_t id);
