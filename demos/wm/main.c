// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#include <aqua/root.h>
#include <aqua/wgpu.h>
#include <aqua/wm.h>

#include <umber.h>

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#define MULTILINE(...) #__VA_ARGS__

// clang-format off
static char const* const SHADER_SRC = MULTILINE(
struct VertOut {
	@builtin(position) pos: vec4f,
	@location(0) colour: vec3f,
};

@vertex
fn vert_main(@builtin(vertex_index) index: u32) -> VertOut {
	var out: VertOut;

	if index == 0u {
		out.pos = vec4(-.5, -.5, 0., 1.);
		out.colour = vec3(1., 0., 0.);
	}

	if index == 1u {
		out.pos = vec4(.5, -.5, 0., 1.);
		out.colour = vec3(0., 1., 0.);
	}

	if index == 2u {
		out.pos = vec4(0., .5, 0., 1.);
		out.colour = vec3(0., 0., 1.);
	}

	return out;
}

struct FragOut {
	@location(0) colour: vec4f,
};

@fragment
fn frag_main(vert: VertOut) -> FragOut {
	var out: FragOut;

	out.colour = vec4(vert.colour, 1.);

	return out;
}
);
// clang-format on

typedef struct {
	wgpu_ctx_t wgpu_ctx;
	WGPUInstance instance;
	WGPUSurface surface;
	WGPUAdapter adapter;
	WGPUDevice device;
	WGPUShaderModule shader;
	WGPUQueue queue;
	WGPUSurfaceCapabilities caps;
	WGPUCompositeAlphaMode alpha_mode;
	WGPURenderPipeline render_pipeline;
	WGPUSurfaceConfiguration config;
	bool configured;
	uint32_t x_res, y_res;
} state_t;

int main(void) {
	(void) SHADER_SRC; // TODO REMME

	umber_class_t const* const cls = umber_class_new("wm", UMBER_LVL_VERBOSE, "WM demo");
	aqua_ctx_t const ctx = aqua_init();

	if (ctx == NULL) {
		LOG_F(cls, "Failed to initialize AQUA library.");
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

	// Get the best WebGPU VDEV.

	kos_vdev_descr_t* const wgpu_vdev = aqua_get_best_vdev(wgpu_init(ctx));

	if (wgpu_vdev == NULL) {
		LOG_F(cls, "No WebGPU VDEV found.");
		return EXIT_FAILURE;
	}

	LOG_I(cls, "Using WebGPU VDEV \"%s\".", (char*) wgpu_vdev->human);
	wgpu_ctx_t wgpu_ctx = wgpu_conn(wgpu_vdev);

	if (wgpu_ctx == NULL) {
		LOG_F(cls, "Failed to connect to WebGPU VDEV.");
		return EXIT_FAILURE;
	}

	// Create WM.

	int rv = EXIT_FAILURE;
	wm_t const wm = wm_create(wm_ctx);

	if (wm == NULL) {
		LOG_F(cls, "Failed to create WM.");
		goto disconn;
	}

	printf("test %d\n", __LINE__);

	// Create WebGPU instance.

	WGPUInstance const instance = aqua_wgpuCreateInstance(wgpu_ctx, &(WGPUInstanceDescriptor) {});
	printf("test %d\n", __LINE__);

	if (instance == NULL) {
		LOG_F(cls, "Failed to create WebGPU instance.");
		goto disconn;
	}
	printf("test %d\n", __LINE__);

	// Get WebGPU device from WM.

	WGPUDevice const dev = wgpu_device_from_wm(wgpu_ctx, instance, wm); // wm_get_wgpu_dev(wm, instance);

	if (dev == NULL) {
		LOG_F(cls, "Failed to get WebGPU device.");
		goto disconn;
	}
	printf("test %d\n", __LINE__);

	LOG_I(cls, "Got WebGPU device: %p.", dev);

	// Loop.

	wm_loop(wm);

	// Clean up.

	wm_destroy(wm);

	rv = EXIT_SUCCESS;

disconn:

	wm_disconn(wm_ctx);

	return rv;
}

/*
	// Get the first window VDEV and create library component context from that.
	// TODO Maybe I should have a simple function to do this for me automatically? I.e. init + find best VDEV + connection. Maybe even roll in aqua_init somehow.

	aqua_component_t const win_comp = win_init(ctx);
	win_ctx_t win_ctx = NULL;

	for (aqua_vdev_it_t it = aqua_vdev_it(win_comp); it.vdev != NULL; aqua_vdev_it_next(&it)) {
		kos_vdev_descr_t* const vdev = it.vdev;
		win_ctx = win_conn(vdev);

		if (win_ctx != NULL) {
			printf("Using window VDEV %s\n", (char*) vdev->human);
			break;
		}
	}

	if (win_ctx == NULL) {
		fprintf(stderr, "No window VDEV found or failed to connect\n");
		goto err_no_win_vdev_found;
	}

	// Get the first WebGPU VDEV and create library component context from that.
	// TODO Maybe I should have a simple function to do this for me automatically? I.e. init + find best VDEV + connection. Maybe even roll in aqua_init somehow.

	aqua_component_t const wgpu_comp = wgpu_init(ctx);
	wgpu_ctx_t wgpu_ctx = NULL;

	for (aqua_vdev_it_t it = aqua_vdev_it(wgpu_comp); it.vdev != NULL; aqua_vdev_it_next(&it)) {
		kos_vdev_descr_t* const vdev = it.vdev;
		wgpu_ctx = wgpu_conn(vdev);

		if (wgpu_ctx != NULL) {
			printf("Using WebGPU VDEV %s\n", (char*) vdev->human);
			break;
		}
	}

	if (wgpu_ctx == NULL) {
		fprintf(stderr, "No WebGPU VDEV found or failed to connect\n");
		goto err_no_wgpu_vdev_found;
	}

	// Create window.

	win_t const win = win_create(win_ctx);

	if (win == NULL) {
		fprintf(stderr, "Failed to create window\n");
		goto err_win_create;
	}

	// Set up WebGPU stuff.

	state_t state = {
		.wgpu_ctx = wgpu_ctx,
	};

	WGPUInstanceDescriptor const create_instance_descr = {};
	state.instance = aqua_wgpuCreateInstance(wgpu_ctx, &create_instance_descr);

	if (state.instance == NULL) {
		fprintf(stderr, "Failed to create WebGPU instance\n");
		goto err_instance;
	}

	printf("WebGPU instance created: %p\n", state.instance);

	// Loop the window.

	win_register_redraw_cb(win, redraw, &state);
	win_register_resize_cb(win, resize, &state);

	win_loop(win);

	// Success!

	rv = EXIT_SUCCESS;

	// Cleanup.

err_instance:

	win_destroy(win);

err_win_create:

	wgpu_disconn(wgpu_ctx);

err_no_wgpu_vdev_found:

	win_disconn(win_ctx);

err_no_win_vdev_found:
err_aqua_init:

	return rv;
}
*/
