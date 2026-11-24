// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package main

import "obiw.ac/aqua/wgpu"

type SolidPipeline struct {
	Pipeline
}

func (b *WgpuBackend) NewSolidPipeline() (*SolidPipeline, error) {
	pipeline, err := b.NewPipeline("Solid", elem_shader_src, "frag_solid_main",
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
		},
		&wgpu.BlendStatePremultipliedAlphaBlending,
	)

	if err != nil {
		return nil, err
	}

	return &SolidPipeline{
		Pipeline: *pipeline,
	}, nil
}

func (pipeline *SolidPipeline) Release() {
	pipeline.Pipeline.Release()
}
