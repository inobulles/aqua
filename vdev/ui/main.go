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
	// Returns how much space an element will take up.
	// The backend will try to fit it within the maximum width and height supplied, but it may not be able to.
	// For example, the element in question could be text, in which case if it must wrap to fit within max_w, it might very well have to extend beyond max_h.
	calculate_size(elem IElem, max_w, max_h float32) (w, h float32)
}

type Ui struct {
	backend Backend
	root    Div

	// This must be set if anything which will affect the flow of the UI is changed.
	// This will trigger a reflow upon the next render (see the Ui.reflow function).
	dirty bool
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

// TODO A lot of the code in GoUiAddDiv and GoUiAddText can be factored out.

//export GoUiAddDiv
func GoUiAddDiv(
	parent_raw C.uintptr_t,
	semantics *C.char,
	semantics_len C.size_t,
) C.uintptr_t {
	parent := elem_from_raw(parent_raw).(*Div)
	ui := parent.ui

	if parent.kind != ElemKindDiv {
		panic("GoUiAddDiv: parent is not a div")
	}

	elem := Div{
		Elem: Elem{
			kind:   ElemKindDiv,
			ui:     ui,
			parent: parent,
		},
	}.defaults()

	parent.children = append(parent.children, &elem)
	ui.dirty = true

	handle := cgo.NewHandle(elem)
	return C.uintptr_t(handle)
}

//export GoUiAddText
func GoUiAddText(
	parent_raw C.uintptr_t,
	semantics *C.char,
	semantics_len C.size_t,
	text *C.char,
	text_len C.size_t,
) C.uintptr_t {
	parent := elem_from_raw(parent_raw).(*Div)
	ui := parent.ui

	if parent.kind != ElemKindDiv {
		panic("GoUiAddText: parent is not a div")
	}

	elem := &Text{
		Elem: Elem{
			kind:   ElemKindText,
			ui:     ui,
			parent: parent,
		},
		text: C.GoString(text),
	}

	parent.children = append(parent.children, elem)
	ui.dirty = true

	handle := cgo.NewHandle(elem)
	return C.uintptr_t(handle)
}

//export GoUiSetAttr
func GoUiSetAttr(elem_raw C.uintptr_t, key_raw *C.char, val *C.char) {
	elem := elem_from_raw(elem_raw).(*Elem)
	key := C.GoString(key_raw)

	if val == nil {
		elem.rem_attr(key)
		return
	}

	elem.set_attr(key, C.GoString(val))
}

func main() {}
