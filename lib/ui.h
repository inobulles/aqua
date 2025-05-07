// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2025 Aymeric Wibo

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
 * TODO Where should these be included from?
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
 * @return The root element.
 */
ui_elem_t ui_get_root(ui_t ui);

/**
 * Add an element to the UI.
 *
 * @param parent The parent element to add the new element under. This must be an element of {@link UI_ELEM_KIND_DIV} kind.
 * @param kind The kind of the new element.
 * @param semantics The semantic string of the new element. This is entirely implementation-specific.
 * @return The new element or `NULL` if allocation failed.
 */
ui_elem_t ui_add(ui_elem_t parent, ui_elem_kind_t kind, char const* semantics);
