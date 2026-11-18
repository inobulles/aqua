// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025-2026 Aymeric Wibo

// A lot of this code is based on the tinywl reference compositor and the "simple.c" example:
// https://gitlab.freedesktop.org/wlroots/wlroots/-/blob/master/tinywl
// https://gitlab.freedesktop.org/wlroots/wlroots/-/blob/master/examples/simple.c

#include "wm.h"
#include "buf_fmt.h"

#include <aqua/vdriver.h>

#include <umber.h>

#include <wlr/backend/wayland.h>
#include <wlr/render/drm_syncobj.h>
#include <wlr/render/interface.h>
#include <wlr/render/swapchain.h>
#include <wlr/render/vulkan.h>
#include <wlr/util/edges.h>

#include <webgpu/webgpu.h>
#include <webgpu/wgpu.h>

#include <assert.h>
#include <stdlib.h>

#define NESTED_X_RES 1920
#define NESTED_Y_RES 1080

typedef struct {
	struct wl_list link;

	wm_t* wm;
	struct wlr_output* output;

	struct wlr_drm_syncobj_timeline* timeline;
	uint64_t timeline_point;

	struct wl_listener frame;
	struct wl_listener destroy;
} output_t;

typedef struct {
	struct wl_list link;

	wm_t* wm;
	struct wlr_buffer* buf;
	VkImage image;

	struct wl_listener destroy;
} image_t;

typedef struct {
	wm_t* wm;
	struct wlr_surface* surf;

	struct wl_listener commit;
	struct wl_listener destroy;
} surf_t;

typedef struct __attribute__((packed)) {
	intr_t intr;
	uint64_t raw_vk_image;
	uint32_t x_res;
	uint32_t y_res;
} redraw_intr_t;

typedef struct __attribute__((packed)) {
	intr_t intr;
	uint64_t win;

	// TODO This is bad.

	char const* app_id;
} new_win_intr_t;

typedef struct __attribute__((packed)) {
	intr_t intr;
	uint64_t win;
} destroy_win_intr_t;

typedef struct __attribute__((packed)) {
	int32_t x, y;
	uint32_t x_res, y_res;
	uint64_t raw_image;
	uint32_t format; // WGPUTextureFormat
	uint8_t opaque;  // Necessary for X* formats, where the alpha channel is just padding and shouldn't be displayed transparently.
} redraw_win_surf_t;

typedef struct __attribute__((packed)) {
	intr_t intr;
	uint64_t win;
	uint32_t x_res;
	uint32_t y_res;
	uint32_t surf_count;
	redraw_win_surf_t surfs[];
} redraw_win_intr_t;

typedef struct __attribute__((packed)) {
	intr_t intr;
	uint64_t raw_image;
} destroy_image_intr_t;

static umber_class_t const* cls = NULL;
static umber_class_t const* cls_wlr = NULL;

void wm_vdev_init(umber_class_t const* _cls, umber_class_t const* _cls_wlr) {
	cls = _cls;
	cls_wlr = _cls_wlr;

	wm_vdev_input_init(_cls);
}

void wm_vdev_interrupt(wm_t* wm, size_t data_size, void const* data) {
	if (!wm->has_ino) {
		return;
	}

	kos_notif_t const notif = {
		.kind = KOS_NOTIF_INTERRUPT,
		.conn_id = 0,
		.cookie = 0,
		.interrupt = {
			.ino = wm->ino,
			.data_size = data_size,
			.data = data,
		},
	};

	VDRIVER.notif_cb(&notif, VDRIVER.notif_data);
}

static void image_free(image_t* image) {
	wl_list_remove(&image->link);
	wl_list_remove(&image->destroy.link);

	free(image);
}

static void image_destroy(struct wl_listener* listener, void* data) {
	image_t* const image = wl_container_of(listener, image, destroy);

	(void) data;

	LOG_V(cls, "Image %p is about to be destroyed.", (void*) image->image);

	destroy_image_intr_t const intr = {
		.intr = INTR_DESTROY_IMAGE,
		.raw_image = (uintptr_t) image->image,
	};

	wm_vdev_interrupt(image->wm, sizeof intr, &intr);
	image_free(image);
}

