// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package main

import (
	"fmt"

	"obiw.ac/aqua/wgpu"
)

type Pipeline struct {
	shader            *wgpu.ShaderModule
	bind_group_layout *wgpu.BindGroupLayout
	pipeline_layout   *wgpu.PipelineLayout
	pipeline          *wgpu.RenderPipeline
}

func (b *WgpuBackend) NewPipeline(
	label string,
	src string,
	bind_group_layout_entries []wgpu.BindGroupLayoutEntry,
	vbo_layouts []wgpu.VertexBufferLayout,
) (*Pipeline, error) {
	pipeline := &Pipeline{}
	var err error

	if pipeline.shader, err = b.dev.CreateShaderModule(&wgpu.ShaderModuleDescriptor{
		Label: fmt.Sprintf("Shader module (%s)", label),
		WGSLSource: &wgpu.ShaderSourceWGSL{
			Code: src,
		},
	}); err != nil {
		fmt.Println(err)
		return nil, err
	}

	if pipeline.bind_group_layout, err = b.dev.CreateBindGroupLayout(&wgpu.BindGroupLayoutDescriptor{
		Label:   fmt.Sprintf("Bind group layout (%s)", label),
		Entries: bind_group_layout_entries,
	}); err != nil {
		fmt.Println(err)
		pipeline.shader.Release()
		return nil, err
	}

	if pipeline.pipeline_layout, err = b.dev.CreatePipelineLayout(&wgpu.PipelineLayoutDescriptor{
		Label: fmt.Sprintf("Pipeline layout (%s)", label),
		BindGroupLayouts: []*wgpu.BindGroupLayout{
			pipeline.bind_group_layout,
		},
	}); err != nil {
		fmt.Println(err)
		pipeline.shader.Release()
		pipeline.bind_group_layout.Release()
		return nil, err
	}

	if pipeline.pipeline, err = b.dev.CreateRenderPipeline(&wgpu.RenderPipelineDescriptor{
		Label:  fmt.Sprintf("Render pipeline (%s)", label),
		Layout: pipeline.pipeline_layout,
		Primitive: wgpu.PrimitiveState{
			Topology:         wgpu.PrimitiveTopologyTriangleList,
			StripIndexFormat: wgpu.IndexFormatUndefined, // Because we're using triangle list topology instead of strip.
			FrontFace:        wgpu.FrontFaceCCW,
			CullMode:         wgpu.CullModeNone,
		},
		Vertex: wgpu.VertexState{
			Module:     pipeline.shader,
			EntryPoint: "vert_main",
			Buffers:    vbo_layouts,
		},
		Fragment: &wgpu.FragmentState{
			Module:     pipeline.shader,
			EntryPoint: "frag_main",
			Targets: []wgpu.ColorTargetState{
				{
					Format:    b.format,
					Blend:     &wgpu.BlendStatePremultipliedAlphaBlending,
					WriteMask: wgpu.ColorWriteMaskAll,
				},
			},
		},
		Multisample: wgpu.MultisampleState{
			Count:                  1,
			Mask:                   0xFFFFFFFF,
			AlphaToCoverageEnabled: false,
		},
	}); err != nil {
		fmt.Println(err)
		pipeline.shader.Release()
		pipeline.bind_group_layout.Release()
		pipeline.pipeline_layout.Release()
		return nil, err
	}

	return pipeline, nil
}

func (pipeline *Pipeline) Set(render_pass *wgpu.RenderPassEncoder, bind_group *wgpu.BindGroup) {
	render_pass.SetPipeline(pipeline.pipeline)
	render_pass.SetBindGroup(0, bind_group, nil)
}

func (pipeline *Pipeline) Release() {
	pipeline.shader.Release()
	pipeline.bind_group_layout.Release()
	pipeline.pipeline_layout.Release()
	pipeline.pipeline.Release()
}
