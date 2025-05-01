// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package aqua

/*
#cgo LDFLAGS: -laqua_win

#include <aqua/win.h>
*/
import "C"

func (c *Context) WinInit() *Component {
	comp := C.win_init(c.ctx)

	if comp == nil {
		return nil
	}

	return &Component{comp: comp}
}
