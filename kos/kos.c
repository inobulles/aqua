// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2024 Aymeric Wibo

#include "lib/kos.h"
#include "action.h"
#include "conn.h"
#include "gv.h"
#include "lib/vdev.h"

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <dlfcn.h>

#include <umber.h>
#define UMBER_COMPONENT "aqua.kos"

#define VDRIVER_PATH_ENVVAR "VDRIVER_PATH"
#define DEFAULT_PREFIX "/usr/local/lib"
#define DEFAULT_VDRIVER_PATH "vdriver"

static bool has_init = false;
static uint64_t local_host_id;
static kos_notif_cb_t client_notif_cb = NULL;
static void* client_notif_data = NULL;

static kos_cookie_t cookies = 0;
static kos_ino_t inos = 0;

void __attribute__((constructor)) kos_init(void) {
	has_init = true;
}

kos_api_vers_t kos_hello(kos_api_vers_t min, kos_api_vers_t max, kos_descr_v4_t* descr) {
	assert(min <= max);
	assert(has_init);

	if (min > KOS_API_V4 || max < KOS_API_V4) {
		return KOS_API_VERS_NONE;
	}

	descr->api_vers = KOS_API_V4;
	descr->best_api_vers = KOS_API_V4;
	strcpy((char*) descr->name, "Generic Unix-like system's KOS");

	local_host_id = 0; // TODO This should be the actual ID that we're gonna advertise on the GV.

	return descr->api_vers;
}

typedef struct {
	void* lib; // TODO Should I just define this on vdriver_t? As like a reserved opaque pointer for the KOS?
	vdriver_t* vdriver;
} kos_vdriver_t;

static uint64_t cur_vid_slice = 0;

static size_t vdriver_count = 0;
static kos_vdriver_t* vdrivers = NULL;

static void notif_cb(kos_notif_t const* notif, void* data) {
	switch (notif->kind) {
	case KOS_NOTIF_CONN:
		assert(notif->conn_id < conn_count);
		conn_t* const conn = &conns[notif->conn_id];

		conn->alive = true;
		conn->fn_count = notif->conn.fn_count;
		conn->fns = notif->conn.fns;

		break;
	default:
		break;
	}

	// Forward the notification to the client.

	client_notif_cb(notif, data);
}

static int load_vdriver_from_path(kos_vdriver_t* kos_vdriver, char const* path) {
	void* const lib = dlopen(path, RTLD_LAZY);

	if (lib == NULL) {
		LOG_ERROR("Failed to load VDEV driver library: dlopen(\"%s\"): %s", path, dlerror());
		return -1;
	}

	// Load the VDEV symbol itself.

	dlerror(); // Clear last error message.
	vdriver_t* const vdriver = dlsym(lib, "VDRIVER");

	if (vdriver == NULL) {
		LOG_ERROR("Failed to load VDRIVER 'VDRIVER' symbol: %s\n", dlerror());
		dlclose(lib);
		return -1;
	}

	LOG_VERBOSE("VDRIVER name is '%s'.", vdriver->human);

	kos_vdriver->lib = lib;
	kos_vdriver->vdriver = vdriver;

	// Allocate a VID slice for the vdriver.

	assert(cur_vid_slice < UINT32_MAX);

	vdriver->vdev_id_lo = cur_vid_slice << 32;
	vdriver->vdev_id_hi = ((cur_vid_slice + 1) << 32) - 1;

	cur_vid_slice++;

	// Set other miscellaneous values on VDRIVER.

	vdriver->notif_cb = notif_cb;
	vdriver->notif_data = client_notif_data;

	// Finally, call init on the vdriver.
	// TODO Maybe these should return errors idk.

	if (vdriver->init != NULL) {
		LOG_VERBOSE("Calling init function on VDRIVER.");
		vdriver->init();
	}

	LOG_VERBOSE("VDRIVER loaded and linked from '%s'.", path);

	return 0;
}

void kos_sub_to_notif(kos_notif_cb_t cb, void* data) {
	assert(has_init);

	client_notif_cb = cb;
	client_notif_data = data;
}

static void strfree(char** str) {
	if (str != NULL) {
		free(*str);
	}
}