static void track_image(wm_t* wm, struct wlr_buffer* buf, VkImage vk_image) {
	image_t* image;

	wl_list_for_each(image, &wm->images, link) {
		if (image->buf == buf) {
			assert(image->image == vk_image);
			return;
		}
	}

	image = calloc(1, sizeof *image);
	assert(image != NULL);

	image->wm = wm;
	image->buf = buf;
	image->image = vk_image;

	image->destroy.notify = image_destroy;
	wl_signal_add(&buf->events.destroy, &image->destroy);

	wl_list_insert(&wm->images, &image->link);
}

static void output_remove_notify(struct wl_listener* listener, void* data) {
	(void) listener;
	(void) data;

	output_t* const output = wl_container_of(listener, output, destroy);

	wl_list_remove(&output->frame.link);
	wl_list_remove(&output->destroy.link);

	if (output->timeline != NULL) {
		wlr_drm_syncobj_timeline_unref(output->timeline);
	}

	// When running nested, the output is destroyed when its window is closed.

	if (wlr_output_is_wl(output->output)) {
		wl_display_terminate(output->wm->display);
	}

	free(output);
}

static void begin_external_iter(struct wlr_surface* surf, int sx, int sy, void* data) {
	struct wlr_texture* const tex = wlr_surface_get_texture(surf);

	(void) sx, (void) sy;
	(void) data;

	if (tex == NULL) {
		return;
	}

	struct wlr_vk_image_attribs tex_attribs;

	if (!wlr_vk_texture_begin_external(tex, &tex_attribs)) {
		LOG_E(cls, "Failed to hand over surface texture to custom renderer.");
	}
}

static void send_frame_done_iter(struct wlr_surface* surf, int sx, int sy, void* data) {
	struct timespec const* const now = data;

	(void) sx;
	(void) sy;

	wlr_surface_send_frame_done(surf, now);
}

static void output_frame_notify(struct wl_listener* listener, void* data) {
	(void) data;

	output_t* const output = wl_container_of(listener, output, frame);

	wm_t* const wm = output->wm;
	struct wlr_output* const wlr_output = output->output;

	LOG_V(cls, "Initialize output state.");

	struct wlr_output_state state;
	wlr_output_state_init(&state);

	LOG_V(cls, "Acquire swapchain image.");

	if (!wlr_output_configure_primary_swapchain(wlr_output, &state, &wlr_output->swapchain)) {
		LOG_E(cls, "Failed to create or configure primary swapchain.");
		return;
	}

	struct wlr_buffer* const buf = wlr_swapchain_acquire(wlr_output->swapchain);

	if (buf == NULL) {
		LOG_E(cls, "Failed to acquire swapchain image.");
		return;
	}

	LOG_V(cls, "Take care of rendering.");

	struct wlr_vk_image_attribs attribs;

	if (!wlr_vk_buffer_begin_external(wm->wlr_renderer, buf, &attribs)) {
		LOG_E(cls, "Failed to hand swapchain image over to custom renderer.");

		wlr_buffer_unlock(buf);
		wlr_output_state_finish(&state);

		return;
	}

	track_image(wm, buf, attribs.image);

	// Hand over window textures to custom renderer for this frame.
	// This submits any pending uploads to them, and makes sure wlroots doesn't destroy them until our drawing code is done with them.

	toplevel_t* win;

	wl_list_for_each(win, &wm->toplevels, link) {
		wlr_surface_for_each_surface(win->xdg_toplevel->base->surface, begin_external_iter, NULL);
	}

	redraw_intr_t intr = {
		.intr = INTR_REDRAW,
		.raw_vk_image = (uintptr_t) attribs.image,
		.x_res = output->output->width,
		.y_res = output->output->height,
	};

	wm_vdev_interrupt(wm, sizeof intr, &intr);

	output->timeline_point++;
	bool const ended = wlr_vk_renderer_end_external(wm->wlr_renderer, output->timeline, output->timeline_point);

	if (!ended) {
		LOG_E(cls, "Failed to take swapchain image and textures back from custom renderer.");
	}

	LOG_V(cls, "Set swapchain image to output and release it.");

	wlr_output_state_set_buffer(&state, buf);

	// If ending fails, the timeline point will never be signalled, so don't wait.

	if (ended && output->timeline != NULL) {
		wlr_output_state_set_wait_timeline(&state, output->timeline, output->timeline_point);
	}

	wlr_buffer_unlock(buf);

	LOG_V(cls, "Commit output state.");

	wlr_output_commit_state(wlr_output, &state);
	wlr_output_state_finish(&state);

	// Let clients know they can draw their next frame.
	// We don't use wlr_scene_output_send_frame_done() for this as that depends on the scene graph, which the app doesn't update (so all windows would be at (0, 0) on top of eachother).
	// TODO We should really only be doing this for the surfaces that are visible. Those completely obscured should not be updated, and we should have an API for this from the app.

	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);

	wl_list_for_each(win, &wm->toplevels, link) {
		wlr_surface_for_each_surface(win->xdg_toplevel->base->surface, send_frame_done_iter, &now);
	}
}

