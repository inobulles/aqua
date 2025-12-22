// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package main

import "obiw.ac/aqua/wgpu"

type WgpuTexture struct {
	tex     *wgpu.Texture
	view    *wgpu.TextureView
	sampler *wgpu.Sampler
}

func (t *WgpuTexture) CreateSampler(b *WgpuBackend) error {
	var err error

	if t.sampler, err = b.dev.CreateSampler(&wgpu.SamplerDescriptor{
		AddressModeU:  wgpu.AddressModeClampToEdge,
		AddressModeV:  wgpu.AddressModeClampToEdge,
		AddressModeW:  wgpu.AddressModeClampToEdge,
		MagFilter:     wgpu.FilterModeLinear,
		MinFilter:     wgpu.FilterModeLinear,
		MipmapFilter:  wgpu.MipmapFilterModeLinear,
		MaxAnisotropy: 1,
	}); err != nil {
		return err
	}

	return nil
}

func (b *WgpuBackend) NewTexture(name string, w, h uint32, data []byte) (*WgpuTexture, error) {
	tex := WgpuTexture{}

	tex_size := wgpu.Extent3D{
		Width:              w,
		Height:             h,
		DepthOrArrayLayers: 1,
	}

	var err error

	if tex.tex, err = b.dev.CreateTexture(&wgpu.TextureDescriptor{
		Label:         name,
		Size:          tex_size,
		MipLevelCount: 1,
		SampleCount:   1,
		Dimension:     wgpu.TextureDimension2D,
		Format:        wgpu.TextureFormatRGBA8Unorm,
		Usage:         wgpu.TextureUsageTextureBinding | wgpu.TextureUsageCopyDst,
	}); err != nil {
		return nil, err
	}

	if err = b.dev.GetQueue().WriteTexture(
		tex.tex.AsImageCopy(),
		data,
		&wgpu.TexelCopyBufferLayout{
			Offset:       0,
			BytesPerRow:  4 * w,
			RowsPerImage: h,
		},
		&tex_size,
	); err != nil {
		tex.tex.Release()
		return nil, err
	}

	if tex.view, err = tex.tex.CreateView(nil); err != nil {
		tex.tex.Release()
		return nil, err
	}

	if err = tex.CreateSampler(b); err != nil {
		tex.tex.Release()
		tex.view.Release()
		return nil, err
	}

	return &tex, nil
}

func (t *WgpuTexture) Release() {
	t.tex.Release()
	t.view.Release()
	t.sampler.Release()
}
