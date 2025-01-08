// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2025 Aymeric Wibo

#include "../../kos/vdev.h"

#include <assert.h>
#include <stdlib.h>

#define SPEC "aquabsd.black.wgpu"
#define VERS 0
#define VDRIVER_HUMAN "WebGPU driver"

vdriver_t VDRIVER;
static vid_t only_vid;

static void probe(void) {
	assert(VDRIVER.notif_cb != NULL);

	// TODO Report all the GPUs on the system?

	only_vid = VDRIVER.vdev_id_lo;

	kos_notif_t const notif = {
		.kind = KOS_NOTIF_ATTACH,
		.attach.vdev = {
							 .kind = KOS_VDEV_KIND_LOCAL,
							 .spec = SPEC,
							 .vers = VERS,
							 .human = "WebGPU GPU", // TODO Should we get the actual GPU's name here?
			.vdriver_human = VDRIVER_HUMAN,
							 },
	};

	VDRIVER.notif_cb(&notif, VDRIVER.notif_data);
}

static void conn(kos_cookie_t cookie, vid_t vid, uint64_t conn_id) {
	assert(VDRIVER.notif_cb != NULL);

	if (vid != only_vid) {
		kos_notif_t const notif = {
			.kind = KOS_NOTIF_CONN_FAIL,
		};

		VDRIVER.notif_cb(&notif, VDRIVER.notif_data);
		return;
	}

	kos_notif_t const notif = {
		.kind = KOS_NOTIF_CONN,
		.cookie = cookie,
		.conn = {
					.conn_id = conn_id,
					// TODO
			.fn_count = 0,
					.fns = NULL,
					},
	};

	VDRIVER.notif_cb(&notif, VDRIVER.notif_data);
}

static void call(kos_cookie_t cookie, uint64_t conn_id, uint64_t fn_id, void const* args) {
	assert(VDRIVER.notif_cb != NULL);

	// TODO

	(void) conn_id;
	(void) fn_id;
	(void) args;

	kos_notif_t const notif = {
		.kind = KOS_NOTIF_CALL_FAIL,
		.cookie = cookie,
	};

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
