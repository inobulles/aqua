// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#include <aqua/root.h>
#include <aqua/wgpu.h>
#include <aqua/win.h>

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

static void resize(win_t win, void* data, uint32_t x_res, uint32_t y_res) {
	state_t* const state = data;
	(void) win;

	x_res = x_res == 0 ? 800 : x_res;
	y_res = y_res == 0 ? 600 : y_res;

	state->x_res = x_res;
	state->y_res = y_res;

	if (state->surface == NULL) {
		return;
	}

	printf("Resizing to %ux%u\n", x_res, y_res);

	// Reconfigure surface.

	state->config.device = state->device;
	state->config.usage = WGPUTextureUsage_RenderAttachment;
	state->config.format = state->caps.formats[0];
	state->config.presentMode = WGPUPresentMode_Fifo;
	state->config.alphaMode = state->alpha_mode;
	state->config.width = x_res;
	state->config.height = y_res;

	aqua_wgpuSurfaceConfigure(state->wgpu_ctx, state->surface, &state->config);
	state->configured = true;
}

static void request_adapter_cb(WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView msg, void* data, void* data2) {
	state_t* const state = data;
	(void) data2;

	if (status == WGPURequestAdapterStatus_Success) {
		state->adapter = adapter;
	}

	else {
		fprintf(stderr, "%s: status = %#.8x, message = \"%.*s\"\n", __func__, status, (int) msg.length, msg.data);
	}
}

static void request_device_cb(WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView msg, void* data, void* data2) {
	state_t* const state = data;
	(void) data2;

	if (status == WGPURequestDeviceStatus_Success) {
		state->device = device;
	}

	else {
		fprintf(stderr, "%s: status = %#.8x, message = \"%.*s\"\n", __func__, status, (int) msg.length, msg.data);
	}
}

