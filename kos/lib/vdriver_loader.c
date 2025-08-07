// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2025 Aymeric Wibo

#include "vdriver_loader.h"

#include <umber.h>

#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <dlfcn.h>

static umber_class_t const* cls = NULL;

static uint64_t cur_vid_slice = 0;
static size_t vdriver_count = 0;
static vdriver_t** vdrivers = NULL;

void vdriver_loader_init(void) {
	assert(cls == NULL);
	cls = umber_class_new("aqua.kos.vdriver_loader", UMBER_LVL_INFO, "VDRIVER loader.");

	assert(vdrivers == NULL);
	assert(vdriver_count == 0);
	assert(cur_vid_slice == 0);
}

static vdriver_t* load_from_path(char const* path, kos_notif_cb_t notif_cb, void* notif_data) {
	LOG_V(cls, "Trying to load VDRIVER from path: %s", path);
	void* const lib = dlopen(path, RTLD_LAZY);

	if (lib == NULL) {
		LOG_E(cls, "Failed to load VDRIVER: dlopen(\"%s\"): %s", path, dlerror());
		return NULL;
	}

	dlerror(); // Clear last error message.

	LOG_V(cls, "Loading VDRIVER symbol.", path);
	vdriver_t* const vdriver = dlsym(lib, "VDRIVER");

	if (vdriver == NULL) {
		LOG_E(cls, "Failed to load VDRIVER 'VDRIVER' symbol: %s", dlerror());
		dlclose(lib);
		return NULL;
	}

	LOG_V(cls, "Allocating VID slice for VDRIVER.", path);

	assert(cur_vid_slice < UINT32_MAX);

	vdriver->vdev_id_lo = cur_vid_slice << 32;
	vdriver->vdev_id_hi = ((cur_vid_slice + 1) << 32) - 1;

	cur_vid_slice++;

	// Set other miscellaneous values on VDRIVER.

	vdriver->notif_cb = notif_cb;
	vdriver->notif_data = notif_data;

	LOG_V(cls, "Call init function on VDRIVER, if it exists.", path);

	if (vdriver->init != NULL) {
		vdriver->init();
	}

	LOG_I(cls, "VDRIVER loaded from '%s' (VID slice [0x%" PRIx64 ", 0x%" PRIx64 "]).", path, vdriver->vdev_id_lo, vdriver->vdev_id_hi);

	// Store the VDRIVER in the global array.

	vdrivers = realloc(vdrivers, (vdriver_count + 1) * sizeof *vdrivers);
	assert(vdrivers != NULL);
	vdrivers[vdriver_count++] = vdriver;

	return vdriver;
}

static void strfree(char** str) {
	if (str != NULL) {
		free(*str);
	}
}

void vdriver_loader_req_local_vdev(char const* spec, kos_notif_cb_t notif_cb, void* notif_data) {
	LOG_V(cls, "Trying to find local VDRIVER providing spec \"%s\".", spec);

	char* __attribute__((cleanup(strfree))) vdriver_path_tmp = NULL;
	char* vdriver_path = getenv(VDRIVER_PATH_ENVVAR);

	if (vdriver_path == NULL) {
		char* const prefix = getenv(
#if defined(__APPLE__)
			"DY" // dyld(1) doesn't recognize `LD_LIBRARY_PATH`.
#endif
			"LD_LIBRARY_PATH"
		);

		if (prefix == NULL) {
			vdriver_path = "/usr/local/lib/" DEFAULT_VDRIVER_PATH;
		}

		else {
			asprintf(&vdriver_path_tmp, "%s/%s:/usr/local/lib/%s", prefix, DEFAULT_VDRIVER_PATH, DEFAULT_VDRIVER_PATH);
			assert(vdriver_path_tmp != NULL);
			vdriver_path = vdriver_path_tmp;
		}
	}

	LOG_V(cls, "VDRIVER path is '%s' - going through each path and trying to load VDRIVER.", vdriver_path);

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

		vdriver_t* const vdriver = load_from_path(candidate, notif_cb, notif_data);

		if (vdriver == NULL) {
			continue;
		}

		// Probe for VDEVs on that driver.

		if (vdriver->probe == NULL) {
			LOG_E(cls, "VDRIVER '%s' doesn't implement a probe function.", spec);
			continue;
		}

		LOG_V(cls, "Probing VDRIVER for '%s' VDEVs.", spec);
		vdriver->probe();
	}

	free(path_copy_orig);
}

int vdriver_loader_vdev_local_inventory(kos_notif_cb_t notif_cb, void* notif_data) {
	LOG_V(cls, "Taking inventory of all VDEVs available on the system.");

	// TODO

	(void) notif_cb;
	(void) notif_data;

	return 0;
}

vdriver_t* vdriver_loader_find_loaded_by_vid(vid_t vid) {
	LOG_V(cls, "Trying to find VDRIVER associated with VDEV ID %" PRIu64 ".", vid);

	for (size_t i = 0; i < vdriver_count; i++) {
		vdriver_t* const vdriver = vdrivers[i];

		if (vid >= vdriver->vdev_id_lo && vid <= vdriver->vdev_id_hi) {
			LOG_V(cls, "Found VDRIVER '%s' for VDEV ID %" PRIu64 ".", vdriver->spec, vid);
			return vdriver;
		}
	}

	LOG_V(cls, "Could not find VDRIVER for VDEV ID %" PRIu64 ".", vid);
	return NULL;
}
