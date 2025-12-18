// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package main

/*
#cgo LDFLAGS: -laqua -lvdriver -lumber

#include <aqua/vdriver.h>
*/
import "C"
import "runtime/cgo"

type Backend interface {
	// Returns how much space an element will take up.
	// The backend will try to fit it within the maximum width and height supplied, but it may not be able to.
	// For example, the element in question could be text, in which case if it must wrap to fit within max_w, it might very well have to extend beyond max_h.
	calculate_size(elem IElem, max_w, max_h uint32) (w, h uint32)
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

	ui.root = *Div{}.construct(ui, nil, "root")

	handle := cgo.NewHandle(ui)
	return C.uintptr_t(handle)
}

//export GoUiDestroy
func GoUiDestroy(ui_raw C.uintptr_t) {
	handle := cgo.Handle(ui_raw)
	handle.Delete()
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

	elem := Div{}.construct(ui, parent, C.GoString(semantics))

	parent.children = append(parent.children, elem)
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

	elem := Text{}.construct(ui, parent, C.GoString(text), C.GoString(semantics))

	parent.children = append(parent.children, elem)
	ui.dirty = true

	handle := cgo.NewHandle(elem)
	return C.uintptr_t(handle)
}

// TODO We should return true if attribute actually exists.
// TODO Maybe we should just rely on CALL_RET_FAIL or whatever instead.

//export GoUiSetAttrStr
func GoUiSetAttrStr(
	elem_raw C.uintptr_t,
	key_raw *C.char,
	key_len C.size_t,
	val_raw *C.char,
	val_len C.size_t,
) bool {
	elem := elem_from_raw(elem_raw).(IElem).ElemBase()
	elem.set_attr(C.GoString(key_raw), C.GoString(val_raw))
	return false
}

//export GoUiSetAttrU32
func GoUiSetAttrU32(
	elem_raw C.uintptr_t,
	key_raw *C.char,
	key_len C.size_t,
	val C.uint32_t,
) bool {
	elem := elem_from_raw(elem_raw).(IElem).ElemBase()
	elem.set_attr(C.GoString(key_raw), val)
	return false
}

//export GoUiSetAttrF32
func GoUiSetAttrF32(
	elem_raw C.uintptr_t,
	key_raw *C.char,
	key_len C.size_t,
	val float32,
) bool {
	elem := elem_from_raw(elem_raw).(IElem).ElemBase()
	elem.set_attr(C.GoString(key_raw), val)
	return false
}

func main() {}
