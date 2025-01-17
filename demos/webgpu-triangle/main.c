// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2025 Aymeric Wibo

#include <aqua/kos.h>
#include <aqua/win.h>

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
	bool has_win;
	win_softc_t win_sc;
} state_t;

static void vdev_notif_cb(kos_notif_t const* notif, void* data) {
	state_t* const state = data;

	switch (notif->kind) {
	case KOS_NOTIF_ATTACH:;
		kos_vdev_descr_t const* const vdev = &notif->attach.vdev;

		if (win_probe(vdev) && !state->has_win) {
			// Connect to the first window VDEV we find.

			printf("Found window VDEV: %s\n", vdev->human);
			state->win_sc = win_conn(vdev);
			kos_flush(true);
		}

		break;
	case KOS_NOTIF_DETACH:
		printf("VDEV detached: %" PRIu64 ":%" PRIu64 "\n", notif->detach.host_id, notif->detach.vdev_id);
		break;
	case KOS_NOTIF_CONN_FAIL:
		fprintf(stderr, "Connection to VDEV failed\n");
		break;
	case KOS_NOTIF_CONN:
		win_notif_conn(state->win_sc, notif);
		break;
	case KOS_NOTIF_CALL_FAIL:
		fprintf(stderr, "Failed to call function on window VDEV\n");
		break;
	case KOS_NOTIF_CALL_RET:
		win_notif_call_ret(state->win_sc, notif);
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

	state_t state = {
		.win_sc = NULL,
	};

	kos_sub_to_notif(vdev_notif_cb, &state);

	// Setup window.

	win_init();

	win_t const win = win_create(state.win_sc);
	win_loop(win);
	win_destroy(win);

	return EXIT_SUCCESS;
}
