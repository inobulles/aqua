// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2024 Aymeric Wibo

#include <aqua/kos.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	bool win_set;
	uint64_t win_hid;
	uint64_t win_vid;

	uint64_t win_conn_cookie;
	uint64_t win_conn_id;
	uint64_t win_create;
} state_t;

static void vdev_notif_cb(kos_notif_t const* notif, void* data) {
	state_t* const state = data;

	switch (notif->kind) {
	case KOS_NOTIF_ATTACH:;
		kos_vdev_descr_t const* const vdev = &notif->attach.vdev;
		char const* const spec = (void*) vdev->spec;

		if (strcmp(spec, "aquabsd.black.cam") == 0) {
			printf("Found camera VDEV: %s (from vdriver \"%s\")\n", vdev->human, vdev->vdriver_human);
		}

		else if (strcmp(spec, "aquabsd.black.win") == 0) {
			printf("Found window VDEV: %s (from vdriver \"%s\")\n", vdev->human, vdev->vdriver_human);

			if (state->win_set) {
				fprintf(stderr, "Multiple window VDEVs found\n");
			}

			state->win_set = true;
			state->win_hid = vdev->host_id;
			state->win_vid = vdev->vdev_id;
		}

		break;
	case KOS_NOTIF_DETACH:
		printf("VDEV detached: %lu:%lu\n", notif->detach.host_id, notif->detach.vdev_id);
		break;
	case KOS_NOTIF_CONN_FAIL:
		if (notif->cookie == state->win_conn_cookie) {
			fprintf(stderr, "Connection to window VDEV failed\n");
		}

		else {
			assert(false);
		}

		break;
	case KOS_NOTIF_CONN:
		if (notif->cookie == state->win_conn_cookie) {
			printf("Connection established with window VDEV: %lu\n", notif->conn.conn_id);
			state->win_conn_id = notif->conn.conn_id;

			for (size_t i = 0; i < notif->conn.fn_count; i++) {
				kos_vdev_fn_t const* const fn = &notif->conn.fns[i];
				printf("Has function %zu:\t%s\n", i, fn->name);

				if (strcmp((char*) fn->name, "create") == 0) {
					state->win_create = i;
				}

				for (size_t j = 0; j < fn->arg_count; j++) {
					kos_vdev_fn_arg_t const* const arg = &fn->args[j];
					printf("\tArg %zu:\t%d\t%s\n", j, arg->type, arg->name);
				}
			}
		}

		else {
			assert(false);
		}

		break;
	}
}

int main(void) {
	// Initiate communication with the KOS.

	kos_descr_v4_t descr;

	if (kos_hello(KOS_API_V4, KOS_API_V4, &descr) != KOS_API_V4) {
		return EXIT_FAILURE;
	}

	printf("Loaded KOS v4: %s\n", descr.name);

	// Get the camera and window VDEVs.

	state_t state = {
		.win_set = false,
	};

	kos_sub_to_notif(vdev_notif_cb, &state);

	kos_req_vdev("aquabsd.black.win");
	kos_req_vdev("aquabsd.black.cam");

	// Since kos_req_vdev is guaranteed to immediately call our VDEV notification callback on a local device, we can check this right after.

	if (!state.win_set) {
		fprintf(stderr, "No window VDEV found\n");
		return EXIT_FAILURE;
	}

	// Establish a connection with the window VDEV.

	state.win_conn_id = -1u;
	state.win_conn_cookie = kos_vdev_conn(state.win_hid, state.win_vid);
	kos_flush(true);

	// TODO Somehow wait for connection to be established.

	if (state.win_conn_id == -1u) {
		fprintf(stderr, "Failed to connect to window VDEV\n");
		return EXIT_FAILURE;
	}

	// Create the window.
	// TODO Cookie this and process response.

	kos_vdev_call(state.win_conn_id, state.win_create, NULL);
	kos_flush(true);

	// Clean up.

	kos_vdev_disconn(state.win_conn_id);
	return EXIT_SUCCESS;
}
