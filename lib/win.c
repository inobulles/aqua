// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#include "win.h"
#include <aqua/kos.h>

#define __AQUA_LIB_COMPONENT__
#include "component.h"
#include "win_internal.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SPEC "aquabsd.black.win"

struct win_ctx_t {
	uint64_t hid;
	uint64_t vid;

	kos_cookie_t last_cookie;

	bool is_conn;
	uint64_t conn_id;

	struct {
		uint32_t INTR_REDRAW;
		uint32_t INTR_RESIZE;
	} consts;

	struct {
		uint32_t create;
		uint32_t destroy;
		uint32_t register_interrupt;
		uint32_t loop;
	} fns;

	bool last_success;
	kos_val_t last_ret;
};

static component_t comp;

aqua_component_t win_init(aqua_ctx_t ctx) {
	aqua_register_component(ctx, &comp);
	kos_req_vdev("aquabsd.black.win");

	return &comp;
}

static bool probe(kos_vdev_descr_t const* vdev) {
	return strcmp((char*) vdev->spec, SPEC) == 0;
}

win_ctx_t win_conn(kos_vdev_descr_t const* vdev) {
	win_ctx_t const ctx = calloc(1, sizeof *ctx);

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

void win_disconn(win_ctx_t ctx) {
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
	win_ctx_t const ctx = data;

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

		if (strcmp(name, "INTR_REDRAW") == 0) {
			ctx->consts.INTR_REDRAW = c->val.u8;
		}

		if (strcmp(name, "INTR_RESIZE") == 0) {
			ctx->consts.INTR_RESIZE = c->val.u8;
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
			strcmp((char*) fn->params[0].name, "win") == 0
		) {
			ctx->fns.destroy = i;
		}

		if (
			strcmp(name, "register_interrupt") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "win") == 0 &&
			fn->params[1].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[1].name, "ino") == 0
		) {
			ctx->fns.register_interrupt = i;
		}

		if (
			strcmp(name, "loop") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "win") == 0
		) {
			ctx->fns.loop = i;
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
	win_ctx_t const ctx = data;

	if (ctx == NULL || !ctx->is_conn || notif->cookie != ctx->last_cookie) {
		return;
	}

	ctx->last_success = true;
	ctx->last_ret = notif->call_ret.ret;
}

static void notif_call_fail(kos_notif_t const* notif, void* data) {
	win_ctx_t const ctx = data;
	ctx->last_success = false;

	(void) notif;

	fprintf(stderr, "TODO Call failed, but how do we handle this?\n");
}

// Actual function implementations.

win_t win_create(win_ctx_t ctx) {
	if (ctx == NULL || !ctx->is_conn) {
		return NULL;
	}

	win_t const win = calloc(1, sizeof *win);

	if (win == NULL) {
		return NULL;
	}

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.create, NULL);
	kos_flush(true);

	win->ctx = ctx;
	win->opaque_ptr = ctx->last_ret.opaque_ptr;
	win->ino = kos_gen_ino();

	kos_val_t const args[] = {
		{.opaque_ptr = win->opaque_ptr},
		{.u32 = 0},
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.register_interrupt, args);
	ctx->last_success = false;
	kos_flush(true);

	// If the interrupt registration was successful, add it to our interrupt vector.

	if (!ctx->last_success) {
		return win;
	}

	aqua_add_interrupt(comp.ctx, win->ino, &comp, win);

	return win;
}

void win_destroy(win_t win) {
	win_ctx_t const ctx = win->ctx;

	if (!ctx->is_conn) {
		free(win);
		return;
	}

	kos_val_t const args[] = {
		{.opaque_ptr = win->opaque_ptr},
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.destroy, args);
	free(win);
}

void win_register_redraw_cb(win_t win, win_redraw_cb_t cb, void* data) {
	win->redraw = cb;
	win->redraw_data = data;
}

void win_register_resize_cb(win_t win, win_resize_cb_t cb, void* data) {
	win->resize = cb;
	win->resize_data = data;
}

void win_loop(win_t win) {
	win_ctx_t const ctx = win->ctx;

	if (!ctx->is_conn) {
		return;
	}

	kos_val_t const args[] = {
		{.opaque_ptr = win->opaque_ptr},
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.loop, args);
	kos_flush(true);
}

typedef struct __attribute__((packed)) {
	uint8_t type;
} intr_generic_t;

typedef struct __attribute__((packed)) {
	intr_generic_t generic;
	uint32_t x_res;
	uint32_t y_res;
} intr_resize_t;

static void interrupt(kos_notif_t const* notif, void* data) {
	assert(notif->kind == KOS_NOTIF_INTERRUPT);

	win_t const win = data;
	win_ctx_t const ctx = win->ctx;

	if (win->ino != notif->interrupt.ino) {
		return;
	}

	if (!ctx->is_conn) {
		return;
	}

	intr_generic_t const* const intr = notif->interrupt.data;

	if (notif->interrupt.data_size < sizeof *intr) {
		return;
	}

	if (intr->type == ctx->consts.INTR_REDRAW) {
		if (win->redraw != NULL) {
			win->redraw(win, win->redraw_data);
		}
	}

	if (intr->type == ctx->consts.INTR_RESIZE) {
		intr_resize_t const* const resize = notif->interrupt.data;

		if (notif->interrupt.data_size < sizeof *resize) {
			return; // TODO Error message.
		}

		if (win->resize != NULL) {
			win->resize(win, win->resize_data, resize->x_res, resize->y_res);
		}
	}
}

static component_t comp = {
	.probe = probe,
	.notif_conn = notif_conn,
	.notif_conn_fail = notif_conn_fail,
	.notif_call_ret = notif_call_ret,
	.notif_call_fail = notif_call_fail,
	.interrupt = interrupt,
	.vdev_count = 0,
	.vdevs = NULL,
};
