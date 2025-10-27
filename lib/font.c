// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#include "font.h"

#define __AQUA_LIB_COMPONENT__
#include "component.h"

#include <umber.h>

#include <string.h>

#define SPEC "aquabsd.black.font"

static umber_class_t const* cls = NULL;

struct font_ctx_t {
	uint64_t hid;
	uint64_t vid;

	kos_cookie_t last_cookie;

	bool is_conn;
	uint64_t conn_id;

	struct {
		uint32_t font_from_str;
		uint32_t font_destroy;
		uint32_t layout_create;
		uint32_t layout_destroy;
		uint32_t layout_set_text;
		uint32_t layout_set_limits;
		uint32_t layout_pos_to_index;
		uint32_t layout_index_to_pos;
		uint32_t layout_get_res;
		uint32_t layout_render;
	} fns;

	bool last_success;
	kos_val_t last_ret;
};

struct font_t {
	font_ctx_t ctx;
	kos_opaque_ptr_t opaque_ptr;
};

struct font_layout_t {
	font_ctx_t ctx;
	kos_opaque_ptr_t opaque_ptr;
};

static component_t comp;

static __attribute__((constructor)) void init(void) {
	cls = umber_class_new("aqua.lib.font", UMBER_LVL_INFO, "AQUA standard library: font library.");
}

aqua_component_t font_init(aqua_ctx_t ctx) {
	aqua_register_component(ctx, &comp);
	kos_req_vdev("aquabsd.black.font");

	return &comp;
}

static bool probe(kos_vdev_descr_t const* vdev) {
	return strcmp((char*) vdev->spec, SPEC) == 0;
}

font_ctx_t font_conn(kos_vdev_descr_t const* vdev) {
	font_ctx_t const ctx = calloc(1, sizeof *ctx);

	if (ctx == NULL) {
		LOG_E(cls, "Failed to allocate context.");
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

void font_disconn(font_ctx_t ctx) {
	if (ctx == NULL) {
		LOG_E(cls, "Context is NULL.");
		return;
	}

	if (!ctx->is_conn) {
		LOG_W(cls, "No connection made; just freeing context memory.");
		free(ctx);
		return;
	}

	kos_vdev_disconn(ctx->conn_id);
	free(ctx);
}

static void notif_conn(kos_notif_t const* notif, void* data) {
	font_ctx_t const ctx = data;

	if (ctx == NULL || notif->cookie != ctx->last_cookie) {
		LOG_E(cls, "No context or bad cookie.");
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
			strcmp(name, "font_from_str") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[0].name, "str") == 0
		) {
			ctx->fns.font_from_str = i;
		}

		if (
			strcmp(name, "font_destroy") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "font") == 0
		) {
			ctx->fns.font_destroy = i;
		}

		if (
			strcmp(name, "layout_create") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "font") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "text") == 0
		) {
			ctx->fns.layout_create = i;
		}

		if (
			strcmp(name, "layout_destroy") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "layout") == 0
		) {
			ctx->fns.layout_destroy = i;
		}

		if (
			strcmp(name, "layout_set_text") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "layout") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "text") == 0
		) {
			ctx->fns.layout_set_text = i;
		}

		if (
			strcmp(name, "layout_set_limits") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 3 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "layout") == 0 &&
			fn->params[1].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[1].name, "x_res_limit") == 0 &&
			fn->params[2].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[2].name, "y_res_limit") == 0
		) {
			ctx->fns.layout_set_limits = i;
		}

		if (
			strcmp(name, "layout_pos_to_index") == 0 &&
			fn->ret_type == KOS_TYPE_I32 &&
			fn->param_count == 3 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "layout") == 0 &&
			fn->params[1].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[1].name, "x") == 0 &&
			fn->params[2].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[2].name, "y") == 0
		) {
			ctx->fns.layout_pos_to_index = i;
		}

		if (
			strcmp(name, "layout_index_to_pos") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 4 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "layout") == 0 &&
			fn->params[1].type == KOS_TYPE_I32 &&
			strcmp((char*) fn->params[1].name, "index") == 0 &&
			fn->params[2].type == KOS_TYPE_PTR &&
			strcmp((char*) fn->params[2].name, "x") == 0 &&
			fn->params[3].type == KOS_TYPE_PTR &&
			strcmp((char*) fn->params[3].name, "y") == 0
		) {
			ctx->fns.layout_index_to_pos = i;
		}

		if (
			strcmp(name, "layout_get_res") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 3 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "layout") == 0 &&
			fn->params[1].type == KOS_TYPE_PTR &&
			strcmp((char*) fn->params[1].name, "x_res") == 0 &&
			fn->params[2].type == KOS_TYPE_PTR &&
			strcmp((char*) fn->params[2].name, "y_res") == 0
		) {
			ctx->fns.layout_get_res = i;
		}

		if (
			strcmp(name, "layout_render") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "layout") == 0 &&
			fn->params[1].type == KOS_TYPE_PTR &&
			strcmp((char*) fn->params[1].name, "buffer") == 0
		) {
			ctx->fns.layout_render = i;
		}
	}

	for (size_t i = 0; i < sizeof ctx->fns / sizeof(uint32_t); i++) {
		if (((uint32_t*) &ctx->fns)[i] != -1u) {
			continue;
		}

		LOG_E(cls, "Missing function %d; disabling connection.", i);
		ctx->is_conn = false;
		break;
	}
}

static void notif_conn_fail(kos_notif_t const* notif, void* data) {
	(void) notif;
	(void) data;

	LOG_E(cls, "TODO Connection failed, but how do we handle this?");
}