static int setup_surface(state_t* state, win_t win) {
	assert(state->surface == NULL);

	// Create surface from window.

	state->surface = wgpu_surface_from_win(state->wgpu_ctx, state->instance, win);

	if (state->surface == NULL) {
		fprintf(stderr, "Failed to create WebGPU surface\n");
		goto err_create_surface;
	}

	printf("WebGPU surface created: %p\n", state->surface);

	// Get adapter.

	WGPURequestAdapterOptions const request_adapter_options = {
		.compatibleSurface = state->surface,
		.backendType = WGPUBackendType_Undefined, // Will use anything.
	};

	WGPURequestAdapterCallbackInfo const request_adapter_cb_info = {
		.callback = request_adapter_cb,
		.userdata1 = state,
	};

	aqua_wgpuInstanceRequestAdapter(state->wgpu_ctx, state->instance, &request_adapter_options, request_adapter_cb_info);

	if (state->adapter == NULL) {
		fprintf(stderr, "Failed to get WebGPU adapter\n");
		goto err_req_adapter;
	}

	WGPUAdapterInfo info;
	aqua_wgpuAdapterGetInfo(state->wgpu_ctx, state->adapter, &info);

	printf("Got adapter: %.*s %.*s %.*s (%.*s)\n", (int) info.vendor.length, info.vendor.data, (int) info.architecture.length, info.architecture.data, (int) info.device.length, info.device.data, (int) info.description.length, info.description.data);

	// Get device.

	WGPURequestDeviceCallbackInfo const request_device_cb_info = {
		.callback = request_device_cb,
		.userdata1 = state,
	};

	aqua_wgpuAdapterRequestDevice(state->wgpu_ctx, state->adapter, NULL, request_device_cb_info);

	if (state->device == NULL) {
		fprintf(stderr, "Failed to get WebGPU device\n");
		goto err_req_device;
	}

	// Create shader module.

	WGPUShaderSourceWGSL const wgsl_source = {
		.chain = {
			.sType = WGPUSType_ShaderSourceWGSL,
		},
		.code = {SHADER_SRC, WGPU_STRLEN},
	};

	WGPUShaderModuleDescriptor const wgsl_descr = {
		.label = {"shader", WGPU_STRLEN},
		.nextInChain = (WGPUChainedStruct*) &wgsl_source,
	};

	WGPUShaderModuleDescriptor const shader_module_descr = {
		.label = {"shader_module", WGPU_STRLEN},
		.nextInChain = (WGPUChainedStruct*) &wgsl_descr,
	};

	state->shader = aqua_wgpuDeviceCreateShaderModule(state->wgpu_ctx, state->device, &shader_module_descr);

	if (state->shader == NULL) {
		fprintf(stderr, "Failed to create WebGPU shader module\n");
		goto err_shader_module;
	}

	// Get queue from device.

	state->queue = aqua_wgpuDeviceGetQueue(state->wgpu_ctx, state->device);

	if (state->queue == NULL) {
		fprintf(stderr, "Failed to get WebGPU queue\n");
		goto err_get_queue;
	}

	// Get surface capabilities.

	aqua_wgpuSurfaceGetCapabilities(state->wgpu_ctx, state->surface, state->adapter, &state->caps);

	printf("Surface formats:");

	for (size_t i = 0; i < state->caps.formatCount; ++i) {
		printf(" %u", state->caps.formats[i]);
	}

	printf("\n");

	state->alpha_mode = state->caps.alphaModes[0];

	bool has_unpremulitplied = false;
	bool has_premultiplied = false;

	for (size_t i = 0; i < state->caps.alphaModeCount; ++i) {
		switch (state->caps.alphaModes[i]) {
		case WGPUCompositeAlphaMode_Unpremultiplied:
			has_unpremulitplied = true;
			break;
		case WGPUCompositeAlphaMode_Premultiplied:
			has_premultiplied = true;
			break;
		default:
			break;
		}
	}

	if (has_premultiplied) {
		state->alpha_mode = WGPUCompositeAlphaMode_Premultiplied;
	}

	else if (has_unpremulitplied) {
		state->alpha_mode = WGPUCompositeAlphaMode_Unpremultiplied;
	}

	printf("Chose alpha mode %u\n", state->alpha_mode);

	// Create pipeline layout.

	WGPUPipelineLayoutDescriptor const pipeline_layout_descr = {
		.label = {"pipeline_layout", WGPU_STRLEN},
	};

	WGPUPipelineLayout const pipeline_layout = aqua_wgpuDeviceCreatePipelineLayout(state->wgpu_ctx, state->device, &pipeline_layout_descr);

	if (pipeline_layout == NULL) {
		fprintf(stderr, "Failed to create pipeline layout\n");
		goto err_pipeline_layout;
	}

	// Create render pipeline.

	WGPUVertexState const vert_state = {
		.module = state->shader,
		.entryPoint = {"vert_main", WGPU_STRLEN},
	};

	WGPUColorTargetState const target_states[] = {
		{
			.format = state->caps.formats[0],
			.writeMask = WGPUColorWriteMask_All,
		}
	};

	WGPUFragmentState const frag_state = {
		.module = state->shader,
		.entryPoint = {"frag_main", WGPU_STRLEN},
		.targetCount = sizeof target_states / sizeof *target_states,
		.targets = target_states,
	};

	WGPUPrimitiveState const primitive_state = {
		.topology = WGPUPrimitiveTopology_TriangleList,
	};

	WGPUMultisampleState const ms_state = {
		.count = 1,
		.mask = 0xFFFFFFFF,
	};

	WGPURenderPipelineDescriptor const render_pipeline_descr = {
		.label = {"render_pipeline", WGPU_STRLEN},
		.layout = pipeline_layout,
		.vertex = vert_state,
		.fragment = &frag_state,
		.primitive = primitive_state,
		.multisample = ms_state,
	};

	state->render_pipeline = aqua_wgpuDeviceCreateRenderPipeline(state->wgpu_ctx, state->device, &render_pipeline_descr);

	if (state->render_pipeline == NULL) {
		fprintf(stderr, "Failed to create render pipeline\n");
		goto err_render_pipeline;
	}

	// Make sure the surface is configured.
	// Some platforms may call the resize callback when the window is created, but some might not, so we must do this ourselves.

	resize(win, state, state->x_res, state->y_res);

	return 0;

	aqua_wgpuRenderPipelineRelease(state->wgpu_ctx, state->render_pipeline);

err_render_pipeline:

	aqua_wgpuPipelineLayoutRelease(state->wgpu_ctx, pipeline_layout);

err_pipeline_layout:
err_get_queue:

	aqua_wgpuShaderModuleRelease(state->wgpu_ctx, state->shader);

err_shader_module:

	aqua_wgpuDeviceRelease(state->wgpu_ctx, state->device);

err_req_device:

	aqua_wgpuAdapterRelease(state->wgpu_ctx, state->adapter);

err_req_adapter:

	aqua_wgpuSurfaceRelease(state->wgpu_ctx, state->surface);

err_create_surface:

	return -1;
}

