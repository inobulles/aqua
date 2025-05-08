// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#include <aqua/ui/wgpu.h>
#include <aqua/wgpu.h>
#include <aqua/win.h>

#include <umber.h>
#define UMBER_COMPONENT "demo.hello_ui_c"

int main(void) {
	int rv = EXIT_FAILURE;
	aqua_ctx_t const ctx = aqua_init();

	if (ctx == NULL) {
		LOG_FATAL("Failed to initialize AQUA library.");
		goto err_aqua_init;
	}

	LOG_INFO("KOS name: %s.", aqua_get_kos_descr(ctx)->name);

	// Get the best window VDEV.

	kos_vdev_descr_t* const win_vdev = aqua_get_best_vdev(win_init(ctx));

	if (win_vdev == NULL) {
		LOG_FATAL("No window VDEV found.");
		goto err_no_win_vdev_found;
	}

	LOG_INFO("Using window VDEV \"%s\".", (char*) win_vdev->human);
	win_ctx_t win_ctx = win_conn(win_vdev);

	// Get the best WebGPU VDEV.

	kos_vdev_descr_t* const wgpu_vdev = aqua_get_best_vdev(wgpu_init(ctx));

	if (wgpu_vdev == NULL) {
		LOG_FATAL("No WebGPU VDEV found.");
		goto err_no_wgpu_vdev_found;
	}

	LOG_INFO("Using WebGPU VDEV \"%s\".", (char*) wgpu_vdev->human);
	wgpu_ctx_t wgpu_ctx = wgpu_conn(wgpu_vdev);

	// Get the best UI VDEV.

	kos_vdev_descr_t* const ui_vdev = aqua_get_best_vdev(ui_init(ctx));

	if (ui_vdev == NULL) {
		LOG_FATAL("No UI VDEV found.");
		goto err_no_ui_vdev_found;
	}

	LOG_INFO("Using UI VDEV \"%s\".", (char*) ui_vdev->human);
	ui_ctx_t ui_ctx = ui_conn(ui_vdev);

	// Create a window.

	win_t const win = win_create(win_ctx);

	if (win == NULL) {
		LOG_FATAL("Failed to create window.");
		goto err_win_create;
	}

	// Create a UI.

	ui_t const ui = ui_create(ui_ctx);

	if (ui == NULL) {
		LOG_FATAL("Failed to create UI.");
		goto err_ui_create;
	}

	// Set up UI backend.

	ui_wgpu_ez_state_t state;

	if (ui_wgpu_ez_setup(&state, ui, win, wgpu_ctx) < 0) {
		LOG_FATAL("Failed to set up WebGPU UI backend.");
		goto err_ui_wgpu_ez_setup;
	}

	// Success!

	rv = EXIT_SUCCESS;

	// Cleanup.

err_ui_wgpu_ez_setup:

	ui_destroy(ui);

err_ui_create:

	win_destroy(win);

err_win_create:

	ui_disconn(ui_ctx);

err_no_ui_vdev_found:

	wgpu_disconn(wgpu_ctx);

err_no_wgpu_vdev_found:

	win_disconn(win_ctx);

err_no_win_vdev_found:
err_aqua_init:

	return rv;
}
