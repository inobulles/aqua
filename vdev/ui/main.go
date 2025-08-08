// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package main

/*
#cgo LDFLAGS: -laqua

#include "../../kos/lib/vdriver.h"
*/
import "C"
import "runtime/cgo"

type Backend interface {
}

type Ui struct {
	backend Backend
}

//export GoUiCreate
func GoUiCreate() C.uintptr_t {
	ui := &Ui{}
	handle := cgo.NewHandle(ui)
	return C.uintptr_t(handle)
}

//export GoUiDestroy
func GoUiDestroy(ui_raw C.uintptr_t) {
	handle := cgo.Handle(ui_raw)
	defer handle.Delete()

	// ui := handle.Value().(*Ui)
}

func main() {}