static void redraw(win_t win, void* data) {
	state_t* const state = data;

	// If not yet set up, setup surface.

	if (state->surface == NULL) {
		setup_surface(state, win);
	}

	// Actually render.

	if (!state->configured) {
		return;
	}

	WGPUSurfaceTexture surface_texture;
	aqua_wgpuSurfaceGetCurrentTexture(state->wgpu_ctx, state->surface, &surface_texture);

	switch (surface_texture.status) {
	case WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal:
	case WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal:
		break;
	case WGPUSurfaceGetCurrentTextureStatus_Error:
	case WGPUSurfaceGetCurrentTextureStatus_Timeout:
	case WGPUSurfaceGetCurrentTextureStatus_Outdated:
	case WGPUSurfaceGetCurrentTextureStatus_Lost:
		if (surface_texture.texture != NULL) {
			aqua_wgpuTextureRelease(state->wgpu_ctx, surface_texture.texture);
		}

		// TODO I am sort of confused about what this code is tryna accomplish?

		uint32_t const width = 1600;
		uint32_t const height = 1200;

		if (width != 0 && height != 0) {
			state->config.width = width;
			state->config.height = height;

			aqua_wgpuSurfaceConfigure(state->wgpu_ctx, state->surface, &state->config);
		}

		return;
	case WGPUSurfaceGetCurrentTextureStatus_OutOfMemory:
	case WGPUSurfaceGetCurrentTextureStatus_DeviceLost:
	case WGPUSurfaceGetCurrentTextureStatus_Force32:
		// Fatal error
		fprintf(stderr, "Failed to get current surface texture status=%#.8x\n", surface_texture.status);
		return; // TODO Errors.
	}

	assert(surface_texture.texture != NULL);

	WGPUTextureView const frame = aqua_wgpuTextureCreateView(state->wgpu_ctx, surface_texture.texture, NULL);
	assert(frame != NULL);

	WGPUCommandEncoderDescriptor const encoder_descr = {
		.label = {"command_encoder", WGPU_STRLEN}
	};

	WGPUCommandEncoder const encoder = aqua_wgpuDeviceCreateCommandEncoder(state->wgpu_ctx, state->device, &encoder_descr);
	assert(encoder != NULL);

	WGPURenderPassColorAttachment const colour_attachments[] = {
		{
			.view = frame,
			.loadOp = WGPULoadOp_Clear,
			.storeOp = WGPUStoreOp_Store,
			.clearValue = (WGPUColor const) {
				.r = 0.,
				.g = 0.,
				.b = 0.,
				.a = .1,
			},
		},
	};

	WGPURenderPassDescriptor const render_pass_descr = {
		.label = {"render_pass_encoder", WGPU_STRLEN},
		.colorAttachmentCount = sizeof colour_attachments / sizeof *colour_attachments,
		.colorAttachments = colour_attachments,
	};

	WGPURenderPassEncoder const render_pass = aqua_wgpuCommandEncoderBeginRenderPass(state->wgpu_ctx, encoder, &render_pass_descr);

	assert(render_pass != NULL);

	aqua_wgpuRenderPassEncoderSetPipeline(state->wgpu_ctx, render_pass, state->render_pipeline);
	aqua_wgpuRenderPassEncoderDraw(state->wgpu_ctx, render_pass, 3, 1, 0, 0);
	aqua_wgpuRenderPassEncoderEnd(state->wgpu_ctx, render_pass);
	aqua_wgpuRenderPassEncoderRelease(state->wgpu_ctx, render_pass);

	// Create final command buffer.

	WGPUCommandBufferDescriptor const cmd_buf_descr = {
		.label = {"command_buffer", WGPU_STRLEN}
	};

	WGPUCommandBuffer const cmd_buf = aqua_wgpuCommandEncoderFinish(state->wgpu_ctx, encoder, &cmd_buf_descr);

	// Submit command buffer to queue.

	WGPUCommandBuffer const cmd_bufs[] = {cmd_buf};
	aqua_wgpuQueueSubmit(state->wgpu_ctx, state->queue, sizeof cmd_bufs / sizeof *cmd_bufs, cmd_bufs);
	aqua_wgpuSurfacePresent(state->wgpu_ctx, state->surface);

	aqua_wgpuCommandBufferRelease(state->wgpu_ctx, cmd_buf);
	aqua_wgpuCommandEncoderRelease(state->wgpu_ctx, encoder);
	aqua_wgpuTextureViewRelease(state->wgpu_ctx, frame);
	aqua_wgpuTextureRelease(state->wgpu_ctx, surface_texture.texture);
}

int main(void) {
	int rv = EXIT_FAILURE;
	aqua_ctx_t const ctx = aqua_init();

	if (ctx == NULL) {
		fprintf(stderr, "Failed to initialize AQUA library\n");
		goto err_aqua_init;
	}

	printf("KOS name: %s\n", aqua_get_kos_descr(ctx)->name);

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
