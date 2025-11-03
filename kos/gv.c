// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2024-2025 Aymeric Wibo

#if defined(__linux__)
# define _POSIX_C_SOURCE 200809L // For fileno().
#endif

#include "lib/gv_ipc.h"

#include <aqua/kos.h>

#include <umber.h>

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/file.h>

static umber_class_t const* cls = NULL;

static bool gvd_running = false;
static size_t node_count = 0;
static gv_node_ent_t nodes[256];

static __attribute__((constructor)) void init(void) {
	cls = umber_class_new("aqua.kos.gv", UMBER_LVL_INFO, "KOS GrapeVine interaction.");
}

static void unlock(FILE* f) {
	flock(fileno(f), LOCK_UN);
	fclose(f);
}

static bool is_gvd_running(void) {
	LOG_V(cls, "Checking if GrapeVine daemon is running by checking lock file %s.", gv_get_lock_path());

	FILE* const lock_file = fopen(gv_get_lock_path(), "r");

	if (lock_file == NULL) {
		LOG_V(cls, "Lock file doesn't exist - not running.");
		return false;
	}

	int const rv = flock(fileno(lock_file), LOCK_EX | LOCK_NB);

	if (rv == 0) { // If we got the lock, gvd is not running.
		LOG_V(cls, "Could lock lock file - not running.");
		unlock(lock_file);
		return false;
	}

	if (rv < 0 && errno != EWOULDBLOCK) { // Some other issue occurred.
		LOG_W(cls, "Something went wrong trying to lock lock file - assuming not running. flock: %s", strerror(errno));
		unlock(lock_file);
		return false;
	}

	LOG_V(cls, "Could not lock lock file - GrapeVine daemon is running.");

	unlock(lock_file);
	return true;
}

int get_gv_host_id(uint64_t* host_id_out) {
	// XXX We only update gvd_running here, because this is called in kos_hello when setting the local host ID, and if gvd is not running this defaults to 0.
	// We aren't allowed to query VDEVs if the local host ID was set to 0, even if gvd was started in the meantime.

	LOG_V(cls, "Getting host ID from GrapeVine daemon.");
	gvd_running = is_gvd_running();

	if (!gvd_running) {
		// This is just a verbose log because this could be a normal situation, e.g. when the KOS is run locally.

		LOG_V(cls, "GrapeVine daemon is not running - cannot get host ID.");
		return -1;
	}

	FILE* const f = fopen(gv_get_host_id_path(), "r");

	if (f == NULL) {
		LOG_W(cls, "Couldn't load GrapeVine host ID file %s: %s", gv_get_host_id_path(), strerror(errno));
		return -1;
	}

	uint64_t host_id;

	if (fread(&host_id, sizeof host_id, 1, f) != 1) {
		LOG_E(cls, "Failed to read host ID from GrapeVine host ID file.");
		fclose(f);
		return -1;
	}

	LOG_I(cls, "Our host ID is 0x%" PRIx64 ".", host_id);
	*host_id_out = host_id;

	fclose(f);
	return 0;
}

ssize_t query_gv_vdevs(kos_vdev_descr_t** vdevs_out) {
	LOG_V(cls, "Querying all VDEVs on GrapeVine network.");

	*vdevs_out = NULL;

	if (!gvd_running) {
		// This is just a verbose log because this could be a normal situation, e.g. when the KOS is run locally.

		LOG_V(cls, "GrapeVine daemon is not running - cannot query VDEVs.");
		return 0;
	}

	// Actually read.

	FILE* const f = fopen(gv_get_nodes_path(), "r");

	if (f == NULL) {
		LOG_W(cls, "Couldn't load GrapeVine nodes file %s: %s", gv_get_nodes_path(), strerror(errno));
		return 0;
	}

	ssize_t vdev_count = 0;
	kos_vdev_descr_t* vdevs = NULL;

	node_count = 0;

	int fread_rv;
	gv_node_ent_t header;

	while ((fread_rv = fread(&header, 1, sizeof header, f)) > 0) {
		nodes[node_count++] = header;

		if (node_count >= sizeof nodes / sizeof *nodes) {
			LOG_E(cls, "Too many nodes in GrapeVine nodes file.");
			goto done;
		}

		LOG_I(cls, "Reading VDEVs of node with host ID 0x%" PRIx64 " (%u VDEVs).", header.host_id, header.vdev_count);

		size_t const vdevs_bytes = header.vdev_count * sizeof *vdevs;
		size_t const ent_size = sizeof header + vdevs_bytes;

		gv_node_ent_t* const ent = malloc(ent_size);
		assert(ent != NULL);
		memcpy(ent, &header, sizeof header);

		if (fread((void*) ent + sizeof header, 1, vdevs_bytes, f) != vdevs_bytes) {
			LOG_E(cls, "Failed to read VDEVs of node.");

			free(ent);
			fclose(f);

			return -1;
		}

		vdevs = realloc(vdevs, (vdev_count + header.vdev_count) * sizeof *vdevs);
		assert(vdevs != NULL);
		memcpy(vdevs + vdev_count, (void*) ent + sizeof header, vdevs_bytes);

		LOG_V(cls, "Check the VDEVs reported. %zu", (vdev_count + header.vdev_count) * sizeof *vdevs);

		for (size_t i = 0; i < header.vdev_count; i++) {
			kos_vdev_descr_t* const vdev = &vdevs[vdev_count + i];

			if (vdev->host_id != header.host_id) {
				LOG_W(
					cls,
					"VDEV (%s, vid=0x%" PRIx64 ") of node with host ID 0x%" PRIx64 " has a different host ID (0x%" PRIx64 ") than the node itself - fixing.",
					vdev->human,
					vdev->vdev_id,
					header.host_id,
					vdev->host_id
				);
				vdev->host_id = header.host_id;
			}

			if (vdev->kind != KOS_VDEV_KIND_GV) {
				LOG_W(
					cls,
					"VDEV (%s, vid=0x%" PRIx64 ") of node with host ID 0x%" PRIx64 " has an unexpected kind (%d) - fixing.",
					vdev->human,
					vdev->vdev_id,
					header.host_id,
					vdev->kind
				);
				vdev->kind = KOS_VDEV_KIND_GV;
			}
		}

		vdev_count += header.vdev_count;
		free(ent);
	}

	if (node_count == 0) {
		LOG_I(cls, "No GrapeVine nodes found on network.", gv_get_nodes_path());
	}

done:

	if (fread_rv < 0) {
		LOG_W(cls, "Failed to read GrapeVine node header.");
	}

	fclose(f);
	*vdevs_out = vdevs;

	return vdev_count;
}

int gv_get_ip_by_host_id(uint64_t host_id, in_addr_t* ipv4) {
	for (size_t i = 0; i < node_count; i++) {
		gv_node_ent_t* const node = &nodes[i];

		if (node->host_id != host_id) {
			continue;
		}

		memcpy(ipv4, &node->ip, sizeof *ipv4);
		return 0;
	}

	return -1;
}
