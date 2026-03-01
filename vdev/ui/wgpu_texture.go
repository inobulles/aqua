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

func create_tex_size(w, h uint32) wgpu.Extent3D {
	return wgpu.Extent3D{
		Width:              w,
		Height:             h,
		DepthOrArrayLayers: 1,
	}
}

func create_texture_common(
	b *WgpuBackend, name string, tex_size wgpu.Extent3D,
	format wgpu.TextureFormat, usage wgpu.TextureUsage,
) (*WgpuTexture, error) {
	tex := WgpuTexture{}

	var err error

	if tex.tex, err = b.dev.CreateTexture(&wgpu.TextureDescriptor{
		Label:         name,
		Size:          tex_size,
		MipLevelCount: 1,
		SampleCount:   1,
		Dimension:     wgpu.TextureDimension2D,
		Format:        format,
		Usage:         usage,
	}); err != nil {
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

func (b *WgpuBackend) NewTexture(name string, w, h uint32, data []byte) (*WgpuTexture, error) {
	tex_size := create_tex_size(w, h)
	tex, err := create_texture_common(b, name, tex_size,
		wgpu.TextureFormatRGBA8Unorm,
		wgpu.TextureUsageTextureBinding|wgpu.TextureUsageCopyDst)

	if err != nil {
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

	return tex, nil
}

func (b *WgpuBackend) NewRenderBuf(name string, w, h uint32, format wgpu.TextureFormat) (*WgpuTexture, error) {
	return create_texture_common(b, name, create_tex_size(w, h), format,
		wgpu.TextureUsageCopySrc|wgpu.TextureUsageRenderAttachment|wgpu.TextureUsageCopyDst)
}

func (t *WgpuTexture) Release() {
	t.tex.Release()
	t.view.Release()
	t.sampler.Release()
}
