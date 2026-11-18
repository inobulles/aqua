// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025-2026 Aymeric Wibo

#include "wm.h"

#include <umber.h>

#include <wlr/config.h>

#if WLR_HAS_LIBINPUT_BACKEND
# include <libinput.h>
# include <wlr/backend/libinput.h>
#endif

#include <assert.h>
#include <stdlib.h>

typedef struct {
	struct wl_list link;

	wm_t* wm;
	struct wlr_keyboard* keyboard;

	struct wl_listener mod;
	struct wl_listener key;
	struct wl_listener destroy;
} keyborp_t;

typedef struct {
	struct wl_list link;

	wm_t* wm;
	struct wlr_pointer* pointer;

	struct wl_listener motion;
	struct wl_listener motion_abs;
	struct wl_listener button;
	struct wl_listener axis;
	struct wl_listener swipe_begin;
	struct wl_listener swipe_update;
	struct wl_listener swipe_end;
	struct wl_listener pinch_begin;
	struct wl_listener pinch_update;
	struct wl_listener pinch_end;
	struct wl_listener hold_begin;
	struct wl_listener hold_end;
	struct wl_listener destroy;
} mouse_t;

typedef struct __attribute__((packed)) {
	intr_t intr;
	uint32_t time;
	uint8_t is_abs;

	union {
		double dx, x;
	};

	union {
		double dy, y;
	};

	double unaccel_dx, unaccel_dy;
	uint32_t mods;
} mouse_motion_intr_t;

typedef struct __attribute__((packed)) {
	intr_t intr;
	uint32_t time;
	uint8_t pressed;
	uint32_t button;
	uint32_t mods;
} mouse_button_intr_t;

typedef struct __attribute__((packed)) {
	intr_t intr;
	uint32_t time;
	axis_source_t source;
	axis_orientation_t orientation;
	axis_direction_t direction;
	double delta;
	int32_t delta_discrete;
	uint32_t mods;
} mouse_axis_intr_t;

typedef struct __attribute__((packed)) {
	intr_t intr;
	uint32_t time;
	gesture_kind_t kind;
	gesture_phase_t phase;
	uint32_t fingers;  // Only valid for begin phase.
	double dx, dy;     // Only valid for update phase.
	double scale;      // Only valid for update phase of pinch gestures.
	double rotation;   // Only valid for update phase of pinch gestures.
	uint8_t cancelled; // Only valid for end phase.
	uint32_t mods;
} gesture_intr_t;

static umber_class_t const* cls = NULL;

