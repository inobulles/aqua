// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2024-2025 Aymeric Wibo

#include "action.h"
#include "conn.h"
#include "gv.h"

#include "lib/kos.h"
#include "lib/vdev.h"
#include "lib/vdriver_loader.h"

#include <umber.h>

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <dlfcn.h>

static umber_class_t const* init_cls = NULL;
static umber_class_t const* notif_cls = NULL;
static umber_class_t const* action_cls = NULL;
static umber_class_t const* conn_cls = NULL;
static umber_class_t const* call_cls = NULL;

static bool has_init = false;
static uint64_t local_host_id;
static kos_notif_cb_t client_notif_cb = NULL;
static void* client_notif_data = NULL;

static kos_cookie_t cookies = 0;
static kos_ino_t inos = 0;

void __attribute__((constructor)) kos_init(void) {
	has_init = true;

	init_cls = umber_class_new("aqua.kos.init", UMBER_LVL_VERBOSE, "KOS initialization.");
	notif_cls = umber_class_new("aqua.kos.notif", UMBER_LVL_WARN, "KOS notifications.");
	action_cls = umber_class_new("aqua.kos.action", UMBER_LVL_WARN, "KOS action queue.");
	conn_cls = umber_class_new("aqua.kos.conn", UMBER_LVL_WARN, "KOS connections.");
	call_cls = umber_class_new("aqua.kos.call", UMBER_LVL_WARN, "KOS calls.");

	vdriver_loader_init();

	LOG_V(init_cls, "KOS initialized.");
}

kos_api_vers_t kos_hello(kos_api_vers_t min, kos_api_vers_t max, kos_descr_v4_t* descr) {
	assert(min <= max);
	assert(has_init);

	LOG_V(init_cls, "KOS hello.");

	if (min > KOS_API_V4 || max < KOS_API_V4) {
		LOG_E(init_cls, "KOS API version mismatch: requested [%" PRIu64 ", %" PRIu64 "], but only KOS_API_V4 is supported.", min, max);
		return KOS_API_VERS_NONE;
	}

	if (get_gv_host_id(&local_host_id) < 0) {
		LOG_I(init_cls, "KOS running locally.");
		local_host_id = 0;
	}

	descr->api_vers = KOS_API_V4;
	descr->best_api_vers = KOS_API_V4;
	strcpy((char*) descr->name, "Generic Unix-like system's KOS");

	LOG_I(init_cls, "KOS hello successful: API version %" PRIu64 ", best API version %" PRIu64 ", name \"%s\".", descr->api_vers, descr->best_api_vers, descr->name);

	return descr->api_vers;
}

static void notif_cb(kos_notif_t const* notif, void* data) {
	if (notif->kind >= KOS_NOTIF_KIND_COUNT) {
		LOG_E(notif_cls, "Received notification of unknown kind %d.", notif->kind);
		return;
	}

	LOG_V(notif_cls, "Received notification of kind %s.", kos_notif_kind_str[notif->kind]);

	switch (notif->kind) {
	case KOS_NOTIF_CONN:
		// TODO Describe what this is doing exactly, cuz even I'm not sure anymore.

		LOG_V(notif_cls, "Received connection notification for connection ID %" PRIu64 ".", notif->conn_id);

		assert(notif->conn_id < conn_count);
		conn_t* const conn = &conns[notif->conn_id];

		conn->alive = true;
		conn->fn_count = notif->conn.fn_count;
		conn->fns = notif->conn.fns;

		break;
	default:
		break;
	}

	LOG_V(notif_cls, "Forwarding notification to client.");
	client_notif_cb(notif, data);
}

void kos_sub_to_notif(kos_notif_cb_t cb, void* data) {
	assert(has_init);

	LOG_V(init_cls, "Subscribing to KOS notifications (cb=%p, data=%p).", cb, data);

	client_notif_cb = cb;
	client_notif_data = data;
}

void kos_req_vdev(char const* spec) {
	assert(has_init);

	// TODO Not sure I like how init_cls is used here.

	LOG_V(init_cls, "Trying to find local VDEV for spec \"%s\".", spec);
	vdriver_loader_req_local_vdev(spec, notif_cb, client_notif_data);

	LOG_V(init_cls, "Trying to find VDEV on the GrapeVine for spec '%s'.", spec);

	kos_vdev_descr_t* gv_vdevs;
	ssize_t const gv_vdev_count = query_gv_vdevs(&gv_vdevs);

	if (gv_vdev_count < 0) {
		return;
	}

	for (size_t i = 0; i < (size_t) gv_vdev_count; i++) {
		kos_vdev_descr_t* const gv_vdev = &gv_vdevs[i];

		if (strcmp((char*) gv_vdev->spec, spec) != 0) {
			continue;
		}

		LOG_V(init_cls, "Found GrapeVine VDEV for '%s' spec ('%s').", spec, gv_vdev->human);

		kos_notif_t notif = {
			.kind = KOS_NOTIF_ATTACH,
			.attach.vdev = *gv_vdev,
		};

		notif_cb(&notif, client_notif_data);
	}

	free(gv_vdevs);

	LOG_V(init_cls, "Done looking for VDEVs.");
}

