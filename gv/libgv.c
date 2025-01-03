// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2024 Aymeric Wibo

#include "../kos/kos.h"
#include "gv.h"
#include "internal.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/file.h>

static void unlock(FILE* f) {
	flock(fileno(f), LOCK_UN);
	fclose(f);
}

ssize_t gv_query_vdevs(kos_vdev_descr_t** vdevs_out) {
	// Make sure gvd is running.

	FILE* const lock_file = fopen(GV_LOCK_PATH, "r");

	if (lock_file == NULL) {
		return 0;
	}

	int const rv = flock(fileno(lock_file), LOCK_EX | LOCK_NB);

	if (rv == 0) { // If we got the lock, gvd is not running.
		unlock(lock_file);
		return 0;
	}

	if (rv < 0 && errno != EWOULDBLOCK) { // Some other issue occurred.
		unlock(lock_file);
		return 0;
	}

	unlock(lock_file);

	// Actually read.

	FILE* const f = fopen(GV_NODES_PATH, "r");

	if (f == NULL) {
		return 0;
	}

	ssize_t vdev_count = 0;
	kos_vdev_descr_t* vdevs = NULL;

	while (!feof(f)) {
		node_ent_t header;

		if (fread(&header, 1, sizeof header, f) != sizeof header) {
			goto done;
		}

		size_t const vdevs_bytes = header.vdev_count * sizeof *vdevs;
		size_t const ent_size = sizeof header + vdevs_bytes;

		node_ent_t* const ent = malloc(ent_size);
		assert(ent != NULL);
		memcpy(ent, &header, sizeof header);

		if (fread(ent + sizeof header, 1, vdevs_bytes, f) != vdevs_bytes) {
			free(ent);
			fclose(f);

			return -1;
		}

		vdevs = realloc(vdevs, (vdev_count + header.vdev_count) * sizeof *vdevs);
		assert(vdevs != NULL);
		memcpy(vdevs + vdev_count, ent + sizeof header, vdevs_bytes);
		vdev_count += header.vdev_count;

		free(ent);
	}

done:

	fclose(f);
	*vdevs_out = vdevs;

	return vdev_count;
}
