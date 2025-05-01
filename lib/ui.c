// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2025 Aymeric Wibo

#include "ui.h"

#define __AQUA_LIB_COMPONENT__
#include "component.h"

#include <string.h>

#define SPEC "aquabsd.black.ui"

struct ui_ctx_t {
	uint64_t hid;
	uint64_t vid;
};

static component_t comp;

aqua_component_t ui_init(aqua_ctx_t ctx) {
	aqua_register_component(ctx, &comp);
	kos_req_vdev("aquabsd.black.ui");

	return &comp;
}

static bool probe(kos_vdev_descr_t const* vdev) {
	return strcmp((char*) vdev->spec, SPEC) == 0;
}

static component_t comp = {
	.probe = probe,
	.notif_conn = NULL,
	.notif_conn_fail = NULL,
	.notif_call_ret = NULL,
	.notif_call_fail = NULL,
	.interrupt = NULL,
	.vdev_count = 0,
	.vdevs = NULL,
};
