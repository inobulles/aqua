// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package main

import (
	_ "embed"
	"unsafe"

	"obiw.ac/aqua/wgpu"
)

type RegularPipeline struct {
	Pipeline
	vbo_layout wgpu.VertexBufferLayout
}

//go:embed shaders/regular.wgsl
var regular_shader_src string

func (b *WgpuBackend) NewRegularPipeline() (*RegularPipeline, error) {
	vbo_layout := wgpu.VertexBufferLayout{
		ArrayStride: uint64(unsafe.Sizeof(Vertex{})),
		StepMode:    wgpu.VertexStepModeVertex,
		Attributes: []wgpu.VertexAttribute{
			{ // Vertex position (x, y).
				Format:         wgpu.VertexFormatFloat32x2,
				Offset:         0,
				ShaderLocation: 0,
			},
			{ // Texture coordinate (u, v).
				Format:         wgpu.VertexFormatFloat32x2,
				Offset:         4 * 2,
				ShaderLocation: 1,
			},
		},
	}

	pipeline, err := b.NewPipeline("Regular", regular_shader_src,
		[]wgpu.BindGroupLayoutEntry{
			{ // Model-view-projection matrix.
				Binding:    0,
				Visibility: wgpu.ShaderStageVertex,
				Buffer: wgpu.BufferBindingLayout{
					Type: wgpu.BufferBindingTypeUniform,
				},
			},
			{ // Texture.
				Binding:    1,
				Visibility: wgpu.ShaderStageFragment,
				Texture: wgpu.TextureBindingLayout{
					Multisampled:  false,
					ViewDimension: wgpu.TextureViewDimension2D,
					SampleType:    wgpu.TextureSampleTypeFloat,
				},
			},
			{ // Sampler.
				Binding:    2,
				Visibility: wgpu.ShaderStageFragment,
				Sampler: wgpu.SamplerBindingLayout{
					Type: wgpu.SamplerBindingTypeFiltering,
				},
			},
		},
		[]wgpu.VertexBufferLayout{
			vbo_layout,
		},
	)

	if err != nil {
		return nil, err
	}

	return &RegularPipeline{
		Pipeline:   *pipeline,
		vbo_layout: vbo_layout,
	}, nil
}

func (pipeline *RegularPipeline) Release() {
	pipeline.Pipeline.Release()
}
