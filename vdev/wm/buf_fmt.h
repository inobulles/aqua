// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2026 Aymeric Wibo

#pragma once

#include <umber.h>

#include <wayland-server-core.h>

#define WLR_USE_UNSTABLE 1

#include <wlr/render/vulkan.h>
#include <wlr/render/wlr_renderer.h>

#include <webgpu/webgpu.h>

/**
 * Initialize the buffer factory protocols (SHM and DMA-BUF).
 *
 * This is the equivalent of wlr_renderer_init_wl_display(), except clients are only offered buffer formats which we can import into WebGPU.
 *
 * @param cls Logging class.
 * @param display Wayland display to create the globals on.
 * @param renderer Renderer to get the supported DMA-BUF formats and modifiers from.
 * @return 0 on success, -1 on failure.
 */
int wm_vdev_init_buf_protocols(umber_class_t const* cls, struct wl_display* display, struct wlr_renderer* renderer);

/**
 * Get the WebGPU format which a client image with a given Vulkan format should be imported as.
 *
 * @param vk Vulkan format of the image, as given by wlr_vk_texture_get_image_attribs().
 * @return The corresponding WebGPU format, or WGPUTextureFormat_Undefined if the format isn't supported.
 */
WGPUTextureFormat wm_vdev_buf_fmt_vk_to_wgpu(VkFormat vk);
