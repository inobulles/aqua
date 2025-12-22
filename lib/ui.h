// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#pragma once

#include "root.h"

/**
 * UI library component context.
 */
typedef struct ui_ctx_t* ui_ctx_t;

/**
 * UI state object.
 *
 * This holds the semantic representation of the UI, as well as the actual layout.
 * This is the object you'll be interacting with the most.
 */
typedef struct ui_t* ui_t;

/**
 * UI element.
 *
 * AQUA's UI VDEV uses a retained mode API design, so the VDEV itself manages the element and you must hold this handle to be able to refer to it.
 */
typedef struct ui_elem_t* ui_elem_t;

/**
 * UI element kind.
 *
 * This is the fundamental type of the element.
 * Do note that the set of UI elements is very limited and simplified; to hint the UI backend to render elements in a more complex manner, you must also provide a semantic string.
 * See {@link ui_add} for more information.
 */
typedef enum {
	UI_ELEM_KIND_DIV,
	UI_ELEM_KIND_TEXT,
} ui_elem_kind_t;

/**
 * Supported UI backends.
 */
typedef enum {
	UI_BACKEND_NONE = 0b01,
	UI_BACKEND_WGPU = 0b10,
} ui_supported_backends_t;

/**
 * Dimension unit kinds.
 */
typedef enum {
	UI_DIM_UNITS_ZERO,
	UI_DIM_UNITS_PARENT_FRAC,
	UI_DIM_UNITS_PIXELS,
} ui_dim_units_t;

/**
 * Dimension structure.
 *
 * A dimension contains the units in which it is expressed and its value.
 */
typedef struct {
	ui_dim_units_t units;
	float val;
} ui_dim_t;

/**
 * Image raster structure.
 */
typedef struct {
	uint32_t x_res, y_res;
	void* data;
} ui_raster_t;

/**
 * Initialize the UI library component.
 *
 * @param ctx The AQUA library context.
 * @return The UI library component handle.
 */
aqua_component_t ui_init(aqua_ctx_t ctx);

/**
 * Connect to a UI VDEV.
 *
 * {@link ui_disconn} must be called to disconnect from the VDEV and to free the context.
 *
 * @param vdev The descriptor of the UI VDEV to connect to.
 * @return The UI library component context or `NULL` if allocation failed.
 */
ui_ctx_t ui_conn(kos_vdev_descr_t const* vdev);

/**
 * Disconnect from a UI VDEV.
 *
 * This function disconnects from the UI VDEV and frees the context.
 *
 * @param ctx The UI library component context.
 */
void ui_disconn(ui_ctx_t ctx);

/**
 * Get supported backends for this UI connection.
 *
 * These are the backends you may call the init function for.
 *
 * @param ctx The UI library component context.
 * @return The supported backends.
 */
ui_supported_backends_t ui_get_supported_backends(ui_ctx_t ctx);

/**
 * Create a new UI.
 *
 * See {@link ui_t} for more information on the object which this creates.
 *
 * @param ctx The UI library component context.
 * @return The UI object or `NULL` if allocation failed.
 */
ui_t ui_create(ui_ctx_t ctx);

/**
 * Destroy a UI.
 */
void ui_destroy(ui_t ui);

/**
 * Get root element.
 *
 * The root element is the top-level element of the UI, which is the common ancestor between all other elements.
 * It fills the entire UI context area.
 * You must first get the root element of the UI before you can add any elements to it with {@link ui_add}.
 *
 * @param ui The UI object.
 * @return The root element or `NULL` if something went wrong. This is allocated on the heap and must be freed with {@link free}.
 */
ui_elem_t ui_get_root(ui_t ui);

/**
 * Add a div element to the UI.
 *
 * A div is a container element which has other elements as its children.
 *
 * @param parent The parent element to add the new element under. This must be a div element.
 * @param semantics The semantic string of the new element. This is entirely implementation specific.
 * @return The new element or `NULL` if something went wrong. This is allocated on the heap and must be freed with {@link free}.
 */
ui_elem_t ui_add_div(ui_elem_t parent, char const* semantics);

/**
 * Add a text element to the UI.
 *
 * @param parent The parent element to add the new element under. This must be a div element.
 * @param semantics The semantic string of the new element. This is entirely implementation specific.
 * @param text The text to display in the element.
 * @return The new element or `NULL` if something went wrong. This is allocated on the heap and must be freed with {@link free}.
 */
ui_elem_t ui_add_text(ui_elem_t parent, char const* semantics, char const* text);

/**
 * Set a string attribute on a UI element.
 *
 * Please use the generic {@link ui_set_attr} macro instead!
 */
bool ui_set_attr_str(ui_elem_t elem, char const* key, char const* val);

/**
 * Set a 32-bit unsigned integer attribute on a UI element.
 *
 * Please use the generic {@link ui_set_attr} macro instead!
 */
bool ui_set_attr_u32(ui_elem_t elem, char const* key, uint32_t val);

/**
 * Set a 32-bit float attribute on a UI element.
 *
 * Please use the generic {@link ui_set_attr} macro instead!
 */
bool ui_set_attr_f32(ui_elem_t elem, char const* key, float val);

/**
 * Set an opaque pointer attribute on a UI element.
 *
 * Please use the generic {@link ui_set_attr} macro instead!
 */
bool ui_set_attr_opaque_ptr(ui_elem_t elem, char const* key, void* val);

/**
 * Set a dimension attribute on a UI element.
 *
 * Please use the generic {@link ui_set_attr} macro instead!
 */
bool ui_set_attr_dim(ui_elem_t elem, char const* key, ui_dim_t dim);

/**
 * Set an image raster attribute on a UI element.
 *
 * Please use the generic {@link ui_set_attr} macro instead!
 */
bool ui_set_attr_raster(ui_elem_t elem, char const* key, ui_raster_t raster);

// clang-format off

/**
 * Set an attribute on a UI element.
 *
 * Attempts to set the attribute identified by {@code key} to the given value on the specified element.
 * Attribute validity is element-specific; if the attribute is not supported by the element, this function will fail.
 * If the attribute already has a value, it will be overwritten.
 *
 * @param elem The element on which to set the attribute.
 * @param key The attribute name.
 * @param val The attribute value. This can either be a (C) string, 32-bit float, 32-bit unsigned integer, opaque pointer, {@link ui_dim_t}, or {@link ui_raster_t}.
 * @return {@code true} if the attribute was successfully set and is valid for the given element, {@code false} otherwise.
 */
#define ui_set_attr(elem, key, val)          \
    _Generic((val),                          \
        char const*: ui_set_attr_str,        \
        char*:       ui_set_attr_str,        \
        float:       ui_set_attr_f32,        \
        double:      ui_set_attr_f32,        \
        uint32_t:    ui_set_attr_u32,        \
        void*:       ui_set_attr_opaque_ptr, \
        ui_dim_t:    ui_set_attr_dim,        \
        ui_raster_t: ui_set_attr_raster      \
    )((elem), (key), (val))

// clang-format on
