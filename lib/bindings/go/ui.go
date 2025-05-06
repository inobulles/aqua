// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package aqua

/*
#cgo LDFLAGS: -laqua_ui

#include <aqua/ui.h>
*/
import "C"

func (c *Context) UiInit() *Component {
	comp := C.ui_init(c.internal)

	if comp == nil {
		return nil
	}

	return &Component{internal: comp}
}
