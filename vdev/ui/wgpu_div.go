// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package main

import (
	"obiw.ac/aqua/wgpu"
)

type WgpuBackendDivData struct {
	IWgpuBackendData

	mvp_buf    *wgpu.Buffer
	colour_buf *wgpu.Buffer
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
	if d.bind_group != nil {
		d.bind_group.Release()
	}
	if d.model != nil {
		d.model.release()
	}
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

	// Bind group shit.

	if data.bind_group, err = b.dev.CreateBindGroup(&wgpu.BindGroupDescriptor{
		Layout: b.solid_pipeline.bind_group_layout,
		Entries: []wgpu.BindGroupEntry{
			{
				Binding: 0,
				Buffer:  data.mvp_buf,
				Size:    wgpu.WholeSize,
			},
			{
				Binding: 1,
				Buffer:  data.colour_buf,
				Size:    wgpu.WholeSize,
			},
		},
	}); err != nil {
		println("Can't create bind group.")
		b.free_elem(e)
		return
	}

	// Generate pane model.

	data.model = &Model{}
	data.model.gen_pane(b, float32(w), float32(h), 10)

	e.backend_data = data
}
