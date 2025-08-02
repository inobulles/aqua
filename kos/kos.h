// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2024-2025 Aymeric Wibo

#pragma once

// Basic types.

#include <stdbool.h>
#include <stdint.h>

_Static_assert(sizeof(void*) <= sizeof(uint64_t), "pointers on your platform are too big");
_Static_assert(sizeof(char) == sizeof(uint8_t), "char is not 8 bits");
_Static_assert(sizeof(float) == 4, "float is not 32 bits");
_Static_assert(sizeof(double) == 8, "double is not 32 bits");

/**
 * A cookie that can be used to identify a request.
 *
 * TODO Detailed explanation with an example to illustrate what we mean.
 */
typedef uint64_t kos_cookie_t;

/**
 * An interrupt number.
 */
typedef uint32_t kos_ino_t;

/**
 * The KOS API version.
 */
typedef enum : uint64_t {
	/**
	 * No API version - this is never used.
	 */
	KOS_API_VERS_NONE = 0,
	/**
	 * Version 4 of the KOS API - this is the minimum all apps linking against the KOS must use.
	 */
	KOS_API_V4 = 4,
} kos_api_vers_t;

/**
 * The KOS description.
 */
typedef struct {
	/**
	 * The current KOS API version.
	 *
	 * Should be set to `KOS_API_V4`.
	 */
	kos_api_vers_t api_vers;
	/**
	 * The best API version theoretically supported by the KOS.
	 *
	 * Can be higher than `api_vers`.
	 */
	kos_api_vers_t best_api_vers;
	/**
	 * The name of the KOS.
	 *
	 * This would be something like "Unix-like system's KOS" or "ZED CPU FPGA KOS", for example.
	 */
	uint8_t name[64];
} kos_descr_v4_t;

/**
 * A KOS type.
 *
 * This is used to type values which may have been deserialized or may be intended for serialization.
 * All values passed to or returned from the KOS are typed.
 */
typedef enum : uint8_t {
	/**
	 * The void type.
	 */
	KOS_TYPE_VOID,
	/**
	 * The boolean type.
	 */
	KOS_TYPE_BOOL,
	/**
	 * The unsigned 8-bit integer type.
	 */
	KOS_TYPE_U8,
	/**
	 * The unsigned 16-bit integer type.
	 */
	KOS_TYPE_U16,
	/**
	 * The unsigned 32-bit integer type.
	 */
	KOS_TYPE_U32,
	/**
	 * The unsigned 64-bit integer type.
	 */
	KOS_TYPE_U64,
	/**
	 * The signed 8-bit integer type.
	 */
	KOS_TYPE_I8,
	/**
	 * The signed 16-bit integer type.
	 */
	KOS_TYPE_I16,
	/**
	 * The signed 32-bit integer type.
	 */
	KOS_TYPE_I32,
	/**
	 * The signed 64-bit integer type.
	 */
	KOS_TYPE_I64,
	/**
	 * The 32-bit IEEE 754 floating-point type.
	 */
	KOS_TYPE_F32,
	/**
	 * The 64-bit IEEE 754 floating-point type.
	 */
	KOS_TYPE_F64,
	/**
	 * The buffer type.
	 *
	 * A buffer is meant to be serialized and deserialized as a whole.
	 * It is always passed by value (i.e. copied) and never by reference.
	 */
	KOS_TYPE_BUF,
	/**
	 * The opaque pointer type.
	 *
	 * An opaque pointer is always passed by reference and may not be vitrified.
	 * It is essentially used as a handle to an object in some other VDEV's memory.
	 */
	KOS_TYPE_OPAQUE_PTR,
	/**
	 * The pointer type.
	 *
	 * A pointer is always passed by reference but may be vitrified.
	 */
	KOS_TYPE_PTR,
} kos_type_t;

/**
 * Mappings from the `kos_type_t` enum to strings.
 *
 * These are used as a helper for logging and debugging purposes.
 */
static char const* const kos_type_str[] = {
	"void",
	"bool",
	"u8",
	"u16",
	"u32",
	"u64",
	"i8",
	"i16",
	"i32",
	"i64",
	"f32",
	"f64",
	"str",
	"buf",
	"opaque_ptr",
	"ptr",
};

/**
 * A KOS value.
 *
 * This is not a serialized value.
 * If, for instance, a buffer (`buf`) is passed to the KOS, it will handle serializing the `size` bytes at `ptr`.
 * Conversely, if a buffer is returned from the KOS, the KOS will handle deserializing it and the client will receive a `buf` with the `ptr` field pointing to the contents of this buffer.
 *
 * TODO Define what happens with this memory. Is it to be freed? Can it point to internal KOS memory?
 */
