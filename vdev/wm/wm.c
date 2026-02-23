// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

// A lot of this code is based on the tinywl reference compositor and the "simple.c" example:
// https://gitlab.freedesktop.org/wlroots/wlroots/-/blob/master/tinywl
// https://gitlab.freedesktop.org/wlroots/wlroots/-/blob/master/examples/simple.c

#include "wm.h"

#include <aqua/vdriver.h>

#include <umber.h>

#include <wlr/render/interface.h>
#include <wlr/render/swapchain.h>
#include <wlr/render/vulkan.h>

#include <webgpu/webgpu.h>
#include <webgpu/wgpu.h>

#include <assert.h>
#include <stdlib.h>

typedef struct {
	struct wl_list link;

	wm_t* wm;
	struct wlr_output* output;

	struct wl_listener frame;
	struct wl_listener destroy;
} output_t;

typedef struct {
	struct wl_list link;

	wm_t* wm;
	struct wlr_keyboard* keyboard;

	struct wl_listener mod;
	struct wl_listener key;
	struct wl_listener destroy;
} keyborp_t;

typedef struct __attribute__((packed)) {
	intr_t intr;
	uint64_t raw_vk_image;
	uint64_t raw_vk_cmd_pool;
	uint64_t raw_vk_cmd_buf;
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
	intr_t intr;
	uint64_t win;
	uint32_t x_res;
	uint32_t y_res;
	uint64_t raw_image;
} redraw_win_intr_t;

static umber_class_t const* cls = NULL;
static umber_class_t const* cls_wlr = NULL;

void wm_vdev_init(umber_class_t const* _cls, umber_class_t const* _cls_wlr) {
	cls = _cls;
	cls_wlr = _cls_wlr;
}

