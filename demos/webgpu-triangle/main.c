// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2025 Aymeric Wibo

// #include <aqua/aqua.h>
// #include <aqua/win.h>

#include "../../lib/aqua.h"
#include "../../lib/wgpu.h"
#include "../../lib/win.h"

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
	wgpu_ctx_t wgpu_ctx;
	WGPUInstance instance;
	WGPUSurface surface;
} state_t;

static void redraw(win_t win, void* data) {
	state_t* const state = data;

	if (state->surface == NULL) {
		state->surface = wgpu_surface_from_win(state->wgpu_ctx, state->instance, win);
		printf("WebGPU surface created: %p\n", state->surface);
	}
}

int main(void) {
	aqua_ctx_t const ctx = aqua_init();

	if (ctx == NULL) {
		fprintf(stderr, "Failed to initialize AQUA library\n");
		return EXIT_FAILURE;
	}

	printf("KOS name: %s\n", aqua_get_kos_descr(ctx)->name);

	// Setup window library component.

	aqua_component_t const win_comp = win_init(ctx);

	// Get the first window VDEV and create library component context from that.
	// TODO Maybe I should have a simple function to do this for me automatically? I.e. init + find best VDEV + connection. Maybe even roll in aqua_init somehow.

	kos_vdev_descr_t* vdev = NULL;

	for (aqua_vdev_it_t it = aqua_vdev_it(win_comp); it.vdev != NULL; aqua_vdev_it_next(&it)) {
		vdev = it.vdev;
		break;
	}

	if (vdev == NULL) {
		fprintf(stderr, "No window VDEV found\n");
		return EXIT_FAILURE;
	}

	printf("Using window VDEV %s\n", (char*) vdev->human);
	win_ctx_t const win_ctx = win_conn(vdev);

	// Setup WebGPU library component.

	aqua_component_t const wgpu_comp = wgpu_init(ctx);

	// Get the first WebGPU VDEV and create library component context from that.
	// TODO Maybe I should have a simple function to do this for me automatically? I.e. init + find best VDEV + connection. Maybe even roll in aqua_init somehow.

	vdev = NULL;

	for (aqua_vdev_it_t it = aqua_vdev_it(wgpu_comp); it.vdev != NULL; aqua_vdev_it_next(&it)) {
		vdev = it.vdev;
		break;
	}

	if (vdev == NULL) {
		fprintf(stderr, "No WebGPU VDEV found\n");
		return EXIT_FAILURE;
	}

	printf("Using WebGPU VDEV %s\n", (char*) vdev->human);
	wgpu_ctx_t const wgpu_ctx = wgpu_conn(vdev);

	// Create window.

	win_t const win = win_create(win_ctx);

	if (win == NULL) {
		fprintf(stderr, "Failed to create window\n");
		return EXIT_FAILURE;
	}

	// Set up WebGPU stuff.

	state_t state = {
		.wgpu_ctx = wgpu_ctx,
	};

	WGPUInstanceDescriptor const create_instance_descr = {};
	state.instance = aqua_wgpuCreateInstance(wgpu_ctx, &create_instance_descr);

	printf("WebGPU instance created: %p\n", state.instance);

	// Loop the window.

	win_register_redraw_cb(win, redraw, &state);
	win_loop(win);
	win_destroy(win);

	// Disconnect from VDEVs.

	win_disconn(win_ctx);
	wgpu_disconn(wgpu_ctx);

	return EXIT_SUCCESS;
}
