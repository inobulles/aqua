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
	ctx->last_success = false;
	kos_flush(true);

	if (!ctx->last_success) {
		LOG_E(cls, "Failed to create UI.");
		free(ui);
		return NULL;
	}

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

ui_elem_t ui_get_root(ui_t ui) {
	ui_ctx_t const ctx = ui->ctx;

	ui_elem_t const elem = calloc(1, sizeof *elem);

	if (elem == NULL) {
		LOG_E(cls, "Failed to allocate UI element object.");
		return NULL;
	}

	kos_val_t const args[] = {
		{.opaque_ptr = ui->opaque_ptr},
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.get_root, args);
	ctx->last_success = false;
	kos_flush(true);

	if (!ctx->last_success) {
		LOG_E(cls, "Failed to get root element.");
		return NULL;
	}

	elem->ui = ui;
	elem->opaque_ptr = ctx->last_ret.opaque_ptr;

	return elem;
}

ui_elem_t ui_add_div(ui_elem_t parent, char const* semantics) {
	ui_t const ui = parent->ui;
	ui_ctx_t const ctx = ui->ctx;

	ui_elem_t const elem = calloc(1, sizeof *elem);

	if (elem == NULL) {
		LOG_E(cls, "Failed to allocate UI element object.");
		return NULL;
	}

	kos_val_t const args[] = {
		{.opaque_ptr = parent->opaque_ptr},
		{.buf = {
			 .ptr = (void*) semantics,
			 .size = strlen(semantics),
		 }},
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.add_div, args);
	ctx->last_success = false;
	kos_flush(true);

	if (!ctx->last_success) {
		LOG_E(cls, "Failed to add div element.");
		return NULL;
	}

	elem->ui = ui;
	elem->opaque_ptr = ctx->last_ret.opaque_ptr;

	return elem;
}

ui_elem_t ui_add_text(ui_elem_t parent, char const* semantics, char const* text) {
	ui_t const ui = parent->ui;
	ui_ctx_t const ctx = ui->ctx;

	ui_elem_t const elem = calloc(1, sizeof *elem);

	if (elem == NULL) {
		LOG_E(cls, "Failed to allocate UI element object.");
		return NULL;
	}

	kos_val_t const args[] = {
		{.opaque_ptr = parent->opaque_ptr},
		{.buf = {
			 .ptr = (void*) semantics,
			 .size = strlen(semantics),
		 }},
		{.buf = {
			 .ptr = (void*) text,
			 .size = strlen(text),
		 }},
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.add_text, args);
	ctx->last_success = false;
	kos_flush(true);

	if (!ctx->last_success) {
		LOG_E(cls, "Failed to add text element.");
		return NULL;
	}

	elem->ui = ui;
	elem->opaque_ptr = ctx->last_ret.opaque_ptr;

	return elem;
}

void ui_rem_elem(ui_elem_t elem) {
	ui_ctx_t const ctx = elem->ui->ctx;

	kos_val_t const args[] = {
		{.opaque_ptr = elem->opaque_ptr},
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.rem_elem, args);
	kos_flush(true);
}

void ui_move_elem(ui_elem_t elem, ui_elem_t new_parent, bool beginning) {
	ui_ctx_t const ctx = elem->ui->ctx;

	kos_val_t const args[] = {
		{.opaque_ptr = elem->opaque_ptr},
		{.opaque_ptr = new_parent == NULL ? (kos_opaque_ptr_t) {0, 0} : new_parent->opaque_ptr},
		{.b = beginning},
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.move_elem, args);
	kos_flush(true);
}

static bool ui_set_attr_common(
	ui_ctx_t ctx,
	uint32_t fn_id,
	ui_elem_t elem,
	char const* key,
	size_t val_count,
	kos_val_t const vals[val_count]
) {
	kos_val_t args[2 + val_count];

	args[0] = (kos_val_t) {.opaque_ptr = elem->opaque_ptr};
	args[1] = (kos_val_t) {.buf = {strlen(key), (void*) key}};

	memcpy(&args[2], vals, val_count * sizeof *vals);

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, fn_id, args);
	ctx->last_success = false;
	kos_flush(true);

	if (!ctx->last_success) {
		LOG_E(cls, "Failed to set attribute.");
		return NULL;
	}

	return ctx->last_ret.b;
}

bool ui_set_attr_str(ui_elem_t elem, char const* key, char const* val) {
	ui_t const ui = elem->ui;
	ui_ctx_t const ctx = ui->ctx;

	kos_val_t const kos_val = {
		.buf = {strlen(val), (void*) val},
	};

	return ui_set_attr_common(ctx, ctx->fns.set_attr_str, elem, key, 1, &kos_val);
}

bool ui_set_attr_bool(ui_elem_t elem, char const* key, bool val) {
	ui_t const ui = elem->ui;
	ui_ctx_t const ctx = ui->ctx;

	return ui_set_attr_common(ctx, ctx->fns.set_attr_bool, elem, key, 1, &(kos_val_t) {.b = val});
}

bool ui_set_attr_u32(ui_elem_t elem, char const* key, uint32_t val) {
	ui_t const ui = elem->ui;
	ui_ctx_t const ctx = ui->ctx;

	return ui_set_attr_common(ctx, ctx->fns.set_attr_u32, elem, key, 1, &(kos_val_t) {.u32 = val});
}

