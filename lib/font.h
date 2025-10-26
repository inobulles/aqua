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
 * Font object.
 */
typedef struct font_t* font_t;

/**
 * Text layout object.
 */
typedef struct font_layout_t* font_layout_t;

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

/**
 * Create a font object from a string representation.
 *
 * This string representation is implementation-defined at the moment, but it will most likely follow the Pango string representation for a given VDRIVER:
 * https://docs.gtk.org/Pango/type_func.FontDescription.from_string.html
 *
 * @param ctx The font library component context.
 * @param str The string representation.
 * @return The font object handle.
 */
font_t font_from_str(font_ctx_t ctx, char const* str);

/**
 * Destroy a font object.
 *
 * @param font The font object to destroy.
 */
void font_destroy(font_t font);

/**
 * Create a layout object from a font.
 *
 * This is what actually contains the information about the text, and which can eventually be rendered out to a buffer.
 *
 * @param font The font to use.
 * @param text The initial text the layout should contain.
 * @return The layout object handle.
 */
font_layout_t font_layout_create(font_t font, char const* text);

/**
 * Destroy a layout object.
 *
 * @param layout The layout object to destroy.
 */
void font_layout_destroy(font_layout_t layout);

/**
 * Set the text of a layout object.
 *
 * This replaced the previous text contained within the layout.
 *
 * @param layout The layout to set the text of.
 * @param text The text to set.
 */
void font_layout_set_text(font_layout_t layout, char const* text);