static void conn_local(kos_cookie_t cookie, action_t* action, bool sync) {
	(void) sync; // This is always done synchronously.

	LOG_V(conn_cls, "Try to connect to VDEV locally (cookie=0x%" PRIx64 ").", cookie);

	vdriver_t* const vdriver = vdriver_loader_find_loaded_by_vid(action->conn.vdev_id);

	if (vdriver == NULL) {
		LOG_E(conn_cls, "Could not find a VDRIVER associated with VDEV ID %" PRIu64, action->conn.vdev_id);

		kos_notif_t notif = {
			.kind = KOS_NOTIF_CONN_FAIL,
			.cookie = cookie,
		};

		client_notif_cb(&notif, client_notif_data);
		return;
	}

	LOG_V(conn_cls, "Found VDRIVER, connecting.");
	vdriver->conn(cookie, action->conn.vdev_id, conn_new(vdriver));
}

static void conn_gv(kos_cookie_t cookie, action_t* action, bool sync) {
	(void) action; // TODO
	(void) sync;   // TODO

	// gv_vdev_conn_t conn;

	// if (gv_vdev_conn(&conn, action->conn.host_id, action->conn.vdev_id) < 0) {
	// 	goto fail;
	// }

	// TODO Here we're going to wanna wait for the connection response if sync.
	// If not, we should return straight away but I do need a way to tell libgv to call the callback when the connection is established (or when it receives other events for a VDEV).
	// Since this is done in a VDEV connection thread, we're going to need some mutex for the callback, which can probably be created here in the KOS and passe on to libgv for each VDEV connection we make.

	LOG_E(conn_cls, "Connecting to GrapeVine VDEVs is not yet implemented (cookie=0x%" PRIx64 ").", cookie);

	return;

	// fail:;

	kos_notif_t notif = {
		.kind = KOS_NOTIF_CONN_FAIL,
		.cookie = cookie,
	};

	client_notif_cb(&notif, client_notif_data);
}

kos_cookie_t kos_vdev_conn(uint64_t host_id, uint64_t vdev_id) {
	// Generate cookie and add action to queue.

	kos_cookie_t const cookie = cookies++;

	action_t const action = {
		.cookie = cookie,
		.cb = host_id == local_host_id ? conn_local : conn_gv,
		.conn = {
			.host_id = host_id,
			.vdev_id = vdev_id,
		},
	};

	LOG_V(conn_cls, "Adding to action queue to request connection to %" PRIx64 ":%" PRIu64 " (cookie=0x%" PRIx64 ").", host_id, vdev_id, cookie);

	PUSH_QUEUE(action);

	if (action_queue_tail - action_queue_head > ACTION_QUEUE_SIZE) {
		LOG_E(conn_cls, "Too many actions in the KOS' action queue, dropping the most recent one.");
		action_queue_tail--;
	}

	return cookie;
}

static void call(kos_cookie_t cookie, action_t* action, bool sync) {
	(void) sync; // This is always done synchronously.

	LOG_V(call_cls, "Passing call on to VDRIVER (cookie=0x%" PRIx64 ").", cookie);

	vdriver_t const* const vdriver = action->call.vdriver;
	vdriver->call(cookie, action->call.conn_id, action->call.fn_id, action->call.args);
}

static void call_fail(kos_cookie_t cookie, action_t* action, bool sync) {
	(void) sync; // This is always done synchronously.
	(void) action;

	LOG_E(call_cls, "Call failed for cookie 0x%" PRIx64 ".", cookie);

	kos_notif_t const notif = {
		.kind = KOS_NOTIF_CALL_FAIL,
		.cookie = cookie,
	};

	client_notif_cb(&notif, client_notif_data);
}

kos_cookie_t kos_vdev_call(uint64_t conn_id, uint32_t fn_id, void const* args) {
	// Generate cookie.

	kos_cookie_t const cookie = cookies++;

	action_t action = {
		.cookie = cookie,
		.cb = call_fail,
	};

	LOG_V(call_cls, "Adding to action queue to call function %u on connection %" PRIu64 " (cookie=0x%" PRIx64 ").", fn_id, conn_id, cookie);

	// Find connection.

	if (conn_id >= conn_count) {
		LOG_E(call_cls, "Connection ID %" PRIu64 " invalid.", conn_id);
		goto fail;
	}

	conn_t* const conn = &conns[conn_id];

	if (!conn->alive) {
		LOG_E(call_cls, "Connection ID %" PRIu64 " is not alive.", conn_id);
		goto fail;
	}

	// Get VDRIVER and validate function ID.

	vdriver_t* const vdriver = conn->vdriver;
	assert(vdriver != NULL);

	if (fn_id >= conn->fn_count) {
		LOG_E(call_cls, "Function ID %u is invalid.", fn_id);
		goto fail;
	}

	// Success!

	action.cb = call;

	action.call.vdriver = vdriver;
	action.call.conn_id = conn_id;
	action.call.fn_id = fn_id;
	action.call.args = args;

fail:;

	// Actually add action to queue.

	PUSH_QUEUE(action);

	if (action_queue_tail - action_queue_head > ACTION_QUEUE_SIZE) {
		LOG_E(call_cls, "Too many actions in the KOS' action queue, dropping the most recent one.");
		action_queue_tail--;
	}

	return cookie;
}

void kos_flush(bool sync) {
	LOG_V(action_cls, "Flushing KOS action queue (sync=%d).", sync);

	while (action_queue_head != action_queue_tail) {
		action_t action;
		POP_QUEUE(action);
		action.cb(action.cookie, &action, sync);
	}
}

void kos_vdev_disconn(uint64_t conn_id) {
	LOG_V(conn_cls, "Disconnecting from VDEV with connection ID %" PRIu64 ".", conn_id);

	if (conn_id >= conn_count) {
		LOG_E(conn_cls, "Connection ID %" PRIu64 " invalid.", conn_id);
		return;
	}

	conns[conn_id].alive = false;
}

kos_ino_t kos_gen_ino(void) {
	return inos++;
}
