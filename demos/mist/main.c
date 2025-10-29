// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#include <aqua/wm.h>
#include <aqua/vr.h>

#include <umber.h>

int main(void) {
	umber_class_t const* const cls = umber_class_new("mist", UMBER_LVL_VERBOSE, "Mist WM demo");

	aqua_ctx_t const ctx = aqua_init();

	if (ctx == NULL) {
		LOG_F(cls, "Failed to initialize AQUA library.");
		return EXIT_FAILURE;
	}

	// Get the best VR VDEV.

	kos_vdev_descr_t* const vr_vdev = aqua_get_best_vdev(vr_init(ctx));

	if (vr_vdev == NULL) {
		LOG_F(cls, "No VR VDEV found.");
		return EXIT_FAILURE;
	}

	LOG_V(cls, "Using VR VDEV \"%s\".", (char*) vr_vdev->human);
	vr_ctx_t const vr_ctx = vr_conn(vr_vdev);

	if (vr_ctx == NULL) {
		LOG_F(cls, "Failed to connect to VR VDEV.");
		return EXIT_FAILURE;
	}

	// Get the best WM VDEV.

	kos_vdev_descr_t* const wm_vdev = aqua_get_best_vdev(wm_init(ctx));

	if (wm_vdev == NULL) {
		LOG_F(cls, "No WM VDEV found.");
		return EXIT_FAILURE;
	}

	LOG_V(cls, "Using WM VDEV \"%s\".", (char*) wm_vdev->human);
	wm_ctx_t const wm_ctx = wm_conn(wm_vdev);

	if (wm_ctx == NULL) {
		LOG_F(cls, "Failed to connect to WM VDEV.");
		return EXIT_FAILURE;
	}

	// Send dummy window.

	uint8_t* const fb = malloc(100 * 100 * 4);

	for (size_t i = 0; i < 100 * 100; i++) {
		fb[i * 4 + 0] = i % 256;
		fb[i * 4 + 1] = 0;
		fb[i * 4 + 2] = 255;
		fb[i * 4 + 3] = 255;
	}

	vr_send_win(vr_ctx, 0, 100, 100, fb);

	// Create WM.

	int rv = EXIT_FAILURE;
	wm_t const wm = wm_create(wm_ctx);

	if (wm == NULL) {
		LOG_F(cls, "Failed to create WM.");
		goto disconn;
	}

	// Loop.

	wm_loop(wm);

	// Clean up.

	wm_destroy(wm);

	rv = EXIT_SUCCESS;

disconn:

	wm_disconn(wm_ctx);

	return rv;
}
