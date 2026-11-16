// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2026 Aymeric Wibo

#include "buf_fmt.h"

#include <wlr/types/wlr_linux_dmabuf_v1.h>
#include <wlr/types/wlr_shm.h>

#include <drm_fourcc.h>

// Formats clients may use for their buffers.
// Client images are imported into WebGPU as BGRA8Unorm (VK_FORMAT_B8G8R8A8_UNORM) for now, so only advertise the DRM formats which correspond to that.
// Otherwise, clients might pick e.g. a 10-bit format (Blender does with ARGB2101010), which then gets interpreted as BGRA8 and has completely wrong colours.
// TODO In the future, we want to support all the formats that can be imported into WebGPU, and communicate what format each surface wants to the renderer.

static uint32_t const BUF_FMTS[] = {
	DRM_FORMAT_ARGB8888,
	DRM_FORMAT_XRGB8888,
};

#define BUF_FMT_COUNT (sizeof BUF_FMTS / sizeof *BUF_FMTS)

static int restrict_fmts(struct wlr_drm_format_set* set) {
	struct wlr_drm_format_set restricted = {0};

	for (size_t i = 0; i < BUF_FMT_COUNT; i++) {
		struct wlr_drm_format const* const fmt = wlr_drm_format_set_get(set, BUF_FMTS[i]);

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

	if (wlr_shm_create(display, 2, BUF_FMTS, BUF_FMT_COUNT) == NULL) {
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
