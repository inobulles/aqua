// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2025 Aymeric Wibo

#include "vdriver_loader.h"
#include "vdev.h"

#include <umber.h>

#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>

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

static int load_from_path(char const* path, kos_notif_cb_t notif_cb, void* notif_data) {
	LOG_V(cls, "Loading VDRIVER from path: %s", path);
	void* const lib = dlopen(path, RTLD_LAZY);

	if (lib == NULL) {
		LOG_E(cls, "Failed to load VDRIVER: dlopen(\"%s\"): %s", path, dlerror());
		return -1;
	}

	dlerror(); // Clear last error message.

	LOG_V(cls, "Loading VDRIVER symbol.", path);
	vdriver_t* const vdriver = dlsym(lib, "VDRIVER");

	if (vdriver == NULL) {
		LOG_E(cls, "Failed to load VDRIVER 'VDRIVER' symbol: %s", dlerror());
		dlclose(lib);
		return -1;
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

	return 0;
}

int vdriver_loader_req_vdev(char const* spec, kos_notif_cb_t notif_cb, void* notif_data) {
	(void) spec;
	(void) notif_cb;
	(void) notif_data;
	(void) load_from_path;

	return 0;
}

int vdriver_loader_vdev_inventory(kos_notif_cb_t notif_cb, void* notif_data) {
	(void) notif_cb;
	(void) notif_data;
	(void) load_from_path;

	return 0;
}
