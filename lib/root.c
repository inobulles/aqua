// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2025 Aymeric Wibo

#include "root.h"

#define __AQUA_LIB_COMPONENT__
#include "component.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct aqua_ctx_t {
	kos_descr_v4_t descr;

	size_t component_count;
	component_t** components;

	size_t pending_conn_count;
	cookie_notif_conn_tuple_t* pending_conns;

	size_t conn_vec_size;
	conn_vec_ent_t* conn_vec;

	size_t interrupt_vec_size;
	interrupt_vec_ent_t* interrupt_vec;

	bool err;
	char err_msg[256];
};

// TODO Really good error reporting and tracing.

static void error(aqua_ctx_t const ctx, char const* msg) {
	ctx->err = true;
	strncpy(ctx->err_msg, msg, sizeof ctx->err_msg);

	fprintf(stderr, "TODO better error handling: %s\n", msg);
}

void aqua_register_component(aqua_ctx_t ctx, component_t* comp) {
	ctx->components = realloc(ctx->components, ++ctx->component_count * sizeof *ctx->components);

	if (ctx->components == NULL) {
		error(ctx, "Failed to allocate memory for component list");
		return;
	}

	comp->ctx = ctx;
	ctx->components[ctx->component_count - 1] = comp;
}

static void notif_cb(kos_notif_t const* notif, void* data) {
	aqua_ctx_t const ctx = data;

	switch (notif->kind) {
	case KOS_NOTIF_ATTACH:;
		kos_vdev_descr_t const* const vdev = &notif->attach.vdev;

		for (size_t i = 0; i < ctx->component_count; i++) {
			component_t* const comp = ctx->components[i];

			assert(comp->probe != NULL);

			if (!comp->probe(vdev)) {
				continue;
			}

			comp->vdevs = realloc(comp->vdevs, ++comp->vdev_count * sizeof *comp->vdevs);

			if (comp->vdevs == NULL) {
				error(ctx, "Failed to allocate memory for VDEV list");
				return;
			}

			memcpy(&comp->vdevs[comp->vdev_count - 1], vdev, sizeof *vdev);
		}

		break;
	case KOS_NOTIF_DETACH:
		// TODO
		break;
	case KOS_NOTIF_CONN_FAIL:
	case KOS_NOTIF_CONN:
		// If we got a connection success or failure notification from the KOS, we expect there to be a corresponding callback in the pending connection list to call.

		for (size_t i = 0; i < ctx->pending_conn_count; i++) {
			cookie_notif_conn_tuple_t* const tuple = &ctx->pending_conns[i];

			if (!tuple->__slot_used) {
				continue;
			}

			if (tuple->cookie != notif->cookie) {
				continue;
			}

			tuple->__slot_used = false;

			if (notif->kind == KOS_NOTIF_CONN_FAIL) {
				tuple->comp->notif_conn_fail(notif, tuple->data);
				return;
			}

			assert(notif->kind == KOS_NOTIF_CONN);

			// We found the pending connection callback for this cookie, add it to the connection vector.

			if (notif->conn_id >= ctx->conn_vec_size) {
				ctx->conn_vec_size = 2 * (ctx->conn_vec_size | 1);
				ctx->conn_vec = realloc(ctx->conn_vec, ctx->conn_vec_size * sizeof *ctx->conn_vec);

				if (ctx->conn_vec == NULL) {
					error(ctx, "Failed to allocate memory for connection vector");
					return;
				}
			}

			conn_vec_ent_t* const ent = &ctx->conn_vec[notif->conn_id];

			ent->comp = tuple->comp;
			ent->data = tuple->data;

			// Finally, actually call the library component-specific connection notification function.

			tuple->comp->notif_conn(notif, tuple->data);

			return;
		}

		error(ctx, "Failed to find pending connection callback for cookie received from KOS");
		break;
	case KOS_NOTIF_CALL_FAIL:
	case KOS_NOTIF_CALL_RET:
		if (notif->conn_id >= ctx->conn_vec_size) {
			error(ctx, "Connection ID is not in connection vector; this suggests something wrong with the KOS");
			return;
		}

		conn_vec_ent_t* const ent = &ctx->conn_vec[notif->conn_id];

		if (notif->kind == KOS_NOTIF_CALL_FAIL) {
			ent->comp->notif_call_fail(notif, ent->data);
		}

		else if (notif->kind == KOS_NOTIF_CALL_RET) {
			ent->comp->notif_call_ret(notif, ent->data);
		}

		break;
	case KOS_NOTIF_INTERRUPT:
		if (notif->interrupt.ino >= ctx->interrupt_vec_size) {
			error(ctx, "Interrupt number is not in interrupt vector; this suggests something wrong with the KOS");
			return;
		}

		interrupt_vec_ent_t* const ent_int = &ctx->interrupt_vec[notif->interrupt.ino];
		ent_int->comp->interrupt(notif, ent_int->data);

		break;
	}
}

