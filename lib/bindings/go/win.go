// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package aqua

/*
#cgo LDFLAGS: -laqua_win

#include <aqua/win.h>

extern void go_lib_bindings_win_redraw_cb(win_t win, void* data);
extern void go_lib_bindings_win_resize_cb(win_t win, void* data, uint32_t x_res, uint32_t y_res);
*/
import "C"
import (
	"runtime/cgo"
	"unsafe"
)

type WinComponent struct {
	Component
}

type WinCtx struct {
	ctx C.win_ctx_t
}

type Win struct {
	ctx *WinCtx
	win C.win_t
}

func (c *Context) WinInit() *WinComponent {
	comp := C.win_init(c.internal)

	if comp == nil {
		return nil
	}

	return &WinComponent{
		Component{internal: comp},
	}
}

func (c *WinComponent) Conn(vdev *VdevDescr) *WinCtx {
	if vdev == nil {
		return nil
	}

	ctx := C.win_conn(&vdev.internal)

	if ctx == nil {
		return nil
	}

	return &WinCtx{ctx}
}

func (c *WinCtx) Create() *Win {
	win := C.win_create(c.ctx)

	if win == nil {
		return nil
	}

	return &Win{
		ctx: c,
		win: win,
	}
}

func (w *Win) Destroy() {
	C.win_destroy(w.win)
}

type WinRedrawCb func()

//export go_lib_bindings_win_redraw_cb
func go_lib_bindings_win_redraw_cb(_ C.win_t, data unsafe.Pointer) {
	handle := (cgo.Handle)(data)

	if cb, ok := handle.Value().(WinRedrawCb); ok {
		cb()
	}
}

func (w *Win) RegisterRedrawCb(cb WinRedrawCb) {
	cb_handle := cgo.NewHandle(cb) // TODO I'm never deleting this handle - should be held on 'w' I guess.

	C.win_register_redraw_cb(
		w.win,
		C.win_redraw_cb_t(C.go_lib_bindings_win_redraw_cb),
		unsafe.Pointer(cb_handle),
	)
}

type WinResizeCb func(x_res, y_res uint32)

//export go_lib_bindings_win_resize_cb
func go_lib_bindings_win_resize_cb(_ C.win_t, data unsafe.Pointer, x_res, y_res C.uint32_t) {
	handle := (cgo.Handle)(data) // TODO I'm never deleting this handle - should be held on 'w' I guess.

	if cb, ok := handle.Value().(WinResizeCb); ok {
		cb(uint32(x_res), uint32(y_res))
	}
}

func (w *Win) RegisterResizeCb(cb WinResizeCb) {
	cb_handle := cgo.NewHandle(cb)

	C.win_register_resize_cb(
		w.win,
		C.win_resize_cb_t(C.go_lib_bindings_win_resize_cb),
		unsafe.Pointer(cb_handle),
	)
}

func (w *Win) Loop() {
	C.win_loop(w.win)
}
