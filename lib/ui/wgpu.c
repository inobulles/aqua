// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2025 Aymeric Wibo

#include "wgpu.h"

// "Easy" functions.

int ui_wgpu_ez_setup(ui_wgpu_ez_state_t* state, ui_t ui, win_t win, wgpu_ctx_t wgpu_ctx) {
	state->ui = ui;
	state->win = win;
	state->wgpu_ctx = wgpu_ctx;

	// Create instance.

	WGPUInstanceDescriptor const create_instance_descr = {};
	state->instance = aqua_wgpuCreateInstance(wgpu_ctx, &create_instance_descr);

	if (state->instance == NULL) {
		return -1;
	}

	// The surface is set to NULL here.
	// When ui_wgpu_ez_render is called, it will be created.

	state->surface = NULL;

	return 0;
}

static void request_adapter_cb(WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView msg, void* data, void* data2) {
	ui_wgpu_ez_state_t* const state = data;
	(void) data2;

	if (status == WGPURequestAdapterStatus_Success) {
		state->adapter = adapter;
	}

	else {
		(void) msg; // TODO Error message.
	}
}

static void request_device_cb(WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView msg, void* data, void* data2) {
	ui_wgpu_ez_state_t* const state = data;
	(void) data2;

	if (status == WGPURequestDeviceStatus_Success) {
		state->device = device;
	}

	else {
		(void) msg; // TODO Error message.
	}
}

static int setup_surface(ui_wgpu_ez_state_t* state) {
	// Create surface from window.

	state->surface = wgpu_surface_from_win(state->wgpu_ctx, state->instance, state->win);

	if (state->surface == NULL) {
		goto err_create_surface;
	}

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
		goto err_req_adapter;
	}

	aqua_wgpuAdapterGetInfo(state->wgpu_ctx, state->adapter, &state->adapter_info);

	// Get device.

	WGPURequestDeviceCallbackInfo const request_device_cb_info = {
		.callback = request_device_cb,
		.userdata1 = state,
	};

	aqua_wgpuAdapterRequestDevice(state->wgpu_ctx, state->adapter, NULL, request_device_cb_info);

	if (state->device == NULL) {
		goto err_req_device;
	}

	// Get queue from device.

	state->queue = aqua_wgpuDeviceGetQueue(state->wgpu_ctx, state->device);

	if (state->queue == NULL) {
		goto err_get_queue;
	}

	// Create WebGPU backend on UI object.

	uint64_t const hid = wgpu_get_hid(state->wgpu_ctx);
	uint64_t const vid = wgpu_get_vid(state->wgpu_ctx);

	if (ui_wgpu_init(state->ui, hid, vid, state->device, state->queue) < 0) {
		goto err_backend_init;
	}

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

void ui_wgpu_ez_render(ui_wgpu_ez_state_t* state) {
	if (state->surface == NULL) {
		setup_surface(state);
	}
}
