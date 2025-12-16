// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package aqua

/*
#cgo LDFLAGS: -laqua_wgpu

#include <aqua/wgpu.h>
*/
import "C"

type WgpuComponent struct {
	Component
}

func (c *Context) WgpuInit() *WgpuComponent {
	comp := C.wgpu_init(c.internal)

	if comp == nil {
		return nil
	}

	return &WgpuComponent{
		Component{internal: comp},
	}
}

type WgpuCtx struct {
	ctx C.wgpu_ctx_t
}

func (c *WgpuComponent) Conn(vdev *VdevDescr) *WgpuCtx {
	if vdev == nil {
		return nil
	}

	ctx := C.wgpu_conn(&vdev.internal)

	if ctx == nil {
		return nil
	}

	return &WgpuCtx{ctx}
}
