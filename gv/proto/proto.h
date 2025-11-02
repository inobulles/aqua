// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

/**
 * Definition of the GrapeVine protocol and helper functions.
 *
 * The helper functions concern serialization and deserialization of KOS types.
 */

#pragma once

#include <aqua/kos.h>

#include <stdlib.h>

/**
 * The port used for GrapeVine connections (TCP).
 */
#define GV_PORT 0xA55

/**
 * The port used for sending GrapeVine ELPs (UDP).
 */
#define GV_ELP_PORT GV_PORT

/**
 * The type of a GrapeVine packet.
 *
 * More detailed descriptions for each one of these can be found in their structure definitions.
 */
typedef enum : uint8_t {
	// Packets purely handled by gvd.

	GV_PACKET_TYPE_ELP = 0,
	GV_PACKET_TYPE_QUERY = 1,
	GV_PACKET_TYPE_QUERY_RES = 2,

	// Packets used for creation and handoff from gvd to KOS agent.

	GV_PACKET_TYPE_CONN_VDEV = 3,
	GV_PACKET_TYPE_CONN_VDEV_FAIL = 4,
	GV_PACKET_TYPE_CONN_VDEV_RES = 5,

	// Packets used by KOS agent for communication.

	GV_PACKET_TYPE_KOS_CALL = 6,
	GV_PACKET_TYPE_KOS_CALL_FAIL = 7,
	GV_PACKET_TYPE_KOS_CALL_RET = 8,

	GV_PACKET_TYPE_LEN,
} gv_packet_type_t;

static char const* const gv_packet_type_strs[] = {
	"ELP",
	"QUERY",
	"QUERY_RES",
	"CONN_VDEV",
	"CONN_VDEV_FAIL",
	"CONN_VDEV_RES",
	"KOS_CALL",
	"KOS_CALL_FAIL",
	"KOS_CALL_RET",
};

_Static_assert(sizeof gv_packet_type_strs / sizeof *gv_packet_type_strs == GV_PACKET_TYPE_LEN, "Bad number of gv_packet_type_t strings.");

/**
 * The ELP packet version.
 */
#define GV_ELP_VERS 0

/**
 * Maximum size of our UDP packets.
 */
#define GV_UDP_BUDGET 300 // Maximum size of our UDP packets.

/**
 * ELP or echolocation packet.
 *
 * These are periodically sent out by each GrapeVine node by gvd to advertise their existence.
 */
typedef struct __attribute__((packed)) {
	/**
	 * The ELP version.
	 *
	 * Should be set to {@link GV_ELP_VERS}.
	 */
	uint8_t vers : 8;

	/**
	 * A unique value.
	 *
	 * If this value changes, it means we want all other hosts to send us a QUERY packet again because something has changed (e.g. new VDEVs are available).
	 */
	uint64_t unique : 56;

	/**
	 * A network-wide unique identifier for this node.
	 *
	 * Usually set to the MAC address.
	 */
	uint64_t host_id : 64;

	/**
	 * A friendly name for the host.
	 */
	uint8_t name[64];
} gv_elp_t;

_Static_assert(sizeof(gv_elp_t) < GV_UDP_BUDGET, "ELP packet is too large for our UDP budget.");

/**
 * QUERY response packet.
 *
 * This contains the VDEVs the node sending this supports, and is sent in response to a QUERY packet.
 */
typedef struct __attribute__((packed)) {
	/**
	 * Number of VDEVs this node supports.
	 */
	uint32_t vdev_count;

	/**
	 * Serialized VDEV descriptors of VDEVs we support.
	 */
	kos_vdev_descr_t vdevs[];
} gv_query_res_t;

/**
 * VDEV connection packet.
 */
typedef struct __attribute__((packed)) {
	/**
	 * The ID of the VDEV we want to connect to.
	 */
	uint64_t vdev_id;
} gv_conn_vdev_t;

/**
 * VDEV connection response packet.
 *
 * Sent when the connection succeeded to advertise the constants and functions this VDEV supports.
 */
typedef struct __attribute__((packed)) {
	/**
	 * Connection ID.
	 */
	uint64_t conn_id;

	/**
	 * Size of this struct.
	 */
	uint32_t size;

	/**
	 * Number of constants this VDEV supports.
	 */
	uint32_t const_count;

	/**
	 * Number of functions this VDEV supports.
	 */
	uint32_t fn_count;
} gv_conn_vdev_res_t;

/**
 * KOS call packet.
 */
