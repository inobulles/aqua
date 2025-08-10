// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#include <aqua/vdriver.h>

#include <umber.h>

#include <assert.h>
#include <stddef.h>

// TODO This VDRIVER is pretty easily testable - write tests!
//      But be careful to actually test stuff relevant to the VDRIVER, i.e. not accidentally just end up testing Pango.
// TODO Also, I need comprehensive documentation for each function here.

#define SPEC "aquabsd.black.font"
#define VERS 0
#define VDRIVER_HUMAN "Font driver"

static umber_class_t const* cls = NULL;
static vid_t only_vid;

static void init(void) {
	cls = umber_class_new(SPEC, UMBER_LVL_WARN, SPEC " Font VDRIVER.");
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
			.human = "Font rendering device",
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
		.name = "descr_from_str",
		.ret_type = KOS_TYPE_OPAQUE_PTR,
		.param_count = 1,
		.params = (kos_param_t[]) {
			{
				.type = KOS_TYPE_BUF,
				.name = "str",
			},
		},
	},
	{
		.name = "descr_destroy",
		.ret_type = KOS_TYPE_VOID,
		.param_count = 1,
		.params = (kos_param_t[]) {
			{
				.type = KOS_TYPE_OPAQUE_PTR,
				.name = "descr",
			},
		},
	},
	{
		.name = "layout_create",
		.ret_type = KOS_TYPE_OPAQUE_PTR,
		.param_count = 2,
		.params = (kos_param_t[]) {
			{
				.type = KOS_TYPE_OPAQUE_PTR,
				.name = "descr",
			},
			{
				.type = KOS_TYPE_BUF,
				.name = "text",
			},
		},
	},
	{
		.name = "layout_destroy",
		.ret_type = KOS_TYPE_VOID,
		.param_count = 1,
		.params = (kos_param_t[]) {
			{
				.type = KOS_TYPE_OPAQUE_PTR,
				.name = "layout",
			},
		},
	},
	{
		.name = "layout_set_text",
		.ret_type = KOS_TYPE_VOID,
		.param_count = 2,
		.params = (kos_param_t[]) {
			{
				.type = KOS_TYPE_OPAQUE_PTR,
				.name = "layout",
			},
			{
				.type = KOS_TYPE_BUF,
				.name = "text",
			},
		},
	},
	{
		.name = "layout_set_limits",
		.ret_type = KOS_TYPE_VOID,
		.param_count = 3,
		.params = (kos_param_t[]) {
			{
				.type = KOS_TYPE_OPAQUE_PTR,
				.name = "layout",
			},
			{
				.type = KOS_TYPE_U32,
				.name = "x_res_limit",
			},
			{
				.type = KOS_TYPE_U32,
				.name = "y_res_limit",
			},
		},
	},
	{
		.name = "layout_pos_to_index",
		.ret_type = KOS_TYPE_I32,
		.param_count = 3,
		.params = (kos_param_t[]) {
			{
				.type = KOS_TYPE_OPAQUE_PTR,
				.name = "layout",
			},
			{
				.type = KOS_TYPE_U32,
				.name = "x",
			},
			{
				.type = KOS_TYPE_U32,
				.name = "y",
			},
		},
	},
	{
		.name = "layout_index_to_pos",
		.ret_type = KOS_TYPE_VOID,
		.param_count = 3,
		.params = (kos_param_t[]) {
			{
				.type = KOS_TYPE_OPAQUE_PTR,
				.name = "layout",
			},
			{
				.type = KOS_TYPE_I32,
				.name = "index",
			},
			{
				.type = KOS_TYPE_PTR,
				.name = "x",
			},
			{
				.type = KOS_TYPE_PTR,
				.name = "y",
			},
		},
	},
	{
		.name = "layout_get_res",
		.ret_type = KOS_TYPE_VOID,
		.param_count = 3,
		.params = (kos_param_t[]) {
			{
				.type = KOS_TYPE_OPAQUE_PTR,
				.name = "layout",
			},
			{
				.type = KOS_TYPE_PTR,
				.name = "x_res",
			},
			{
				.type = KOS_TYPE_PTR,
				.name = "y_res",
			},
		},
	},
	{
		.name = "layout_render",
		.ret_type = KOS_TYPE_VOID, // TODO Is it best to have the VDRIVER write into a buffer supplied by the caller or for us to allocate a buffer and return it? If we allocate the buffer it makes things harder because we have to provide a way for the client to let us know when we can free it I guess.
		.param_count = 2,
		.params = (kos_param_t[]) {
			{
				.type = KOS_TYPE_OPAQUE_PTR,
				.name = "layout",
			},
			{
				.type = KOS_TYPE_PTR,
				.name = "buffer",
			},
		},
	},
	// TODO Functions for setting colour, markup on/off, alignment (constants!), word/char wrapping.
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
	case 0:
		(void) args; // TODO
		notif.kind = KOS_NOTIF_CALL_FAIL;
		break;
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