typedef union {
	/**
	 * The boolean value.
	 */
	bool b;
	/**
	 * The unsigned 8-bit integer value.
	 */
	uint8_t u8;
	/**
	 * The unsigned 16-bit integer value.
	 */
	uint16_t u16;
	/**
	 * The unsigned 32-bit integer value.
	 */
	uint32_t u32;
	/**
	 * The unsigned 64-bit integer value.
	 */
	uint64_t u64;
	/**
	 * The signed 8-bit integer value.
	 */
	int8_t i8;
	/**
	 * The signed 16-bit integer value.
	 */
	int16_t i16;
	/**
	 * The signed 32-bit integer value.
	 */
	int32_t i32;
	/**
	 * The signed 64-bit integer value.
	 */
	int64_t i64;
	/**
	 * The 32-bit IEEE 754 floating-point value.
	 */
	float f32;
	/**
	 * The 64-bit IEEE 754 floating-point value.
	 */
	double f64;

	/**
	 * The buffer value.
	 */
	struct {
		/**
		 * The size of the buffer.
		 */
		uint32_t size;
		/**
		 * The pointer to the buffer.
		 *
		 * TODO Talk about the memory management requirements of this pointer once we've figured that out.
		 */
		void* ptr;
	} buf;

	/**
	 * The opaque pointer value.
	 *
	 * The client may not vitrify this pointer to access the memory it is pointing to.
	 */
	struct {
		/**
		 * The ID of the host on which this opaque pointer is.
		 */
		uint64_t host_id;
		/**
		 * The host-defined pointer.
		 */
		uint64_t ptr;
	} opaque_ptr;

	/**
	 * The pointer value.
	 *
	 * The client must vitrify this pointer to access the memory it is pointing to.
	 */
	struct {
		/**
		 * The ID of the host on which this pointer is.
		 */
		uint64_t host_id;
		/**
		 * The host-defined pointer.
		 */
		uint64_t ptr;
	} ptr;
} kos_val_t;

/**
 * A KOS parameter.
 *
 * This is a type-name pair that describes a parameter to a function.
 * Do note the nomenclature here: a parameter is considered to be the schema of a value, whereas the argument would be the value itself.
 */
typedef struct {
	/**
	 * The type of the parameter.
	 */
	kos_type_t type;
	/**
	 * The name of the parameter.
	 */
	uint8_t name[64];
} kos_param_t;

/**
 * A KOS function.
 *
 * This associates the name of a function with an expected schema for its arguments and return value.
 */
typedef struct {
	/**
	 * The name of the function.
	 */
	uint8_t name[64];
	/**
	 * The type of the return value.
	 *
	 * This is the expected schema for the return value of the function.
	 * E.g., if the function wasn't expected to return anything, this would be `KOS_TYPE_VOID`.
	 */
	kos_type_t ret_type;
	/**
	 * The number of parameters the function expects.
	 */
	uint32_t param_count;
	/**
	 * The parameters the function expects.
	 */
	kos_param_t const* params;
} kos_fn_t;

/**
 * A KOS constant.
 */
typedef struct {
	/**
	 * The type of the constant.
	 */
	kos_type_t type;
	/**
	 * The name of the constant.
	 */
	uint8_t name[64];
	/**
	 * The value of the constant.
	 */
	kos_val_t val;
} kos_const_t;

/**
 * The kind of a VDEV.
 *
 * I.e. where the VDEV is located and through which channel communication will happen.
 */
typedef enum : uint8_t {
	/**
	 * The VDEV is only available through the GrapeVine.
	 *
	 * This means that the VDEV is on another device and communication will happen through the GrapeVine network.
	 * All memory will be explicitly copied when it needs to be vitrified.
	 * This is the slowest kind of VDEV.
	 */
	KOS_VDEV_KIND_GV,
	/**
	 * The VDEV is local to the system with communication happening over a UDS (UNIX domain socket).
	 *
	 * All memory will be shared between the client and the KOS, so no explicit copies are required when vitrifying large amounts memory (though smaller amounts may still be vitrified for performance reasons).
	 * These VDEVs usually communicate over an actual UDS, but may use an implementation-defined mechanism.
	 */
	KOS_VDEV_KIND_UDS,
	/**
	 * The VDEV is local to the system with communication happening directly.
	 *
	 * These VDEVs are loaded into the program's memory space and communicate directly with the client, so they have the lowest overhead of any kind of VDEV.
	 */
	KOS_VDEV_KIND_LOCAL,
} kos_vdev_kind_t;

/**
 * A VDEV descriptor.
 *
 * TODO Write this up.
 */
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

/**
 * The kind of a notification.
 */
typedef enum {
	KOS_NOTIF_ATTACH, // TODO Rename to `KOS_NOTIF_ATTACH_VDEV`?
	KOS_NOTIF_DETACH,
	KOS_NOTIF_CONN_FAIL,
	KOS_NOTIF_CONN,
	KOS_NOTIF_CALL_FAIL,
	KOS_NOTIF_CALL_RET,
	KOS_NOTIF_INTERRUPT,
} kos_notif_kind_t;