void wm_vdev_input_init(umber_class_t const* _cls) {
	cls = _cls;
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

			wm_vdev_focus_toplevel(to_focus);
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

static void add_seat_cap(wm_t* wm, uint32_t cap) {
	wm->seat_caps |= cap;
	wlr_seat_set_capabilities(wm->seat, wm->seat_caps);
}

static void new_keyboard(wm_t* wm, struct wlr_input_device* dev) {
	struct wlr_keyboard* const wlr_keyboard = wlr_keyboard_from_input_device(dev);

	keyborp_t* const keyborp = malloc(sizeof *keyborp);
	assert(keyborp != NULL);

	keyborp->wm = wm;
	keyborp->keyboard = wlr_keyboard;

	// Prepare XKB keymap and assign it to the keyboard.
	// TODO Other layouts (simply a function for the client to set the keymap, and have this as some kind of state on wm_t?).

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

	add_seat_cap(wm, WL_SEAT_CAPABILITY_KEYBOARD);
}

void wm_vdev_toplevel_notify_mouse_motion(toplevel_t* toplevel, uint32_t time, uint32_t x, uint32_t y) {
	struct wlr_seat* const seat = toplevel->wm->seat;
	struct wlr_xdg_surface* const base = toplevel->xdg_toplevel->base;

	double const root_x = x + base->geometry.x;
	double const root_y = y + base->geometry.y;

	double sx, sy;
	struct wlr_surface* const surf = wlr_xdg_surface_surface_at(base, root_x, root_y, &sx, &sy);

	if (surf == NULL) {
		wlr_seat_pointer_notify_clear_focus(seat);
		return;
	}

	wlr_seat_pointer_notify_enter(seat, surf, sx, sy);
	wlr_seat_pointer_notify_motion(seat, time, sx, sy);
	wlr_seat_pointer_notify_frame(seat);
}

void wm_vdev_toplevel_notify_mouse_button(toplevel_t* toplevel, uint32_t time, bool pressed, uint32_t button) {
	struct wlr_seat* const seat = toplevel->wm->seat;
	enum wl_pointer_button_state const state =
		pressed ? WL_POINTER_BUTTON_STATE_PRESSED : WL_POINTER_BUTTON_STATE_RELEASED;

	wlr_seat_pointer_notify_button(seat, time, button, state);
	wlr_seat_pointer_notify_frame(seat);

	wm_vdev_focus_toplevel(toplevel);
}

void wm_vdev_toplevel_notify_mouse_axis(
	toplevel_t* toplevel,
	uint32_t time,
	axis_source_t source,
	axis_orientation_t orientation,
	axis_direction_t direction,
	double delta,
	int32_t delta_discrete
) {
	struct wlr_seat* const seat = toplevel->wm->seat;

	enum wl_pointer_axis_source wl_source = WL_POINTER_AXIS_SOURCE_WHEEL;

	switch (source) {
	case AXIS_SOURCE_WHEEL:
		wl_source = WL_POINTER_AXIS_SOURCE_WHEEL;
		break;
	case AXIS_SOURCE_FINGER:
		wl_source = WL_POINTER_AXIS_SOURCE_FINGER;
		break;
	case AXIS_SOURCE_CONTINUOUS:
		wl_source = WL_POINTER_AXIS_SOURCE_CONTINUOUS;
		break;
	case AXIS_SOURCE_WHEEL_TILT:
		wl_source = WL_POINTER_AXIS_SOURCE_WHEEL_TILT;
		break;
	}

	enum wl_pointer_axis const wl_orientation =
		orientation == AXIS_ORIENTATION_HORIZONTAL ? WL_POINTER_AXIS_HORIZONTAL_SCROLL : WL_POINTER_AXIS_VERTICAL_SCROLL;

	enum wl_pointer_axis_relative_direction const wl_direction =
		direction == AXIS_DIRECTION_INVERTED ? WL_POINTER_AXIS_RELATIVE_DIRECTION_INVERTED : WL_POINTER_AXIS_RELATIVE_DIRECTION_IDENTICAL;

	// A zero delta from a finger source is sent as an axis_stop event by wlroots.
	// Clients rely on this to know when to start kinetic scrolling or e.g. to finish swipe-to-navigate in Firefox.

	wlr_seat_pointer_notify_axis(seat, time, wl_orientation, delta, delta_discrete, wl_source, wl_direction);
	wlr_seat_pointer_notify_frame(seat);
}

void wm_vdev_toplevel_notify_gesture(
	toplevel_t* toplevel,
	uint32_t time,
	gesture_kind_t kind,
	gesture_phase_t phase,
	uint32_t fingers,
	double dx,
	double dy,
	double scale,
	double rotation,
	bool cancelled
) {
	wm_t* const wm = toplevel->wm;
	struct wlr_pointer_gestures_v1* const gestures = wm->pointer_gestures;
	struct wlr_seat* const seat = wm->seat;

	switch (kind) {
	case GESTURE_SWIPE:
		switch (phase) {
		case GESTURE_PHASE_BEGIN:
			wlr_pointer_gestures_v1_send_swipe_begin(gestures, seat, time, fingers);
			break;
		case GESTURE_PHASE_UPDATE:
			wlr_pointer_gestures_v1_send_swipe_update(gestures, seat, time, dx, dy);
			break;
		case GESTURE_PHASE_END:
			wlr_pointer_gestures_v1_send_swipe_end(gestures, seat, time, cancelled);
			break;
		}

		break;
	case GESTURE_PINCH:
		switch (phase) {
		case GESTURE_PHASE_BEGIN:
			wlr_pointer_gestures_v1_send_pinch_begin(gestures, seat, time, fingers);
			break;
		case GESTURE_PHASE_UPDATE:
			wlr_pointer_gestures_v1_send_pinch_update(gestures, seat, time, dx, dy, scale, rotation);
			break;
		case GESTURE_PHASE_END:
			wlr_pointer_gestures_v1_send_pinch_end(gestures, seat, time, cancelled);
			break;
		}

		break;
	case GESTURE_HOLD:
		switch (phase) {
		case GESTURE_PHASE_BEGIN:
			wlr_pointer_gestures_v1_send_hold_begin(gestures, seat, time, fingers);
			break;
		case GESTURE_PHASE_UPDATE:
			break; // Hold gestures don't have updates.
		case GESTURE_PHASE_END:
			wlr_pointer_gestures_v1_send_hold_end(gestures, seat, time, cancelled);
			break;
		}

		break;
	}
}

static uint32_t get_mods(wm_t* wm) {
	struct wlr_keyboard* const kbd = wlr_seat_get_keyboard(wm->seat);

	if (kbd == NULL) {
		return 0;
	}

	return wlr_keyboard_get_modifiers(kbd);
}

static void mouse_motion(struct wl_listener* listener, void* data) {
	mouse_t* const mouse = wl_container_of(listener, mouse, motion);
	wm_t* const wm = mouse->wm;
	struct wlr_pointer_motion_event* const event = data;

	mouse_motion_intr_t intr = {
		.intr = INTR_MOUSE_MOTION,
		.time = event->time_msec,
		.is_abs = false,
		.dx = event->delta_x,
		.dy = event->delta_y,
		.unaccel_dx = event->unaccel_dx,
		.unaccel_dy = event->unaccel_dy,
		.mods = get_mods(wm),
	};

	wm_vdev_interrupt(wm, sizeof intr, &intr);
}

static void mouse_motion_abs(struct wl_listener* listener, void* data) {
	mouse_t* const mouse = wl_container_of(listener, mouse, motion_abs);
	wm_t* const wm = mouse->wm;
	struct wlr_pointer_motion_absolute_event* const event = data;

	mouse_motion_intr_t intr = {
		.intr = INTR_MOUSE_MOTION,
		.time = event->time_msec,
		.is_abs = true,
		.x = event->x,
		.y = event->y,
		.mods = get_mods(wm),
	};

	wm_vdev_interrupt(wm, sizeof intr, &intr);
}

static void mouse_button(struct wl_listener* listener, void* data) {
	mouse_t* const mouse = wl_container_of(listener, mouse, button);
	wm_t* const wm = mouse->wm;
	struct wlr_pointer_button_event* const event = data;

	assert(
		event->state == WL_POINTER_BUTTON_STATE_PRESSED ||
		event->state == WL_POINTER_BUTTON_STATE_RELEASED
	);

	mouse_button_intr_t intr = {
		.intr = INTR_MOUSE_BUTTON,
		.time = event->time_msec,
		.pressed = event->state == WL_POINTER_BUTTON_STATE_PRESSED,
		.button = event->button,
		.mods = get_mods(wm),
	};

	wm_vdev_interrupt(wm, sizeof intr, &intr);
}

static void mouse_axis(struct wl_listener* listener, void* data) {
	mouse_t* const mouse = wl_container_of(listener, mouse, axis);
	wm_t* const wm = mouse->wm;
	struct wlr_pointer_axis_event* const event = data;

	axis_source_t source = AXIS_SOURCE_WHEEL;

	switch (event->source) {
	case WL_POINTER_AXIS_SOURCE_WHEEL:
		source = AXIS_SOURCE_WHEEL;
		break;
	case WL_POINTER_AXIS_SOURCE_FINGER:
		source = AXIS_SOURCE_FINGER;
		break;
	case WL_POINTER_AXIS_SOURCE_CONTINUOUS:
		source = AXIS_SOURCE_CONTINUOUS;
		break;
	case WL_POINTER_AXIS_SOURCE_WHEEL_TILT:
		source = AXIS_SOURCE_WHEEL_TILT;
		break;
	}

	mouse_axis_intr_t intr = {
		.intr = INTR_MOUSE_AXIS,
		.time = event->time_msec,
		.source = source,
		.orientation = event->orientation == WL_POINTER_AXIS_HORIZONTAL_SCROLL ? AXIS_ORIENTATION_HORIZONTAL : AXIS_ORIENTATION_VERTICAL,
		.direction = event->relative_direction == WL_POINTER_AXIS_RELATIVE_DIRECTION_INVERTED ? AXIS_DIRECTION_INVERTED : AXIS_DIRECTION_IDENTICAL,
		.delta = event->delta,
		.delta_discrete = event->delta_discrete,
		.mods = get_mods(wm),
	};

	wm_vdev_interrupt(wm, sizeof intr, &intr);
}

static void gesture_intr(wm_t* wm, gesture_intr_t* intr) {
	intr->intr = INTR_GESTURE;
	intr->mods = get_mods(wm);

	wm_vdev_interrupt(wm, sizeof *intr, intr);
}

static void swipe_begin(struct wl_listener* listener, void* data) {
	mouse_t* const mouse = wl_container_of(listener, mouse, swipe_begin);
	struct wlr_pointer_swipe_begin_event* const event = data;

	gesture_intr_t intr = {
		.time = event->time_msec,
		.kind = GESTURE_SWIPE,
		.phase = GESTURE_PHASE_BEGIN,
		.fingers = event->fingers,
	};

	gesture_intr(mouse->wm, &intr);
}

static void swipe_update(struct wl_listener* listener, void* data) {
	mouse_t* const mouse = wl_container_of(listener, mouse, swipe_update);
	struct wlr_pointer_swipe_update_event* const event = data;

	gesture_intr_t intr = {
		.time = event->time_msec,
		.kind = GESTURE_SWIPE,
		.phase = GESTURE_PHASE_UPDATE,
		.fingers = event->fingers,
		.dx = event->dx,
		.dy = event->dy,
	};

	gesture_intr(mouse->wm, &intr);
}

static void swipe_end(struct wl_listener* listener, void* data) {
	mouse_t* const mouse = wl_container_of(listener, mouse, swipe_end);
	struct wlr_pointer_swipe_end_event* const event = data;

	gesture_intr_t intr = {
		.time = event->time_msec,
		.kind = GESTURE_SWIPE,
		.phase = GESTURE_PHASE_END,
		.cancelled = event->cancelled,
	};

	gesture_intr(mouse->wm, &intr);
}

static void pinch_begin(struct wl_listener* listener, void* data) {
	mouse_t* const mouse = wl_container_of(listener, mouse, pinch_begin);
	struct wlr_pointer_pinch_begin_event* const event = data;

	gesture_intr_t intr = {
		.time = event->time_msec,
		.kind = GESTURE_PINCH,
		.phase = GESTURE_PHASE_BEGIN,
		.fingers = event->fingers,
	};

	gesture_intr(mouse->wm, &intr);
}

static void pinch_update(struct wl_listener* listener, void* data) {
	mouse_t* const mouse = wl_container_of(listener, mouse, pinch_update);
	struct wlr_pointer_pinch_update_event* const event = data;

	gesture_intr_t intr = {
		.time = event->time_msec,
		.kind = GESTURE_PINCH,
		.phase = GESTURE_PHASE_UPDATE,
		.fingers = event->fingers,
		.dx = event->dx,
		.dy = event->dy,
		.scale = event->scale,
		.rotation = event->rotation,
	};

	gesture_intr(mouse->wm, &intr);
}

static void pinch_end(struct wl_listener* listener, void* data) {
	mouse_t* const mouse = wl_container_of(listener, mouse, pinch_end);
	struct wlr_pointer_pinch_end_event* const event = data;

	gesture_intr_t intr = {
		.time = event->time_msec,
		.kind = GESTURE_PINCH,
		.phase = GESTURE_PHASE_END,
		.cancelled = event->cancelled,
	};

	gesture_intr(mouse->wm, &intr);
}

static void hold_begin(struct wl_listener* listener, void* data) {
	mouse_t* const mouse = wl_container_of(listener, mouse, hold_begin);
	struct wlr_pointer_hold_begin_event* const event = data;

	gesture_intr_t intr = {
		.time = event->time_msec,
		.kind = GESTURE_HOLD,
		.phase = GESTURE_PHASE_BEGIN,
		.fingers = event->fingers,
	};

	gesture_intr(mouse->wm, &intr);
}

static void hold_end(struct wl_listener* listener, void* data) {
	mouse_t* const mouse = wl_container_of(listener, mouse, hold_end);
	struct wlr_pointer_hold_end_event* const event = data;

	gesture_intr_t intr = {
		.time = event->time_msec,
		.kind = GESTURE_HOLD,
		.phase = GESTURE_PHASE_END,
		.cancelled = event->cancelled,
	};

	gesture_intr(mouse->wm, &intr);
}

static void mouse_destroy(struct wl_listener* listener, void* data) {
	mouse_t* const mouse = wl_container_of(listener, mouse, destroy);

	(void) data;

	wl_list_remove(&mouse->motion.link);
	wl_list_remove(&mouse->motion_abs.link);
	wl_list_remove(&mouse->button.link);
	wl_list_remove(&mouse->axis.link);
	wl_list_remove(&mouse->swipe_begin.link);
	wl_list_remove(&mouse->swipe_update.link);
	wl_list_remove(&mouse->swipe_end.link);
	wl_list_remove(&mouse->pinch_begin.link);
	wl_list_remove(&mouse->pinch_update.link);
	wl_list_remove(&mouse->pinch_end.link);
	wl_list_remove(&mouse->hold_begin.link);
	wl_list_remove(&mouse->hold_end.link);
	wl_list_remove(&mouse->destroy.link);
	wl_list_remove(&mouse->link);

	free(mouse);
}

static void config_mouse(struct wlr_input_device* dev) {
#if WLR_HAS_LIBINPUT_BACKEND
	// Only devices from the libinput backend (i.e. when running under DRM) can be configured.
	// When running nested, the parent compositor is the one which is meant to deal with this.

	if (!wlr_input_device_is_libinput(dev)) {
		return;
	}

	struct libinput_device* const ldev = wlr_libinput_get_device_handle(dev);

	// TODO These are hardcoded for now (taken from my Sway config), but should eventually be configurable.

	if (libinput_device_config_accel_is_available(ldev)) {
		libinput_device_config_accel_set_profile(ldev, LIBINPUT_CONFIG_ACCEL_PROFILE_ADAPTIVE);
		libinput_device_config_accel_set_speed(ldev, 0.3);
	}

	if (libinput_device_config_tap_get_finger_count(ldev) > 0) {
		libinput_device_config_tap_set_enabled(ldev, LIBINPUT_CONFIG_TAP_ENABLED);
		libinput_device_config_tap_set_drag_enabled(ldev, LIBINPUT_CONFIG_DRAG_ENABLED);
		libinput_device_config_tap_set_drag_lock_enabled(ldev, LIBINPUT_CONFIG_DRAG_LOCK_DISABLED);
	}

	if (libinput_device_config_dwt_is_available(ldev)) {
		libinput_device_config_dwt_set_enabled(ldev, LIBINPUT_CONFIG_DWT_DISABLED);
	}

	if (libinput_device_config_scroll_has_natural_scroll(ldev)) {
		libinput_device_config_scroll_set_natural_scroll_enabled(ldev, true);
	}
#else
	(void) dev;
#endif
}

static void new_mouse(wm_t* wm, struct wlr_input_device* dev) {
	struct wlr_pointer* const wlr_pointer = wlr_pointer_from_input_device(dev);

	mouse_t* const mouse = malloc(sizeof *mouse);
	assert(mouse != NULL);

	mouse->wm = wm;
	mouse->pointer = wlr_pointer;

	config_mouse(dev);

	mouse->motion.notify = mouse_motion;
	wl_signal_add(&wlr_pointer->events.motion, &mouse->motion);

	mouse->motion_abs.notify = mouse_motion_abs;
	wl_signal_add(&wlr_pointer->events.motion_absolute, &mouse->motion_abs);

	mouse->button.notify = mouse_button;
	wl_signal_add(&wlr_pointer->events.button, &mouse->button);

	mouse->axis.notify = mouse_axis;
	wl_signal_add(&wlr_pointer->events.axis, &mouse->axis);

	mouse->swipe_begin.notify = swipe_begin;
	wl_signal_add(&wlr_pointer->events.swipe_begin, &mouse->swipe_begin);

	mouse->swipe_update.notify = swipe_update;
	wl_signal_add(&wlr_pointer->events.swipe_update, &mouse->swipe_update);

	mouse->swipe_end.notify = swipe_end;
	wl_signal_add(&wlr_pointer->events.swipe_end, &mouse->swipe_end);

	mouse->pinch_begin.notify = pinch_begin;
	wl_signal_add(&wlr_pointer->events.pinch_begin, &mouse->pinch_begin);

	mouse->pinch_update.notify = pinch_update;
	wl_signal_add(&wlr_pointer->events.pinch_update, &mouse->pinch_update);

	mouse->pinch_end.notify = pinch_end;
	wl_signal_add(&wlr_pointer->events.pinch_end, &mouse->pinch_end);

	mouse->hold_begin.notify = hold_begin;
	wl_signal_add(&wlr_pointer->events.hold_begin, &mouse->hold_begin);

	mouse->hold_end.notify = hold_end;
	wl_signal_add(&wlr_pointer->events.hold_end, &mouse->hold_end);

	mouse->destroy.notify = mouse_destroy;
	wl_signal_add(&dev->events.destroy, &mouse->destroy);

	wl_list_insert(&wm->mice, &mouse->link);

	add_seat_cap(wm, WL_SEAT_CAPABILITY_POINTER);
}

void wm_vdev_new_input(struct wl_listener* listener, void* data) {
	wm_t* const wm = wl_container_of(listener, wm, new_input);
	struct wlr_input_device* dev = data;

	switch (dev->type) {
	case WLR_INPUT_DEVICE_KEYBOARD:
		new_keyboard(wm, dev);
		break;
	case WLR_INPUT_DEVICE_POINTER:
		new_mouse(wm, dev);
		break;
	case WLR_INPUT_DEVICE_TOUCH:
	case WLR_INPUT_DEVICE_TABLET:
	case WLR_INPUT_DEVICE_TABLET_PAD:
	case WLR_INPUT_DEVICE_SWITCH:
		LOG_F(cls, "TODO %s: dev->type = %d", __func__, dev->type);
		break;
	}
}
