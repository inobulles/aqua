// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package main

import "obiw.ac/aqua/wgpu"

type TextPipeline struct {
	Pipeline
}

func (b *WgpuBackend) NewTextPipeline() (*TextPipeline, error) {
	pipeline, err := b.NewPipeline("Text", elem_shader_src, "frag_text_main",
		[]wgpu.BindGroupLayoutEntry{
			{ // Model-view-projection matrix.
				Binding:    0,
				Visibility: wgpu.ShaderStageVertex,
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

	return &TextPipeline{
		Pipeline: *pipeline,
	}, nil
}

func (pipeline *TextPipeline) Release() {
	pipeline.Pipeline.Release()
}
