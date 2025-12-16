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
	data *image.RGBA

	tex     *wgpu.Texture
	view    *wgpu.TextureView
	sampler *wgpu.Sampler

	mvp_buf    *wgpu.Buffer
	bind_group *wgpu.BindGroup

	model *Model
}

func (d *WgpuBackendTextData) release() {
	if d.tex != nil {
		d.tex.Release()
	}
	if d.view != nil {
		d.view.Release()
	}
	if d.sampler != nil {
		d.sampler.Release()
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

	data := WgpuBackendTextData{
		data: b.get_font(e).Render(e.text, int(e.ElemBase().flow_w)),
	}

	w := uint32(data.data.Bounds().Dx())
	h := uint32(data.data.Bounds().Dy())

	tex_size := wgpu.Extent3D{
		Width:              w,
		Height:             h,
		DepthOrArrayLayers: 1,
	}

	var err error

	if data.tex, err = b.dev.CreateTexture(&wgpu.TextureDescriptor{
		Label:         fmt.Sprintf("Texture (%s)", e.text),
		Size:          tex_size,
		MipLevelCount: 1,
		SampleCount:   1,
		Dimension:     wgpu.TextureDimension2D,
		Format:        wgpu.TextureFormatRGBA8Unorm,
		Usage:         wgpu.TextureUsageTextureBinding | wgpu.TextureUsageCopyDst,
	}); err != nil {
		println("Can't create texture.")
		b.free_elem(e)
		return
	}

	if err = b.dev.GetQueue().WriteTexture(
		data.tex.AsImageCopy(),
		data.data.Pix,
		&wgpu.TexelCopyBufferLayout{
			Offset:       0,
			BytesPerRow:  4 * w,
			RowsPerImage: h,
		},
		&tex_size,
	); err != nil {
		println("Can't write texture.")
		b.free_elem(e)
		return
	}

	if data.view, err = data.tex.CreateView(nil); err != nil {
		println("Can't create texture view.")
		b.free_elem(e)
		return
	}

	if data.sampler, err = b.dev.CreateSampler(&wgpu.SamplerDescriptor{
		AddressModeU:  wgpu.AddressModeClampToEdge,
		AddressModeV:  wgpu.AddressModeClampToEdge,
		AddressModeW:  wgpu.AddressModeClampToEdge,
		MagFilter:     wgpu.FilterModeLinear,
		MinFilter:     wgpu.FilterModeLinear,
		MipmapFilter:  wgpu.MipmapFilterModeLinear,
		MaxAnisotropy: 1,
	}); err != nil {
		println("Can't create sampler.")
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
				TextureView: data.view,
			},
			{
				Binding: 2,
				Sampler: data.sampler,
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
