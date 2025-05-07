// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2025 Aymeric Wibo

#include "ui.h"

#define __AQUA_LIB_COMPONENT__
#include "component.h"

#include <string.h>

#define SPEC "aquabsd.black.ui"

typedef struct {
	uint32_t init;
	uint32_t render;
} backend_fns_t;

struct ui_ctx_t {
	// TODO This is pretty repetitive between library components - maybe we should inherit from a base struct?

	uint64_t hid;
	uint64_t vid;

	kos_cookie_t last_cookie;

	bool is_conn;
	uint64_t conn_id;

	struct {
		uint32_t ELEM_KIND_DIV;
		uint32_t ELEM_KIND_TEXT;
	} consts;

	struct {
		uint32_t create;
		uint32_t destroy;
		uint32_t get_root;
		uint32_t add;
	} fns;

	// Backend-specific functions.

	ui_supported_backends_t supported_backends;
	backend_fns_t backend_wgpu_fns;
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

ui_ctx_t ui_conn(kos_vdev_descr_t const* vdev) {
	ui_ctx_t const ctx = calloc(1, sizeof *ctx);

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

void ui_disconn(ui_ctx_t ctx) {
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

ui_supported_backends_t ui_get_supported_backends(ui_ctx_t ctx) {
	return ctx->supported_backends;
}

static void notif_conn(kos_notif_t const* notif, void* data) {
	ui_ctx_t const ctx = data;

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

		if (strcmp(name, "ELEM_KIND_DIV") == 0) {
			ctx->consts.ELEM_KIND_DIV = c->val.u8;
		}

		if (strcmp(name, "ELEM_KIND_TEXT") == 0) {
			ctx->consts.ELEM_KIND_TEXT = c->val.u8;
		}
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
			strcmp(name, "create") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 0
		) {
			ctx->fns.create = i;
		}

		if (
			strcmp(name, "destroy") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "ui") == 0
		) {
			ctx->fns.destroy = i;
		}

		if (
			strcmp(name, "get_root") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "ui") == 0
		) {
			ctx->fns.get_root = i;
		}

		if (
			strcmp(name, "add") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 3 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "ui") == 0 &&
			fn->params[1].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[1].name, "kind") == 0 &&
			fn->params[2].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[2].name, "semantics") == 0
		) {
			ctx->fns.add = i;
		}
	}

	for (size_t i = 0; i < sizeof ctx->fns / sizeof(uint32_t); i++) {
		if (((uint32_t*) &ctx->fns)[i] == -1u) {
			ctx->is_conn = false;
			break;
		}
	}

	// Read backend functions.
	// By default, we assume all backends are supported, and we remove them if they're missing functions.

	ctx->supported_backends = UI_BACKEND_NONE | UI_BACKEND_WGPU;

	memset(&ctx->backend_wgpu_fns, 0xFF, sizeof ctx->backend_wgpu_fns);

	for (size_t i = 0; i < notif->conn.fn_count; i++) {
		kos_fn_t const* const fn = &notif->conn.fns[i];
		char const* const name = (void*) fn->name;

		if (
			strcmp(name, "backend_wgpu_init") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 6 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "ui") == 0 &&
			fn->params[1].type == KOS_TYPE_U64 &&
			strcmp((char*) fn->params[1].name, "hid") == 0 &&
			fn->params[2].type == KOS_TYPE_U64 &&
			strcmp((char*) fn->params[2].name, "vid") == 0 &&
			fn->params[3].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[3].name, "device") == 0 &&
			fn->params[4].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[4].name, "queue") == 0 &&
			fn->params[5].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[5].name, "surface_caps") == 0
		) {
			ctx->backend_wgpu_fns.init = i;
		}

		if (
			strcmp(name, "backend_wgpu_render") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "ui") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "command_encoder") == 0
		) {
			ctx->backend_wgpu_fns.render = i;
		}
	}

	for (size_t i = 0; i < sizeof ctx->fns / sizeof(uint32_t); i++) {
		if (((uint32_t*) &ctx->fns)[i] == -1u) {
			ctx->supported_backends &= ~UI_BACKEND_WGPU;
			break;
		}
	}
}

static component_t comp = {
	.probe = probe,
	.notif_conn = notif_conn,
	.notif_conn_fail = NULL,
	.notif_call_ret = NULL,
	.notif_call_fail = NULL,
	.interrupt = NULL,
	.vdev_count = 0,
	.vdevs = NULL,
};
