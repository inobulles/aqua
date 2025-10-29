// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#include <aqua/vdriver.h>

#include <umber.h>

#include <assert.h>
#include <stddef.h>

#define SPEC "aquabsd.black.test"
#define VERS 0
#define VDRIVER_HUMAN "Test driver"

static umber_class_t const* cls = NULL;
static vid_t only_vid;

static void init(void) {
	cls = umber_class_new(SPEC, UMBER_LVL_WARN, SPEC " test VDRIVER.");
	assert(cls != NULL);
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
			.human = "Test device",
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
		.name = "SIXTY_NINE",
		.type = KOS_TYPE_U8,
		.val = {
			.u8 = 69,
		},
	},
	{
		.name = "FOUR_TWENTY",
		.type = KOS_TYPE_U16,
		.val = {
			.u16 = 420,
		},
	},
};

static kos_fn_t const FNS[] = {
	{
		.name = "add",
		.ret_type = KOS_TYPE_U64,
		.param_count = 2,
		.params = (kos_param_t[]) {
			{KOS_TYPE_U64, "a"},
			{KOS_TYPE_U64, "b"},
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
			.const_count = sizeof CONSTS / sizeof *CONSTS,
			.consts = CONSTS,
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
	case 0: { // add
		uint64_t const a = args[0].u64;
		uint64_t const b = args[1].u64;

		notif.call_ret.ret.u64 = a + b;
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
