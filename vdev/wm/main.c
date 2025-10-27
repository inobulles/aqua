// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

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
	/*
	{
		.name = "get_win_fb",
		.ret_type = KOS_TYPE_PTR,
		.param_count = 4,
		.params = (kos_param_t[]) {
			{KOS_TYPE_OPAQUE_PTR, "wm"},
			{KOS_TYPE_OPAQUE_PTR, "win"},
			{KOS_TYPE_PTR, "x_res"},
			{KOS_TYPE_PTR, "y_res"},
		},
	},
	*/
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
		},
	};

	VDRIVER.notif_cb(&notif, VDRIVER.notif_data);
}

static void call(kos_cookie_t cookie, uint64_t conn_id, uint64_t fn_id, kos_val_t const* args) {
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
