// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package main

/*
#cgo LDFLAGS: -laqua -lvdriver -lumber

#include "../../kos/lib/vdriver.h"
*/
import "C"
import "runtime/cgo"

type Backend interface {
}

type Ui struct {
	backend Backend
	root    Div
}

//export GoUiCreate
func GoUiCreate() C.uintptr_t {
	ui := &Ui{}

	ui.root = Div{
		Elem: Elem{
			kind:   ElemKindDiv,
			ui:     ui,
			parent: nil,
		},
	}.defaults()

	handle := cgo.NewHandle(ui)
	return C.uintptr_t(handle)
}

//export GoUiDestroy
func GoUiDestroy(ui_raw C.uintptr_t) {
	handle := cgo.Handle(ui_raw)
	defer handle.Delete()
}

//export GoUiGetRoot
func GoUiGetRoot(ui_raw C.uintptr_t) C.uintptr_t {
	ui_handle := cgo.Handle(ui_raw)
	ui := ui_handle.Value().(*Ui)

	root_handle := cgo.NewHandle(&ui.root)
	return C.uintptr_t(root_handle)
}

}

func main() {}
