// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package main

/*
#include "../../kos/lib/vdriver.h"
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
	cid C.uint64_t,
	dev_raw unsafe.Pointer,
) {
	ui := cgo.Handle(ui_raw).Value().(*Ui)

	ui.backend = &WgpuBackend{
		dev: wgpu.CreateDeviceFromRaw(dev_raw),
	}

	wgpu.SetGlobalCtx(unsafe.Pointer(uintptr(cid)))

	// TODO Have to set global context somehow.
	// Oof, this is gonna be complicated actually.
	// Okay, so this is how I'm gonna approach this:
	// - First, we need the client's connection to be passed on to us so that we can use it.
	// - Or, we could still need to connect, but be able to "inherit" an existing connection somehow? But the connection ID entirely depends on the KOS and the VDEV we're trying to share doesn't know of this number.
	// - This is probably going to require quite a bit of reworking of stuff.
	// - I can use this opportunity to figure out how the .wgpu device's surface-from-window creation is going to work. We must reject creation on a window which is not on the same host. But even then being able to do this is not a guarantee cuz the devices could be in different processes. What's the solution to this? Do we use the hid:vid to figure out if the device is local or UDS to us?

	// - Should we figure out a way to do .wgpu and .win on different devices?
	// - This means we gotta render .wgpu on an offscreen buffer and then transfer it to .win -> not too bad a solution actually.
	// - Same solution for UDS devices.
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
