// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2025 Aymeric Wibo

#pragma once

#include "../ui.h"
#include "../wgpu.h"

/**
 * Initialize the WebGPU backend on the UI.
 *
 * You are responsible for setting up and managing the WebGPU context, though you can use the {@link ui_wgpu_ez_setup} helper function to do most of the work and save on boilerplate.
 *
 * @param ui The UI object to initialize the backend on.
 * @param hid The host ID of the WebGPU VDEV.
 * @param vid The VDEV ID of the WebGPU VDEV.
 * @param device The WebGPU device to use.
 * @param queue The WebGPU queue to use.
 * @return 0 on success, or a negative error code on failure.
 */
int ui_wgpu_init(ui_t ui, uint64_t hid, uint64_t vid, WGPUDevice device, WGPUQueue queue);

/**
 * Render the UI using the WebGPU backend.
 *
 * This function should be called in your WebGPU rendering loop, and it will add the commands necessary to render the UI to the given command encoder.
 * You must finish the command encoder yourself and create a command buffer from it with {@link aqua_wgpuCommandEncoderFinish}.
 *
 * @param ui The UI object to render.
 * @param command_encoder The WebGPU command encoder to use for rendering.
 * @return 0 on success, or a negative error code on failure.
 */
int ui_wgpu_render(ui_t ui, WGPUCommandEncoder command_encoder);

/**
 * Easy setup WebGPU UI backend state.
 */
typedef struct {
	/**
	 * The UI object used.
	 */
	ui_t ui;
	/**
	 * The window used.
	 */
	win_t win;
	/**
	 * The WebGPU connection used for all WebGPU calls.
	 */
	wgpu_ctx_t wgpu_ctx;
	/**
	 * The WebGPU instance.
	 */
	WGPUInstance instance;
	/**
	 * The WebGPU surface.
	 *
	 * This is created from the window passed in {@link ui_wgpu_ez_setup}.
	 */
	WGPUSurface surface;
	/**
	 * The WebGPU adapter.
	 */
	WGPUAdapter adapter;
	/**
	 * The WebGPU adapter information.
	 */
	WGPUAdapterInfo adapter_info;
	/**
	 * The WebGPU device.
	 */
	WGPUDevice device;
	/**
	 * The WebGPU queue.
	 */
	WGPUQueue queue;
	/**
	 * The WebGPU surface capabilities.
	 */
	WGPUSurfaceCapabilities caps;
	/**
	 * The WebGPU surface configuration object.
	 */
	WGPUSurfaceConfiguration config;
	/**
	 * Has the surface been configured yet?
	 *
	 * For the surface to be configured, you must call the {@link ui_wgpu_ez_resize} function whenever the window is resized.
	 */
	bool configured;
} ui_wgpu_ez_state_t;

/**
 * Set up a WebGPU UI backend on a window.
 *
 * You may also set up a WebGPU context manually and create a backend on the UI object manually with {@link ui_wgpu_init}.
 *
 * TODO An example here would be nice.
 *
 * @param state The state object to use for the WebGPU backend. This is where all the WebGPU context will be stored.
 * @param ui The UI object to set up the WebGPU backend on.
 * @param win The window to set up the WebGPU backend on.
 * @param wgpu_ctx The WebGPU connection to use.
 * @return 0 on success, or a negative error code on failure.
 */
int ui_wgpu_ez_setup(ui_wgpu_ez_state_t* state, ui_t ui, win_t win, wgpu_ctx_t wgpu_ctx);

/**
 * Full rendering routine for the WebGPU backend.
 *
 * This function should be called in your window's redraw callback.
 * You can also write the rendering code manually, which is useful if you're rendering more than just the UI to the screen.
 *
 * @param state The state object to use for the WebGPU backend.
 * @return 0 on success, or a negative error code on failure.
 */
int ui_wgpu_ez_render(ui_wgpu_ez_state_t* state);

/**
 * Resize the WebGPU backend.
 *
 * This function should be called whenever the window is resized.
 * It will reconfigure the WebGPU surface to match the new window size.
 * This function must be called, otherwise the WebGPU backend will not render anything!
 *
 * @param state The state object to use for the WebGPU backend.
 * @param x_res The new width (i.e. X resolution) of the window.
 * @param y_res The new height (i.e. Y resolution) of the window.
 */
void ui_wgpu_ez_resize(ui_wgpu_ez_state_t* state, uint32_t x_res, uint32_t y_res);
