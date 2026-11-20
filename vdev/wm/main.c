// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025-2026 Aymeric Wibo

#include "wm.h"

#include <aqua/vdriver.h>

#include <umber.h>

#include <assert.h>

#define SPEC "aquabsd.black.wm"
#define VERS 0
#define VDRIVER_HUMAN "WM driver"

static umber_class_t const* cls = NULL;
static umber_class_t const* cls_wlr = NULL;

static vid_t only_vid;

static void init(void) {
	cls = umber_class_new(SPEC, UMBER_LVL_WARN, SPEC " Font VDRIVER.");
	assert(cls != NULL);

	cls_wlr = umber_class_new(SPEC ".wlr", UMBER_LVL_WARN, SPEC " Font VDRIVER (wlroots logs).");
	assert(cls_wlr != NULL);

	wm_vdev_init(cls, cls_wlr);
}

static void probe(void) {
	assert(VDRIVER.notif_cb != NULL);

	only_vid = VDRIVER.vdev_id_lo;

	kos_notif_t const notif = {
		.kind = KOS_NOTIF_ATTACH,
		.attach.vdev = {
			.kind = KOS_VDEV_KIND_LOCAL,
			.spec = SPEC,
			.vers = VERS,
			.human = "Window management device",
			.vdriver_human = VDRIVER_HUMAN,

			.pref = 0,
			.host_id = VDRIVER.host_id,
			.vdev_id = only_vid,
		},
	};

	VDRIVER.notif_cb(&notif, VDRIVER.notif_data);
}

static kos_const_t const CONSTS[] = {
	{
		.name = "INTR_REDRAW",
		.type = KOS_TYPE_U8,
		.val = {.u8 = INTR_REDRAW},
	},
	{
		.name = "INTR_NEW_WIN",
		.type = KOS_TYPE_U8,
		.val = {.u8 = INTR_NEW_WIN},
	},
	{
		.name = "INTR_DESTROY_WIN",
		.type = KOS_TYPE_U8,
		.val = {.u8 = INTR_DESTROY_WIN},
	},
	{
		.name = "INTR_REDRAW_WIN",
		.type = KOS_TYPE_U8,
		.val = {.u8 = INTR_REDRAW_WIN},
	},
	{
		.name = "INTR_DESTROY_IMAGE",
		.type = KOS_TYPE_U8,
		.val = {.u8 = INTR_DESTROY_IMAGE},
	},
	{
		.name = "INTR_MOUSE_MOTION",
		.type = KOS_TYPE_U8,
		.val = {.u8 = INTR_MOUSE_MOTION},
	},
	{
		.name = "INTR_MOUSE_BUTTON",
		.type = KOS_TYPE_U8,
		.val = {.u8 = INTR_MOUSE_BUTTON},
	},
	{
		.name = "INTR_MOUSE_AXIS",
		.type = KOS_TYPE_U8,
		.val = {.u8 = INTR_MOUSE_AXIS},
	},
	{
		.name = "INTR_GESTURE",
		.type = KOS_TYPE_U8,
		.val = {.u8 = INTR_GESTURE},
	},
	{
		.name = "INTR_KEY",
		.type = KOS_TYPE_U8,
		.val = {.u8 = INTR_KEY},
	},
	// clang-format off
// CONSTS:BEGIN
	{
		.name = "MOD_SHIFT",
		.type = KOS_TYPE_U32,
		.val = {.u32 = WLR_MODIFIER_SHIFT},
	},
	{
		.name = "MOD_CAPS",
		.type = KOS_TYPE_U32,
		.val = {.u32 = WLR_MODIFIER_CAPS},
	},
	{
		.name = "MOD_CTRL",
		.type = KOS_TYPE_U32,
		.val = {.u32 = WLR_MODIFIER_CTRL},
	},
	{
		.name = "MOD_ALT",
		.type = KOS_TYPE_U32,
		.val = {.u32 = WLR_MODIFIER_ALT},
	},
	{
		.name = "MOD_MOD2",
		.type = KOS_TYPE_U32,
		.val = {.u32 = WLR_MODIFIER_MOD2},
	},
	{
		.name = "MOD_MOD3",
		.type = KOS_TYPE_U32,
		.val = {.u32 = WLR_MODIFIER_MOD3},
	},
	{
		.name = "MOD_META",
		.type = KOS_TYPE_U32,
		.val = {.u32 = WLR_MODIFIER_LOGO},
	},
	{
		.name = "MOD_MOD5",
		.type = KOS_TYPE_U32,
		.val = {.u32 = WLR_MODIFIER_MOD5},
	},
	{
		.name = "BACKEND_DRM",
		.type = KOS_TYPE_U8,
		.val = {.u8 = BACKEND_DRM},
	},
	{
		.name = "BACKEND_WAYLAND",
		.type = KOS_TYPE_U8,
		.val = {.u8 = BACKEND_WAYLAND},
	},
	{
		.name = "BACKEND_X11",
		.type = KOS_TYPE_U8,
		.val = {.u8 = BACKEND_X11},
	},
	{
		.name = "BACKEND_HEADLESS",
		.type = KOS_TYPE_U8,
		.val = {.u8 = BACKEND_HEADLESS},
	},
// CONSTS:END
	// clang-format on
	// TODO We should really add constants for axis_source_t, axis_orientation_t, axis_direction_t, gesture_kind_t, and gesture_phase_t here too. Or think of a better solution?
};

