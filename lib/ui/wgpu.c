// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2025 Aymeric Wibo

#include "wgpu.h"

#include <umber.h>
#define UMBER_COMPONENT "aqua.lib.ui.wgpu"

int ui_wgpu_init(ui_t ui, uint64_t hid, uint64_t vid, WGPUDevice device, WGPUQueue queue) {
	(void) ui;
	(void) hid;
	(void) vid;
	(void) device;
	(void) queue;

	// TODO

	return -1;
}

// "Easy" functions.

int ui_wgpu_ez_setup(ui_wgpu_ez_state_t* state, ui_t ui, win_t win, wgpu_ctx_t wgpu_ctx) {
	state->ui = ui;
	state->win = win;
	state->wgpu_ctx = wgpu_ctx;
	state->configured = false;

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

	uint64_t const hid = wgpu_get_hid(state->wgpu_ctx);
	uint64_t const vid = wgpu_get_vid(state->wgpu_ctx);

	LOG_VERBOSE("Creating WebGPU UI backend with WebGPU device %lu:%lu.", hid, vid);

	if (ui_wgpu_init(state->ui, hid, vid, state->device, state->queue) < 0) {
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

	WGPUSurfaceTexture surf_tex;
	aqua_wgpuSurfaceGetCurrentTexture(state->wgpu_ctx, state->surface, &surf_tex);

	// TODO The rest.

	return 0;
}

void ui_wgpu_ez_resize(ui_wgpu_ez_state_t* state, uint32_t x_res, uint32_t y_res) {
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
