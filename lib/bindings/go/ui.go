// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package aqua

/*
#cgo LDFLAGS: -laqua_ui

#include <aqua/ui.h>
*/
import "C"
import "unsafe"

type UiComponent struct {
	Component
}

type UiCtx struct {
	ctx C.ui_ctx_t
}

type Ui struct {
	ctx *UiCtx
	ui  C.ui_t
}

type UiElem struct {
	ui   *Ui
	elem C.ui_elem_t
}

type UiSupportedBackends int

const (
	UI_BACKEND_NONE UiSupportedBackends = C.UI_BACKEND_NONE
	UI_BACKEND_WGPU UiSupportedBackends = C.UI_BACKEND_WGPU
)

func (c *Context) UiInit() *UiComponent {
	comp := C.ui_init(c.internal)

	if comp == nil {
		return nil
	}

	return &UiComponent{
		Component{internal: comp},
	}
}

func (c *UiComponent) Conn(vdev *VdevDescr) *UiCtx {
	if vdev == nil {
		return nil
	}

	ctx := C.ui_conn(&vdev.internal)

	if ctx == nil {
		return nil
	}

	return &UiCtx{ctx}
}

func (c *UiCtx) GetSupportedBackends() UiSupportedBackends {
	return UiSupportedBackends(C.ui_get_supported_backends(c.ctx))
}

func (c *UiCtx) Create() *Ui {
	ui := C.ui_create(c.ctx)

	if ui == nil {
		return nil
	}

	return &Ui{
		ctx: c,
		ui:  ui,
	}
}

func (u *Ui) Destroy() {
	C.ui_destroy(u.ui)
}

func (u *Ui) GetRoot() *UiElem {
	return &UiElem{
		ui:   u,
		elem: C.ui_get_root(u.ui),
	}
}

func (e *UiElem) AddText(semantics, text string) *UiElem {
	c_semantics := C.CString(semantics)
	defer C.free(unsafe.Pointer(c_semantics))

	c_text := C.CString(text)
	defer C.free(unsafe.Pointer(c_text))

	return &UiElem{
		ui:   e.ui,
		elem: C.ui_add_text(e.elem, c_semantics, c_text),
	}
}
