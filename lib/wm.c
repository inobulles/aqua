// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#include "wm.h"
#include <aqua/kos.h>

#define __AQUA_LIB_COMPONENT__
#include "component.h"

#include <umber.h>

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define SPEC "aquabsd.black.wm"

struct wm_ctx_t {
	uint64_t hid;
	uint64_t vid;

	kos_cookie_t last_cookie;

	bool is_conn;
	uint64_t conn_id;

	struct {
		uint32_t create;
		uint32_t destroy;
		uint32_t loop;
	} fns;

	bool last_success;
	kos_val_t last_ret;
};

struct wm_t {
	wm_ctx_t ctx;
	kos_opaque_ptr_t opaque_ptr;
};

static umber_class_t const* cls = NULL;
static component_t comp;

static __attribute__((constructor)) void init(void) {
	cls = umber_class_new("aqua.lib.wm", UMBER_LVL_INFO, "AQUA standard library: WM library.");
}

aqua_component_t wm_init(aqua_ctx_t ctx) {
	aqua_register_component(ctx, &comp);
	kos_req_vdev(SPEC);

	return &comp;
}

static bool probe(kos_vdev_descr_t const* vdev) {
	return strcmp((char*) vdev->spec, SPEC) == 0;
}

wm_ctx_t wm_conn(kos_vdev_descr_t const* vdev) {
	wm_ctx_t const ctx = calloc(1, sizeof *ctx);

	if (ctx == NULL) {
		LOG_E(cls, "Failed to allocate context.");
		return NULL;
	}

	ctx->hid = vdev->host_id;
	ctx->vid = vdev->vdev_id;

	ctx->is_conn = false;
	ctx->last_cookie = kos_vdev_conn(ctx->hid, ctx->vid);

	cookie_notif_conn_tuple_t tuple = {
		.cookie = ctx->last_cookie,
		.comp = &comp,
		.data = ctx,
	};

	aqua_add_pending_conn(comp.ctx, &tuple);

	kos_flush(true);

	return ctx;
}

void wm_disconn(wm_ctx_t ctx) {
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
	wm_ctx_t const ctx = data;

	if (ctx == NULL || notif->cookie != ctx->last_cookie) {
		LOG_E(cls, "No context or bad cookie.");
		return;
	}

	ctx->conn_id = notif->conn_id;
	ctx->is_conn = true;

	// Read functions.

	memset(&ctx->fns, 0xFF, sizeof ctx->fns);

	for (size_t i = 0; i < notif->conn.fn_count; i++) {
		kos_fn_t const* fn = &notif->conn.fns[i];
		char const* name = (void*) fn->name;

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
			strcmp((char*) fn->params[0].name, "wm") == 0
		) {
			ctx->fns.destroy = i;
		}

		if (
			strcmp(name, "loop") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "wm") == 0
		) {
			ctx->fns.loop = i;
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
	wm_ctx_t const ctx = data;

	if (ctx == NULL || !ctx->is_conn || notif->cookie != ctx->last_cookie) {
		LOG_E(cls, "No context, not connected, or bad cookie.");
		return;
	}

	ctx->last_success = true;
	ctx->last_ret = notif->call_ret.ret;
}

static void notif_call_fail(kos_notif_t const* notif, void* data) {
	wm_ctx_t const ctx = data;
	ctx->last_success = false;

	(void) notif;

	LOG_E(cls, "TODO Call failed, but how do we handle this?");
}

// Actual function implementations.

wm_t wm_create(wm_ctx_t ctx) {
	if (ctx == NULL || !ctx->is_conn) {
		LOG_E(cls, "No context or not connected.");
		return NULL;
	}

	wm_t const wm = calloc(1, sizeof *wm);

	if (wm == NULL) {
		LOG_E(cls, "Failed to allocate WM.");
		return NULL;
	}

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.create, NULL);
	kos_flush(true);

	wm->ctx = ctx;
	wm->opaque_ptr = ctx->last_ret.opaque_ptr;

	return wm;
}

void wm_destroy(wm_t wm) {
	wm_ctx_t const ctx = wm->ctx;

	if (ctx == NULL || !ctx->is_conn) {
		LOG_W(cls, "No connection made; just freeing font memory.");
		free(wm);
		return;
	}

	kos_val_t const args[] = {
		{.opaque_ptr = wm->opaque_ptr},
	};

	kos_vdev_call(ctx->conn_id, ctx->fns.destroy, args);
	free(wm);
}

void wm_loop(wm_t wm) {
	wm_ctx_t const ctx = wm->ctx;

	if (ctx == NULL || !ctx->is_conn) {
		LOG_E(cls, "No context or not connected.");
		return;
	}

	kos_val_t const args[] = {
		{.opaque_ptr = wm->opaque_ptr},
	};

	kos_vdev_call(ctx->conn_id, ctx->fns.loop, args);
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
