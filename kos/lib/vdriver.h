// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2024-2025 Aymeric Wibo

/**
 * Interface for developing VDEV drivers (VDRIVERs).
 *
 * The documentation here is written for the VDRIVER developer, not the KOS implementer.
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

#include "kos.h"

/**
 * A VDEV ID is a unique identifier for a VDEV within a host.
 */
typedef uint64_t vid_t;

/**
 * See {@link vdriver_t.write_ptr}.
 */
typedef int (*vdriver_write_ptr_t)(kos_ptr_t ptr, void const* data, uint32_t size);

/**
 * The descriptor for a VDRIVER.
 *
 * The `VDRIVER` global variable should be of this type.
 */
typedef struct {
	/**
	 * The specification this VDRIVER implements.
	 *
	 * E.g. aquabsd.alps.win.
	 */
	char spec[64];
	/**
	 * A human-readable name for this specific VDRIVER implementation.
	 *
	 * This should contain which physical devices this VDRIVER supports.
	 * E.g., if this VDRIVER handles macOS webcams through the aquabsd.alps.cam spec, it should say something like "macOS webcam driver".
	 */
	char human[256];
	/**
	 * The version of this VDRIVER.
	 */
	uint32_t vers;

	/**
	 * The host ID this VDRIVER is running on.
	 */
	uint64_t host_id;

	/**
	 * The bottom of the VDEV ID slice this VDRIVER is allowed to use.
	 *
	 * This is set by the KOS when loading the VDRIVER; it should not be written to by the VDRIVER, only read from.
	 */
	vid_t vdev_id_lo;
	/**
	 * The top of the VDEV ID slice this VDRIVER is allowed to use.
	 *
	 * This is set by the KOS when loading the VDRIVER; it should not be written to by the VDRIVER, only read from.
	 */
	vid_t vdev_id_hi;

	/**
	 * The callback the VDRIVER should use for sending notifications to the KOS.
	 *
	 * This is set by the KOS when loading the VDRIVER; it should not be written to by the VDRIVER, only read from.
	 */
	kos_notif_cb_t notif_cb;
	/**
	 * The data to pass to the notification callback when the VDRIVER wants to call it.
	 *
	 * This is set by the KOS when loading the VDRIVER; it should not be written to by the VDRIVER, only read from.
	 */
	void* notif_data;

	/**
	 * Write to memory potentially on another host.
	 *
	 * This is set by the KOS when loading the VDRIVER; it should not be written to by the VDRIVER, only read from.
	 *
	 * @param ptr The KOS pointer to write to.
	 * @param data The data to write.
	 * @param size The size of the data to write.
	 * @return 0 on success, or -1 on failure.
	 */
	vdriver_write_ptr_t write_ptr;

	/**
	 * The initialization function of the VDRIVER.
	 *
	 * This is called by the KOS when the VDRIVER is loaded.
	 */
	void (*init)(void);
	/**
	 * The probe function of the VDRIVER.
	 *
	 * This should send a {@link KOS_NOTIF_ATTACH} notification for each VDEV the VDRIVER supports using the notification callback.
	 */
	void (*probe)(void);
	/**
	 * The connection function of the VDRIVER.
	 *
	 * This is called by the KOS when a connection to a VDEV is requested.
	 * The VDRIVER should send a {@link KOS_NOTIF_CONN} or {@link KOS_NOTIF_CONN_FAIL} notification in response.
	 *
	 * @param cookie The cookie used to identify the request. This should be passed back to any structs sent back to the KOS which need it.
	 * @param vdev_id The ID of the VDEV to connect to.
	 * @param conn_id The ID of the connection to create. These connection IDs are allocated and managed by the KOS.
	 */
	void (*conn)(kos_cookie_t cookie, vid_t vdev_id, uint64_t conn_id);
	/**
	 * The call function of the VDRIVER.
	 *
	 * This is called by the KOS when a function on a VDEV is called.
	 * The VDRIVER should send a {@link KOS_NOTIF_CALL_RET} or {@link KOS_NOTIF_CALL_FAIL} notification in response.
	 *
	 * @param cookie The cookie used to identify the request. This should be passed back to any structs sent back to the KOS which need it.
	 * @param conn_id The ID of the connection to call the function on.
	 * @param fn_id The ID of the function to call.
	 * @param args The arguments to pass to the function.
	 */
	void (*call)(kos_cookie_t cookie, uint64_t conn_id, uint64_t fn_id, kos_val_t const* args);
} vdriver_t;

/**
 * The `VDRIVER` symbol.
 *
 * The VDRIVER should populate this.
 */
extern vdriver_t VDRIVER;

/**
 * Unwrap local pointer from an opaque pointer.
 *
 * Only VDRIVERs are allowed to read the contents of opaque pointers.
 *
 * @param opaque_ptr The opaque pointer to unwrap.
 * @return The local pointer if the opaque pointer is local, or `NULL` if it is not.
 */
void* vdriver_unwrap_local_opaque_ptr(kos_opaque_ptr_t opaque_ptr);

/**
 * Unwrap local pointer from a KOS pointer.
 *
 * @param ptr The KOS pointer to unwrap.
 * @return The local pointer if the KOS pointer is local, or `NULL` if it is not.
 */
void* vdriver_unwrap_local_ptr(kos_ptr_t ptr);

/**
 * Make a KOS opaque pointer from a local pointer.
 *
 * This is used by VDRIVERs to create opaque pointers that can be passed to the KOS.
 *
 * @param ptr The local pointer to wrap.
 * @return The KOS opaque pointer wrapping the local pointer.
 */
kos_opaque_ptr_t vdriver_make_opaque_ptr(void const* ptr);

/**
 * Make a KOS pointer from a local pointer.
 *
 * @param ptr The local pointer to wrap.
 * @return The KOS pointer wrapping the local pointer.
 */
kos_ptr_t vdriver_make_ptr(void const* ptr);
