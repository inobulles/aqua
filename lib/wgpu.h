// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2025 Aymeric Wibo

#pragma once

#include "aqua.h"

#include <aqua/kos.h>

/**
 * WebGPU library component context.
 */
typedef struct wgpu_ctx_t* wgpu_ctx_t;

/**
 * Initialize the WebGPU library component.
 *
 * @param ctx The AQUA library context.
 * @return The WebGPU library component handle.
 */
aqua_component_t wgpu_init(aqua_ctx_t ctx);

/**
 * Connect to a WebGPU VDEV.
 *
 * {@link wgpu_disconn} must be called to disconnect from the VDEV and to free the context.
 *
 * @param vdev The descriptor of the WebGPU VDEV to connect to.
 * @return The WebGPU library component context or `NULL` if allocation failed.
 */
wgpu_ctx_t wgpu_conn(kos_vdev_descr_t const* vdev);

/**
 * Disconnect from a WebGPU VDEV.
 *
 * This function disconnects from the WebGPU VDEV and frees the context.
 *
 * @param ctx The WebGPU library component context.
 */
void wgpu_disconn(wgpu_ctx_t ctx);
