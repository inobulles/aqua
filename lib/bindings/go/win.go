// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package aqua

/*
#cgo LDFLAGS: -laqua_win

#include <aqua/win.h>
*/
import "C"

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

func (w *Win) Loop() {
	C.win_loop(w.win)
}