static kos_fn_t const FNS[] = {
	{
		.name = "create",
		.ret_type = KOS_TYPE_OPAQUE_PTR,
		.param_count = 0,
	},
	{
		.name = "destroy",
		.ret_type = KOS_TYPE_VOID,
		.param_count = 1,
		.params = (kos_param_t[]) {
			{KOS_TYPE_OPAQUE_PTR, "wm"},
		},
	},
	{
		.name = "loop",
		.ret_type = KOS_TYPE_VOID,
		.param_count = 1,
		.params = (kos_param_t[]) {
			{KOS_TYPE_OPAQUE_PTR, "wm"},
		},
	},
	{
		.name = "register_interrupt",
		.ret_type = KOS_TYPE_VOID,
		.param_count = 2,
		.params = (kos_param_t[]) {
			{KOS_TYPE_OPAQUE_PTR, "wm"},
			{KOS_TYPE_U32, "ino"},
		},
	},
	{
		.name = "win_notify_mouse_motion",
		.ret_type = KOS_TYPE_VOID,
		.param_count = 4,
		.params = (kos_param_t[]) {
			{KOS_TYPE_OPAQUE_PTR, "win"},
			{KOS_TYPE_U32, "time"},
			{KOS_TYPE_U32, "x"},
			{KOS_TYPE_U32, "y"},
		},
	},
	{
		.name = "win_notify_mouse_button",
		.ret_type = KOS_TYPE_VOID,
		.param_count = 4,
		.params = (kos_param_t[]) {
			{KOS_TYPE_OPAQUE_PTR, "win"},
			{KOS_TYPE_U32, "time"},
			{KOS_TYPE_BOOL, "pressed"},
			{KOS_TYPE_U32, "button"},
		},
	},
	{
		.name = "win_set_size",
		.ret_type = KOS_TYPE_VOID,
		.param_count = 3,
		.params = (kos_param_t[]) {
			{KOS_TYPE_OPAQUE_PTR, "win"},
			{KOS_TYPE_U32, "x_res"},
			{KOS_TYPE_U32, "y_res"},
		},
	},
	{
		.name = "win_set_resizing",
		.ret_type = KOS_TYPE_VOID,
		.param_count = 2,
		.params = (kos_param_t[]) {
			{KOS_TYPE_OPAQUE_PTR, "win"},
			{KOS_TYPE_BOOL, "resizing"},
		},
	},
	{
		.name = "win_notify_mouse_axis",
		.ret_type = KOS_TYPE_VOID,
		.param_count = 7,
		.params = (kos_param_t[]) {
			{KOS_TYPE_OPAQUE_PTR, "win"},
			{KOS_TYPE_U32, "time"},
			{KOS_TYPE_U8, "source"},
			{KOS_TYPE_U8, "orientation"},
			{KOS_TYPE_U8, "direction"},
			{KOS_TYPE_F64, "delta"},
			{KOS_TYPE_I32, "delta_discrete"},
		},
	},
	{
		.name = "win_notify_gesture",
		.ret_type = KOS_TYPE_VOID,
		.param_count = 10,
		.params = (kos_param_t[]) {
			{KOS_TYPE_OPAQUE_PTR, "win"},
			{KOS_TYPE_U32, "time"},
			{KOS_TYPE_U8, "kind"},
			{KOS_TYPE_U8, "phase"},
			{KOS_TYPE_U32, "fingers"},
			{KOS_TYPE_F64, "dx"},
			{KOS_TYPE_F64, "dy"},
			{KOS_TYPE_F64, "scale"},
			{KOS_TYPE_F64, "rotation"},
			{KOS_TYPE_BOOL, "cancelled"},
		},
	},
	{
		.name = "win_notify_key",
		.ret_type = KOS_TYPE_VOID,
		.param_count = 4,
		.params = (kos_param_t[]) {
			{KOS_TYPE_OPAQUE_PTR, "win"},
			{KOS_TYPE_U32, "time"},
			{KOS_TYPE_BOOL, "pressed"},
			{KOS_TYPE_U32, "keycode"},
		},
	},
	{
		.name = "get_backend",
		.ret_type = KOS_TYPE_U8,
		.param_count = 1,
		.params = (kos_param_t[]) {
			{KOS_TYPE_OPAQUE_PTR, "wm"},
		},
	},
	{
		.name = "schedule_frame",
		.ret_type = KOS_TYPE_VOID,
		.param_count = 1,
		.params = (kos_param_t[]) {
			{KOS_TYPE_OPAQUE_PTR, "wm"},
		},
	},
};

