// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#include "../../kos/vdev.h"

// XXX We need this guy to be defined in its own C file, as we'll get a duplicate symbol error if we try to do so in the cgo C comment.

vdriver_t VDRIVER;

extern void GoInit(void);

__attribute__((constructor)) void init(void) {
	GoInit();
}