static void new_output(struct wl_listener* listener, void* data) {
	struct wlr_output* const wlr_output = data;
	wm_t* const wm = wl_container_of(listener, wm, new_output);

	LOG_V(cls, "New output found, configure.");
	wlr_output_init_render(wlr_output, wm->allocator, wm->wlr_renderer);

	LOG_V(cls, "Enabling output.");

	struct wlr_output_state state;
	wlr_output_state_init(&state);
	wlr_output_state_set_enabled(&state, true);

	LOG_V(cls, "Set mode of output to preferred mode.");

	// TODO In fine, this mode should be selectable.

	struct wlr_output_mode* const mode = wlr_output_preferred_mode(wlr_output);

	if (mode != NULL) {
		wlr_output_state_set_mode(&state, mode);
	}

	else if (wlr_output_is_wl(wlr_output)) {
		wlr_output_state_set_custom_mode(&state, NESTED_X_RES, NESTED_Y_RES, 0);
	}

	LOG_V(cls, "Apply new output state atomically.");

	wlr_output_commit_state(wlr_output, &state);
	wlr_output_state_finish(&state);

	LOG_V(cls, "Set up state for output.");

	output_t* const output = calloc(1, sizeof *output);
	assert(output != NULL);

	output->wm = wm;
	output->output = wlr_output;

	int const drm_fd = wlr_backend_get_drm_fd(wlr_output->backend);

	if (drm_fd >= 0 && wlr_output->backend->features.timeline && wm->wlr_renderer->features.timeline) {
		LOG_V(cls, "Output supports explicit sync, create timeline.");
		output->timeline = wlr_drm_syncobj_timeline_create(drm_fd);

		if (output->timeline == NULL) {
			LOG_W(cls, "Failed to create timeline, falling back to implicit sync.");
		}
	}

	LOG_V(cls, "Set up listeners (frame, destroy) for output.");

	wl_signal_add(&wlr_output->events.frame, &output->frame);
	output->frame.notify = output_frame_notify;

	wl_signal_add(&wlr_output->events.destroy, &output->destroy);
	output->destroy.notify = output_remove_notify;

	LOG_V(cls, "Add output to output layout.");

	// TODO In fine, the output layout should be configurable.

	struct wlr_output_layout_output* output_layout = wlr_output_layout_add_auto(wm->output_layout, wlr_output);
	struct wlr_scene_output* scene_output = wlr_scene_output_create(wm->scene, wlr_output);
	wlr_scene_output_layout_add_output(wm->scene_layout, output_layout, scene_output);
}

void wm_vdev_focus_toplevel(toplevel_t* toplevel) {
	wm_t* const wm = toplevel->wm;

	struct wlr_surface* const prev_surf = wm->seat->keyboard_state.focused_surface;
	struct wlr_surface* const surf = toplevel->xdg_toplevel->base->surface;

	// Don't refocus already focused surface.

	if (prev_surf == surf) {
		return;
	}

	// Deactivate the previously focused surface. This lets the client know it no longer has focus and the client will repaint accordingly, e.g. stop displaying a caret.

	if (prev_surf != NULL) {
		struct wlr_xdg_toplevel* const prev_toplevel = wlr_xdg_toplevel_try_from_wlr_surface(prev_surf);

		if (prev_toplevel != NULL) {
			wlr_xdg_toplevel_set_activated(prev_toplevel, false);
		}
	}

	struct wlr_keyboard* const kbd = wlr_seat_get_keyboard(wm->seat);

	// Move the toplevel to the front.

	wlr_scene_node_raise_to_top(&toplevel->scene_tree->node);
	wl_list_remove(&toplevel->link);
	wl_list_insert(&wm->toplevels, &toplevel->link);

	// Activate the new surface.

	wlr_xdg_toplevel_set_activated(toplevel->xdg_toplevel, true);

	// Tell the seat to have the keyboard enter this surface.
	// wlroots will keep track of this and automatically send key events to the appropriate clients without additional work on the WM's part.

	if (kbd != NULL) {
		wlr_seat_keyboard_notify_enter(wm->seat, surf, kbd->keycodes, kbd->num_keycodes, &kbd->modifiers);
	}

	wm_vdev_update_pointer_constraint(wm);
}

