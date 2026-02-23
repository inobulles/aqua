// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025-2026 Aymeric Wibo

#include "wm.h"
#include <aqua/kos.h>

#define __AQUA_LIB_COMPONENT__
#include "component.h"
#include "wm_internal.h"

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
		uint32_t INTR_REDRAW;
		uint32_t INTR_NEW_WIN;
		uint32_t INTR_DESTROY_WIN;
		uint32_t INTR_REDRAW_WIN;
		uint32_t INTR_MOUSE_MOTION;
		uint32_t INTR_MOUSE_BUTTON;
	} consts;

	struct {
		uint32_t create;
		uint32_t destroy;
		uint32_t register_interrupt;
		uint32_t loop;
		uint32_t get_win_fb;
	} fns;

	bool last_success;
	kos_val_t last_ret;
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

	// Read constants.

	memset(&ctx->consts, 0xFF, sizeof ctx->consts);

	for (size_t i = 0; i < notif->conn.const_count; i++) {
		kos_const_t const* const c = &notif->conn.consts[i];
		char const* const name = (void*) c->name;

		if (strcmp(name, "INTR_REDRAW") == 0) {
			ctx->consts.INTR_REDRAW = c->val.u8;
		}

		if (strcmp(name, "INTR_NEW_WIN") == 0) {
			ctx->consts.INTR_NEW_WIN = c->val.u8;
		}

		if (strcmp(name, "INTR_DESTROY_WIN") == 0) {
			ctx->consts.INTR_DESTROY_WIN = c->val.u8;
		}

		if (strcmp(name, "INTR_REDRAW_WIN") == 0) {
			ctx->consts.INTR_REDRAW_WIN = c->val.u8;
		}

		if (strcmp(name, "INTR_MOUSE_MOTION") == 0) {
			ctx->consts.INTR_MOUSE_MOTION = c->val.u8;
		}

		if (strcmp(name, "INTR_MOUSE_BUTTON") == 0) {
			ctx->consts.INTR_MOUSE_BUTTON = c->val.u8;
		}
	}

	for (size_t i = 0; i < sizeof ctx->consts / sizeof(uint32_t); i++) {
		if (((uint32_t*) &ctx->consts)[i] != -1u) {
			continue;
		}

		LOG_E(cls, "Missing constant %d; disabling connection.", i);
		ctx->is_conn = false;
		break;
	}

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
			strcmp(name, "register_interrupt") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "wm") == 0 &&
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
			strcmp((char*) fn->params[0].name, "wm") == 0
		) {
			ctx->fns.loop = i;
		}

		if (
			strcmp(name, "get_win_fb") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "win") == 0 &&
			fn->params[1].type == KOS_TYPE_PTR &&
			strcmp((char*) fn->params[1].name, "buf") == 0
		) {
			ctx->fns.get_win_fb = i;
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
	wm->ino = kos_gen_ino();

	kos_val_t const args[] = {
		{.opaque_ptr = wm->opaque_ptr},
		{.u32 = 0},
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.register_interrupt, args);
	ctx->last_success = false;
	kos_flush(true);

	// If the interrupt registration was successful, add it to our interrupt vector.

	if (!ctx->last_success) {
		return wm;
	}

	aqua_add_interrupt(comp.ctx, wm->ino, &comp, wm);

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

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.destroy, args);
	free(wm);
}

void wm_register_redraw_cb(wm_t wm, wm_redraw_cb_t cb, void* data) {
	wm->redraw = cb;
	wm->redraw_data = data;
}

void wm_register_new_win_cb(wm_t wm, wm_new_win_cb_t cb, void* data) {
	wm->new_win = cb;
	wm->new_win_data = data;
}

void wm_register_destroy_win_cb(wm_t wm, wm_destroy_win_cb_t cb, void* data) {
	wm->destroy_win = cb;
	wm->destroy_win_data = data;
}

void wm_register_redraw_win_cb(wm_t wm, wm_redraw_win_cb_t cb, void* data) {
	wm->redraw_win = cb;
	wm->redraw_win_data = data;
}

void wm_register_mouse_motion_cb(wm_t wm, wm_mouse_motion_cb_t cb, void* data) {
	wm->mouse_motion = cb;
	wm->mouse_motion_data = data;
}

void wm_register_mouse_button_cb(wm_t wm, wm_mouse_button_cb_t cb, void* data) {
	wm->mouse_button = cb;
	wm->mouse_button_data = data;
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

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.loop, args);
	kos_flush(true);
}

