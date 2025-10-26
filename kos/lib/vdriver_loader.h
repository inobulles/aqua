// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

/**
 * Interface for the (local) VDRIVER loader.
 *
 * This is used by the KOS to request VDEV specs and load relevant VDRIVERs.
 * It is also used by the GrapeVine daemon to take inventory of all VDEVs available on the system.
 */

#pragma once

#include "kos.h"
#include "vdriver.h"

/**
 * The VDRIVER_PATH environment variable name.
 *
 * This environment variable may contain multiple paths separated by colons.
 */
#define VDRIVER_PATH_ENVVAR "VDRIVER_PATH"

/**
 * The default VDRIVER path.
 *
 * This is the path where the VDEV drivers are stored, relative to the library prefix.
 * It is used if the VDRIVER_PATH environment variable is not set.
 */
#define DEFAULT_VDRIVER_PATH "vdriver"

/**
 * The file extension of VDRIVER files.
 *
 * Files with other extensions should be ignored.
 */
#define VDRIVER_EXT ".vdriver"

/**
 * Initialize VDRIVER loader global state.
 *
 * Call this before interacting with the loader.
 */
void vdriver_loader_init(void);

/**
 * Request all the VDEVs of a specific spec.
 *
 * This will look for and load the VDRIVER implementing this spec if it exists, 
 *
 * @param spec The specification of the VDEV to request.
 * @param host_id The host ID to pass to the VDRIVER when loading it.
 * @param notif_cb The callback to call for {@link KOS_NOTIF_ATTACH} notifications.
 * @param notif_data The data to pass to the notification callback.
 * @param write_ptr The function the VDRIVER will use to write to pointers.
 */
void vdriver_loader_req_local_vdev(
	char const* spec,
	uint64_t host_id,
	kos_notif_cb_t notif_cb,
	void* notif_data,
	vdriver_write_ptr_t write_ptr
);

/**
 * Take inventory of all VDEVs available on the system.
 *
 * This will load all VDRIVERs found and probe for all their VDEVs.
 *
 * @param host_id The host ID to pass to the VDRIVERs when loading them.
 * @param notif_cb The callback to call for {@link KOS_NOTIF_ATTACH} notifications.
 * @param notif_data The data to pass to the notification callback.
 */
void vdriver_loader_vdev_local_inventory(
	uint64_t host_id,
	kos_notif_cb_t notif_cb,
	void* notif_data
);

/**
 * Find an already loaded VDRIVER by a VDEV ID.
 *
 * Concretely, this just looks for the VDRIVER which has the given VDEV ID in its allocated VID slice.
 *
 * @param vid The VDEV ID to look for.
 * @return The VDRIVER associated with the given VDEV ID, or NULL if not found.
 */
vdriver_t* vdriver_loader_find_loaded_by_vid(vid_t vid);