static void toplevel_map(struct wl_listener* listener, void* data) {
	toplevel_t* const toplevel = wl_container_of(listener, toplevel, map);
	wm_t* const wm = toplevel->wm;

	(void) data;

	LOG_V(cls, "Map toplevel %p (app_id=%s).", toplevel->xdg_toplevel, toplevel->xdg_toplevel->app_id);

	wl_list_insert(&wm->toplevels, &toplevel->link);
	wm_vdev_focus_toplevel(toplevel);

	// Send interrupt.

	new_win_intr_t const intr = {
		.intr = INTR_NEW_WIN,
		.win = (uint64_t) (uintptr_t) toplevel,
		.app_id = toplevel->xdg_toplevel->app_id,
	};

	wm_vdev_interrupt(wm, sizeof intr, &intr);
}

static void toplevel_unmap(struct wl_listener* listener, void* data) {
	toplevel_t* const toplevel = wl_container_of(listener, toplevel, unmap);
	wm_t* const wm = toplevel->wm;

	(void) data;

	LOG_V(cls, "Unmap %s.", toplevel->xdg_toplevel->app_id);

	wl_list_remove(&toplevel->link);

	// If this toplevel had focus, give it to the next one.

	struct wlr_surface* const surf = toplevel->xdg_toplevel->base->surface;

	if (wm->seat->keyboard_state.focused_surface == surf) {
		wlr_seat_keyboard_notify_clear_focus(wm->seat);

		if (!wl_list_empty(&wm->toplevels)) {
			toplevel_t* const next = wl_container_of(wm->toplevels.next, next, link);
			wm_vdev_focus_toplevel(next);
		}
	}

	// Send interrupt.

	destroy_win_intr_t const intr = {
		.intr = INTR_DESTROY_WIN,
		.win = (uint64_t) (uintptr_t) toplevel,
	};

	wm_vdev_interrupt(wm, sizeof intr, &intr);
}

typedef struct {
	toplevel_t* toplevel;
	struct wlr_box geom;
	redraw_win_intr_t* intr;
} redraw_win_ctx_t;

static void redraw_win_iter(struct wlr_surface* surf, int sx, int sy, void* data) {
	redraw_win_ctx_t* const ctx = data;
	struct wlr_texture* const tex = wlr_surface_get_texture(surf);

	if (tex == NULL || surf->buffer == NULL) {
		return;
	}

	struct wlr_vk_image_attribs attribs;
	wlr_vk_texture_get_image_attribs(tex, &attribs);

	// We only advertise formats we can import, so this shouldn't happen.

	WGPUTextureFormat const format = wm_vdev_buf_fmt_vk_to_wgpu(attribs.format);

	if (format == WGPUTextureFormat_Undefined) {
		LOG_E(cls, "Surface image has unsupported VkFormat %d; not drawing it.", attribs.format);
		return;
	}

	track_image(ctx->toplevel->wm, &surf->buffer->base, attribs.image);

	ctx->intr = realloc(ctx->intr, sizeof *ctx->intr + (ctx->intr->surf_count + 1) * sizeof *ctx->intr->surfs);
	assert(ctx->intr != NULL);

	ctx->intr->surfs[ctx->intr->surf_count++] = (redraw_win_surf_t) {
		.x = sx - ctx->geom.x,
		.y = sy - ctx->geom.y,
		.x_res = tex->width,
		.y_res = tex->height,
		.raw_image = (uintptr_t) attribs.image,
		.format = format,
		.opaque = !wlr_vk_texture_has_alpha(tex),
	};
}

