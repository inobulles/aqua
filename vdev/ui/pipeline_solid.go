// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package main

import (
	_ "embed"
	"unsafe"

	"obiw.ac/aqua/wgpu"
)

type SolidPipeline struct {
	Pipeline
	vbo_layout wgpu.VertexBufferLayout
}

//go:embed shaders/solid.wgsl
var solid_shader_src string

func (b *WgpuBackend) NewSolidPipeline() (*SolidPipeline, error) {
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
			{ // Normal (nx, ny, nz).
				Format:         wgpu.VertexFormatFloat32x3,
				Offset:         4 * (2 + 2),
				ShaderLocation: 2,
			},
		},
	}

	pipeline, err := b.NewPipeline("Solid", solid_shader_src,
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
		[]wgpu.VertexBufferLayout{
			vbo_layout,
		},
		&wgpu.BlendStateAlphaBlending,
	)

	if err != nil {
		return nil, err
	}

	return &SolidPipeline{
		Pipeline:   *pipeline,
		vbo_layout: vbo_layout,
	}, nil
}

func (pipeline *SolidPipeline) Release() {
	pipeline.Pipeline.Release()
}
