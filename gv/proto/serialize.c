// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#include "proto.h"

#include <assert.h>
#include <string.h>

size_t gv_serialize_val_size(kos_type_t t, kos_val_t const* v) {
	switch (t) {
	case KOS_TYPE_VOID:
		return 0;
	case KOS_TYPE_BOOL:
		return sizeof v->b;
	case KOS_TYPE_U8:
		return sizeof v->u8;
	case KOS_TYPE_U16:
		return sizeof v->u16;
	case KOS_TYPE_U32:
		return sizeof v->u32;
	case KOS_TYPE_U64:
		return sizeof v->u64;
	case KOS_TYPE_I8:
		return sizeof v->i8;
	case KOS_TYPE_I16:
		return sizeof v->i16;
	case KOS_TYPE_I32:
		return sizeof v->i32;
	case KOS_TYPE_I64:
		return sizeof v->i64;
	case KOS_TYPE_F32:
		return sizeof v->f32;
	case KOS_TYPE_F64:
		return sizeof v->f64;
	case KOS_TYPE_BUF:
		return sizeof v->buf.size + v->buf.size;
	case KOS_TYPE_OPAQUE_PTR:
		return sizeof v->opaque_ptr;
	case KOS_TYPE_PTR:
		return sizeof v->ptr;
	}

	assert(false);
}

size_t gv_serialize_val(void* buf, kos_type_t t, kos_val_t const* v) {
	switch (t) {
	case KOS_TYPE_VOID:
		break;
	case KOS_TYPE_BOOL:
		memcpy(buf, &v->b, sizeof v->b);
		break;
	case KOS_TYPE_U8:
		memcpy(buf, &v->u8, sizeof v->u8);
		break;
	case KOS_TYPE_U16:
		memcpy(buf, &v->u16, sizeof v->u16);
		break;
	case KOS_TYPE_U32:
		memcpy(buf, &v->u32, sizeof v->u32);
		break;
	case KOS_TYPE_U64:
		memcpy(buf, &v->u64, sizeof v->u64);
		break;
	case KOS_TYPE_I8:
		memcpy(buf, &v->i8, sizeof v->i8);
		break;
	case KOS_TYPE_I16:
		memcpy(buf, &v->i16, sizeof v->i16);
		break;
	case KOS_TYPE_I32:
		memcpy(buf, &v->i32, sizeof v->i32);
		break;
	case KOS_TYPE_I64:
		memcpy(buf, &v->i64, sizeof v->i64);
		break;
	case KOS_TYPE_F32:
		memcpy(buf, &v->f32, sizeof v->f32);
		break;
	case KOS_TYPE_F64:
		memcpy(buf, &v->f64, sizeof v->f64);
		break;
	case KOS_TYPE_BUF:
		memcpy(buf, &v->buf.size, sizeof v->buf.size);
		memcpy(buf + sizeof v->buf.size, v->buf.ptr, v->buf.size);
		break;
	case KOS_TYPE_OPAQUE_PTR:
		memcpy(buf, &v->opaque_ptr, sizeof v->opaque_ptr);
		break;
	case KOS_TYPE_PTR:
		memcpy(buf, &v->ptr, sizeof v->ptr);
		break;
	default:
		assert(false);
	}

	return gv_serialize_val_size(t, v);
}

size_t gv_serialize_const_size(kos_const_t const* c) {
	return sizeof c->type + sizeof c->name + gv_serialize_val_size(c->type, &c->val);
}

size_t gv_serialize_const(void* buf, kos_const_t const* c) {
	memcpy(buf, &c->type, sizeof c->type);
	size_t size = sizeof c->type;
	memcpy(buf + size, c->name, sizeof c->name);
	size += sizeof c->name;
	size += gv_serialize_val(buf + size, c->type, &c->val);
	return size;
}

size_t gv_serialize_param_size(kos_param_t const* p) {
	return sizeof *p;
}

size_t gv_serialize_param(void* buf, kos_param_t const* p) {
	memcpy(buf, p, sizeof *p);
	return sizeof *p;
}

size_t gv_serialize_fn_size(kos_fn_t const* fn) {
	size_t size = sizeof fn->name + sizeof fn->ret_type + sizeof fn->param_count;

	for (size_t i = 0; i < fn->param_count; i++) {
		size += gv_serialize_param_size(&fn->params[i]);
	}

	return size;
}

size_t gv_serialize_fn(void* buf, kos_fn_t const* fn) {
	memcpy(buf, fn->name, sizeof fn->name);
	size_t size = sizeof fn->name;
	memcpy(buf + size, &fn->ret_type, sizeof fn->ret_type);
	size += sizeof fn->ret_type;

	for (size_t i = 0; i < fn->param_count; i++) {
		size += gv_serialize_param(buf + size, &fn->params[i]);
	}

	return size;
}