static void redraw_win(toplevel_t* toplevel) {
	wm_t* const wm = toplevel->wm;
	struct wlr_xdg_surface* const base = toplevel->xdg_toplevel->base;

	redraw_win_ctx_t ctx = {
		.toplevel = toplevel,
		.geom = base->geometry,
		.intr = calloc(1, sizeof *ctx.intr),
	};

	assert(ctx.intr != NULL);

	// Surfaces are iterated back to front, and positions are relative to the window geometry.

	wlr_surface_for_each_surface(base->surface, redraw_win_iter, &ctx);

	LOG_V(cls, "Redraw %s: %u surfaces.", toplevel->xdg_toplevel->app_id, ctx.intr->surf_count);

	if (ctx.intr->surf_count > 0) {
		ctx.intr->intr = INTR_REDRAW_WIN;
		ctx.intr->win = (uintptr_t) toplevel;
		ctx.intr->x_res = ctx.geom.width;
		ctx.intr->y_res = ctx.geom.height;

		wm_vdev_interrupt(wm, sizeof *ctx.intr + ctx.intr->surf_count * sizeof *ctx.intr->surfs, ctx.intr);
	}

	free(ctx.intr);
}

static void surf_commit(struct wl_listener* listener, void* data) {
	surf_t* const surf = wl_container_of(listener, surf, commit);

	(void) data;

	struct wlr_surface* const root = wlr_surface_get_root_surface(surf->surf);
	struct wlr_xdg_toplevel* const xdg_toplevel = wlr_xdg_toplevel_try_from_wlr_surface(root);

	if (xdg_toplevel == NULL || !root->mapped) {
		return;
	}

	redraw_win(xdg_toplevel->base->data);
}

static void surf_destroy(struct wl_listener* listener, void* data) {
	surf_t* const surf = wl_container_of(listener, surf, destroy);

	(void) data;

	wl_list_remove(&surf->commit.link);
	wl_list_remove(&surf->destroy.link);

	free(surf);
}

static void new_surf(struct wl_listener* listener, void* data) {
	wm_t* const wm = wl_container_of(listener, wm, new_surf);
	struct wlr_surface* const wlr_surf = data;

	surf_t* const surf = calloc(1, sizeof *surf);
	assert(surf != NULL);

	surf->wm = wm;
	surf->surf = wlr_surf;

	surf->commit.notify = surf_commit;
	wl_signal_add(&wlr_surf->events.commit, &surf->commit);

	surf->destroy.notify = surf_destroy;
	wl_signal_add(&wlr_surf->events.destroy, &surf->destroy);
}