bool ui_set_attr_f32(ui_elem_t elem, char const* key, float val) {
	ui_t const ui = elem->ui;
	ui_ctx_t const ctx = ui->ctx;

	return ui_set_attr_common(ctx, ctx->fns.set_attr_f32, elem, key, 1, &(kos_val_t) {.f32 = val});
}

bool ui_set_attr_opaque_ptr(ui_elem_t elem, char const* key, void* val) {
	ui_t const ui = elem->ui;
	ui_ctx_t const ctx = ui->ctx;

	kos_val_t const opaque_ptr = {
		.opaque_ptr = {.ptr = (uint64_t) (uintptr_t) val},
	};

	return ui_set_attr_common(ctx, ctx->fns.set_attr_opaque_ptr, elem, key, 1, &opaque_ptr);
}

bool ui_set_attr_dim(ui_elem_t elem, char const* key, ui_dim_t dim) {
	ui_t const ui = elem->ui;
	ui_ctx_t const ctx = ui->ctx;

	kos_val_t const vals[] = {
		{.u32 = dim.units},
		{.f32 = dim.val},
	};

	return ui_set_attr_common(ctx, ctx->fns.set_attr_dim, elem, key, sizeof vals / sizeof *vals, vals);
}

bool ui_set_attr_raster(ui_elem_t elem, char const* key, ui_raster_t raster) {
	ui_t const ui = elem->ui;
	ui_ctx_t const ctx = ui->ctx;

	size_t const size = raster.x_res * raster.y_res * 4;

	kos_val_t const vals[] = {
		{.u32 = raster.x_res},
		{.u32 = raster.y_res},
		{.buf = {size, raster.data}},
	};

	return ui_set_attr_common(ctx, ctx->fns.set_attr_raster, elem, key, sizeof vals / sizeof *vals, vals);
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
			strcmp(name, "add_div") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "parent") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "semantics") == 0
		) {
			ctx->fns.add_div = i;
		}

		if (
			strcmp(name, "add_text") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 3 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "parent") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "semantics") == 0 &&
			fn->params[2].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[2].name, "text") == 0
		) {
			ctx->fns.add_text = i;
		}

		if (
			strcmp(name, "rem_elem") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "elem") == 0
		) {
			ctx->fns.rem_elem = i;
		}

		if (
			strcmp(name, "move_elem") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 3 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "elem") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "new_parent") == 0 &&
			fn->params[2].type == KOS_TYPE_BOOL &&
			strcmp((char*) fn->params[2].name, "beginning") == 0
		) {
			ctx->fns.move_elem = i;
		}

		if (
			fn->ret_type == KOS_TYPE_BOOL &&
			fn->param_count == 3 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "elem") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "key") == 0 &&
			strcmp((char*) fn->params[2].name, "val") == 0
		) {
			if (strcmp(name, "set_attr_str") == 0 && fn->params[2].type == KOS_TYPE_BUF) {
				ctx->fns.set_attr_str = i;
			}
			if (strcmp(name, "set_attr_bool") == 0 && fn->params[2].type == KOS_TYPE_BOOL) {
				ctx->fns.set_attr_bool = i;
			}
			if (strcmp(name, "set_attr_u32") == 0 && fn->params[2].type == KOS_TYPE_U32) {
				ctx->fns.set_attr_u32 = i;
			}
			if (strcmp(name, "set_attr_f32") == 0 && fn->params[2].type == KOS_TYPE_F32) {
				ctx->fns.set_attr_f32 = i;
			}
			if (strcmp(name, "set_attr_opaque_ptr") == 0 && fn->params[2].type == KOS_TYPE_OPAQUE_PTR) {
				ctx->fns.set_attr_opaque_ptr = i;
			}
		}

		if (
			strcmp(name, "set_attr_dim") == 0 &&
			fn->ret_type == KOS_TYPE_BOOL &&
			fn->param_count == 4 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "elem") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "key") == 0 &&
			fn->params[2].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[2].name, "units") == 0 &&
			fn->params[3].type == KOS_TYPE_F32 &&
			strcmp((char*) fn->params[3].name, "val") == 0
		) {
			ctx->fns.set_attr_dim = i;
		}

		if (
			strcmp(name, "set_attr_raster") == 0 &&
			fn->ret_type == KOS_TYPE_BOOL &&
			fn->param_count == 5 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "elem") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "key") == 0 &&
			fn->params[2].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[2].name, "x_res") == 0 &&
			fn->params[3].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[3].name, "y_res") == 0 &&
			fn->params[4].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[4].name, "data") == 0
		) {
			ctx->fns.set_attr_raster = i;
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
			fn->param_count == 5 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "ui") == 0 &&
			fn->params[1].type == KOS_TYPE_U64 &&
			strcmp((char*) fn->params[1].name, "hid") == 0 &&
			fn->params[2].type == KOS_TYPE_U64 &&
			strcmp((char*) fn->params[2].name, "cid") == 0 &&
			fn->params[3].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[3].name, "device") == 0 &&
			fn->params[4].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[4].name, "format") == 0
		) {
			ctx->backend_wgpu_fns.init = i;
		}

		if (
			strcmp(name, "backend_wgpu_render") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 5 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "ui") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "frame") == 0 &&
			fn->params[2].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[2].name, "command_encoder") == 0 &&
			fn->params[3].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[3].name, "x_res") == 0 &&
			fn->params[4].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[4].name, "y_res") == 0
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
