// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#pragma once

#include "root.h"

#include <aqua/kos.h>

/**
 * Font library component context.
 */
typedef struct font_ctx_t* font_ctx_t;

/**
 * Initialize the font library component.
 *
 * @param ctx The AQUA library context.
 * @return The font library component handle.
 */
aqua_component_t font_init(aqua_ctx_t ctx);

/**
 * Connect to a font VDEV.
 *
 * {@link font_disconn} must be called to disconnect from the VDEV and to free the context.
 *
 * @param vdev The descriptor of the window VDEV to connect to.
 * @return The window library component context or `NULL` if allocation failed.
 */
font_ctx_t font_conn(kos_vdev_descr_t const* vdev);

/**
 * Disconnect from a font VDEV.
 *
 * This function disconnects from the font VDEV and frees the context.
 *
 * @param ctx The font library component context.
 */
void font_disconn(font_ctx_t ctx);
