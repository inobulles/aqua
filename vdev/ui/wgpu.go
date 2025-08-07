// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package main

/*
#include "../../kos/lib/vdev.h"
*/
import "C"
import (
	"runtime/cgo"
	"unsafe"

	"obiw.ac/aqua/wgpu"
)

type WgpuBackend struct {
	Backend
	dev wgpu.Device
}

//export GoUiBackendWgpuInit
func GoUiBackendWgpuInit(
	ui_raw C.uintptr_t,
	hid C.uint64_t,
	vid C.uint64_t,
	dev_raw unsafe.Pointer,
) {
	ui := cgo.Handle(ui_raw).Value().(*Ui)

	ui.backend = &WgpuBackend{
		dev: wgpu.CreateDeviceFromRaw(dev_raw),
	}

	wgpu.SetGlobalCtx(unsafe.Pointer(uintptr(hid)))

	// TODO Have to set global context somehow.
	// Oof, this is gonna be complicated actually.
	// Okay, so this is how I'm gonna approach this:
	// - First, we gonna wanna
}

//export GoUiBackendWgpuRender
func GoUiBackendWgpuRender(
	ui_raw C.uintptr_t,
	frame_raw unsafe.Pointer,
	cmd_enc_raw unsafe.Pointer,
) {
	ui := cgo.Handle(ui_raw).Value().(*Ui)
	backend := ui.backend.(*WgpuBackend)

	cmd_enc := backend.dev.CommandEncoderFromRaw(cmd_enc_raw)
	frame := wgpu.TextureViewFromRaw(frame_raw)

	render_pass_descr := wgpu.RenderPassDescriptor{
		Label: "render_pass",
		ColorAttachments: []wgpu.RenderPassColorAttachment{
			{
				View:    &frame,
				LoadOp:  wgpu.LoadOpClear,
				StoreOp: wgpu.StoreOpStore,
				ClearValue: wgpu.Color{
					R: 0.0,
					G: 1.0,
					B: 0.0,
					A: 1.0,
				},
			},
		},
	}

	render_pass := cmd_enc.BeginRenderPass(&render_pass_descr)
	defer render_pass.Release()

	// TODO render_pass.SetPipeline()
	// TODO render_pass.Draw()

	render_pass.End()
}
