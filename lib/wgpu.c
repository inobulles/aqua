// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2025 Aymeric Wibo

#include "wgpu.h"

#define __AQUA_LIB_COMPONENT__
#include "component.h"
#include "win_internal.h"

#include <stdio.h>
#include <string.h>

#define SPEC "aquabsd.black.wgpu"

struct wgpu_ctx_t {
	uint64_t hid;
	uint64_t vid;

	kos_cookie_t last_cookie;

	bool is_conn;
	uint64_t conn_id;

	struct {
	} consts;

	struct {
		uint32_t surface_from_win;
		uint32_t wgpuCreateInstance;
	} fns;

	bool last_success;
	kos_val_t last_ret;
};

static component_t comp;

static bool probe(kos_vdev_descr_t const* vdev) {
	return strcmp((char*) vdev->spec, SPEC) == 0;
}

WGPUSurface wgpu_surface_from_win(wgpu_ctx_t ctx, WGPUInstance instance, win_t win) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = instance,
		},
		{
			.opaque_ptr = win->opaque_ptr,
		},
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.surface_from_win, args);
	kos_flush(true);

	return ctx->last_ret.opaque_ptr;
}

WGPUInstance aqua_wgpuCreateInstance(wgpu_ctx_t ctx, WGPUInstanceDescriptor const* descriptor) {
	kos_val_t const args[] = {
		{
			.buf.size = sizeof *descriptor,
			.buf.ptr = (void*) descriptor,
		},
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuCreateInstance, args);
	kos_flush(true);

	return ctx->last_ret.opaque_ptr;
}

aqua_component_t wgpu_init(aqua_ctx_t ctx) {
	aqua_register_component(ctx, &comp);
	kos_req_vdev("aquabsd.black.wgpu");

	return &comp;
}

wgpu_ctx_t wgpu_conn(kos_vdev_descr_t const* vdev) {
	wgpu_ctx_t const ctx = calloc(1, sizeof *ctx);

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

void wgpu_disconn(wgpu_ctx_t ctx) {
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
	wgpu_ctx_t const ctx = data;

	if (ctx == NULL || notif->cookie != ctx->last_cookie) {
		return;
	}

	ctx->conn_id = notif->conn_id;
	ctx->is_conn = true;

	// Read constants.

	memset(&ctx->consts, 0xFF, sizeof ctx->consts);

	for (size_t i = 0; i < notif->conn.const_count; i++) {
		kos_const_t const* const c = &notif->conn.consts[i];
		char const* const name = (void*) c->name;

		printf("WebGPU const: %s\n", name);
	}

	for (size_t i = 0; i < sizeof ctx->consts / sizeof(uint32_t); i++) {
		if (((uint32_t*) &ctx->consts)[i] == -1u) {
			ctx->is_conn = false;
			break;
		}
	}

	// Read functions.

	memset(&ctx->fns, 0xFF, sizeof ctx->fns);

	for (size_t i = 0; i < notif->conn.fn_count; i++) {
		kos_fn_t const* const fn = &notif->conn.fns[i];
		char const* const name = (void*) fn->name;

		if (
			strcmp(name, "surface_from_win") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "instance") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "win") == 0
		) {
			ctx->fns.surface_from_win = i;
		}

		if (
			strcmp(name, "wgpuCreateInstance") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[0].name, "descriptor") == 0
		) {
			ctx->fns.wgpuCreateInstance = i;
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
	wgpu_ctx_t const ctx = data;

	if (ctx == NULL || !ctx->is_conn || notif->cookie != ctx->last_cookie) {
		return;
	}

	ctx->last_success = true;
	ctx->last_ret = notif->call_ret.ret;
}

static void notif_call_fail(kos_notif_t const* notif, void* data) {
	wgpu_ctx_t const ctx = data;
	ctx->last_success = false;

	(void) notif;

	fprintf(stderr, "TODO Call failed, but how do we handle this?\n");
}

static component_t comp = {
	.probe = probe,
	.notif_conn = notif_conn,
	.notif_conn_fail = notif_conn_fail,
	.notif_call_ret = notif_call_ret,
	.notif_call_fail = notif_call_fail,
	.vdev_count = 0,
	.vdevs = NULL,
};
