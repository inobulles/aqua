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

/**
 * Set the rendering limits of a layout.
 *
 * This defines the maximum horizontal and vertical resolution the layout may occupy when being rendered or laid out.
 * The implementation may perform line wrapping or clipping depending on these limits.
 *
 * @param layout The layout to modify.
 * @param x_res_limit The maximum horizontal resolution.
 * @param y_res_limit The maximum vertical resolution.
 */
void font_layout_set_limits(font_layout_t layout, uint32_t x_res_limit, uint32_t y_res_limit);

/**
 * Get the character index at a given position in the layout.
 *
 * This performs a position-to-index lookup
 * Given a pixel position within the layout, it returns the closest character index within the text.
 * This is typically used for text hit-testing or cursor placement.
 *
 * @param layout The layout to query.
 * @param x The X coordinate, in pixels.
 * @param y The Y coordinate, in pixels.
 * @return The character index corresponding to that position, or -1 if unavailable.
 */
int32_t font_layout_pos_to_index(font_layout_t layout, uint32_t x, uint32_t y);

/**
 * Get the position of a given character index in the layout.
 *
 * This performs an index-to-position lookup.
 * Given a character index within the text, it returns the corresponding X and Y pixel position in the layout.
 * This can be used for drawing cursors or highlighting selections.
 *
 * @param layout The layout to query.
 * @param index The character index to locate.
 * @param x Pointer to where the X coordinate should be written.
 * @param y Pointer to where the Y coordinate should be written.
 */
void font_layout_index_to_pos(font_layout_t layout, int32_t index, uint32_t* x, uint32_t* y);

/**
 * Get the resolution of the layout if it were to be rendered.
 *
 * @param layout The layout to get the resolution of.
 * @param x_res Pointer to where the X resolution should be copied to.
 * @param y_res Pointer to where the Y resolution should be copied to.
 */
void font_layout_get_res(font_layout_t layout, uint32_t* x_res, uint32_t* y_res);

/**
 * Render the layout into a buffer.
 *
 * This rasterizes the layout into a caller-provided buffer.
 * The buffer must be large enough to hold the rendered output, determined via {@link font_layout_get_res}.
 *
 * @param layout The layout to render.
 * @param buffer Pointer to the buffer where the rendered output will be written.
 */
void font_layout_render(font_layout_t layout, void* buffer);