static void conn(kos_cookie_t cookie, vid_t vid, uint64_t conn_id) {
	assert(VDRIVER.notif_cb != NULL);

	if (vid != only_vid) {
		kos_notif_t const notif = {
			.kind = KOS_NOTIF_CONN_FAIL,
			.cookie = cookie,
		};

		VDRIVER.notif_cb(&notif, VDRIVER.notif_data);
		return;
	}

	kos_notif_t const notif = {
		.kind = KOS_NOTIF_CONN,
		.conn_id = conn_id,
		.cookie = cookie,
		.conn = {
			.fn_count = sizeof FNS / sizeof *FNS,
			.fns = FNS,
			.const_count = sizeof CONSTS / sizeof *CONSTS,
			.consts = CONSTS,
		},
	};

	VDRIVER.notif_cb(&notif, VDRIVER.notif_data);
}

static void call(kos_cookie_t cookie, uint64_t vdev_id, uint64_t conn_id, uint64_t fn_id, kos_val_t const* args) {
	(void) vdev_id;

	assert(VDRIVER.notif_cb != NULL);

	kos_notif_t notif = {
		.kind = KOS_NOTIF_CALL_RET,
		.conn_id = conn_id,
		.cookie = cookie,
	};

	switch (fn_id) {
	case 0: { // create
		wm_t* const wm = wm_vdev_create();
		notif.call_ret.ret.opaque_ptr = vdriver_make_opaque_ptr(wm);
		break;
	}
	case 1: { // destroy
		wm_t* const wm = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);

		if (wm == NULL) {
			LOG_E(cls, "Tried to destroy non-local or NULL WM.");
			notif.kind = KOS_NOTIF_CALL_FAIL;
			break;
		}

		wm_vdev_destroy(wm);
		break;
	}
	case 2: { // loop
		wm_t* const wm = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);

		if (wm == NULL) {
			LOG_E(cls, "Tried to loop non-local or NULL WM.");
			notif.kind = KOS_NOTIF_CALL_FAIL;
			break;
		}

		wm_vdev_loop(wm);
		break;
	}
	case 3: { // register_interrupt
		wm_t* const wm = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);

		if (wm == NULL) {
			LOG_E(cls, "Tried to register interrupt for non-local or NULL WM.");
			notif.kind = KOS_NOTIF_CALL_FAIL;
			break;
		}

		wm->has_ino = true;
		wm->ino = args[1].u32;

		break;
	}
	case 4: { // win_notify_mouse_motion
		toplevel_t* const toplevel = (toplevel_t*) (uintptr_t) args[0].opaque_ptr.ptr;

		if (toplevel == NULL) {
			LOG_E(cls, "Tried to notify bad window of mouse motion.");
			notif.kind = KOS_NOTIF_CALL_FAIL;
			break;
		}

		wm_vdev_toplevel_notify_mouse_motion(toplevel, args[1].u32, args[2].u32, args[3].u32);

		break;
	}
	case 5: { // win_notify_mouse_button
		toplevel_t* const toplevel = (toplevel_t*) (uintptr_t) args[0].opaque_ptr.ptr;

		if (toplevel == NULL) {
			LOG_E(cls, "Tried to notify bad window of mouse button press/release.");
			notif.kind = KOS_NOTIF_CALL_FAIL;
			break;
		}

		wm_vdev_toplevel_notify_mouse_button(toplevel, args[1].u32, args[2].b, args[3].u32);

		break;
	}
	case 6: { // win_set_size
		toplevel_t* const toplevel = (toplevel_t*) (uintptr_t) args[0].opaque_ptr.ptr;

		if (toplevel == NULL) {
			LOG_E(cls, "Tried to set size of bad window.");
			notif.kind = KOS_NOTIF_CALL_FAIL;
			break;
		}

		wm_vdev_toplevel_set_size(toplevel, args[1].u32, args[2].u32);

		break;
	}
	case 7: { // win_set_resizing
		toplevel_t* const toplevel = (toplevel_t*) (uintptr_t) args[0].opaque_ptr.ptr;

		if (toplevel == NULL) {
			LOG_E(cls, "Tried to set resizing state of bad window.");
			notif.kind = KOS_NOTIF_CALL_FAIL;
			break;
		}

		wm_vdev_toplevel_set_resizing(toplevel, args[1].b);

		break;
	}
	case 8: { // win_notify_mouse_axis
		toplevel_t* const toplevel = (toplevel_t*) (uintptr_t) args[0].opaque_ptr.ptr;

		if (toplevel == NULL) {
			LOG_E(cls, "Tried to notify bad window of mouse axis.");
			notif.kind = KOS_NOTIF_CALL_FAIL;
			break;
		}

		if (args[2].u8 > AXIS_SOURCE_WHEEL_TILT || args[3].u8 > AXIS_ORIENTATION_HORIZONTAL || args[4].u8 > AXIS_DIRECTION_INVERTED) {
			LOG_E(cls, "Invalid mouse axis source, orientation, or direction.");
			notif.kind = KOS_NOTIF_CALL_FAIL;
			break;
		}

		wm_vdev_toplevel_notify_mouse_axis(toplevel, args[1].u32, args[2].u8, args[3].u8, args[4].u8, args[5].f64, args[6].i32);

		break;
	}
	case 9: { // win_notify_gesture
		toplevel_t* const toplevel = (toplevel_t*) (uintptr_t) args[0].opaque_ptr.ptr;

		if (toplevel == NULL) {
			LOG_E(cls, "Tried to notify bad window of gesture.");
			notif.kind = KOS_NOTIF_CALL_FAIL;
			break;
		}

		if (args[2].u8 > GESTURE_HOLD || args[3].u8 > GESTURE_PHASE_END) {
			LOG_E(cls, "Invalid gesture kind or phase.");
			notif.kind = KOS_NOTIF_CALL_FAIL;
			break;
		}

		wm_vdev_toplevel_notify_gesture(
			toplevel,
			args[1].u32,
			args[2].u8,
			args[3].u8,
			args[4].u32,
			args[5].f64,
			args[6].f64,
			args[7].f64,
			args[8].f64,
			args[9].b
		);

		break;
	}
	case 10: { // win_notify_key
		toplevel_t* const toplevel = (toplevel_t*) (uintptr_t) args[0].opaque_ptr.ptr;

		if (toplevel == NULL) {
			LOG_E(cls, "Tried to notify bad window of key press/release.");
			notif.kind = KOS_NOTIF_CALL_FAIL;
			break;
		}

		wm_vdev_toplevel_notify_key(toplevel, args[1].u32, args[2].b, args[3].u32);

		break;
	}
	case 11: { // get_backend
		wm_t* const wm = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);

		if (wm == NULL) {
			LOG_E(cls, "Tried to get backend of non-local or NULL WM.");
			notif.kind = KOS_NOTIF_CALL_FAIL;
			break;
		}

		notif.call_ret.ret.u8 = wm_vdev_get_backend(wm);
		break;
	}
	case 12: { // schedule_frame
		wm_t* const wm = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);

		if (wm == NULL) {
			LOG_E(cls, "Tried to schedule frame on non-local or NULL WM.");
			notif.kind = KOS_NOTIF_CALL_FAIL;
			break;
		}

		wm_vdev_schedule_frame(wm);
		break;
	}
	default:
		notif.kind = KOS_NOTIF_CALL_FAIL;
		break;
	}

	VDRIVER.notif_cb(&notif, VDRIVER.notif_data);
}

vdriver_t VDRIVER = {
	.spec = SPEC,
	.human = VDRIVER_HUMAN,
	.vers = VERS,
	.init = init,
	.probe = probe,
	.conn = conn,
	.call = call,
};
