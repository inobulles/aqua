// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2026 Aymeric Wibo

package main

import (
	"obiw.ac/aqua/wgpu"
)

func (b *WgpuBackend) encounter_frost() {
	// We've encountered frost, end the current render pass.

	b.render_pass.End()
	b.render_pass.Release()

	// Then, swap the current and previous render buffers.

	tmp := b.render_buf
	b.render_buf = b.prev_render_buf
	b.prev_render_buf = tmp

	// Then, copy over the contents of the previous render buffer into the new one.

	b.cmd_enc.CopyTextureToTexture(b.prev_render_buf.tex.AsImageCopy(), b.render_buf.tex.AsImageCopy(), &wgpu.Extent3D{
		Width:              b.x_res,
		Height:             b.y_res,
		DepthOrArrayLayers: 1,
	})

	// Finally, start a new render pass with our new render buffer.

	b.render_pass = b.cmd_enc.BeginRenderPass(&wgpu.RenderPassDescriptor{
		Label: "Intermediate render pass",
		ColorAttachments: []wgpu.RenderPassColorAttachment{
			{
				View:    b.render_buf.view,
				LoadOp:  wgpu.LoadOpLoad,
				StoreOp: wgpu.StoreOpStore,
			},
		},
	})
}
