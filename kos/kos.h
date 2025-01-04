// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2024 Aymeric Wibo

#pragma once

// Basic types.

#include <stdbool.h>
#include <stdint.h>

_Static_assert(sizeof(void*) <= sizeof(uint64_t), "pointers on your platform are too big");
_Static_assert(sizeof(char) == sizeof(uint8_t), "char is not 8 bits");

typedef uint64_t kos_cookie_t;

// Tell the KOS what range of versions of the API we, the client, support.
// The KOS will respond with the best version it supports that is within the range we specify, and if there is one, will fill in the 'descr' struct.
// If you're including this file, you should necessarily set min and max to `KOS_API_V4`.
// TODO Is this overkill? I mean with the 'kos_hello_req_t' and all?

typedef enum : uint64_t {
	KOS_API_VERS_NONE = 0,
	KOS_API_V4 = 4,
} kos_api_vers_t;

typedef struct {
	kos_api_vers_t api_vers;      // Current KOS API version. Should be set to `KOS_API_V4`.
	kos_api_vers_t best_api_vers; // Best API version theoretically supported by the KOS. Can be higher that `api_vers`.
	uint8_t name[64];             // Something like "Unix-like system's KOS" or "ZED CPU FPGA KOS".
} kos_descr_v4_t;

kos_api_vers_t kos_hello(kos_api_vers_t min, kos_api_vers_t max, kos_descr_v4_t* descr);

// Types.
// TODO Better comments.

typedef enum : uint8_t {
	KOS_TYPE_VOID,
	KOS_TYPE_BOOL,
	KOS_TYPE_U8,
	KOS_TYPE_U16,
	KOS_TYPE_U32,
	KOS_TYPE_U64,
	KOS_TYPE_I8,
	KOS_TYPE_I16,
	KOS_TYPE_I32,
	KOS_TYPE_I64,
	KOS_TYPE_STR,
	KOS_TYPE_BUF,
	KOS_TYPE_OPAQUE_PTR,
	KOS_TYPE_PTR,
} kos_type_t;

typedef struct {
	kos_type_t type;
	uint8_t name[64];
} kos_vdev_fn_arg_t;

typedef struct {
	uint8_t name[64];
	kos_type_t ret_type;

	uint32_t arg_count;
	kos_vdev_fn_arg_t const* args;
} kos_vdev_fn_t;

// Subscribe to notifications about the creation and destruction of VDEVs by registering a callback.
// Note that `kos_req_vdev` needs to be called after this one for the client to let the KOS know which VDEV specs it requires.

typedef enum : uint8_t {
	KOS_VDEV_KIND_GV,    // Only available through the GrapeVine (because on another device e.g.), with explicit memory copies when memory needs to be actualized, i.e. slowest.
	KOS_VDEV_KIND_UDS,   // System-local to the app, communicating through UNIX domain sockets (or some other implementation-defined mechanism) and shared memory segments, i.e. very fast.
	KOS_VDEV_KIND_LOCAL, // Memory-local to the app, i.e. potentially a little faster than `VDEV_KIND_UDS`, with less overhead.
} kos_vdev_kind_t;

#define KOS_LOCAL_HOST_ID 0 // TODO This might change in the future, but for now it simplifies things for us to just have this as a fixed value.

typedef struct __attribute__((packed)) {
	uint64_t host_id; // A unique identifier for the host, could be MAC address of one of its interfaces e.g. All `VDEV_KIND_LOCAL` and `VDEV_KIND_UDS` VDEV's should have the same host ID a priori.
	uint64_t vdev_id; // A unique identifier for the device within the host's namespace. Thus, a VDEV should be uniquely identifiable by just HID:VID.
	uint8_t spec[64]; // Something like "aquabsd.black.wgpu".
	uint32_t vers;    // I don't know? Maybe an app could use this as grounds for a rejection before even discovering what commands the VDEV supports.
	uint8_t human[256];
	uint8_t vdriver_human[256]; // Something like "Default .wgpu device for aquaBSD Black".
	kos_vdev_kind_t kind;       // If the WGPU VDEV was on another machine, `VDEV_KIND_GV`.
	uint32_t pref;              // The KOS' preference level for this device. Could just be 0, or something higher to indicate the KOS would rather the app use this one than another.
} kos_vdev_descr_t;

typedef enum {
	KOS_NOTIF_ATTACH, // TODO Rename to `KOS_NOTIF_ATTACH_VDEV`?
	KOS_NOTIF_DETACH,
	KOS_NOTIF_CONN_FAIL,
	KOS_NOTIF_CONN,
} kos_notif_kind_t;

typedef struct {
	kos_notif_kind_t kind;
	kos_cookie_t cookie;

	union {
		struct {
			kos_vdev_descr_t vdev;
		} attach; // TODO Rename to `attach_vdev`?

		struct {
			uint64_t host_id;
			uint64_t vdev_id;
		} detach;

		struct {
		} conn_fail;

		struct {
			uint64_t conn_id;
			uint32_t fn_count;
			kos_vdev_fn_t const* fns;
		} conn;
	};
} kos_notif_t;

typedef void (*kos_notif_cb_t)(kos_notif_t const* notif, void* data);

void kos_sub_to_notif(kos_notif_cb_t cb, void* data);
void kos_flush(bool sync);

// Request a VDEV's following the given spec to be loaded.
// This function is guaranteed to immediately call the callback for all `VDEV_KIND_LOCAL` and `VDEV_KIND_UDS` VDEVs, so the client can exit if it doesn't immediately find the VDEV it needs.

void kos_req_vdev(char const* spec);

// Connect to or disconnect from a VDEV.

kos_cookie_t kos_vdev_conn(uint64_t host_id, uint64_t vdev_id);
void kos_vdev_disconn(uint64_t conn_id);

// Call a function on a VDEV.

kos_cookie_t kos_vdev_call(uint64_t conn_id, uint32_t fn_id, kos_vdev_fn_arg_t const* args);
