// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package main

import "obiw.ac/aqua/wgpu"
import "unsafe"

//export GoUiBackendWgpuInit
func GoUiBackendWgpuInit() {
	// Have to set global context somehow.
	// Oof, this is gonna be complicated actually.
}

//export GoUiBackendWgpuRender
func GoUiBackendWgpuRender(dev_raw unsafe.Pointer, frame_raw unsafe.Pointer, cmd_enc_raw unsafe.Pointer) {
	cmd_enc := wgpu.CommandEncoderFromRaw(dev_raw, cmd_enc_raw)
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
