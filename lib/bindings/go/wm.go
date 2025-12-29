// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package aqua

/*
#cgo LDFLAGS: -laqua_wm

#include <aqua/wm.h>

extern void go_lib_bindings_wm_redraw_cb(wm_t wm, void* raw_image, void* data);
*/
import "C"
import (
	"runtime/cgo"
	"unsafe"
)

type WmComponent struct {
	Component
}

type WmCtx struct {
	ctx C.wm_ctx_t
}

type Wm struct {
	ctx *WmCtx
	wm  C.wm_t
}

func (c *Context) WmInit() *WmComponent {
	comp := C.wm_init(c.internal)

	if comp == nil {
		return nil
	}

	return &WmComponent{
		Component{internal: comp},
	}
}

func (c *WmComponent) Conn(vdev *VdevDescr) *WmCtx {
	if vdev == nil {
		return nil
	}

	ctx := C.wm_conn(&vdev.internal)

	if ctx == nil {
		return nil
	}

	return &WmCtx{ctx}
}

func (c *WmCtx) Disconn() {
	C.wm_disconn(c.ctx)
}

func (c *WmCtx) Create() *Wm {
	wm := C.wm_create(c.ctx)

	if wm == nil {
		return nil
	}

	return &Wm{
		ctx: c,
		wm:  wm,
	}
}

func (w *Wm) Destroy() {
	C.wm_destroy(w.wm)
}

type WmRedrawCb func(raw_image unsafe.Pointer)

//export go_lib_bindings_wm_redraw_cb
func go_lib_bindings_wm_redraw_cb(_ C.wm_t, raw_image unsafe.Pointer, data unsafe.Pointer) {
	handle := (cgo.Handle)(data)

	if cb, ok := handle.Value().(WmRedrawCb); ok {
		cb(raw_image)
	}
}

func (w *Wm) RegisterRedrawCb(cb WmRedrawCb) {
	cb_handle := cgo.NewHandle(cb) // TODO I'm never deleting this handle - should be held on 'w' I guess.

	C.wm_register_redraw_cb(
		w.wm,
		C.wm_redraw_cb_t(C.go_lib_bindings_wm_redraw_cb),
		unsafe.Pointer(cb_handle),
	)
}

func (w *Wm) Loop() {
	C.wm_loop(w.wm)
}

func (w *Wm) GetInternalYesIKnowWhatImDoing() unsafe.Pointer {
	return unsafe.Pointer(w.wm)
}
