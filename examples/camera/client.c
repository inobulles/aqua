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

	uint64_t win_conn_id;
	uint64_t win_create;
	uint64_t win_destroy;
	uint64_t win_loop;
	uint64_t win_opaque_ptr;

	// Since we're going to be flushing after each operation, we only need one cookie at a time.

	kos_cookie_t last_cookie;
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
		assert(notif->cookie == state->last_cookie);
		fprintf(stderr, "Connection to window VDEV failed\n");

		break;
	case KOS_NOTIF_CONN:
		assert(notif->cookie == state->last_cookie);

		printf("Connection established with window VDEV: %lu\n", notif->conn.conn_id);
		state->win_conn_id = notif->conn.conn_id;

		for (size_t i = 0; i < notif->conn.fn_count; i++) {
			kos_fn_t const* const fn = &notif->conn.fns[i];
			printf("Has function %zu:\t%s:\t%s\n", i, fn->name, kos_type_str[fn->ret_type]);

			if (strcmp((char*) fn->name, "create") == 0) {
				state->win_create = i;
			}

			else if (strcmp((char*) fn->name, "destroy") == 0) {
				state->win_destroy = i;
			}

			else if (strcmp((char*) fn->name, "loop") == 0) {
				state->win_loop = i;
			}

			for (size_t j = 0; j < fn->param_count; j++) {
				kos_param_t const* const param = &fn->params[j];
				printf("\tParam %zu:\t%s:\t%s\n", j, param->name, kos_type_str[param->type]);
			}
		}

		break;
	case KOS_NOTIF_CALL_FAIL:
		assert(notif->cookie == state->last_cookie);
		fprintf(stderr, "Failed to call function on window VDEV\n");

		break;
	case KOS_NOTIF_CALL_RET:
		assert(notif->cookie == state->last_cookie);
		state->win_opaque_ptr = notif->call_ret.ret.opaque_ptr;

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
	state.last_cookie = kos_vdev_conn(state.win_hid, state.win_vid);
	kos_flush(true);

	// TODO Somehow wait for connection to be established.

	if (state.win_conn_id == -1u) {
		fprintf(stderr, "Failed to connect to window VDEV\n");
		return EXIT_FAILURE;
	}

	// Create the window.

	state.last_cookie = kos_vdev_call(state.win_conn_id, state.win_create, NULL);
	kos_flush(true);

	// Loop the window.

	kos_val_t val = {
		.opaque_ptr = state.win_opaque_ptr,
	};

	uint8_t args_buf[sizeof val.opaque_ptr];
	memcpy(args_buf, &val, sizeof val.opaque_ptr);

	state.last_cookie = kos_vdev_call(state.win_conn_id, state.win_loop, args_buf);
	kos_flush(true);

	// Destroy the window.

	state.last_cookie = kos_vdev_call(state.win_conn_id, state.win_destroy, args_buf);
	kos_flush(true);

	// Clean up.

	kos_vdev_disconn(state.win_conn_id);
	return EXIT_SUCCESS;
}
