// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#include "ui.h"

#define __AQUA_LIB_COMPONENT__
#include "component.h"

#include "ui_internal.h"

#include <umber.h>

#include <assert.h>
#include <string.h>

#define SPEC "aquabsd.black.ui"

static component_t comp;
struct umber_class_t const* cls = NULL;

static __attribute__((constructor)) void init(void) {
	cls = umber_class_new("aqua.lib.ui", UMBER_LVL_INFO, "AQUA standard library: UI.");
}

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

	if (!ctx->is_conn) {
		free(ctx);
		return NULL;
	}

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

ui_t ui_create(ui_ctx_t ctx) {
	assert(ctx != NULL && ctx->is_conn);
	ui_t const ui = calloc(1, sizeof *ui);

	if (ui == NULL) {
		LOG_E(cls, "Failed to allocate UI object.");
		return NULL;
	}

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.create, NULL);
	kos_flush(true);

	ui->ctx = ctx;
	ui->opaque_ptr = ctx->last_ret.opaque_ptr;

	return ui;
}

void ui_destroy(ui_t ui) {
	ui_ctx_t const ctx = ui->ctx;

	kos_val_t const args[] = {
		{.opaque_ptr = ui->opaque_ptr},
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.destroy, args);
	kos_flush(true);

	free(ui);
}

static void notif_call_ret(kos_notif_t const* notif, void* data) {
	ui_ctx_t const ctx = data;

	if (ctx == NULL || !ctx->is_conn || notif->cookie != ctx->last_cookie) {
		return;
	}

	ctx->last_success = true;
	ctx->last_ret = notif->call_ret.ret;
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
			LOG_W(cls, "Missing constant %zu. Refusing connection.", i);
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
			LOG_W(cls, "Missing function %zu. Refusing connection.", i);
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
			fn->param_count == 4 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "ui") == 0 &&
			fn->params[1].type == KOS_TYPE_U64 &&
			strcmp((char*) fn->params[1].name, "hid") == 0 &&
			fn->params[2].type == KOS_TYPE_U64 &&
			strcmp((char*) fn->params[2].name, "cid") == 0 &&
			fn->params[3].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[3].name, "device") == 0
		) {
			ctx->backend_wgpu_fns.init = i;
		}

		if (
			strcmp(name, "backend_wgpu_render") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 4 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "ui") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "frame") == 0 &&
			fn->params[2].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[2].name, "command_encoder") == 0
		) {
			ctx->backend_wgpu_fns.render = i;
		}
	}

	for (size_t i = 0; i < sizeof ctx->backend_wgpu_fns / sizeof(uint32_t); i++) {
		if (((uint32_t*) &ctx->backend_wgpu_fns)[i] == -1u) {
			ctx->supported_backends &= ~UI_BACKEND_WGPU;
			break;
		}
	}

	if (ctx->supported_backends & UI_BACKEND_WGPU) {
		LOG_I(cls, "WebGPU UI backend is supported.");
	}

	else {
		LOG_W(cls, "WebGPU UI backend is not supported.");
	}
}

static component_t comp = {
	.probe = probe,
	.notif_conn = notif_conn,
	.notif_conn_fail = NULL,
	.notif_call_ret = notif_call_ret,
	.notif_call_fail = NULL,
	.interrupt = NULL,
	.vdev_count = 0,
	.vdevs = NULL,
};
