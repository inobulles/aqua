// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2026 Aymeric Wibo

package main

import (
	"fmt"

	"obiw.ac/aqua/wgpu"
)

type WgpuBackendButtonData struct {
	IWgpuBackendData

	mvp_buf       *wgpu.Buffer
	colour_buf    *wgpu.Buffer
	bg_bind_group *wgpu.BindGroup
	bg_model      *Model

	// Optional text label.
	label_tex        *WgpuTexture
	label_bind_group *wgpu.BindGroup
	label_model      *Model
}

func (d *WgpuBackendButtonData) release() {
	if d.mvp_buf != nil {
		d.mvp_buf.Release()
	}
	if d.colour_buf != nil {
		d.colour_buf.Release()
	}
	if d.bg_bind_group != nil {
		d.bg_bind_group.Release()
	}
	if d.bg_model != nil {
		d.bg_model.release()
	}
	if d.label_tex != nil {
		d.label_tex.Release()
	}
	if d.label_bind_group != nil {
		d.label_bind_group.Release()
	}
	if d.label_model != nil {
		d.label_model.release()
	}
}

func (b *WgpuBackend) gen_button_backend_data(e *Button) {
	if e.backend_data != nil {
		b.free_elem(e)
	}

	data := &WgpuBackendButtonData{}
	var err error

	if data.mvp_buf, err = b.dev.CreateBuffer(&wgpu.BufferDescriptor{
		Size:  64,
		Usage: wgpu.BufferUsageUniform | wgpu.BufferUsageCopyDst,
	}); err != nil {
		println("Can't create MVP matrix buffer for button.")
		return
	}

	if data.colour_buf, err = b.dev.CreateBuffer(&wgpu.BufferDescriptor{
		Size:  16,
		Usage: wgpu.BufferUsageUniform | wgpu.BufferUsageCopyDst,
	}); err != nil {
		println("Can't create colour buffer for button.")
		return
	}

	if data.bg_bind_group, err = b.dev.CreateBindGroup(&wgpu.BindGroupDescriptor{
		Layout: b.solid_pipeline.bind_group_layout,
		Entries: []wgpu.BindGroupEntry{
			{Binding: 0, Buffer: data.mvp_buf, Size: wgpu.WholeSize},
			{Binding: 1, Buffer: data.colour_buf, Size: wgpu.WholeSize},
		},
	}); err != nil {
		println("Can't create button background bind group.")
		return
	}

	data.bg_model = &Model{}
	data.bg_model.gen_pane(b, float32(e.flow_w), float32(e.flow_h), 8)

	if e.text != "" {
		img := b.paragraph_font.Render(e.text, 0)
		lw := uint32(img.Bounds().Dx())
		lh := uint32(img.Bounds().Dy())

		if data.label_tex, err = b.NewTexture(
			fmt.Sprintf("Button label (%s)", e.text), lw, lh, img.Pix,
		); err != nil {
			println("Can't create button label texture.")
		} else if data.label_bind_group, err = b.dev.CreateBindGroup(&wgpu.BindGroupDescriptor{
			Layout: b.regular_pipeline.bind_group_layout,
			Entries: []wgpu.BindGroupEntry{
				{Binding: 0, Buffer: data.mvp_buf, Size: wgpu.WholeSize},
				{Binding: 1, TextureView: data.label_tex.view},
				{Binding: 2, Sampler: data.label_tex.sampler},
			},
		}); err != nil {
			println("Can't create button label bind group.")
		} else {
			data.label_model = &Model{}
			data.label_model.gen_quad(b, float32(lw), float32(lh))
		}
	}

	e.backend_data = data
}