static void interrupt(wm_t* wm, size_t data_size, void const* data) {
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

static void output_remove_notify(struct wl_listener* listener, void* data) {
	(void) listener;
	(void) data;

	output_t* const output = wl_container_of(listener, output, destroy);

	wl_list_remove(&output->frame.link);
	wl_list_remove(&output->destroy.link);

	free(output);
}

static struct wlr_surface* target_surf(struct wlr_xdg_toplevel* toplevel) {
	if (toplevel->app_id == NULL || strcmp(toplevel->app_id, "firefox") != 0) {
		return toplevel->base->surface;
	}

	// XXX For Firefox, get the non-container surface.
	// This container does ugly CSD stuff like a shadow and I can't for the life of me get it to rely on SSD.

	struct wlr_subsurface* subsurface;

	wl_list_for_each(subsurface, &toplevel->base->surface->current.subsurfaces_above, current.link) {
		if (!subsurface->surface->mapped) {
			continue;
		}

		return subsurface->surface;
	}

	return toplevel->base->surface; // Fallback, always guaranteed to exist.
}

typedef struct {
	wm_t* wm;
	struct wlr_vk_image_attribs attribs;
} render_pass_state_t;

static bool begin_buffer_pass(void* data, struct wlr_buffer* buf) {
	render_pass_state_t* const s = data;
	wlr_vk_buffer_get_image_attribs(s->wm->wlr_renderer, buf, &s->attribs);

	return true;
}

static bool submit_render_pass(void* data) {
	render_pass_state_t* const s = data;

	LOG_V(cls, "Take care of rendering.");

	redraw_intr_t intr = {
		.intr = INTR_REDRAW,
		.raw_vk_image = (uintptr_t) s->attribs.image,
		.raw_vk_cmd_pool = (uintptr_t) NULL,
		.raw_vk_cmd_buf = (uintptr_t) NULL,
	};

	interrupt(s->wm, sizeof intr, &intr);

	return true;
}

static void render(void* wm, struct wlr_render_pass* render_pass) {
	LOG_V(cls, "Take care of rendering.");

	VkImage image;
	VkCommandPool cmd_pool;
	VkCommandBuffer cmd_buf;

	wlr_vk_render_pass_get_info(render_pass, &image, &cmd_pool, &cmd_buf);

	redraw_intr_t intr = {
		.intr = INTR_REDRAW,
		.raw_vk_image = (uintptr_t) image,
		.raw_vk_cmd_pool = (uintptr_t) cmd_pool,
		.raw_vk_cmd_buf = (uintptr_t) cmd_buf,
	};

	interrupt(wm, sizeof intr, &intr);
}

// XXX This is only for debugging until I don't have a preferred solution:
// - WE_HANDLE: We handle swapchain acquisition and do all the rendering ourselves.
// - WLR_OVERRIDE: We set hooks in the regular wlroots render path for render pass creation/submission in wlr_scene_output_build_state.
//                 If just overriding the render function, we even let wlroots handle the command buffer creation and import that into WebGPU.

#define WE_HANDLE

// #define WLR_OVERRIDE

static void output_frame_notify(struct wl_listener* listener, void* data) {
	(void) data;

	output_t* const output = wl_container_of(listener, output, frame);

	wm_t* const wm = output->wm;
	struct wlr_output* const wlr_output = output->output;
	struct wlr_scene_output* scene_output = wlr_scene_get_scene_output(wm->scene, wlr_output);

	LOG_V(cls, "Initialize output state.");

	struct wlr_output_state state;
	wlr_output_state_init(&state);

#ifdef WE_HANDLE
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
#endif

	render_pass_state_t render_pass = {.wm = wm};

	(void) render_pass, (void) begin_buffer_pass, (void) submit_render_pass, (void) render;

#ifdef WLR_OVERRIDE
	wlr_scene_override_begin_buffer_pass(scene_output, begin_buffer_pass, &render_pass);
	wlr_scene_override_submit_render_pass(scene_output, submit_render_pass, &render_pass);
	// wlr_scene_override_render(scene_output, render, wm);

	wlr_scene_output_build_state(scene_output, &state, NULL);
#endif

#ifdef WE_HANDLE
	struct wlr_vk_image_attribs attribs;
	wlr_vk_buffer_get_image_attribs(wm->wlr_renderer, buf, &attribs);

	LOG_V(cls, "Take care of rendering.");

	redraw_intr_t intr = {
		.intr = INTR_REDRAW,
		.raw_vk_image = (uintptr_t) attribs.image,
	};

	interrupt(wm, sizeof intr, &intr);

	LOG_V(cls, "Set swapchain image to output and release it.");

	wlr_output_state_set_buffer(&state, buf);
	wlr_buffer_unlock(buf);
#endif

	LOG_V(cls, "Commit output state.");

	wlr_output_commit_state(wlr_output, &state);
	wlr_output_state_finish(&state);

	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	wlr_scene_output_send_frame_done(scene_output, &now);

	// If we have a Firefox window, redraw it too.

	toplevel_t* toplevel;

	wl_list_for_each(toplevel, &wm->toplevels, link) {
		if (toplevel->xdg_toplevel->app_id == NULL || strcmp(toplevel->xdg_toplevel->app_id, "firefox") != 0) {
			continue;
		}

		struct wlr_texture* const tex = wlr_surface_get_texture(target_surf(toplevel->xdg_toplevel));

		redraw_win_intr_t const intr = {
			.intr = INTR_REDRAW_WIN,
			.win = (uint64_t) toplevel,
			.x_res = tex->width,
			.y_res = tex->height,
		};

		interrupt(toplevel->wm, sizeof intr, &intr);
	}

	// wlr_vk_dummy_cb_destroy_textures(wm->cur_dummy_cmd_buf);
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

	LOG_V(cls, "Apply new output state atomically.");

	wlr_output_commit_state(wlr_output, &state);
	wlr_output_state_finish(&state);

	LOG_V(cls, "Set up state for output.");

	output_t* const output = calloc(1, sizeof *output);
	assert(output != NULL);

	output->wm = wm;
	output->output = wlr_output;

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

static void focus_toplevel(toplevel_t* toplevel) {
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
}

static void toplevel_map(struct wl_listener* listener, void* data) {
	toplevel_t* const toplevel = wl_container_of(listener, toplevel, map);
	wm_t* const wm = toplevel->wm;

	(void) data;

	LOG_V(cls, "Map toplevel %p (app_id=%s).", toplevel->xdg_toplevel, toplevel->xdg_toplevel->app_id);

	wl_list_insert(&wm->toplevels, &toplevel->link);
	focus_toplevel(toplevel);

	wlr_xdg_toplevel_set_size(toplevel->xdg_toplevel, 1280, 768);

	// Send interrupt.

	new_win_intr_t const intr = {
		.intr = INTR_NEW_WIN,
		.win = (uint64_t) (uintptr_t) toplevel,
		.app_id = toplevel->xdg_toplevel->app_id,
	};

	interrupt(wm, sizeof intr, &intr);
}

static void toplevel_unmap(struct wl_listener* listener, void* data) {
	toplevel_t* const toplevel = wl_container_of(listener, toplevel, unmap);
	wm_t* const wm = toplevel->wm;

	(void) data;

	LOG_V(cls, "Unmap %s.", toplevel->xdg_toplevel->app_id);

	wl_list_remove(&toplevel->link);

	// Send interrupt.

	destroy_win_intr_t const intr = {
		.intr = INTR_DESTROY_WIN,
		.win = (uint64_t) (uintptr_t) toplevel,
	};

	interrupt(wm, sizeof intr, &intr);
}

static void toplevel_commit(struct wl_listener* listener, void* data) {
	toplevel_t* const toplevel = wl_container_of(listener, toplevel, commit);
	wm_t* const wm = toplevel->wm;
	struct wlr_xdg_toplevel* xdg_toplevel = toplevel->xdg_toplevel;

	(void) data;

	LOG_V(cls, "Commit %s.", xdg_toplevel->app_id);

	if (toplevel->xdg_toplevel->base->initial_commit) {
		LOG_V(cls, "Initial commit for %s; setting size to (0, 0) so client can pick size.", xdg_toplevel->app_id);
		wlr_xdg_toplevel_set_size(toplevel->xdg_toplevel, 0, 0);
		return;
	}

	// Send interrupt.

	struct wlr_surface* const surf = target_surf(xdg_toplevel);
	struct wlr_texture* const tex = wlr_surface_get_texture(surf);

	int width = surf->current.width;
	int height = surf->current.height;

	// XXX Bit of a crazy workaround for allowing us to import wl_shm-only VkImages.
	// The image contents are never updated if we don't do this (see #32 for more details).

	int stride = width * 4;
	uint8_t* tmp = calloc(1, stride * height);

	struct wlr_texture_read_pixels_options opts = {
		.data = tmp,
		.dst_x = 0,
		.dst_y = 0,
		.stride = stride,
		.format = wlr_texture_preferred_read_format(tex),
		.src_box = {.width = -1},
	};

	if (!wlr_texture_read_pixels(tex, &opts)) {
		free(tmp);
		return;
	}

	free(tmp);

	struct wlr_vk_image_attribs attribs;
	wlr_vk_texture_get_image_attribs(tex, &attribs);

	// TODO We need to figure out a way to not free the VkImage (in vulkan_texture_destroy) before the scene has had a chance to render, i.e. before INTR_REDRAW returns.
	// We could create some kind of dummy command buffer on wlr_texture.last_used_cb, so that when vulkan_texture_destroy is called, we can just add stuff there and then extract it and destroy the textures ourselves when INTR_REDRAW returns control back to us.
	// We'll have to make it mega clear either way then that window textures are only valid until the next redraw.
	// Or maybe the client program should be responsible for freeing this?

	wlr_vk_texture_attach_dummy_last_used_cb(tex, wm->cur_dummy_cmd_buf);

	// TODO Send format (now assuming its VK_FORMAT_B8G8R8A8_UNORM).

	redraw_win_intr_t const intr = {
		.intr = INTR_REDRAW_WIN,
		.win = (uintptr_t) toplevel,
		.x_res = tex->width,
		.y_res = tex->height,
		.raw_image = (uintptr_t) attribs.image,
	};

	interrupt(wm, sizeof intr, &intr);
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
	xdg_toplevel->base->data = toplevel->scene_tree;

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

static void new_popup(struct wl_listener* listener, void* data) {
	(void) listener;
	(void) data;

	LOG_F(cls, "TODO: %s", __func__);
}

static void surf_config(struct wl_listener* listener, void* data) {
	surf_t* const surf = wl_container_of(listener, surf, config);
	struct wlr_box const geom = surf->xdg_surface->geometry;

	(void) data;

	LOG_V(cls, "Configure XDG surface (%ux%u+%u+%u) for toplevel %p.", geom.width, geom.height, geom.x, geom.y, surf->xdg_surface->toplevel);
}

static void surf_destroy(struct wl_listener* listener, void* data) {
	surf_t* const surf = wl_container_of(listener, surf, destroy);

	(void) data;

	LOG_V(cls, "Destroying XDG surface.");

	wl_list_remove(&surf->config.link);
	wl_list_remove(&surf->destroy.link);

	free(surf);
}

static void new_xdg_surface(struct wl_listener* listener, void* data) {
	wm_t* const wm = wl_container_of(listener, wm, new_xdg_toplevel);
	struct wlr_xdg_surface* const xdg_surface = data;

	LOG_V(cls, "New XDG surface.");

	surf_t* const surf = calloc(1, sizeof *surf);
	assert(surf != NULL);

	surf->wm = wm;
	surf->xdg_surface = xdg_surface;

	surf->config.notify = surf_config;
	wl_signal_add(&xdg_surface->events.configure, &surf->config);

	surf->destroy.notify = surf_destroy;
	wl_signal_add(&xdg_surface->events.destroy, &surf->destroy);
}

static void cursor_motion(struct wl_listener* listener, void* data) {
	(void) listener;
	(void) data;

	LOG_F(cls, "TODO: %s", __func__);
}

static void cursor_motion_absolute(struct wl_listener* listener, void* data) {
	(void) listener;
	(void) data;

	LOG_F(cls, "TODO: %s", __func__);
}

static void cursor_button(struct wl_listener* listener, void* data) {
	(void) listener;
	(void) data;

	LOG_F(cls, "TODO: %s", __func__);
}

static void cursor_axis(struct wl_listener* listener, void* data) {
	(void) listener;
	(void) data;

	LOG_F(cls, "TODO: %s", __func__);
}

static void cursor_frame(struct wl_listener* listener, void* data) {
	(void) listener;
	(void) data;

	LOG_F(cls, "TODO: %s", __func__);
}

static void keyborp_mod(struct wl_listener* listener, void* data) {
	keyborp_t* const keyborp = wl_container_of(listener, keyborp, mod);
	struct wlr_keyboard* const wlr_keyboard = keyborp->keyboard;
	wm_t* const wm = keyborp->wm;

	(void) data;

	wlr_seat_set_keyboard(wm->seat, wlr_keyboard);
	wlr_seat_keyboard_notify_modifiers(wm->seat, &wlr_keyboard->modifiers);
}

static void keyborp_key(struct wl_listener* listener, void* data) {
	keyborp_t* const keyborp = wl_container_of(listener, keyborp, key);
	struct wlr_keyboard* const wlr_keyboard = keyborp->keyboard;
	wm_t* const wm = keyborp->wm;
	struct wlr_keyboard_key_event* const event = data;

	// This is where we intercept anything we needed to intercept if we needed to intercept it.

	uint32_t const mods = wlr_keyboard_get_modifiers(wlr_keyboard);
	bool handled = false;

	if ((mods & WLR_MODIFIER_LOGO) && event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
		uint32_t const keycode = event->keycode + 8;
		xkb_keysym_t const* syms;
		size_t const sym_count = xkb_state_key_get_syms(wlr_keyboard->xkb_state, keycode, &syms);

		for (size_t i = 0; i < sym_count; i++) {
			if (syms[i] != XKB_KEY_Tab) {
				continue;
			}

			handled = true;

			if (wm->toplevels.prev == NULL) {
				continue;
			}

			toplevel_t* const to_focus = wl_container_of(wm->toplevels.prev, to_focus, link);
			assert(to_focus != NULL);

			focus_toplevel(to_focus);
		}
	}

	// Pass on keyboard key notification.

	if (handled) {
		return;
	}

	wlr_seat_set_keyboard(wm->seat, wlr_keyboard);
	wlr_seat_keyboard_notify_key(wm->seat, event->time_msec, event->keycode, event->state);
}

static void keyborp_destroy(struct wl_listener* listener, void* data) {
	keyborp_t* const keyborp = wl_container_of(listener, keyborp, destroy);

	(void) data;

	wl_list_remove(&keyborp->mod.link);
	wl_list_remove(&keyborp->key.link);
	wl_list_remove(&keyborp->destroy.link);
	wl_list_remove(&keyborp->link);

	free(keyborp);
}

static void new_keyboard(wm_t* wm, struct wlr_input_device* dev) {
	struct wlr_keyboard* const wlr_keyboard = wlr_keyboard_from_input_device(dev);

	keyborp_t* const keyborp = malloc(sizeof *keyborp);
	assert(keyborp != NULL);

	keyborp->wm = wm;
	keyborp->keyboard = wlr_keyboard;

	// Prepare XKB keymap and assign it to the keyboard.
	// TODO Non-US layouts.

	struct xkb_rule_names const rule_names = {
		.rules = NULL,
		.model = NULL,
		.layout = "gb",
		.variant = "extd",
		.options = "caps:swapescape",
	};

	struct xkb_context* const context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	struct xkb_keymap* const keymap = xkb_keymap_new_from_names(context, &rule_names, XKB_KEYMAP_COMPILE_NO_FLAGS);

	wlr_keyboard_set_keymap(wlr_keyboard, keymap);
	xkb_keymap_unref(keymap);
	xkb_context_unref(context);
	wlr_keyboard_set_repeat_info(wlr_keyboard, 25, 600); // TODO

	keyborp->mod.notify = keyborp_mod;
	wl_signal_add(&wlr_keyboard->events.modifiers, &keyborp->mod);

	keyborp->key.notify = keyborp_key;
	wl_signal_add(&wlr_keyboard->events.key, &keyborp->key);

	keyborp->destroy.notify = keyborp_destroy;
	wl_signal_add(&dev->events.destroy, &keyborp->destroy);

	wlr_seat_set_keyboard(wm->seat, wlr_keyboard);
	wl_list_insert(&wm->keyboards, &keyborp->link);

	// TODO We're going to have to track these cap flags once we have more than just keyborps.

	wlr_seat_set_capabilities(wm->seat, WL_SEAT_CAPABILITY_KEYBOARD);
}

static void new_input(struct wl_listener* listener, void* data) {
	wm_t* const wm = wl_container_of(listener, wm, new_input);
	struct wlr_input_device* dev = data;

	switch (dev->type) {
	case WLR_INPUT_DEVICE_KEYBOARD:
		new_keyboard(wm, dev);
		break;
	case WLR_INPUT_DEVICE_POINTER:
	case WLR_INPUT_DEVICE_TOUCH:
	case WLR_INPUT_DEVICE_TABLET:
	case WLR_INPUT_DEVICE_TABLET_PAD:
	case WLR_INPUT_DEVICE_SWITCH:
		LOG_F(cls, "TODO %s: dev->type = %d", __func__, dev->type);
		break;
	}
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

	wm->cur_dummy_cmd_buf = wlr_vk_create_dummy_cb();

	LOG_V(cls, "Initialize buffer factory protocols.");

	if (wlr_renderer_init_wl_display(wm->wlr_renderer, wm->display) == false) {
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

	wm->new_input.notify = new_input;
	wl_signal_add(&wm->backend->events.new_input, &wm->new_input);

	LOG_V(cls, "Creating compositor (version 5).");
	struct wlr_compositor* const compositor = wlr_compositor_create(wm->display, 5, wm->wlr_renderer);

	if (compositor == NULL) {
		FAIL("Failed to create compositor.");
	}

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
	wm->xdg_shell = wlr_xdg_shell_create(wm->display, 3);

	if (wm->xdg_shell == NULL) {
		FAIL("Failed to create XDG-shell.");
	}

	LOG_V(cls, "Add listener for when new XDG toplevels or popups are available");

	wm->new_xdg_toplevel.notify = new_xdg_toplevel;
	wl_signal_add(&wm->xdg_shell->events.new_toplevel, &wm->new_xdg_toplevel);

	wm->new_xdg_popup.notify = new_popup;
	wl_signal_add(&wm->xdg_shell->events.new_popup, &wm->new_xdg_popup);

	wm->new_xdg_surface.notify = new_xdg_surface;
	wl_signal_add(&wm->xdg_shell->events.new_surface, &wm->new_xdg_surface);

	LOG_V(cls, "Create cursor.");
	wm->cursor = wlr_cursor_create();

	if (wm->cursor == NULL) {
		FAIL("Failed to create cursor");
	}

	wlr_cursor_attach_output_layout(wm->cursor, wm->output_layout);

	LOG_V(cls, "Add listeners for cursor events.");

	wm->cursor_motion.notify = cursor_motion;
	wl_signal_add(&wm->cursor->events.motion, &wm->cursor_motion);

	wm->cursor_motion_absolute.notify = cursor_motion_absolute;
	wl_signal_add(&wm->cursor->events.motion_absolute, &wm->cursor_motion_absolute);

	wm->cursor_button.notify = cursor_button;
	wl_signal_add(&wm->cursor->events.button, &wm->cursor_button);

	wm->cursor_axis.notify = cursor_axis;
	wl_signal_add(&wm->cursor->events.axis, &wm->cursor_axis);

	wm->cursor_frame.notify = cursor_frame;
	wl_signal_add(&wm->cursor->events.frame, &wm->cursor_frame);

	LOG_V(cls, "Create new seat.");
	wm->seat = wlr_seat_create(wm->display, "seat0");

	if (wm->seat == NULL) {
		FAIL("Failed to create seat");
	}

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
	wl_display_destroy_clients(wm->display);

	if (wm->display != NULL) {
		wl_display_destroy(wm->display);
	}

	if (wm->backend != NULL) {
		wlr_backend_destroy(wm->backend);
	}

	if (wm->allocator != NULL) {
		wlr_allocator_destroy(wm->allocator);
	}

	if (wm->cur_dummy_cmd_buf) {
		wlr_vk_destroy_dummy_cb(wm->cur_dummy_cmd_buf);
	}

	LOG_F(cls, "TODO alles die hier nog niet gefree'd zijn");
	free(wm);
}

void wm_vdev_loop(wm_t* wm) {
	LOG_V(cls, "Starting WM loop.");
	wl_display_run(wm->display);
}

static void read_surface_pixels(struct wlr_surface* surface, int sx, int sy, void* data) {
	struct {
		uint8_t* buf;
		int stride;
		int origin_x, origin_y;
	}* ctx = data;

	struct wlr_texture* tex = wlr_surface_get_texture(surface);

	if (!tex) {
		return;
	}

	int width = surface->current.width;
	int height = surface->current.height;
	int dst_x = ctx->origin_x + sx;
	int dst_y = ctx->origin_y + sy;

	int stride = width * 4;
	uint8_t* tmp = calloc(1, stride * height);

	struct wlr_texture_read_pixels_options opts = {
		.data = tmp,
		.dst_x = 0,
		.dst_y = 0,
		.stride = stride,
		.format = wlr_texture_preferred_read_format(tex),
		.src_box = {.width = -1},
	};

	if (!wlr_texture_read_pixels(tex, &opts)) {
		free(tmp);
		return;
	}

	for (int y = 0; y < height; y++) {
		uint32_t* src_row = (uint32_t*) (tmp + y * stride);
		uint32_t* dst_row = (uint32_t*) (ctx->buf + (dst_y + y) * ctx->stride + dst_x * 4);

		// XXX This is technically incorrect as we are overwriting pixels instead of compositing them.
		// But, then again, nothing here is correct so like whatever.

		memcpy(dst_row, src_row, stride);
	}

	free(tmp);
}

void wm_vdev_get_fb(toplevel_t* toplevel, void* buf) {
	struct wlr_xdg_toplevel* xdg_toplevel = toplevel->xdg_toplevel;
	struct wlr_surface* base = target_surf(xdg_toplevel);

	memset(buf, 0, base->current.width * base->current.height * 4);

	struct {
		uint8_t* buf;
		int stride;
		int origin_x, origin_y;
	} ctx = {
		.buf = buf,
		.stride = base->current.width * 4,
		.origin_x = 0,
		.origin_y = 0,
	};

	wlr_surface_for_each_surface(base, read_surface_pixels, &ctx);
}
