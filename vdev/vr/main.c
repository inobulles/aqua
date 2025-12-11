// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include "mist.h"

#include <aqua/vdriver.h>

#include <umber.h>

#include <assert.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define SPEC "aquabsd.black.vr"
#define VERS 0
#define VDRIVER_HUMAN "Experimental VR driver (for Project Mist)"

typedef struct {
	uint32_t x_res;
	uint32_t y_res;
	uint64_t seq;
} win_hdr_t;

static umber_class_t const* cls = NULL;
static vid_t only_vid;

mist_ops_t MIST_OPS = {
	.set = false,
};

static void init(void) {
	cls = umber_class_new(SPEC, UMBER_LVL_WARN, SPEC " VR VDRIVER.");
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
			.human = "VR device",
			.vdriver_human = VDRIVER_HUMAN,

			.pref = 0,
			.host_id = VDRIVER.host_id,
			.vdev_id = only_vid,
		},
	};

	VDRIVER.notif_cb(&notif, VDRIVER.notif_data);
}

static kos_const_t const CONSTS[] = {};

static kos_fn_t const FNS[] = {
	{
		.name = "send_win",
		.ret_type = KOS_TYPE_VOID,
		.param_count = 7,
		.params = (kos_param_t[]) {
			{KOS_TYPE_U32, "id"},
			{KOS_TYPE_U32, "x_res"},
			{KOS_TYPE_U32, "y_res"},
			{KOS_TYPE_U32, "tiles_x"},
			{KOS_TYPE_U32, "tiles_y"},
			{KOS_TYPE_BUF, "tile_update_bitmap"},
			{KOS_TYPE_BUF, "tile_data"},
		},
	},
	{
		.name = "destroy_win",
		.ret_type = KOS_TYPE_VOID,
		.param_count = 1,
		.params = (kos_param_t[]) {
			{KOS_TYPE_U32, "id"},
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

static void call(kos_cookie_t cookie, vid_t vdev_id, uint64_t conn_id, uint64_t fn_id, kos_val_t const* args) {
	(void) vdev_id;

	assert(VDRIVER.notif_cb != NULL);

	kos_notif_t notif = {
		.kind = KOS_NOTIF_CALL_RET,
		.conn_id = conn_id,
		.cookie = cookie,
	};

	switch (MIST_OPS.set ? fn_id : -1) {
	case 0: { // send_win
		uint32_t const id = args[0].u32;
		uint32_t const x_res = args[1].u32;
		uint32_t const y_res = args[2].u32;
		uint32_t const tiles_x = args[3].u32;
		uint32_t const tiles_y = args[4].u32;

		uint32_t const tile_x_res = x_res / tiles_x;
		uint32_t const tile_y_res = y_res / tiles_y;

		uint64_t const* const tile_update_bitmap = args[5].buf.ptr;
		assert(args[5].buf.size == tiles_x * tiles_y / 8);

		size_t tile_update_count = 0;

		for (size_t i = 0; i < tiles_x * tiles_y / sizeof *tile_update_bitmap / 8; i++) {
			tile_update_count += __builtin_popcountll(tile_update_bitmap[i]);
		}

		void const* const tile_data = args[6].buf.ptr;
		assert(args[6].buf.size == tile_x_res * tile_y_res * tile_update_count * 4);

		MIST_OPS.send_win(id, x_res, y_res, tiles_x, tiles_y, tile_update_bitmap, tile_data);
		break;
	}
	case 1: { // destroy_win
		uint32_t const id = args[0].u32;
		MIST_OPS.destroy_win(id);
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
