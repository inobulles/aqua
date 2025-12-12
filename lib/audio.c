// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#include "audio.h"

#define __AQUA_LIB_COMPONENT__
#include "component.h"

#include <stdio.h>
#include <string.h>

#define SPEC "aquabsd.black.audio"

struct audio_ctx_t {
	uint64_t hid;
	uint64_t vid;

	kos_cookie_t last_cookie;

	bool is_conn;
	uint64_t conn_id;

	struct {
		uint32_t get_configs;
	} fns;

	struct {
		uint8_t SAMPLE_FORMAT_I8;
		uint8_t SAMPLE_FORMAT_I16;
		uint8_t SAMPLE_FORMAT_I24;
		uint8_t SAMPLE_FORMAT_I32;
		uint8_t SAMPLE_FORMAT_I64;
		uint8_t SAMPLE_FORMAT_U8;
		uint8_t SAMPLE_FORMAT_U16;
		uint8_t SAMPLE_FORMAT_U32;
		uint8_t SAMPLE_FORMAT_U64;
		uint8_t SAMPLE_FORMAT_F32;
		uint8_t SAMPLE_FORMAT_F64;
	} consts;

	bool last_success;
	kos_val_t last_ret;
};

static component_t comp;

aqua_component_t audio_init(aqua_ctx_t ctx) {
	aqua_register_component(ctx, &comp);
	kos_req_vdev("aquabsd.black.audio");

	return &comp;
}

static bool probe(kos_vdev_descr_t const* vdev) {
	return strcmp((char*) vdev->spec, SPEC) == 0;
}

audio_ctx_t audio_conn(kos_vdev_descr_t const* vdev) {
	audio_ctx_t const ctx = calloc(1, sizeof *ctx);

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

void audio_disconn(audio_ctx_t ctx) {
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
	audio_ctx_t const ctx = data;

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

#define SAMPLE_FORMAT_CONST(type)   \
	if (strcmp(name, #type) == 0) {  \
		ctx->consts.type = c->val.u8; \
		AUDIO_##type = c->val.u8; /* XXX Yeah, this is disgusting. */ \
	}

		SAMPLE_FORMAT_CONST(SAMPLE_FORMAT_I8);
		SAMPLE_FORMAT_CONST(SAMPLE_FORMAT_I16);
		SAMPLE_FORMAT_CONST(SAMPLE_FORMAT_I24);
		SAMPLE_FORMAT_CONST(SAMPLE_FORMAT_I32);
		SAMPLE_FORMAT_CONST(SAMPLE_FORMAT_I64);
		SAMPLE_FORMAT_CONST(SAMPLE_FORMAT_U8);
		SAMPLE_FORMAT_CONST(SAMPLE_FORMAT_U16);
		SAMPLE_FORMAT_CONST(SAMPLE_FORMAT_U32);
		SAMPLE_FORMAT_CONST(SAMPLE_FORMAT_U64);
		SAMPLE_FORMAT_CONST(SAMPLE_FORMAT_F32);
		SAMPLE_FORMAT_CONST(SAMPLE_FORMAT_F64);

#undef SAMPLE_FORMAT_CONST
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
			strcmp(name, "get_configs") == 0 &&
			fn->ret_type == KOS_TYPE_BUF &&
			fn->param_count == 0
		) {
			ctx->fns.get_configs = i;
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
	audio_ctx_t const ctx = data;

	if (ctx == NULL || !ctx->is_conn || notif->cookie != ctx->last_cookie) {
		return;
	}

	ctx->last_success = true;
	ctx->last_ret = notif->call_ret.ret;
}

static void notif_call_fail(kos_notif_t const* notif, void* data) {
	audio_ctx_t const ctx = data;
	ctx->last_success = false;

	(void) notif;

	fprintf(stderr, "TODO Call failed, but how do we handle this?\n");
}

audio_config_t const* audio_get_configs(audio_ctx_t ctx, size_t* config_count_ref) {
	if (!ctx->is_conn) {
		return NULL;
	}

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.get_configs, NULL);
	kos_flush(true);

	*config_count_ref = ctx->last_ret.buf.size / sizeof(audio_config_t);
	return ctx->last_ret.buf.ptr;
}

static component_t comp = {
	.probe = probe,
	.notif_conn = notif_conn,
	.notif_conn_fail = notif_conn_fail,
	.notif_call_ret = notif_call_ret,
	.notif_call_fail = notif_call_fail,
};