static void set_ssd(toplevel_t* toplevel) {
	struct wlr_xdg_toplevel_decoration_v1* deco;

	wl_list_for_each(deco, &toplevel->wm->xdg_deco_mgr->decorations, link) {
		if (deco->toplevel == toplevel->xdg_toplevel) {
			wlr_xdg_toplevel_decoration_v1_set_mode(deco, WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
		}
	}

	// This helps convince some more annoying clients (e.g. Firefox) not to draw CSD.

	wlr_xdg_toplevel_set_tiled(toplevel->xdg_toplevel, WLR_EDGE_TOP | WLR_EDGE_BOTTOM | WLR_EDGE_LEFT | WLR_EDGE_RIGHT);
}

static void toplevel_commit(struct wl_listener* listener, void* data) {
	toplevel_t* const toplevel = wl_container_of(listener, toplevel, commit);
	struct wlr_xdg_toplevel* xdg_toplevel = toplevel->xdg_toplevel;

	(void) data;

	LOG_V(cls, "Commit %s.", xdg_toplevel->app_id);

	if (toplevel->xdg_toplevel->base->initial_commit) {
		LOG_V(cls, "Initial commit for %s; setting size to (0, 0) so client can pick size.", xdg_toplevel->app_id);
		wlr_xdg_toplevel_set_size(toplevel->xdg_toplevel, 0, 0);
		set_ssd(toplevel);
	}
}

static void toplevel_destroy(struct wl_listener* listener, void* data) {
	toplevel_t* const toplevel = wl_container_of(listener, toplevel, destroy);

	(void) data;

	LOG_V(cls, "Destroying toplevel.");

	wl_list_remove(&toplevel->map.link);
	wl_list_remove(&toplevel->unmap.link);
	wl_list_remove(&toplevel->commit.link);
	wl_list_remove(&toplevel->destroy.link);

	free(toplevel);
}

static void new_xdg_toplevel(struct wl_listener* listener, void* data) {
	wm_t* const wm = wl_container_of(listener, wm, new_xdg_toplevel);
	struct wlr_xdg_toplevel* const xdg_toplevel = data;

	LOG_V(cls, "New XDG toplevel (app_id=%s).", xdg_toplevel->app_id);

	toplevel_t* const toplevel = calloc(1, sizeof *toplevel);
	assert(toplevel != NULL);

	toplevel->wm = wm;
	toplevel->xdg_toplevel = xdg_toplevel;

	LOG_V(cls, "Add node for this XDG toplevel's surface to scene graph.");

	toplevel->scene_tree = wlr_scene_xdg_surface_create(&wm->scene->tree, xdg_toplevel->base);
	xdg_toplevel->base->data = toplevel;

	LOG_V(cls, "Listen to XDG toplevel events.");

	toplevel->map.notify = toplevel_map;
	wl_signal_add(&xdg_toplevel->base->surface->events.map, &toplevel->map);

	toplevel->unmap.notify = toplevel_unmap;
	wl_signal_add(&xdg_toplevel->base->surface->events.unmap, &toplevel->unmap);

	toplevel->commit.notify = toplevel_commit;
	wl_signal_add(&xdg_toplevel->base->surface->events.commit, &toplevel->commit);

	toplevel->destroy.notify = toplevel_destroy;
	wl_signal_add(&xdg_toplevel->events.destroy, &toplevel->destroy);
}

static void new_xdg_deco(struct wl_listener* listener, void* data) {
	struct wlr_xdg_toplevel_decoration_v1* const deco = data;

	(void) listener;

	if (deco->toplevel->base->initialized) {
		wlr_xdg_toplevel_decoration_v1_set_mode(deco, WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
	}
}

static void new_popup(struct wl_listener* listener, void* data) {
	(void) listener;
	(void) data;

	LOG_F(cls, "TODO: %s", __func__);
}

void wm_vdev_toplevel_set_size(toplevel_t* toplevel, uint32_t x_res, uint32_t y_res) {
	LOG_V(cls, "Set size of %s to %ux%u.", toplevel->xdg_toplevel->app_id, x_res, y_res);
	wlr_xdg_toplevel_set_size(toplevel->xdg_toplevel, x_res, y_res);
}

void wm_vdev_toplevel_set_resizing(toplevel_t* toplevel, bool resizing) {
	// Lets the client know it's being interactively resized, so it can e.g. avoid doing expensive work on each resize.

	wlr_xdg_toplevel_set_resizing(toplevel->xdg_toplevel, resizing);
}

static void request_set_selection(struct wl_listener* listener, void* data) {
	wm_t* const wm = wl_container_of(listener, wm, request_set_selection);
	struct wlr_seat_request_set_selection_event* const event = data;

	wlr_seat_set_selection(wm->seat, event->source, event->serial);
}

static void wlr_log_cb(enum wlr_log_importance importance, char const* fmt, va_list args) {
	char* msg;
	vasprintf(&msg, fmt, args);

	if (msg == NULL) {
		LOG_F(cls, "vasprintf failed");
		return;
	}

	// If the importance is anything other than the three here, we consider that necessarily abnormal behaviour, so we log it as a bona fide error.
	// A WLR_ERROR could just be a warning, so we log it as a warning, and hope that if it truly is an error, it will be logged elsewhere.

	umber_lvl_t lvl = UMBER_LVL_ERROR;

	if (importance == WLR_DEBUG) {
		lvl = UMBER_LVL_VERBOSE;
	}

	else if (importance == WLR_INFO) {
		lvl = UMBER_LVL_INFO;
	}

	else if (importance == WLR_ERROR) {
		lvl = UMBER_LVL_WARN;
	}

	umber_log(cls_wlr, lvl, "wlroots", 69, msg);
	free(msg);
}

static bool wlr_logging_set_up = false;

static void setup_logging(void) {
	if (wlr_logging_set_up) {
		return;
	}

	wlr_log_init(WLR_DEBUG, wlr_log_cb);
	wlr_logging_set_up = true;
}

wm_t* wm_vdev_create(void) {
	LOG_V(cls, "Creating WM.");

	wm_t* const wm = malloc(sizeof *wm);
	assert(wm != NULL);

	wm->has_ino = false;

	setup_logging();

#define FAIL(...)              \
	do {                        \
		LOG_E(cls, __VA_ARGS__); \
		wm_vdev_destroy(wm);     \
		return NULL;             \
	} while (0)

	LOG_V(cls, "Creating the Wayland display.");
	wm->display = wl_display_create();

	if (wm->display == NULL) {
		FAIL("Failed to create Wayland display.");
	}

	LOG_V(cls, "Getting the Wayland event loop.");
	wm->event_loop = wl_display_get_event_loop(wm->display);

	LOG_V(cls, "Creating appropriate backend.");
	wm->backend = wlr_backend_autocreate(wm->event_loop, NULL);

	if (wm->backend == NULL) {
		FAIL("Failed to create backend.");
	}

	LOG_V(cls, "Creating Vulkan renderer.");

	setenv("WLR_RENDERER", "vulkan", true);
	wm->wlr_renderer = wlr_renderer_autocreate(wm->backend);

	if (wm->wlr_renderer == NULL) {
		FAIL("Failed to create renderer.");
	}

	wm->public.vk_instance = wlr_vk_renderer_get_instance(wm->wlr_renderer);
	wm->public.vk_phys_dev = wlr_vk_renderer_get_physical_device(wm->wlr_renderer);
	wm->public.vk_dev = wlr_vk_renderer_get_device(wm->wlr_renderer);
	wm->public.vk_queue_family = wlr_vk_renderer_get_queue_family(wm->wlr_renderer);

	LOG_V(cls, "Initialize buffer factory protocols.");

	if (wm_vdev_init_buf_protocols(cls, wm->display, wm->wlr_renderer) < 0) {
		FAIL("Failed to initialize buffer factory protocols.");
	}

	LOG_V(cls, "Creating wlroots allocator.");
	wm->allocator = wlr_allocator_autocreate(wm->backend, wm->wlr_renderer);

	if (wm->allocator == NULL) {
		FAIL("Failed to create wlroots allocator.");
	}

	LOG_V(cls, "Add listener for when new outputs are available.");

	wl_list_init(&wm->outputs);
	wm->new_output.notify = new_output;
	wl_signal_add(&wm->backend->events.new_output, &wm->new_output);

	LOG_V(cls, "Add listener for when new input methods are available.");

	wl_list_init(&wm->keyboards);
	wl_list_init(&wm->mice);

	wm->new_input.notify = wm_vdev_new_input;
	wl_signal_add(&wm->backend->events.new_input, &wm->new_input);

	LOG_V(cls, "Creating compositor (version 5).");
	struct wlr_compositor* const compositor = wlr_compositor_create(wm->display, 5, wm->wlr_renderer);

	if (compositor == NULL) {
		FAIL("Failed to create compositor.");
	}

	wm->new_surf.notify = new_surf;
	wl_signal_add(&compositor->events.new_surface, &wm->new_surf);

	LOG_V(cls, "Creating subcompositor.");

	if (wlr_subcompositor_create(wm->display) == NULL) {
		FAIL("Failed to create subcompositor.");
	}

	LOG_V(cls, "Creating data device manager.");

	if (wlr_data_device_manager_create(wm->display) == NULL) {
		FAIL("Failed to create data device manager.");
	}

	LOG_V(cls, "Creating output layout.");
	wm->output_layout = wlr_output_layout_create(wm->display);

	if (wm->output_layout == NULL) {
		FAIL("Failed to create output layout.");
	}

	LOG_V(cls, "Create scene graph.");
	wm->scene = wlr_scene_create();

	if (wm->scene == NULL) {
		FAIL("Failed to create scene graph.");
	}

	LOG_V(cls, "Create scene layout.");
	wm->scene_layout = wlr_scene_attach_output_layout(wm->scene, wm->output_layout);

	if (wm->scene_layout == NULL) {
		FAIL("Failed to create scene layout.");
	}

	LOG_V(cls, "Set up XDG-shell (version 3).");

	wl_list_init(&wm->toplevels);
	wl_list_init(&wm->images);
	wm->xdg_shell = wlr_xdg_shell_create(wm->display, 3);

	if (wm->xdg_shell == NULL) {
		FAIL("Failed to create XDG-shell.");
	}

	LOG_V(cls, "Add listener for when new XDG toplevels or popups are available");

	wm->new_xdg_toplevel.notify = new_xdg_toplevel;
	wl_signal_add(&wm->xdg_shell->events.new_toplevel, &wm->new_xdg_toplevel);

	wm->new_xdg_popup.notify = new_popup;
	wl_signal_add(&wm->xdg_shell->events.new_popup, &wm->new_xdg_popup);

	LOG_V(cls, "Tell clients we draw decorations ourselves (KDE protocol for GTK, xdg-decoration for everyone else).");

	struct wlr_server_decoration_manager* const kde_deco_mgr = wlr_server_decoration_manager_create(wm->display);

	if (kde_deco_mgr == NULL) {
		FAIL("Failed to create KDE server decoration manager.");
	}

	wlr_server_decoration_manager_set_default_mode(kde_deco_mgr, WLR_SERVER_DECORATION_MANAGER_MODE_SERVER);
	wm->xdg_deco_mgr = wlr_xdg_decoration_manager_v1_create(wm->display, 1);

	if (wm->xdg_deco_mgr == NULL) {
		FAIL("Failed to create XDG decoration manager.");
	}

	wm->new_xdg_deco.notify = new_xdg_deco;
	wl_signal_add(&wm->xdg_deco_mgr->events.new_toplevel_decoration, &wm->new_xdg_deco);

	LOG_V(cls, "Create new seat.");
	wm->seat = wlr_seat_create(wm->display, "seat0");

	if (wm->seat == NULL) {
		FAIL("Failed to create seat");
	}

	wm->seat_caps = 0;

	LOG_V(cls, "Create pointer gestures manager.");
	wm->pointer_gestures = wlr_pointer_gestures_v1_create(wm->display);

	if (wm->pointer_gestures == NULL) {
		FAIL("Failed to create pointer gestures manager.");
	}

	LOG_V(cls, "Create relative pointer manager.");
	wm->relative_pointer_mgr = wlr_relative_pointer_manager_v1_create(wm->display);

	if (wm->relative_pointer_mgr == NULL) {
		FAIL("Failed to create relative pointer manager.");
	}

	LOG_V(cls, "Create pointer constraints.");
	wm->pointer_constraints = wlr_pointer_constraints_v1_create(wm->display);

	if (wm->pointer_constraints == NULL) {
		FAIL("Failed to create pointer constraints.");
	}

	wm->active_constraint = NULL;
	wm->new_pointer_constraint.notify = wm_vdev_new_pointer_constraint;
	wl_signal_add(&wm->pointer_constraints->events.new_constraint, &wm->new_pointer_constraint);

	wm->request_set_selection.notify = request_set_selection;
	wl_signal_add(&wm->seat->events.request_set_selection, &wm->request_set_selection);

	LOG_V(cls, "Add UNIX socket to the Wayland display.");
	char const* const sock = wl_display_add_socket_auto(wm->display);

	if (sock == NULL) {
		FAIL("Failed to add UNIX socket to Wayland display.");
	}

	LOG_I(cls, "Wayland display is listening on %s.", sock);

	LOG_V(cls, "Start backend.");

	if (!wlr_backend_start(wm->backend)) {
		FAIL("Failed to start backend.");
	}

	LOG_V(cls, "Setting WAYLAND_DISPLAY environment variable to socket.");
	setenv("WAYLAND_DISPLAY", sock, true);

	return wm;
}

void wm_vdev_destroy(wm_t* wm) {
	image_t* image;
	image_t* image_tmp;

	wl_list_for_each_safe(image, image_tmp, &wm->images, link) {
		image_free(image); // No point using image_destroy() and alerting client.
	}

	if (wm->display != NULL) {
		wl_display_destroy_clients(wm->display);
	}

	if (wm->backend != NULL) {
		wlr_backend_destroy(wm->backend);
	}

	if (wm->allocator != NULL) {
		wlr_allocator_destroy(wm->allocator);
	}

	if (wm->display != NULL) {
		wl_display_destroy(wm->display);
	}

	LOG_F(cls, "TODO alles die hier nog niet gefree'd zijn");
	free(wm);
}

void wm_vdev_loop(wm_t* wm) {
	LOG_V(cls, "Starting WM loop.");
	wl_display_run(wm->display);
}
