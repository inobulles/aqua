// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025-2026 Aymeric Wibo

package main

import (
	"fmt"

	"obiw.ac/aqua/wgpu"
)

type TexturePipeline struct {
	Pipeline
}

type TextureAlpha int

const (
	TextureAlphaTranslucent TextureAlpha = iota
	TextureAlphaOpaque
)

func (b *WgpuBackend) NewTexturePipeline(alpha TextureAlpha) (*TexturePipeline, error) {
	var label, frag_entry string

	switch alpha {
	case TextureAlphaTranslucent:
		label = "Texture"
		frag_entry = "frag_texture_main"
	case TextureAlphaOpaque:
		label = "Texture (opaque)"
		frag_entry = "frag_texture_opaque_main"
	default:
		panic(fmt.Sprintf("unexpected main.TextureAlpha: %#v", alpha))
	}

	pipeline, err := b.NewPipeline(label, elem_shader_src, frag_entry,
		[]wgpu.BindGroupLayoutEntry{
			{ // Model-view-projection matrix.
				Binding:    0,
				Visibility: wgpu.ShaderStageVertex,
				Buffer: wgpu.BufferBindingLayout{
					Type: wgpu.BufferBindingTypeUniform,
				},
			},
			{ // Colour.
				Binding:    1,
				Visibility: wgpu.ShaderStageFragment,
				Buffer: wgpu.BufferBindingLayout{
					Type: wgpu.BufferBindingTypeUniform,
				},
			},
			{ // Texture.
				Binding:    2,
				Visibility: wgpu.ShaderStageFragment,
				Texture: wgpu.TextureBindingLayout{
					Multisampled:  false,
					ViewDimension: wgpu.TextureViewDimension2D,
					SampleType:    wgpu.TextureSampleTypeFloat,
				},
			},
			{ // Sampler.
				Binding:    3,
				Visibility: wgpu.ShaderStageFragment,
				Sampler: wgpu.SamplerBindingLayout{
					Type: wgpu.SamplerBindingTypeFiltering,
				},
			},
		},
		&wgpu.BlendStatePremultipliedAlphaBlending,
	)

	if err != nil {
		return nil, err
	}

	return &TexturePipeline{
		Pipeline: *pipeline,
	}, nil
}

func (pipeline *TexturePipeline) Release() {
	pipeline.Pipeline.Release()
}
