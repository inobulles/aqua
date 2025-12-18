// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package main

import (
	"fmt"
	"image"

	"obiw.ac/aqua/wgpu"
)

type WgpuBackendTextData struct {
	IWgpuBackendData

	flow_w uint32
	data   *image.RGBA

	tex        *WgpuTexture
	mvp_buf    *wgpu.Buffer
	bind_group *wgpu.BindGroup

	model *Model
}

func (d *WgpuBackendTextData) release() {
	if d.tex != nil {
		d.tex.Release()
	}
	if d.mvp_buf != nil {
		d.mvp_buf.Release()
	}
	if d.bind_group != nil {
		d.bind_group.Release()
	}
	if d.model != nil {
		d.model.release()
	}
}

func (b *WgpuBackend) gen_text_backend_data(e *Text) {
	// TODO Split into multiple functions when I make quad thing made only once for backend, because cleaning when error is kinda disgusting currently.

	// If we already have backend data, free everything.

	if e.backend_data != nil {
		b.free_elem(e)
	}

	// Generate new text.

	flow_w := e.ElemBase().flow_w

	data := WgpuBackendTextData{
		flow_w: flow_w,
		data:   b.get_font(e).Render(e.text, int(flow_w)),
	}

	w := uint32(data.data.Bounds().Dx())
	h := uint32(data.data.Bounds().Dy())

	var err error

	if data.tex, err = b.NewTexture(
		fmt.Sprintf("Texture (%s)", e.text),
		w, h, data.data.Pix,
	); err != nil {
		println("Can't create texture.")
		b.free_elem(e)
		return
	}

	// Create MVP matrix buffer.

	if data.mvp_buf, err = b.dev.CreateBuffer(&wgpu.BufferDescriptor{
		Size:  64,
		Usage: wgpu.BufferUsageUniform | wgpu.BufferUsageCopyDst,
	}); err != nil {
		println("Can't create MVP matrix buffer.")
		b.free_elem(e)
		return
	}

	// Bind group shit.

	if data.bind_group, err = b.dev.CreateBindGroup(&wgpu.BindGroupDescriptor{
		Layout: b.regular_pipeline.bind_group_layout,
		Entries: []wgpu.BindGroupEntry{
			{
				Binding: 0,
				Buffer:  data.mvp_buf,
				Size:    wgpu.WholeSize,
			},
			{
				Binding:     1,
				TextureView: data.tex.view,
			},
			{
				Binding: 2,
				Sampler: data.tex.sampler,
			},
		},
	}); err != nil {
		println("Can't create bind group.")
		b.free_elem(e)
		return
	}

	data.model = &Model{}
	data.model.gen_quad(b, float32(w), float32(h))

	e.backend_data = data
}
