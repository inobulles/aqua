// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025-2026 Aymeric Wibo

package main

import (
	_ "embed"
	"fmt"
	"unsafe"

	"obiw.ac/aqua/wgpu"
)

type TexturePipeline struct {
	Pipeline
	vbo_layout wgpu.VertexBufferLayout
}

type TextureAlpha int

const (
	TextureAlphaTranslucent TextureAlpha = iota
	TextureAlphaOpaque
)

//go:embed shaders/texture.wgsl
var texture_shader_src string

func (b *WgpuBackend) NewTexturePipeline(alpha TextureAlpha) (*TexturePipeline, error) {
	var label, frag_entry string

	switch alpha {
	case TextureAlphaTranslucent:
		label = "Texture"
		frag_entry = "frag_main"
	case TextureAlphaOpaque:
		label = "Texture (opaque)"
		frag_entry = "frag_opaque_main"
	default:
		panic(fmt.Sprintf("unexpected main.TextureAlpha: %#v", alpha))
	}

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

	pipeline, err := b.NewPipeline(label, texture_shader_src, frag_entry,
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
		&wgpu.BlendStatePremultipliedAlphaBlending,
	)

	if err != nil {
		return nil, err
	}

	return &TexturePipeline{
		Pipeline:   *pipeline,
		vbo_layout: vbo_layout,
	}, nil
}

func (pipeline *TexturePipeline) Release() {
	pipeline.Pipeline.Release()
}