typedef struct {
	/**
	 * The kind of notification.
	 */
	kos_notif_kind_t kind;

	/**
	 * Cookie used to identify the request that prompted this notification.
	 */
	kos_cookie_t cookie;

	/**
	 * The connection ID of the notification, if applicable.
	 *
	 * If not applicable (i.e. on VDEV attach, VDEV detach, connection fail, and connection success notifications), this will be 0.
	 */
	uint64_t conn_id;

	union {
		/**
		 * Contents of the notification struct when a VDEV is attached, i.e. is discovered by the KOS.
		 *
		 * This can be induced with a call to {@link kos_req_vdev}.
		 */
		struct {
			/**
			 * The descriptor of the attached VDEV.
			 */
			kos_vdev_descr_t vdev;
		} attach; // TODO Rename to `attach_vdev`?

		/**
		 * Contents of the notification struct when a VDEV is detached, i.e. is not anymore accessible by the KOS.
		 */
		struct {
			/**
			 * The host ID of the detached VDEV.
			 */
			uint64_t host_id;
			/**
			 * The VDEV ID of the detached VDEV.
			 */
			uint64_t vdev_id;
		} detach;

		/**
		 * Contents of the notification struct when a connection to a VDEV fails.
		 *
		 * A connection can be made to a VDEV with a call to {@link kos_vdev_conn}.
		 */
		struct {
			// TODO
		} conn_fail;

		/**
		 * Contents of the notification struct when a connection to a VDEV succeeds.
		 *
		 * A connection can be made to a VDEV with a call to {@link kos_vdev_conn}.
		 */
		struct {
			/**
			 * The number of constants the VDEV supports.
			 */
			uint32_t const_count;

			/**
			 * The constants the VDEV supports.
			 */
			kos_const_t const* consts;

			/**
			 * The number of functions the VDEV supports.
			 */
			uint32_t fn_count;

			/**
			 * The functions the VDEV supports.
			 */
			kos_fn_t const* fns;
		} conn;

		/**
		 * Contents of the notification struct when a call to a VDEV fails.
		 *
		 * A call can be made to a VDEV with a call to {@link kos_vdev_call}.
		 */
		struct {
			// TODO
		} call_fail;

		/**
		 * Contents of the notification struct when a call to a VDEV succeeds.
		 *
		 * A call can be made to a VDEV with a call to {@link kos_vdev_call}.
		 */
		struct {
			/**
			 * The return value of the call.
			 */
			kos_val_t ret;
		} call_ret;

		/**
		 * Contents of the notification struct when an interrupt is received.
		 *
		 * An interrupt is generated when an interrupt number ({@link kos_ino_t}) is registered by the application.
		 */
		struct {
			/**
			 * The interrupt number this interrupt corresponds to.
			 */
			kos_ino_t ino;
			/**
			 * The size of the data associated with this interrupt.
			 */
			uint32_t data_size;
			/**
			 * The data associated with this interrupt.
			 *
			 * The format of this data is defined by the VDEV.
			 * TODO Could we not have some kind of object type in the KOS for this?
			 */
			void const* data;
		} interrupt;
	};
} kos_notif_t;

typedef void (*kos_notif_cb_t)(kos_notif_t const* notif, void* data);

/**
 * Initialize the KOS.
 *
 * This function should be called before any other KOS function.
 * If it fails, all other KOS operations are undefined and the client should exit immediately.
 *
 * @param min The minimum API version the client supports. If you're including this file, this should be `KOS_API_V4`.
 * @param max The maximum API version the client supports. If you're including this file, this should be `KOS_API_V4`.
 * @param descr A pointer to a KOS descriptor struct (if this function returns API version 4, that would be `kos_descr_v4_t`), which will be filled with the KOS information.
 * @return The best API version the KOS supports within the given version range, or `KOS_API_VERS_NONE` if the KOS doesn't support any of the versions within that range.
 */
kos_api_vers_t kos_hello(kos_api_vers_t min, kos_api_vers_t max, kos_descr_v4_t* descr);

// Subscribe to notifications about the creation and destruction of VDEVs by registering a callback.
// Note that `kos_req_vdev` needs to be called after this one for the client to let the KOS know which VDEV specs it requires.
void kos_sub_to_notif(kos_notif_cb_t cb, void* data);
void kos_flush(bool sync);

// Request a VDEV's following the given spec to be loaded.
// This function is guaranteed to immediately call the callback for all `VDEV_KIND_LOCAL` and `VDEV_KIND_UDS` VDEVs, so the client can exit if it doesn't immediately find the VDEV it needs.

void kos_req_vdev(char const* spec);

// Connect to or disconnect from a VDEV.

kos_cookie_t kos_vdev_conn(uint64_t host_id, uint64_t vdev_id);
void kos_vdev_disconn(uint64_t conn_id);

// Call a function on a VDEV.

kos_cookie_t kos_vdev_call(uint64_t conn_id, uint32_t fn_id, void const* args);

// Get a new interrupt number.

kos_ino_t kos_gen_ino(void);
