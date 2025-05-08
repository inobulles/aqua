// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package wgpu

/*
#cgo LDFLAGS: -laqua_wgpu

#include <aqua/wgpu.h>
wgpu_ctx_t gowebgpu_ctx;
*/
import "C"
import "unsafe"

import "obiw.ac/aqua"

type WgpuCtx struct {
	ctx C.wgpu_ctx_t
}

var global_ctx *WgpuCtx = nil

func Init(c *aqua.Context) *aqua.Component {
	comp := C.wgpu_init((C.aqua_ctx_t)(unsafe.Pointer(c.GetInternal())))

	if comp == nil {
		return nil
	}

	return aqua.ComponentFromInternal(unsafe.Pointer(comp))
}

func Conn(vdev *aqua.VdevDescr) *WgpuCtx {
	if global_ctx != nil {
		return global_ctx
	}

	if vdev == nil {
		return nil
	}

	ctx := C.wgpu_conn((*C.kos_vdev_descr_t)(unsafe.Pointer(vdev.GetInternal())))

	if ctx == nil {
		return nil
	}

	C.gowebgpu_ctx = ctx
	global_ctx = &WgpuCtx{ctx: ctx}

	return global_ctx
}

func Disconn() {
	if global_ctx == nil {
		return
	}

	C.wgpu_disconn(global_ctx.ctx)
	global_ctx = nil
}
