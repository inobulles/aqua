// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2025 Aymeric Wibo

#if !defined(__AQUA_LIB_COMPONENT__)
# error "This file should only be included by AQUA library component source files."
#endif

// TODO This should probably be renamed "internal.h or something like that.

#include "aqua.h"

#include <aqua/kos.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * Parent struct of an AQUA library component.
 *
 * A library component is a sort of driver which wraps a VDEV and provides a more high-level interface for interacting with it.
 * Library component should inherit from this struct.
 */
typedef struct {
	/**
	 * The library context.
	 *
	 * This will be set by the library when registering with {@link aqua_register_component}.
	 */
	aqua_ctx_t ctx;

	/**
	 * The probe function for this library component.
	 *
	 * This should simply check if the VDEV is handled by this library component.
	 *
	 * @param vdev The VDEV received in the attach notification.
	 * @return `true` if the VDEV is supported by this library component, `false` if not.
	 */
	bool (*probe)(kos_vdev_descr_t const* vdev);

	/**
	 * The connection notification function for this library component.
	 *
	 * This is called when the library receives a connection success notification from the KOS for this library component.
	 * This should read all the constants and functions from the notification object and store them in the library component context.
	 * Not all constants/functions have to be supported or known by the library component, and not all constants/functions supported by the library component have to be advertised by the KOS for this VDEV.
	 * Deciding whether or not enough constants/functions are advertised for the library component to be able to work is at the library component's discretion.
	 *
	 * @param notif The notification received from the KOS. This must be a connection success notification (i.e. {@link KOS_NOTIF_CONN}).
	 * @param data User-defined data passed to the function.
	 */
	void (*notif_conn)(kos_notif_t const* notif, void* data);

	/**
	 * The connection failure notification function for this library component.
	 *
	 * This is called when the library receives a connection failure notification from the KOS for this library component.
	 * TODO Explain what the responsibilities of this function are.
	 *
	 * @param notif The notification received from the KOS. This must be a connection failure notification (i.e. {@link KOS_NOTIF_CONN_FAIL}).
	 * @param data User-defined data passed to the function.
	 */
	void (*notif_conn_fail)(kos_notif_t const* notif, void* data);

	/**
	 * TODO
	 */
	void (*notif_call_ret)(kos_notif_t const* notif, void* data);

	/**
	 * TODO
	 */
	void (*notif_call_fail)(kos_notif_t const* notif, void* data);

	/**
	 * The interrupt handler function for this library component.
	 *
	 * @param notif The notification received from the KOS. This must be an interrupt notification (i.e. {@link KOS_NOTIF_INTERRUPT}).
	 */
	void (*interrupt)(kos_notif_t const* notif, void* data);

	/**
	 * The number of VDEVs found that match this library component.
	 */
	size_t vdev_count;

	/**
	 * The list of VDEVs found that match this library component.
	 */
	kos_vdev_descr_t* vdevs;
} component_t;

/**
 * Register a library component with the AQUA library context.
 *
 * @param comp The library component to register.
 */
void aqua_register_component(aqua_ctx_t ctx, component_t* comp);

/**
 * A (cookie, connection notification callback) tuple.
 */
typedef struct {
	/**
	 * Used internally by the library.
	 *
	 * Is this slot currently in use by a pending connection?
	 */
	bool __slot_used;

	/**
	 * The cookie of the connection request.
	 *
	 * When responding with a connection success or failure notification, the KOS will include this cookie in the notification.
	 * This is how we know which connection notification callback to call.
	 */
	kos_cookie_t cookie;

	/**
	 * The component to call the connection success/failure notification functions of.
	 */
	component_t* comp;

	/**
	 * User-defined data to be passed to the function when called.
	 */
	void* data;
} cookie_notif_conn_tuple_t;

typedef struct {
	component_t* comp;
	void* data;
} conn_vec_ent_t;

typedef struct {
	kos_ino_t ino;
	component_t* comp;
	void* data;
} interrupt_vec_ent_t;

/**
 * Add pending (cookie, connection notification callback) tuple.
 *
 * The library will go through the list of pending connection tuples when receiving a connection success or failure notification from the KOS, and call the callback within if the cookies match.
 * See {@link cookie_notif_conn_tuple_t}.
 *
 * @param ctx The AQUA library context.
 * @param tuple The (cookie, connection notification callback) tuple to add.
 */
void aqua_add_pending_conn(aqua_ctx_t ctx, cookie_notif_conn_tuple_t* tuple);

/**
 * Add interrupt handler and component to the interrupt vector by its interrupt number.
 *
 * @param ctx The AQUA library context.
 * @param ino The interrupt number to index the interrupt vector by.
 * @param comp The component to add to that interrupt vector entry.
 * @param data User-defined data to be passed to the interrupt handler when called.
 */
void aqua_add_interrupt(aqua_ctx_t ctx, kos_ino_t ino, component_t* comp, void* data);