aqua_ctx_t aqua_init(void) {
	aqua_ctx_t const ctx = calloc(1, sizeof *ctx);

	if (ctx == NULL) {
		return NULL;
	}

	if (kos_hello(KOS_API_V4, KOS_API_V4, &ctx->descr) != KOS_API_V4) {
		free(ctx);
		return NULL;
	}

	kos_sub_to_notif(notif_cb, ctx);

	return ctx;
}

kos_descr_v4_t* aqua_get_kos_descr(aqua_ctx_t const ctx) {
	return &ctx->descr;
}

aqua_vdev_it_t aqua_vdev_it(aqua_component_t comp) {
	assert(comp != NULL);

	aqua_vdev_it_t it = {.vdev = NULL, .__comp = comp, .__i = 0};
	component_t* const c = comp;

	if (c->vdev_count == 0) {
		return it;
	}

	it.vdev = &c->vdevs[it.__i];
	assert(it.vdev != NULL);

	return it;
}

void aqua_vdev_it_next(aqua_vdev_it_t* it) {
	assert(it != NULL);
	assert(it->vdev != NULL);

	component_t* const c = it->__comp;
	it->__i += 1;

	if (it->__i >= c->vdev_count) {
		assert(it->__i == c->vdev_count);

		it->vdev = NULL;
		return;
	}

	it->vdev = &c->vdevs[it->__i];
	assert(it->vdev != NULL);
}

void aqua_add_pending_conn(aqua_ctx_t ctx, cookie_notif_conn_tuple_t* tuple) {
	// Find an unused slot first.

	for (size_t i = 0; i < ctx->pending_conn_count; i++) {
		if (ctx->pending_conns[i].__slot_used) {
			continue;
		}

		ctx->pending_conns[i] = *tuple;
		ctx->pending_conns[i].__slot_used = true;

		return;
	}

	// Otherwise, expand the list.

	ctx->pending_conns = realloc(ctx->pending_conns, ++ctx->pending_conn_count * sizeof *ctx->pending_conns);

	if (ctx->pending_conns == NULL) {
		error(ctx, "Failed to allocate memory for pending connection list");
		return;
	}

	tuple->__slot_used = true;
	ctx->pending_conns[ctx->pending_conn_count - 1] = *tuple;
}

void aqua_add_interrupt(aqua_ctx_t ctx, kos_ino_t ino, component_t* comp, void* data) {
	if (ino >= ctx->interrupt_vec_size) {
		ctx->interrupt_vec_size = 2 * (ctx->interrupt_vec_size | 1);
		ctx->interrupt_vec = realloc(ctx->interrupt_vec, ctx->interrupt_vec_size * sizeof *ctx->interrupt_vec);

		if (ctx->interrupt_vec == NULL) {
			error(ctx, "Failed to allocate memory for interrupt vector");
			return;
		}
	}

	interrupt_vec_ent_t* const ent = &ctx->interrupt_vec[ino];

	ent->comp = comp;
	ent->data = data;
}
