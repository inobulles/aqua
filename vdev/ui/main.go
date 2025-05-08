// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package main

/*
#cgo LDFLAGS: -laqua

#include "../../kos/vdev.h"
*/
import "C"
import "unsafe"

type Ui struct {
}

//export GoUiCreate
func GoUiCreate() unsafe.Pointer {
	ui := &Ui{}
	return unsafe.Pointer(ui)
}

//export GoUiDestroy
func GoUiDestroy(ui_raw unsafe.Pointer) {
	// ui := (*Ui)(ui_raw)
}

func main() {}
