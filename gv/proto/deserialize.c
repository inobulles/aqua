// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#include "proto.h"

#include <assert.h>
#include <string.h>

size_t gv_deserialize_val(void const* buf, kos_type_t t, kos_val_t* v) {
	switch (t) {
	case KOS_TYPE_VOID:
		return 0;
	case KOS_TYPE_BOOL:
		memcpy(&v->b, buf, sizeof v->b);
		return sizeof v->b;
	case KOS_TYPE_U8:
		memcpy(&v->u8, buf, sizeof v->u8);
		return sizeof v->u8;
	case KOS_TYPE_U16:
		memcpy(&v->u16, buf, sizeof v->u16);
		return sizeof v->u16;
	case KOS_TYPE_U32:
		memcpy(&v->u32, buf, sizeof v->u32);
		return sizeof v->u32;
	case KOS_TYPE_U64:
		memcpy(&v->u64, buf, sizeof v->u64);
		return sizeof v->u64;
	case KOS_TYPE_I8:
		memcpy(&v->i8, buf, sizeof v->i8);
		return sizeof v->i8;
	case KOS_TYPE_I16:
		memcpy(&v->i16, buf, sizeof v->i16);
		return sizeof v->i16;
	case KOS_TYPE_I32:
		memcpy(&v->i32, buf, sizeof v->i32);
		return sizeof v->i32;
	case KOS_TYPE_I64:
		memcpy(&v->i64, buf, sizeof v->i64);
		return sizeof v->i64;
	case KOS_TYPE_F32:
		memcpy(&v->f32, buf, sizeof v->f32);
		return sizeof v->f32;
	case KOS_TYPE_F64:
		memcpy(&v->f64, buf, sizeof v->f64);
		return sizeof v->f64;
	case KOS_TYPE_BUF: {
		memcpy(&v->buf.size, buf, sizeof v->buf.size);

		v->buf.ptr = malloc(v->buf.size);
		assert(v->buf.ptr != NULL);

		memcpy((void*) v->buf.ptr, buf + sizeof v->buf.size, v->buf.size);
		return sizeof v->buf.size + v->buf.size;
	}
	case KOS_TYPE_OPAQUE_PTR:
		memcpy(&v->opaque_ptr, buf, sizeof v->opaque_ptr);
		return sizeof v->opaque_ptr;
	case KOS_TYPE_PTR:
		memcpy(&v->ptr, buf, sizeof v->ptr);
		return sizeof v->ptr;
	}

	assert(false);
	return 0;
}

size_t gv_deserialize_const(void const* buf, kos_const_t* c) {
	size_t size = 0;

	memcpy(&c->type, buf + size, sizeof c->type);
	size += sizeof c->type;

	memcpy(&c->name, buf + size, sizeof c->name);
	size += sizeof c->name;

	size += gv_deserialize_val(buf + size, c->type, &c->val);
	return size;
}

size_t gv_deserialize_param(void const* buf, kos_param_t* p) {
	memcpy(p, buf, sizeof *p);
	return sizeof *p;
}

size_t gv_deserialize_fn(void const* buf, kos_fn_t* fn) {
	size_t size = 0;

	memcpy(&fn->name, buf + size, sizeof fn->name);
	size += sizeof fn->name;

	memcpy(&fn->ret_type, buf + size, sizeof fn->ret_type);
	size += sizeof fn->ret_type;

	memcpy(&fn->param_count, buf + size, sizeof fn->param_count);
	size += sizeof fn->param_count;

	fn->params = malloc(fn->param_count * sizeof *fn->params);
	assert(fn->params != NULL);

	for (size_t i = 0; i < fn->param_count; i++) {
		size += gv_deserialize_param(buf + size, (kos_param_t*) &fn->params[i]);
	}

	return size;
}
