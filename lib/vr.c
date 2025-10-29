// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#include "vr.h"

#define __AQUA_LIB_COMPONENT__
#include "component.h"

#include <stdio.h>
#include <string.h>

#define SPEC "aquabsd.black.vr"

struct vr_ctx_t {
	uint64_t hid;
	uint64_t vid;

	kos_cookie_t last_cookie;

	bool is_conn;
	uint64_t conn_id;

	struct {
		uint32_t send_win;
	} fns;

	bool last_success;
	kos_val_t last_ret;
};

static component_t comp;

aqua_component_t vr_init(aqua_ctx_t ctx) {
	aqua_register_component(ctx, &comp);
	kos_req_vdev("aquabsd.black.vr");

	return &comp;
}

static bool probe(kos_vdev_descr_t const* vdev) {
	return strcmp((char*) vdev->spec, SPEC) == 0;
}

vr_ctx_t vr_conn(kos_vdev_descr_t const* vdev) {
	vr_ctx_t const ctx = calloc(1, sizeof *ctx);

	if (ctx == NULL) {
		return NULL;
	}

	ctx->hid = vdev->host_id;
	ctx->vid = vdev->vdev_id;

	ctx->is_conn = false;
	ctx->last_cookie = kos_vdev_conn(ctx->hid, ctx->vid);

	// Add pending connection.

	cookie_notif_conn_tuple_t tuple = {
		.cookie = ctx->last_cookie,
		.comp = &comp,
		.data = ctx,
	};

	aqua_add_pending_conn(comp.ctx, &tuple);

	// Finally, flush.

	kos_flush(true);

	return ctx;
}

void vr_disconn(vr_ctx_t ctx) {
	if (ctx == NULL) {
		return;
	}

	if (!ctx->is_conn) {
		free(ctx);
		return;
	}

	kos_vdev_disconn(ctx->conn_id);
	free(ctx);
}

static void notif_conn(kos_notif_t const* notif, void* data) {
	vr_ctx_t const ctx = data;

	if (ctx == NULL || notif->cookie != ctx->last_cookie) {
		return;
	}

	ctx->conn_id = notif->conn_id;
	ctx->is_conn = true;

	// Read functions.

	memset(&ctx->fns, 0xFF, sizeof ctx->fns);

	for (size_t i = 0; i < notif->conn.fn_count; i++) {
		kos_fn_t const* const fn = &notif->conn.fns[i];
		char const* const name = (void*) fn->name;

		if (
			strcmp(name, "send_win") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 4 &&
			strcmp((char*) fn->params[0].name, "id") == 0 &&
			fn->params[0].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[1].name, "x_res") == 0 &&
			fn->params[1].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[2].name, "y_res") == 0 &&
			fn->params[2].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[3].name, "fb") == 0 &&
			fn->params[3].type == KOS_TYPE_BUF
		) {
			ctx->fns.send_win = i;
		}
	}

	for (size_t i = 0; i < sizeof ctx->fns / sizeof(uint32_t); i++) {
		if (((uint32_t*) &ctx->fns)[i] == -1u) {
			ctx->is_conn = false;
			break;
		}
	}
}

static void notif_conn_fail(kos_notif_t const* notif, void* data) {
	(void) notif;
	(void) data;

	fprintf(stderr, "TODO Connection failed, but how do we handle this?\n");
}

static void notif_call_ret(kos_notif_t const* notif, void* data) {
	vr_ctx_t const ctx = data;

	if (ctx == NULL || !ctx->is_conn || notif->cookie != ctx->last_cookie) {
		return;
	}

	ctx->last_success = true;
	ctx->last_ret = notif->call_ret.ret;
}

static void notif_call_fail(kos_notif_t const* notif, void* data) {
	vr_ctx_t const ctx = data;
	ctx->last_success = false;

	(void) notif;

	fprintf(stderr, "TODO Call failed, but how do we handle this?\n");
}

void vr_send_win(vr_ctx_t ctx, uint32_t id, uint32_t x_res, uint32_t y_res, void* fb) {
	if (!ctx->is_conn) {
		return;
	}

	kos_val_t const args[] = {
		{.u32 = id},
		{.u32 = x_res},
		{.u32 = y_res},
		{.buf = {x_res * y_res * 4, fb}},
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.send_win, args);
	kos_flush(true);
}

static component_t comp = {
	.probe = probe,
	.notif_conn = notif_conn,
	.notif_conn_fail = notif_conn_fail,
	.notif_call_ret = notif_call_ret,
	.notif_call_fail = notif_call_fail,
};
