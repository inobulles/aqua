// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2025 Aymeric Wibo

#include "wgpu.h"

#define __AQUA_LIB_COMPONENT__
#include "../ui_internal.h"

#include <umber.h>
#define UMBER_COMPONENT "aqua.lib.ui.wgpu"

#include <assert.h>
#include <string.h>

int ui_wgpu_init(ui_t ui, uint64_t hid, uint64_t vid, WGPUDevice device) {
	assert(ui != NULL);
	ui_ctx_t const ctx = ui->ctx;

	if (!(ctx->supported_backends & UI_BACKEND_WGPU)) {
		LOG_ERROR("WebGPU backend not supported.");
		return -1;
	}

	kos_val_t const args[] = {
		{
			.opaque_ptr = ui->opaque_ptr,
		},
		{.u64 = hid},
		{.u64 = vid},
		{.opaque_ptr = device},
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->backend_wgpu_fns.init, args);
	ctx->last_success = false;
	kos_flush(true);

	if (!ctx->last_success) {
		LOG_ERROR("Failed to initialize WebGPU backend.");
		return -1;
	}

	LOG_VERBOSE("Initialized WebGPU backend.");

	return 0;
}

int ui_wgpu_render(ui_t ui, WGPUTextureView frame, WGPUCommandEncoder command_encoder) {
	assert(ui != NULL);
	ui_ctx_t const ctx = ui->ctx;

	kos_val_t const args[] = {
		{.opaque_ptr = ui->opaque_ptr},
		{.opaque_ptr = frame},
		{.opaque_ptr = command_encoder},
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->backend_wgpu_fns.render, args);
	ctx->last_success = false;
	kos_flush(true);

	if (!ctx->last_success) {
		LOG_ERROR("Failed to render WebGPU backend.");
		return -1;
	}

	LOG_VERBOSE("Rendered UI with WebGPU backend.");

	return 0;
}

// "Easy" functions.

int ui_wgpu_ez_setup(ui_wgpu_ez_state_t* state, ui_t ui, win_t win, wgpu_ctx_t wgpu_ctx) {
	state->ui = ui;
	state->win = win;
	state->wgpu_ctx = wgpu_ctx;

	// Create instance.

	WGPUInstanceDescriptor const create_instance_descr = {};
	state->instance = aqua_wgpuCreateInstance(wgpu_ctx, &create_instance_descr);

	if (state->instance == NULL) {
		LOG_ERROR("Failed to create instance.");
		return -1;
	}

	LOG_VERBOSE("Created instance: %p.", state->instance);

	// The surface is set to NULL here.
	// When ui_wgpu_ez_render is called, it will be created.

	state->surface = NULL;
	memset(&state->config, 0, sizeof state->config);
	state->configured = false;

	return 0;
}

static void req_adapter_cb(WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView msg, void* data, void* data2) {
	ui_wgpu_ez_state_t* const state = data;
	(void) data2;

	if (status == WGPURequestAdapterStatus_Success) {
		state->adapter = adapter;
	}

	else {
		LOG_ERROR("Failed to request adapter (status=%#.8x, message=\"%.*s\").", status, (int) msg.length, msg.data);
	}
}

static void req_device_cb(WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView msg, void* data, void* data2) {
	ui_wgpu_ez_state_t* const state = data;
	(void) data2;

	if (status == WGPURequestDeviceStatus_Success) {
		state->device = device;
	}

	else {
		LOG_ERROR("Failed to request device (status=%#.8x, message=\"%.*s\").", status, (int) msg.length, msg.data);
	}
}

