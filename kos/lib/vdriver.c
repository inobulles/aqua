// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#include "vdriver.h"

void* vdriver_unwrap_local_opaque_ptr(kos_opaque_ptr_t opaque_ptr) {
	if (opaque_ptr.host_id != VDRIVER.host_id) {
		return NULL;
	}

	return (void*) (uintptr_t) opaque_ptr.ptr;
}

void* vdriver_unwrap_local_ptr(kos_ptr_t ptr) {
	if (ptr.host_id != VDRIVER.host_id) {
		return NULL;
	}

	return (void*) (uintptr_t) ptr.ptr;
}

kos_opaque_ptr_t vdriver_make_opaque_ptr(void const* ptr) {
	kos_opaque_ptr_t opaque_ptr = {
		.host_id = VDRIVER.host_id,
		.ptr = (uintptr_t) ptr,
	};

	return opaque_ptr;
}

kos_ptr_t vdriver_make_ptr(void const* ptr) {
	kos_ptr_t kos_ptr = {
		.host_id = VDRIVER.host_id,
		.ptr = (uintptr_t) ptr,
	};

	return kos_ptr;
}
