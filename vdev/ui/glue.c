// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#include "../../kos/vdev.h"

#include <assert.h>
#include <unistd.h>

#define SPEC "aquabsd.black.ui"
#define VERS 0
#define VDRIVER_HUMAN "UI driver"

vdriver_t VDRIVER;
static vid_t only_vid;

static void probe(void) {
	assert(VDRIVER.notif_cb != NULL);

	only_vid = VDRIVER.vdev_id_lo;

	kos_notif_t const notif = {
		.kind = KOS_NOTIF_ATTACH,
		.attach.vdev = {
			.kind = KOS_VDEV_KIND_LOCAL,
			.spec = SPEC,
			.vers = VERS,
			.human = "UI device",
			.vdriver_human = VDRIVER_HUMAN,

			.pref = 0,
			.host_id = 0,
			.vdev_id = only_vid,
		},
	};

	VDRIVER.notif_cb(&notif, VDRIVER.notif_data);
}

static kos_const_t const CONSTS[] = {
	{
		.name = "ELEM_KIND_DIV",
		.type = KOS_TYPE_U32,
		.val.u32 = 0,
	},
	{
		.name = "ELEM_KIND_TEXT",
		.type = KOS_TYPE_U32,
		.val.u32 = 1,
	},
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
			{
				.type = KOS_TYPE_OPAQUE_PTR,
				.name = "ui",
			},
		},
	},
	{
		.name = "get_root",
		.ret_type = KOS_TYPE_OPAQUE_PTR,
		.param_count = 1,
		.params = (kos_param_t[]) {
			{
				.type = KOS_TYPE_OPAQUE_PTR,
				.name = "ui",
			},
		},
	},
	{
		.name = "add",
		.ret_type = KOS_TYPE_OPAQUE_PTR,
		.param_count = 3,
		.params = (kos_param_t[]) {
			{
				.type = KOS_TYPE_OPAQUE_PTR,
				.name = "ui",
			},
			{
				.type = KOS_TYPE_U32,
				.name = "kind",
			},
			{
				.type = KOS_TYPE_BUF,
				.name = "semantics",
			},
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
			.const_count = sizeof(CONSTS) / sizeof(*CONSTS),
			.consts = CONSTS,
			.fn_count = sizeof(FNS) / sizeof(*FNS),
			.fns = FNS,
		},
	};

	VDRIVER.notif_cb(&notif, VDRIVER.notif_data);
}

extern void* GoCreate(void);
extern void GoDestroy(void* ui);

static void call(kos_cookie_t cookie, uint64_t conn_id, uint64_t fn_id, kos_val_t const* args) {
	assert(VDRIVER.notif_cb != NULL);

	kos_notif_t notif = {
		.kind = KOS_NOTIF_CALL_RET,
		.conn_id = conn_id,
		.cookie = cookie,
	};

	switch (fn_id) {
	case 0:
		notif.call_ret.ret.opaque_ptr = GoCreate();
		break;
	case 1:
		GoDestroy(args[0].opaque_ptr);
		break;
	default:
		assert(false); // TODO This should probably return CALL_FAIL or something.
	}

	VDRIVER.notif_cb(&notif, VDRIVER.notif_data);
}

vdriver_t VDRIVER = {
	.spec = SPEC,
	.human = VDRIVER_HUMAN,
	.vers = VERS,
	.probe = probe,
	.conn = conn,
	.call = call,
};