static int setup_surface(ui_wgpu_ez_state_t* state) {
	assert(state->surface == NULL);

	// Create surface from window.

	LOG_VERBOSE("Creating surface from window %p.", state->win);
	state->surface = wgpu_surface_from_win(state->wgpu_ctx, state->instance, state->win);

	if (state->surface == NULL) {
		LOG_ERROR("Failed to create surface.");
		goto err_create_surface;
	}

	// Get adapter.

	WGPURequestAdapterOptions const req_adapter_opts = {
		.compatibleSurface = state->surface,
		.backendType = WGPUBackendType_Undefined, // Will use anything.
	};

	WGPURequestAdapterCallbackInfo const req_adapter_cb_info = {
		.callback = req_adapter_cb,
		.userdata1 = state,
	};

	aqua_wgpuInstanceRequestAdapter(state->wgpu_ctx, state->instance, &req_adapter_opts, req_adapter_cb_info);

	if (state->adapter == NULL) {
		LOG_ERROR("Failed to get adapter.");
		goto err_req_adapter;
	}

	LOG_VERBOSE("Got adapter: %p.", state->adapter);

	aqua_wgpuAdapterGetInfo(state->wgpu_ctx, state->adapter, &state->adapter_info);

	if (state->adapter_info.vendor.length) {
		LOG_INFO("Adapter vendor: %.*s.", (int) state->adapter_info.vendor.length, state->adapter_info.vendor.data);
	}

	if (state->adapter_info.architecture.length) {
		LOG_INFO("Adapter architecture: %.*s.", (int) state->adapter_info.architecture.length, state->adapter_info.architecture.data);
	}

	if (state->adapter_info.device.length) {
		LOG_INFO("Adapter device ID: %.*s.", (int) state->adapter_info.device.length, state->adapter_info.device.data);
	}

	if (state->adapter_info.description.length) {
		LOG_INFO("Adapter description: %.*s.", (int) state->adapter_info.description.length, state->adapter_info.description.data);
	}

	// Get device.

	WGPURequestDeviceCallbackInfo const req_device_cb_info = {
		.callback = req_device_cb,
		.userdata1 = state,
	};

	aqua_wgpuAdapterRequestDevice(state->wgpu_ctx, state->adapter, NULL, req_device_cb_info);

	if (state->device == NULL) {
		LOG_ERROR("Failed to get device.");
		goto err_req_device;
	}

	LOG_VERBOSE("Got device: %p.", state->device);

	// Get queue from device.

	state->queue = aqua_wgpuDeviceGetQueue(state->wgpu_ctx, state->device);

	if (state->queue == NULL) {
		LOG_ERROR("Failed to get queue.");
		goto err_get_queue;
	}

	LOG_VERBOSE("Got queue: %p.", state->queue);

	// Get surface capabilities.
	// TODO I'll want to intelligently select the format and alpha mode I want.
	//      Should probably accept some hints argument for flags that the caller can pass to this function to select the format and alpha mode.

	aqua_wgpuSurfaceGetCapabilities(state->wgpu_ctx, state->surface, state->adapter, &state->caps);
	LOG_VERBOSE("Got surface capabilities.");

	// Create WebGPU backend on UI object.

	uint64_t const hid = (uint64_t) state->wgpu_ctx; // TODO wgpu_get_hid(state->wgpu_ctx);
	uint64_t const vid = wgpu_get_vid(state->wgpu_ctx);

	LOG_VERBOSE("Creating WebGPU UI backend with WebGPU device %lu:%lu.", hid, vid);

	if (ui_wgpu_init(state->ui, hid, vid, state->device) < 0) {
		LOG_ERROR("Failed to create WebGPU UI backend.");
		goto err_backend_init;
	}

	LOG_VERBOSE("Created WebGPU UI backend.");

	// Done!

	return 0;

err_backend_init:
err_get_queue:

	aqua_wgpuDeviceRelease(state->wgpu_ctx, state->device);

err_req_device:

	aqua_wgpuAdapterRelease(state->wgpu_ctx, state->adapter);

err_req_adapter:

	aqua_wgpuSurfaceRelease(state->wgpu_ctx, state->surface);

err_create_surface:

	return -1;
}