static void notif_call_ret(kos_notif_t const* notif, void* data) {
	font_ctx_t const ctx = data;

	if (ctx == NULL || !ctx->is_conn || notif->cookie != ctx->last_cookie) {
		LOG_E(cls, "No context, not connected, or bad cookie.");
		return;
	}

	ctx->last_success = true;
	ctx->last_ret = notif->call_ret.ret;
}

static void notif_call_fail(kos_notif_t const* notif, void* data) {
	font_ctx_t const ctx = data;
	ctx->last_success = false;

	(void) notif;

	LOG_E(cls, "TODO Call failed, but how do we handle this?");
}

// Actual function implementations.

font_t font_from_str(font_ctx_t ctx, char const* str) {
	if (ctx == NULL || !ctx->is_conn) {
		LOG_E(cls, "No context or not connected.");
		return NULL;
	}

	font_t const font = calloc(1, sizeof *font);

	if (font == NULL) {
		LOG_E(cls, "Failed to allocate font.");
		return NULL;
	}

	kos_val_t const args[] = {
		{.buf = {strlen(str), str}},
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.font_from_str, args);
	kos_flush(true);

	font->ctx = ctx;
	font->opaque_ptr = ctx->last_ret.opaque_ptr;

	return font;
}

void font_destroy(font_t font) {
	font_ctx_t const ctx = font->ctx;

	if (!ctx->is_conn) {
		LOG_W(cls, "No connection made; just freeing font memory.");
		free(font);
		return;
	}

	kos_val_t const args[] = {
		{.opaque_ptr = font->opaque_ptr},
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.font_destroy, args);
	free(font);
}

font_layout_t font_layout_create(font_t font, char const* text) {
	font_ctx_t const ctx = font->ctx;
	font_layout_t const layout = calloc(1, sizeof *layout);

	if (layout == NULL) {
		LOG_E(cls, "Failed to allocate layout.");
		return NULL;
	}

	kos_val_t const args[] = {
		{.opaque_ptr = font->opaque_ptr},
		{.buf = {strlen(text), text}},
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.layout_create, args);
	kos_flush(true);

	layout->ctx = ctx;
	layout->opaque_ptr = ctx->last_ret.opaque_ptr;

	return layout;
}

void font_layout_destroy(font_layout_t layout) {
	font_ctx_t const ctx = layout->ctx;

	kos_val_t const args[] = {
		{.opaque_ptr = layout->opaque_ptr},
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.layout_destroy, args);
}

void font_layout_set_text(font_layout_t layout, char const* text) {
	font_ctx_t const ctx = layout->ctx;

	kos_val_t const args[] = {
		{.opaque_ptr = layout->opaque_ptr},
		{.buf = {strlen(text), text}},
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.layout_set_text, args);
	kos_flush(true);
}

void font_layout_set_limits(font_layout_t layout, uint32_t x_res_limit, uint32_t y_res_limit) {
	font_ctx_t const ctx = layout->ctx;

	if (ctx == NULL || !ctx->is_conn) {
		LOG_E(cls, "No context or not connected.");
		return;
	}

	kos_val_t const args[] = {
		{.opaque_ptr = layout->opaque_ptr},
		{.u32 = x_res_limit},
		{.u32 = y_res_limit},
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.layout_set_limits, args);
	kos_flush(true);
}

int32_t font_layout_pos_to_index(font_layout_t layout, uint32_t x, uint32_t y) {
	font_ctx_t const ctx = layout->ctx;

	if (ctx == NULL || !ctx->is_conn) {
		LOG_E(cls, "No context or not connected.");
		return -1;
	}

	kos_val_t const args[] = {
		{.opaque_ptr = layout->opaque_ptr},
		{.u32 = x},
		{.u32 = y},
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.layout_pos_to_index, args);
	kos_flush(true);

	return ctx->last_ret.i32;
}

void font_layout_index_to_pos(font_layout_t layout, int32_t index, uint32_t* x, uint32_t* y) {
	font_ctx_t const ctx = layout->ctx;

	if (ctx == NULL || !ctx->is_conn) {
		LOG_E(cls, "No context or not connected.");
		return;
	}

	kos_val_t const args[] = {
		{.opaque_ptr = layout->opaque_ptr},
		{.i32 = index},
		{.ptr = {0, (uint64_t) (uintptr_t) x}},
		{.ptr = {0, (uint64_t) (uintptr_t) y}},
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.layout_index_to_pos, args);
	kos_flush(true);
}

void font_layout_get_res(font_layout_t layout, uint32_t* x_res, uint32_t* y_res) {
	font_ctx_t const ctx = layout->ctx;

	kos_val_t const args[] = {
		{.opaque_ptr = layout->opaque_ptr},
		// TODO wtf is this.
		{.ptr = {0, (uint64_t) (uintptr_t) x_res}},
		{.ptr = {0, (uint64_t) (uintptr_t) y_res}},
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.layout_get_res, args);
	kos_flush(true);
}

void font_layout_render(font_layout_t layout, void* buffer) {
	font_ctx_t const ctx = layout->ctx;

	if (ctx == NULL || !ctx->is_conn) {
		LOG_E(cls, "No context or not connected.");
		return;
	}

	kos_val_t const args[] = {
		{.opaque_ptr = layout->opaque_ptr},
		{.ptr = {0, (uint64_t) (uintptr_t) buffer}},
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.layout_render, args);
	kos_flush(true);
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
