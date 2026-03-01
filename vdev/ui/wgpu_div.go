// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package main

import (
	"unsafe"

	"obiw.ac/aqua/wgpu"
)

type WgpuBackendDivData struct {
	IWgpuBackendData

	mvp_buf    *wgpu.Buffer
	colour_buf *wgpu.Buffer

	tex *WgpuTexture

	pipeline   *Pipeline
	bind_group *wgpu.BindGroup

	model *Model
}

func (d *WgpuBackendDivData) release() {
	if d.mvp_buf != nil {
		d.mvp_buf.Release()
	}
	if d.colour_buf != nil {
		d.colour_buf.Release()
	}
	if d.tex != nil {
		d.tex.Release()
	}
	if d.bind_group != nil {
		d.bind_group.Release()
	}
	if d.model != nil {
		d.model.release()
	}
}

func (d *WgpuBackendDivData) create_solid_bind_group(b *WgpuBackend) error {
	var err error

	if d.bind_group, err = b.dev.CreateBindGroup(&wgpu.BindGroupDescriptor{
		Layout: b.solid_pipeline.bind_group_layout,
		Entries: []wgpu.BindGroupEntry{
			{
				Binding: 0,
				Buffer:  d.mvp_buf,
				Size:    wgpu.WholeSize,
			},
			{
				Binding: 1,
				Buffer:  d.colour_buf,
				Size:    wgpu.WholeSize,
			},
		},
	}); err != nil {
		println("Can't create solid bind group.")
	}

	return err
}

func (d *WgpuBackendDivData) create_texture_bind_group(b *WgpuBackend) error {
	var err error

	if d.bind_group, err = b.dev.CreateBindGroup(&wgpu.BindGroupDescriptor{
		Layout: b.texture_pipeline.bind_group_layout,
		Entries: []wgpu.BindGroupEntry{
			{
				Binding: 0,
				Buffer:  d.mvp_buf,
				Size:    wgpu.WholeSize,
			},
			{
				Binding:     1,
				TextureView: d.tex.view,
			},
			{
				Binding: 2,
				Sampler: d.tex.sampler,
			},
		},
	}); err != nil {
		println("Can't create texture bind group.")
	}

	return err
}

func (d *WgpuBackendDivData) create_frost_bind_group(b *WgpuBackend) error {
	var err error

	if d.bind_group, err = b.dev.CreateBindGroup(&wgpu.BindGroupDescriptor{
		Layout: b.frost_pipeline.bind_group_layout,
		Entries: []wgpu.BindGroupEntry{
			{
				Binding: 0,
				Buffer:  d.mvp_buf,
				Size:    wgpu.WholeSize,
			},
			{
				Binding:     1,
				TextureView: d.tex.view,
			},
			{
				Binding: 2,
				Sampler: d.tex.sampler,
			},
			{
				Binding:     3,
				TextureView: b.prev_render_buf.view,
			},
			{
				Binding: 4,
				Sampler: b.prev_render_buf.sampler,
			},
		},
	}); err != nil {
		println("Can't create frost bind group.")
	}

	return err
}

func (b *WgpuBackend) gen_div_backend_data(e *Div, w, h uint32) {
	// If we already have backend data, free everything.

	if e.backend_data != nil {
		b.free_elem(e)
	}

	data := WgpuBackendDivData{}

	// Create MVP matrix buffer.

	var err error

	if data.mvp_buf, err = b.dev.CreateBuffer(&wgpu.BufferDescriptor{
		Size:  64,
		Usage: wgpu.BufferUsageUniform | wgpu.BufferUsageCopyDst,
	}); err != nil {
		println("Can't create MVP matrix buffer.")
		b.free_elem(e)
		return
	}

	// Create colour vector buffer.

	if data.colour_buf, err = b.dev.CreateBuffer(&wgpu.BufferDescriptor{
		Size:  16,
		Usage: wgpu.BufferUsageUniform | wgpu.BufferUsageCopyDst,
	}); err != nil {
		println("Can't create colour vector buffer.")
		b.free_elem(e)
		return
	}

	// If we have a raster background attribute, create texture.

	switch bg := e.get_attr("bg").(type) {
	case Raster:
		if data.tex, err = b.NewTexture("Div texture", bg.x_res, bg.y_res, bg.data); err != nil {
			panic(err)
		}
	}

	// If we have a WebGPU background attribute, get it.

	if bg := e.get_attr("bg.wgpu_tex_view"); bg != nil {
		view := wgpu.TextureViewFromRaw(bg.(unsafe.Pointer))
		data.tex = &WgpuTexture{view: &view}
		data.tex.CreateSampler(b)
	}

	// Bind group shit.

	if data.tex == nil {
		if err = data.create_solid_bind_group(b); err != nil {
			b.free_elem(e)
			return
		}
	} else if e.do_frost() {
		if err = data.create_frost_bind_group(b); err != nil {
			b.free_elem(e)
			return
		}
	} else {
		if err = data.create_texture_bind_group(b); err != nil {
			b.free_elem(e)
			return
		}
	}

	// Generate pane model.

	data.model = &Model{}
	data.model.gen_pane(b, float32(w), float32(h), 10)

	e.backend_data = data
}
