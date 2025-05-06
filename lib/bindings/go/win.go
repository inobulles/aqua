// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package aqua

/*
#cgo LDFLAGS: -laqua_win

#include <aqua/win.h>
*/
import "C"

type WinCtx struct {
	ctx C.win_ctx_t
}

type Win struct {
	ctx *WinCtx
	win C.win_t
}

func (c *Context) WinInit() *Component {
	comp := C.win_init(c.internal)

	if comp == nil {
		return nil
	}

	return &Component{internal: comp}
}

func (c *Component) Conn(vdev *VdevDescr) *WinCtx {
	if vdev == nil {
		return nil
	}

	ctx := C.win_conn(&vdev.internal)

	if ctx == nil {
		return nil
	}

	return &WinCtx{ctx: ctx}
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

func (w *Win) Loop() {
	C.win_loop(w.win)
}