void wm_get_win_fb(wm_t wm, wm_win_t win, void* buf) {
	wm_ctx_t const ctx = wm->ctx;

	if (ctx == NULL || !ctx->is_conn) {
		LOG_E(cls, "No context or not connected.");
		return;
	}

	kos_val_t const args[] = {
		{.opaque_ptr = {0, win}},
		{.ptr = {0, (uint64_t) (uintptr_t) buf}},
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.get_win_fb, args);
	kos_flush(true);
}

typedef struct __attribute__((packed)) {
	uint8_t type;
} intr_generic_t;

typedef struct __attribute__((packed)) {
	intr_generic_t generic;
	uint64_t raw_vk_image;
	uint64_t raw_vk_cmd_pool;
	uint64_t raw_vk_cmd_buf;
} intr_redraw_t;

typedef struct __attribute__((packed)) {
	intr_generic_t generic;
	uint64_t win;
	char const* app_id;
} intr_new_win_t;

typedef struct __attribute__((packed)) {
	intr_generic_t generic;
	uint64_t win;
} intr_destroy_win_t;

typedef struct __attribute__((packed)) {
	intr_generic_t generic;
	uint64_t win;
	uint32_t x_res;
	uint32_t y_res;
	uint64_t raw_image;
} intr_redraw_win_t;

typedef struct __attribute__((packed)) {
	intr_generic_t generic;
	uint8_t is_abs;

	union {
		double dx, x;
	};

	union {
		double dy, y;
	};

	double unaccel_dx, unaccel_dy;
} intr_mouse_motion_t;

typedef struct __attribute__((packed)) {
	intr_generic_t generic;
	uint8_t press;
	uint32_t button;
} intr_mouse_button_t;

static void interrupt(kos_notif_t const* notif, void* data) {
	assert(notif->kind == KOS_NOTIF_INTERRUPT);

	wm_t const wm = data;
	wm_ctx_t const ctx = wm->ctx;

	if (wm->ino != notif->interrupt.ino) {
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
		intr_redraw_t const* const redraw = notif->interrupt.data;

		if (notif->interrupt.data_size < sizeof *redraw) {
			LOG_E(cls, "TODO Error message.");
			return;
		}

		if (wm->redraw != NULL) {
			wm->redraw(
				wm,
				(void*) (uintptr_t) redraw->raw_vk_image,
				(void*) (uintptr_t) redraw->raw_vk_cmd_pool,
				(void*) (uintptr_t) redraw->raw_vk_cmd_buf,
				wm->redraw_data
			);
		}
	}

	else if (intr->type == ctx->consts.INTR_NEW_WIN) {
		intr_new_win_t const* const new_win = notif->interrupt.data;

		if (notif->interrupt.data_size < sizeof *new_win) {
			LOG_E(cls, "TODO Error message.");
			return;
		}

		if (wm->new_win != NULL) {
			wm->new_win(wm, new_win->win, new_win->app_id, wm->new_win_data);
		}
	}

	else if (intr->type == ctx->consts.INTR_DESTROY_WIN) {
		intr_destroy_win_t const* const destroy_win = notif->interrupt.data;

		if (notif->interrupt.data_size < sizeof *destroy_win) {
			LOG_E(cls, "TODO Error message.");
			return;
		}

		if (wm->destroy_win != NULL) {
			wm->destroy_win(wm, destroy_win->win, wm->destroy_win_data);
		}
	}

	else if (intr->type == ctx->consts.INTR_REDRAW_WIN) {
		intr_redraw_win_t const* const redraw_win = notif->interrupt.data;

		if (notif->interrupt.data_size < sizeof *redraw_win) {
			LOG_E(cls, "TODO Error message.");
			return;
		}

		if (wm->redraw_win != NULL) {
			wm->redraw_win(wm, redraw_win->win, redraw_win->x_res, redraw_win->y_res, (void*) (uintptr_t) redraw_win->raw_image, wm->redraw_win_data);
		}
	}

	else if (intr->type == ctx->consts.INTR_MOUSE_MOTION) {
		intr_mouse_motion_t const* const motion = notif->interrupt.data;

		if (notif->interrupt.data_size < sizeof *motion) {
			LOG_E(cls, "TODO Error message.");
			return;
		}

		if (wm->mouse_motion != NULL) {
			wm->mouse_motion(
				wm,
				motion->is_abs,
				motion->dx,
				motion->dy,
				motion->x,
				motion->y,
				motion->unaccel_dx,
				motion->unaccel_dy,
				wm->mouse_motion_data
			);
		}
	}

	else if (intr->type == ctx->consts.INTR_MOUSE_BUTTON) {
		intr_mouse_button_t const* const button = notif->interrupt.data;

		if (notif->interrupt.data_size < sizeof *button) {
			LOG_E(cls, "TODO Error message.");
			return;
		}

		if (wm->mouse_button != NULL) {
			wm->mouse_button(wm, button->press, button->button, wm->mouse_button_data);
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
