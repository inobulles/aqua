// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2026 Aymeric Wibo

#include "buf_fmt.h"

#include <wlr/types/wlr_linux_dmabuf_v1.h>
#include <wlr/types/wlr_shm.h>

#include <drm_fourcc.h>

typedef struct {
	uint32_t drm;           // DRM fourcc, which is what's advertised to clients.
	VkFormat vk;            // Vulkan format wlroots' Vulkan renderer creates images of this DRM format with.
	WGPUTextureFormat wgpu; // WebGPU format the WM should import those images as.
} buf_fmt_t;

// Formats clients may use for their buffers.
// See render/vulkan/pixel_format.c in wlroots for its DRM to Vulkan mapping.

static buf_fmt_t const BUF_FMTS[] = {
	{DRM_FORMAT_ARGB8888, VK_FORMAT_B8G8R8A8_UNORM, WGPUTextureFormat_BGRA8Unorm},
	{DRM_FORMAT_XRGB8888, VK_FORMAT_B8G8R8A8_UNORM, WGPUTextureFormat_BGRA8Unorm},
	{DRM_FORMAT_ABGR8888, VK_FORMAT_R8G8B8A8_UNORM, WGPUTextureFormat_RGBA8Unorm},
	{DRM_FORMAT_XBGR8888, VK_FORMAT_R8G8B8A8_UNORM, WGPUTextureFormat_RGBA8Unorm},
	{DRM_FORMAT_ABGR2101010, VK_FORMAT_A2B10G10R10_UNORM_PACK32, WGPUTextureFormat_RGB10A2Unorm},
	{DRM_FORMAT_XBGR2101010, VK_FORMAT_A2B10G10R10_UNORM_PACK32, WGPUTextureFormat_RGB10A2Unorm},
	{DRM_FORMAT_ABGR16161616F, VK_FORMAT_R16G16B16A16_SFLOAT, WGPUTextureFormat_RGBA16Float},
	{DRM_FORMAT_XBGR16161616F, VK_FORMAT_R16G16B16A16_SFLOAT, WGPUTextureFormat_RGBA16Float},
	// WebGPU does not support ARGB2101010/XRGB2101010.
};

#define BUF_FMT_COUNT (sizeof BUF_FMTS / sizeof *BUF_FMTS)

static int restrict_fmts(struct wlr_drm_format_set* set) {
	struct wlr_drm_format_set restricted = {0};

	for (size_t i = 0; i < BUF_FMT_COUNT; i++) {
		struct wlr_drm_format const* const fmt = wlr_drm_format_set_get(set, BUF_FMTS[i].drm);

		if (fmt == NULL) {
			continue;
		}

		for (size_t j = 0; j < fmt->len; j++) {
			if (!wlr_drm_format_set_add(&restricted, fmt->format, fmt->modifiers[j])) {
				wlr_drm_format_set_finish(&restricted);
				return -1;
			}
		}
	}

	wlr_drm_format_set_finish(set);
	*set = restricted;

	return 0;
}

int wm_vdev_init_buf_protocols(umber_class_t const* cls, struct wl_display* display, struct wlr_renderer* renderer) {
	// SHM.

	uint32_t shm_fmts[BUF_FMT_COUNT];

	for (size_t i = 0; i < BUF_FMT_COUNT; i++) {
		shm_fmts[i] = BUF_FMTS[i].drm;
	}

	if (wlr_shm_create(display, 2, shm_fmts, BUF_FMT_COUNT) == NULL) {
		LOG_E(cls, "Failed to create SHM global.");
		return -1;
	}

	// DMA-BUF.

	if (wlr_renderer_get_texture_formats(renderer, WLR_BUFFER_CAP_DMABUF) == NULL || wlr_renderer_get_drm_fd(renderer) < 0) {
		LOG_W(cls, "Renderer doesn't support DMA-BUF; clients will only be able to use SHM.");
		return 0;
	}

	struct wlr_linux_dmabuf_feedback_v1_init_options const options = {
		.main_renderer = renderer,
	};

	struct wlr_linux_dmabuf_feedback_v1 feedback = {0};

	if (!wlr_linux_dmabuf_feedback_v1_init_with_options(&feedback, &options)) {
		LOG_E(cls, "Failed to create DMA-BUF feedback.");
		return -1;
	}

	struct wlr_linux_dmabuf_feedback_v1_tranche* tranche;
	int rv = 0;

	wl_array_for_each(tranche, &feedback.tranches) {
		if (restrict_fmts(&tranche->formats) < 0) {
			LOG_E(cls, "Failed to restrict DMA-BUF formats.");
			rv = -1;
			break;
		}
	}

	if (rv == 0 && wlr_linux_dmabuf_v1_create(display, 4, &feedback) == NULL) {
		LOG_E(cls, "Failed to create DMA-BUF global.");
		rv = -1;
	}

	wlr_linux_dmabuf_feedback_v1_finish(&feedback);
	return rv;
}

WGPUTextureFormat wm_vdev_buf_fmt_vk_to_wgpu(VkFormat vk) {
	for (size_t i = 0; i < BUF_FMT_COUNT; i++) {
		if (BUF_FMTS[i].vk == vk) {
			return BUF_FMTS[i].wgpu;
		}
	}

	return WGPUTextureFormat_Undefined;
}
