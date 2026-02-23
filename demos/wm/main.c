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
#include <string.h>

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

static void new_win(wm_t wm, wm_win_t win, char const* app_id, void* data) {
	(void) wm, (void) win;

	state_t* const s = data;
	(void) s;

	printf("New window: %s\n", app_id);
}

float x = 0;

static void redraw(wm_t wm, void* raw_image, void* data) {
	(void) wm;

	state_t* const s = data;

	WGPUTexture const tex = aqua_wgpuRenderTextureFromVkImage(s->wgpu_ctx, s->device, raw_image, WGPUTextureFormat_RGBA8Snorm, 1280, 720);
	WGPUTextureView const view = aqua_wgpuTextureCreateView(s->wgpu_ctx, tex, NULL);

	WGPUQueue const queue = aqua_wgpuDeviceGetQueue(s->wgpu_ctx, s->device);
	assert(queue != NULL);

	WGPUCommandEncoder const cmd_enc = aqua_wgpuDeviceCreateCommandEncoder(s->wgpu_ctx, s->device, &(WGPUCommandEncoderDescriptor) {});
	assert(cmd_enc != NULL);

	WGPURenderPassColorAttachment const colour_attachments[] = {
		{
			.view = view,
			.loadOp = WGPULoadOp_Clear,
			.storeOp = WGPUStoreOp_Store,
			.clearValue = (WGPUColor const) {x, 1, 1, 1},
		},
	};

	x += 0.01;

	WGPURenderPassDescriptor const render_pass_descr = {
		.label = {"render_pass_encoder", WGPU_STRLEN},
		.colorAttachmentCount = sizeof colour_attachments / sizeof *colour_attachments,
		.colorAttachments = colour_attachments,
	};

	WGPURenderPassEncoder const render_pass = aqua_wgpuCommandEncoderBeginRenderPass(s->wgpu_ctx, cmd_enc, &render_pass_descr);
	assert(render_pass != NULL);

	aqua_wgpuRenderPassEncoderEnd(s->wgpu_ctx, render_pass);
	aqua_wgpuRenderPassEncoderRelease(s->wgpu_ctx, render_pass);

	WGPUCommandBuffer const cmd_buf = aqua_wgpuCommandEncoderFinish(s->wgpu_ctx, cmd_enc, &(WGPUCommandBufferDescriptor) {});
	assert(cmd_buf != NULL);

	WGPUCommandBuffer const cmd_bufs[] = {cmd_buf};
	aqua_wgpuQueueSubmit(s->wgpu_ctx, queue, sizeof cmd_bufs / sizeof *cmd_bufs, cmd_bufs);

	aqua_wgpuCommandEncoderRelease(s->wgpu_ctx, cmd_enc);
	aqua_wgpuTextureRelease(s->wgpu_ctx, tex);
}

int main(void) {
	(void) SHADER_SRC; // TODO REMME

	state_t s;
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
	s.wgpu_ctx = wgpu_conn(wgpu_vdev);

	if (s.wgpu_ctx == NULL) {
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

	// Create WebGPU instance.

	WGPUInstance const instance = aqua_wgpuCreateInstance(s.wgpu_ctx, &(WGPUInstanceDescriptor) {});

	if (instance == NULL) {
		LOG_F(cls, "Failed to create WebGPU instance.");
		goto disconn;
	}

	// Get WebGPU device from WM.

	s.device = wgpu_device_from_wm(s.wgpu_ctx, instance, wm); // wm_get_wgpu_dev(wm, instance);

	if (s.device == NULL) {
		LOG_F(cls, "Failed to get WebGPU device.");
		goto disconn;
	}

	LOG_I(cls, "Got WebGPU device: %p.", s.device);

	wm_register_redraw_cb(wm, redraw, &s);
	wm_register_new_win_cb(wm, new_win, &s);

	// Loop.

	wm_loop(wm);

	// Clean up.

	wm_destroy(wm);

	rv = EXIT_SUCCESS;

disconn:

	wm_disconn(wm_ctx);

	return rv;
}