int ui_wgpu_ez_render(ui_wgpu_ez_state_t* state) {
	// If not yet set up, setup surface.

	if (state->surface == NULL) {
		LOG_VERBOSE("Surface needs to be set up.");

		if (setup_surface(state) < 0) {
			return -1;
		}
	}

	// Actually render.

	if (!state->configured) {
		LOG_VERBOSE("Surface not yet configured for this frame.");
		return 0;
	}

	LOG_VERBOSE("Getting current surface texture.");

	WGPUSurfaceTexture surf_tex;
	aqua_wgpuSurfaceGetCurrentTexture(state->wgpu_ctx, state->surface, &surf_tex);

	switch (surf_tex.status) {
	case WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal:
	case WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal:
		break;
	case WGPUSurfaceGetCurrentTextureStatus_Error:
	case WGPUSurfaceGetCurrentTextureStatus_Timeout:
	case WGPUSurfaceGetCurrentTextureStatus_Outdated:
	case WGPUSurfaceGetCurrentTextureStatus_Lost:
		if (surf_tex.texture != NULL) {
			aqua_wgpuTextureRelease(state->wgpu_ctx, surf_tex.texture);
		}

		LOG_WARN("Surface texture lost, reconfiguring surface.");

		aqua_wgpuSurfaceConfigure(state->wgpu_ctx, state->surface, &state->config);
		return 0;
	case WGPUSurfaceGetCurrentTextureStatus_OutOfMemory:
	case WGPUSurfaceGetCurrentTextureStatus_DeviceLost:
	case WGPUSurfaceGetCurrentTextureStatus_Force32:
		LOG_ERROR("Failed to get current surface texture (status=%#.8x).", surf_tex.status);
		return -1;
	}

	assert(surf_tex.texture != NULL);
	int rv = -1;

	LOG_VERBOSE("Creating texture view from surface texture.");

	WGPUTextureView const frame = aqua_wgpuTextureCreateView(state->wgpu_ctx, surf_tex.texture, NULL);

	if (frame == NULL) {
		LOG_ERROR("Failed to create texture view.");
		goto err_create_view;
	}

	LOG_VERBOSE("Creating command encoder.");

	WGPUCommandEncoderDescriptor const encoder_descr = {
		.label = {"command_encoder", WGPU_STRLEN}
	};

	WGPUCommandEncoder const encoder = aqua_wgpuDeviceCreateCommandEncoder(state->wgpu_ctx, state->device, &encoder_descr);

	if (encoder == NULL) {
		LOG_ERROR("Failed to create command encoder.");
		goto err_create_encoder;
	}

	LOG_VERBOSE("Rendering UI itself.");

	if (ui_wgpu_render(state->ui, frame, encoder) < 0) {
		LOG_ERROR("Failed to render UI.");
		goto err_render_ui;
	}

	LOG_VERBOSE("Creating final command buffer.");

	WGPUCommandBufferDescriptor const cmd_buf_descr = {
		.label = {"command_buffer", WGPU_STRLEN}
	};

	WGPUCommandBuffer const cmd_buf = aqua_wgpuCommandEncoderFinish(state->wgpu_ctx, encoder, &cmd_buf_descr);

	if (cmd_buf == NULL) {
		LOG_ERROR("Failed to create command buffer.");
		goto err_cmd_buf;
	}

	LOG_VERBOSE("Submitting command buffer to queue.");

	WGPUCommandBuffer const cmd_bufs[] = {cmd_buf};
	aqua_wgpuQueueSubmit(state->wgpu_ctx, state->queue, sizeof cmd_bufs / sizeof *cmd_bufs, cmd_bufs);

	LOG_VERBOSE("Presenting frame.");

	aqua_wgpuSurfacePresent(state->wgpu_ctx, state->surface);

	LOG_VERBOSE("Rendered frame successfully.");
	rv = 0;

err_cmd_buf:
err_render_ui:

	aqua_wgpuCommandEncoderRelease(state->wgpu_ctx, encoder);

err_create_encoder:

	aqua_wgpuTextureViewRelease(state->wgpu_ctx, frame);

err_create_view:

	aqua_wgpuTextureRelease(state->wgpu_ctx, surf_tex.texture);

	return rv;
}

void ui_wgpu_ez_resize(ui_wgpu_ez_state_t* state, uint32_t x_res, uint32_t y_res) {
	if (state->surface == NULL) {
		LOG_VERBOSE("Surface not yet created, skipping resize.");
		return;
	}

	LOG_VERBOSE("Resizing to %ux%u and (re)configuring surface.", x_res, y_res);

	state->config.device = state->device;
	state->config.usage = WGPUTextureUsage_RenderAttachment;
	state->config.format = state->caps.formats[0];
	state->config.presentMode = WGPUPresentMode_Fifo;
	state->config.alphaMode = state->caps.alphaModes[0];
	state->config.width = x_res;
	state->config.height = y_res;

	aqua_wgpuSurfaceConfigure(state->wgpu_ctx, state->surface, &state->config);
	state->configured = true;
}