typedef struct __attribute__((packed)) {
	/**
	 * Connection ID.
	 */
	uint64_t conn_id;

	/**
	 * Size of this struct.
	 */
	uint32_t size;

	/**
	 * ID of function to call.
	 */
	uint32_t fn_id;
} gv_kos_call_t;

/**
 * KOS call return packet.
 */
typedef struct __attribute__((packed)) {
	/**
	 * Size of return value.
	 *
	 * Necessary because otherwise it would be annoying to read buffer values.
	 */
	uint32_t size;
} gv_kos_call_ret_t;

// TODO All the others: writing pointer back, vitrification, writing chunks of memory (for when vitrification is happening and all of the syncing for that), and interrupts.

/**
 * GrapeVine packet description.
 */
typedef struct __attribute__((packed)) {
	struct {
		gv_packet_type_t type;
	} header;

	union {
		gv_elp_t elp;
		gv_query_res_t query_res;
		gv_conn_vdev_t conn_vdev;
		gv_conn_vdev_res_t conn_vdev_res;
		gv_kos_call_t kos_call;
		gv_kos_call_ret_t kos_call_ret;
	};
} gv_packet_t;

/**
 * Free a packet.
 *
 * Intended to be used with `__attribute__((cleanup()))`.
 */
static inline void gv_packet_free(gv_packet_t** packet) {
	free(*packet);
}

// Serialization functions.

/**
 * Get size of serialized value.
 *
 * @param t Type of value to serialize.
 * @param v Value to serialize.
 * @return Size of serialized value in bytes.
 */
size_t gv_serialize_val_size(kos_type_t t, kos_val_t const* v);

/**
 * Serialize value.
 *
 * @param buf Buffer to serialize the value in to. Expected to have the right size (see {@link gv_serialize_val_size}).
 * @param t Type of value to serialize.
 * @param v Value to serialize.
 * @return Size of serialized value in bytes.
 */
size_t gv_serialize_val(void* buf, kos_type_t t, kos_val_t const* v);

/**
 * Get size of serialized constant.
 *
 * @param c Constant to serialize.
 * @return Size of serialized constant in bytes.
 */
size_t gv_serialize_const_size(kos_const_t const* c);

/**
 * Serialize constant.
 *
 * @param buf Buffer to serialize the constant into. Expected to have the right size (see {@link gv_serialize_const_size}).
 * @param c Constant to serialize.
 * @return Size of serialized constant in bytes.
 */
size_t gv_serialize_const(void* buf, kos_const_t const* c);

/**
 * Get size of serialized parameter.
 *
 * @param p Parameter to serialize.
 * @return Size of serialized parameter in bytes.
 */
size_t gv_serialize_param_size(kos_param_t const* p);

/**
 * Serialize parameter.
 *
 * @param buf Buffer to serialize the parameter into. Expected to have the right size (see {@link gv_serialize_param_size}).
 * @param p Parameter to serialize.
 * @return Size of serialized parameter in bytes.
 */
size_t gv_serialize_param(void* buf, kos_param_t const* p);

/**
 * Get size of serialized function.
 *
 * @param fn Function to serialize.
 * @return Size of serialized function in bytes.
 */
size_t gv_serialize_fn_size(kos_fn_t const* fn);

/**
 * Serialize function.
 *
 * @param buf Buffer to serialize the function into. Expected to have the right size (see {@link gv_serialize_fn_size}).
 * @param fn Function to serialize.
 * @return Size of serialized function in bytes.
 */
size_t gv_serialize_fn(void* buf, kos_fn_t const* fn);

// Deserialization functions.

/**
 * Deserialize a value.
 *
 * @param buf Buffer containing the serialized value.
 * @param t Type of value to deserialize.
 * @param v Output location for the deserialized value.
 * @return Number of bytes consumed from the buffer.
 */
size_t gv_deserialize_val(void const* buf, kos_type_t t, kos_val_t* v);

/**
 * Deserialize a constant.
 *
 * @param buf Buffer containing the serialized constant.
 * @param c Output location for the deserialized constant.
 * @return Number of bytes consumed from the buffer.
 */
size_t gv_deserialize_const(void const* buf, kos_const_t* c);

/**
 * Deserialize a parameter.
 *
 * @param buf Buffer containing the serialized parameter.
 * @param p Output location for the deserialized parameter.
 * @return Number of bytes consumed from the buffer.
 */
size_t gv_deserialize_param(void const* buf, kos_param_t* p);

/**
 * Deserialize a function.
 *
 * @param buf Buffer containing the serialized function.
 * @param fn Output location for the deserialized function.
 * @return Number of bytes consumed from the buffer.
 */
size_t gv_deserialize_fn(void const* buf, kos_fn_t* fn);
