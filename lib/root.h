// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2025 Aymeric Wibo

#pragma once

#include <aqua/kos.h>

#include <stdbool.h>
#include <stdlib.h>

/**
 * AQUA library context.
 */
typedef struct aqua_ctx_t* aqua_ctx_t;

/**
 * AQUA library component.
 */
typedef void* aqua_component_t;

/**
 * AQUA VDEV iterator.
 *
 * This is used to iterate over all the VDEVs found for a library component.
 * The iterator is created with {@link aqua_vdev_it} and should be iterated over with {@link aqua_vdev_it_next}.
 */
typedef struct {
	/**
	 * The current VDEV descriptor.
	 *
	 * Once the iterator runs out of VDEVs, this will be set to `NULL`.
	 */
	kos_vdev_descr_t* vdev;

	/**
	 * The component whose VDEVs we're iterating over.
	 *
	 * This is a private field and should not be directly accessed outside of the library.
	 */
	aqua_component_t __comp;

	/**
	 * The index of the iterator.
	 *
	 * This is a private field and should not be directly accessed outside of the library.
	 */
	size_t __i;
} aqua_vdev_it_t;

/**
 * Initialize the library.
 *
 * Essentially this initializes the driver context and establishes initial communication to the KOS ({@link kos_hello}) as well as subscribing to its notifications ({@link kos_sub_to_notif}).
 *
 * @return The AQUA library context or `NULL` if something went wrong.
 */
aqua_ctx_t aqua_init(void);

/**
 * Get descriptor of the KOS we're using.
 *
 * @param ctx The AQUA driver context.
 * @return The v4 KOS descriptor.
 */
kos_descr_v4_t* aqua_get_kos_descr(aqua_ctx_t const ctx);

/**
 * Create a VDEV iterator.
 *
 * If there are no VDEVs to iterate over, the `vdev` field will be set to `NULL`.
 * Otherwise, the first VDEV will be stored in the `vdev` field.
 *
 * @param comp The component to iterate over.
 * @return The VDEV iterator object.
 */
aqua_vdev_it_t aqua_vdev_it(aqua_component_t comp);

/**
 * Get the next VDEV in the iterator.
 *
 * If there are no more VDEVs to iterate over, the `vdev` field will be set to `NULL`.
 * Otherwise, the next VDEV will be stored in the `vdev` field.
 *
 * This function must not be called on an iterator that has already has no more VDEVs to iterate over.
 *
 * @param it The VDEV iterator object.
 */
void aqua_vdev_it_next(aqua_vdev_it_t* it);
