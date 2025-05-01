// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2025 Aymeric Wibo

#pragma once

#include "root.h"

/**
 * UI library component context.
 */
typedef struct ui_ctx_t* ui_ctx_t;

/**
 * Initialize the UI library component.
 *
 * @param ctx The AQUA library context.
 * @return The UI library component handle.
 */
aqua_component_t ui_init(aqua_ctx_t ctx);
