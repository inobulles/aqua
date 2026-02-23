// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package wgpu

/*
#cgo LDFLAGS: -laqua_wgpu -laqua_ui

#include <aqua/wgpu.h>
#include <aqua/ui/wgpu.h>

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

	SetGlobalCtx(unsafe.Pointer(ctx))
	return global_ctx
}

func Disconn() {
	if global_ctx == nil {
		return
	}

	C.wgpu_disconn(global_ctx.ctx)
	global_ctx = nil
}

// TODO Should this be brought out into its own source file?

func SetGlobalCtx(c unsafe.Pointer) {
	C.gowebgpu_ctx = C.wgpu_ctx_t(c)
	global_ctx = &WgpuCtx{ctx: C.wgpu_ctx_t(c)}
}

func CreateDeviceFromRaw(dev_raw unsafe.Pointer) Device {
	return Device{
		ref: (C.WGPUDevice)(dev_raw),
	}
}

func (i *Instance) DeviceFromWm(wm *aqua.Wm) Device {
	return CreateDeviceFromRaw(unsafe.Pointer(C.wgpu_device_from_wm(
		global_ctx.ctx,
		i.ref,
		C.wm_t(wm.GetInternalYesIKnowWhatImDoing()),
	)))
}

func (d *Device) CommandEncoderFromRaw(cmd_enc_raw unsafe.Pointer) CommandEncoder {
	return CommandEncoder{
		deviceRef: d.ref,
		ref:       (C.WGPUCommandEncoder)(cmd_enc_raw),
	}
}

func TextureViewFromRaw(view_raw unsafe.Pointer) TextureView {
	return TextureView{
		ref: (C.WGPUTextureView)(view_raw),
	}
}

func (v *TextureView) ToRaw() unsafe.Pointer {
	return unsafe.Pointer(v.ref)
}

func (d *Device) TextureFromVkImage(
	raw_image unsafe.Pointer,
	usage TextureUsage,
	format TextureFormat,
	w, h uint32,
) Texture {
	return Texture{
		deviceRef: d.ref,
		ref: C.aqua_wgpuTextureFromVkImage(
			global_ctx.ctx,
			d.ref, raw_image,
			C.WGPUTextureUsage(usage),
			C.WGPUTextureFormat(format),
			C.uint32_t(w), C.uint32_t(h),
		),
	}
}

func (d *Device) CommandEncoderFromVk(
	raw_cmd_pool unsafe.Pointer,
	raw_cmd_buf unsafe.Pointer,
) CommandEncoder {
	return CommandEncoder{
		deviceRef: d.ref,
		ref: C.aqua_wgpuCommandEncoderFromVk(
			global_ctx.ctx,
			d.ref,
			raw_cmd_pool,
			raw_cmd_buf,
		),
	}
}

func (d *Device) UiInit(ui *aqua.Ui, format TextureFormat) {
	C.ui_wgpu_init(
		C.ui_t(ui.GetInternalYesIKnowWhatImDoing()),
		C.wgpu_get_hid(global_ctx.ctx),
		C.uint64_t(uintptr(unsafe.Pointer(global_ctx.ctx))),
		d.ref,
		C.WGPUTextureFormat(format),
	)
}

func (e *CommandEncoder) UiRender(ui *aqua.Ui, frame *TextureView, x_res, y_res uint32) {
	C.ui_wgpu_render(
		C.ui_t(ui.GetInternalYesIKnowWhatImDoing()),
		frame.ref, e.ref,
		C.uint32_t(x_res), C.uint32_t(y_res),
	)
}