void kos_req_vdev(char const* spec) {
	assert(has_init);

	LOG_VERBOSE("Trying to find local VDEV for spec \"%s\".", spec);

	char* __attribute__((cleanup(strfree))) vdriver_path_tmp = NULL;
	char* vdriver_path = getenv(VDRIVER_PATH_ENVVAR);

	if (vdriver_path == NULL) {
		char* prefix = getenv(
#if defined(__APPLE__)
			"DY" // dyld(1) doesn't recognize `LD_LIBRARY_PATH`.
#endif
			"LD_LIBRARY_PATH"
		);

		if (prefix == NULL) {
			vdriver_path = DEFAULT_PREFIX "/" DEFAULT_VDRIVER_PATH;
		}

		else {
			asprintf(&vdriver_path_tmp, "%s/%s:%s/%s", prefix, DEFAULT_VDRIVER_PATH, DEFAULT_PREFIX, DEFAULT_VDRIVER_PATH);
			assert(vdriver_path_tmp != NULL);
			vdriver_path = vdriver_path_tmp;
		}
	}

	LOG_VERBOSE("VDRIVER path is '%s'.", vdriver_path);

	char* const path_copy_orig = strdup(vdriver_path);
	char* path_copy = path_copy_orig;
	char* tok;

	while ((tok = strsep(&path_copy, ":")) != NULL) {
		char* __attribute__((cleanup(strfree))) candidate = NULL;
		asprintf(&candidate, "%s/%s.vdriver", tok, spec);
		assert(candidate != NULL);

		if (access(candidate, F_OK) != 0) {
			continue;
		}

		// Driver file exists, we should be able to load it.

		kos_vdriver_t vdriver;

		if (load_vdriver_from_path(&vdriver, candidate) < 0) {
			LOG_ERROR("Failed to load VDRIVER from '%s'.", candidate);
			continue;
		}

		vdrivers = realloc(vdrivers, (vdriver_count + 1) * sizeof *vdrivers);
		assert(vdrivers != NULL);
		vdrivers[vdriver_count++] = vdriver;

		// Probe for VDEVs on that driver.

		if (vdriver.vdriver->probe == NULL) {
			LOG_ERROR("VDRIVER '%s' doesn't implement a probe function.", spec);
			continue;
		}

		LOG_VERBOSE("Probing VDRIVER for '%s' VDEVs.", spec);
		vdriver.vdriver->probe();
	}

	free(path_copy_orig);

	LOG_VERBOSE("Trying to find VDEV on the GrapeVine for spec '%s'.", spec);

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

		LOG_VERBOSE("Found GrapeVine VDEV for '%s' spec ('%s').", spec, gv_vdev->human);

		kos_notif_t notif = {
			.kind = KOS_NOTIF_ATTACH,
			.attach.vdev = *gv_vdev,
		};

		client_notif_cb(&notif, client_notif_data);
	}

	free(gv_vdevs);

	LOG_VERBOSE("Done looking for VDEVs.");
}

static void conn_local(kos_cookie_t cookie, action_t* action, bool sync) {
	(void) sync; // This is always done synchronously.

	// Look for the vdriver this 'vdev_id' is associated with.

	for (size_t i = 0; i < vdriver_count; i++) {
		kos_vdriver_t* const kos_vdriver = &vdrivers[i];
		vdriver_t* const vdriver = kos_vdriver->vdriver;

		if (action->conn.vdev_id >= vdriver->vdev_id_lo && action->conn.vdev_id <= vdriver->vdev_id_hi) {
			vdriver->conn(cookie, action->conn.vdev_id, conn_new(vdriver));
			return;
		}
	}

	// If we couldn't find anything, emit a connection failure.

	LOG_ERROR("Could not find a VDRIVER associated with VDEV ID %" PRIu64, action->conn.vdev_id);

	kos_notif_t notif = {
		.kind = KOS_NOTIF_CONN_FAIL,
		.cookie = cookie,
	};

	client_notif_cb(&notif, client_notif_data);
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

	LOG_ERROR("Connecting to GrapeVine VDEVs is not yet implemented.");

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

	PUSH_QUEUE(action);

	if (action_queue_tail - action_queue_head > ACTION_QUEUE_SIZE) {
		LOG_ERROR("Too many actions in the KOS' action queue, dropping the most recent one.");
		action_queue_tail--;
	}

	return cookie;
}

static void call(kos_cookie_t cookie, action_t* action, bool sync) {
	(void) sync; // This is always done synchronously.

	vdriver_t const* const vdriver = action->call.vdriver;
	vdriver->call(cookie, action->call.conn_id, action->call.fn_id, action->call.args);
}

static void call_fail(kos_cookie_t cookie, action_t* action, bool sync) {
	(void) sync; // This is always done synchronously.
	(void) action;

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

	// Find connection.

	if (conn_id >= conn_count) {
		LOG_ERROR("Connection ID %" PRIu64 " invalid.", conn_id);
		goto fail;
	}

	conn_t* const conn = &conns[conn_id];

	if (!conn->alive) {
		LOG_ERROR("Connection ID %" PRIu64 " is not alive.", conn_id);
		goto fail;
	}

	// Get VDRIVER and validate function ID.

	vdriver_t* const vdriver = conn->vdriver;
	assert(vdriver != NULL);

	if (fn_id >= conn->fn_count) {
		LOG_ERROR("Function ID %u is invalid.", fn_id);
		goto fail;
	}

	// Success!

	action.cb = call;

	action.call.vdriver = vdriver;
	action.call.conn_id = conn_id;
	action.call.fn_id = fn_id;
	action.call.args = args;

fail:;

	// Add action to queue.

	PUSH_QUEUE(action);

	if (action_queue_tail - action_queue_head > ACTION_QUEUE_SIZE) {
		LOG_ERROR("Too many actions in the KOS' action queue, dropping the most recent one.");
		action_queue_tail--;
	}

	return cookie;
}

void kos_flush(bool sync) {
	// Clear out the queue.

	while (action_queue_head != action_queue_tail) {
		action_t action;
		POP_QUEUE(action);
		action.cb(action.cookie, &action, sync);
	}
}

void kos_vdev_disconn(uint64_t conn_id) {
	if (conn_id >= conn_count) {
		LOG_ERROR("Connection ID %" PRIu64 " invalid.", conn_id);
		return;
	}

	conns[conn_id].alive = false;
}

kos_ino_t kos_gen_ino(void) {
	return inos++;
}
