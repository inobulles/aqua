// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2024-2025 Aymeric Wibo

#include "gv.h"

#include <aqua/vdev.h>

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <dlfcn.h>

// TODO A lot of this code is common with KOS; should we bring this out into a libvdev library or something like that.

#define VDRIVER_PATH_ENVVAR "VDRIVER_PATH"
#define DEFAULT_VDRIVER_PATH "/usr/local/share/aqua/vdriver/"
#define EXT ".vdriver"

static void strfree(char** str) {
	if (str != NULL) {
		free(*str);
	}
}

typedef struct {
	void* lib; // TODO Should I just define this on vdriver_t? As like a reserved opaque pointer for the KOS?
	vdriver_t* vdriver;
} kos_vdriver_t;

static uint64_t cur_vid_slice = 0;

static size_t vdriver_count = 0;
static kos_vdriver_t* vdrivers = NULL;

static void notif_cb(kos_notif_t const* notif, void* data) {
	state_t* const state = data;

	switch (notif->kind) {
	case KOS_NOTIF_ATTACH:
		state->vdevs = realloc(state->vdevs, ++state->vdev_count * sizeof *state->vdevs);
		assert(state->vdevs != NULL);
		state->vdevs[state->vdev_count - 1] = notif->attach.vdev;

		printf("Attached VDEV: %s\n", notif->attach.vdev.human);

		break;
	default:
		break;
	}
}

static int load_vdriver_from_path(kos_vdriver_t* kos_vdriver, state_t* state, char const* path) {
	void* const lib = dlopen(path, RTLD_LAZY);

	if (lib == NULL) {
		fprintf(stderr, "Failed to load VDEV driver library: dlopen(\"%s\"): %s\n", path, dlerror());
		return -1;
	}

	// Load the VDRIVER symbol itself.

	dlerror(); // Clear last error message.
	vdriver_t* const vdriver = dlsym(lib, "VDRIVER");

	if (vdriver == NULL) {
		fprintf(stderr, "Failed to load VDEV driver 'VDRIVER' symbol: %s\n", dlerror());
		dlclose(lib);
		return -1;
	}

	kos_vdriver->lib = lib;
	kos_vdriver->vdriver = vdriver;

	// Allocate a VID slice for the vdriver.

	assert(cur_vid_slice < UINT32_MAX);

	vdriver->vdev_id_lo = cur_vid_slice << 32;
	vdriver->vdev_id_hi = ((cur_vid_slice + 1) << 32) - 1;

	cur_vid_slice++;

	// Set other miscellaneous values on VDRIVER.

	vdriver->notif_cb = notif_cb;
	vdriver->notif_data = state;

	// Finally, call init on the vdriver.
	// TODO Maybe these should return errors idk.

	if (vdriver->init != NULL) {
		vdriver->init();
	}

	return 0;
}

static int vdev_inventory_path(state_t* state, char const* search) {
	DIR* const dir = opendir(search);

	if (dir == NULL) {
		fprintf(stderr, "opendir(\"%s\"): %s\n", search, strerror(errno));
		return -1;
	}

	struct dirent* ent;

	while ((ent = readdir(dir)) != NULL) {
		if (ent->d_type != DT_REG) {
			continue;
		}

		if (strstr(ent->d_name, EXT) != ent->d_name + strlen(ent->d_name) - strlen(EXT)) {
			continue;
		}

		char* __attribute__((cleanup(strfree))) path = NULL;
		asprintf(&path, "%s/%s", search, ent->d_name);
		assert(path != NULL);

		kos_vdriver_t vdriver;

		if (load_vdriver_from_path(&vdriver, state, path) != 0) {
			continue;
		}

		vdrivers = realloc(vdrivers, (vdriver_count + 1) * sizeof *vdrivers);
		assert(vdrivers != NULL);
		vdrivers[vdriver_count++] = vdriver;

		// Probe for VDEVs on that driver.

		vdriver.vdriver->probe();
	}

	closedir(dir);
	return 0;
}

int vdev_inventory(state_t* state) {
	char* vdriver_path = getenv(VDRIVER_PATH_ENVVAR);

	if (vdriver_path == NULL) {
		vdriver_path = DEFAULT_VDRIVER_PATH;
	}

	char* const path_copy_orig = strdup(vdriver_path);
	char* path_copy = path_copy_orig;
	char* tok;

	while ((tok = strsep(&path_copy, ":")) != NULL) {
		vdev_inventory_path(state, tok);
	}

	free(path_copy_orig);

	return 0;
}
