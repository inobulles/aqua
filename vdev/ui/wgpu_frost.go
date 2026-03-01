// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2026 Aymeric Wibo

package main

import (
	_ "embed"

	"obiw.ac/aqua/wgpu"
)

const WGPU_FROST_DOWNSAMPLE_STEPS = 3

type WgpuBackendFrost struct {
	kawase_down_first_pipeline *Pipeline
}

type WgpuBackendDivDataFrost struct {
	render_bufs [WGPU_FROST_DOWNSAMPLE_STEPS]*WgpuTexture

	bg_crop_bufs [WGPU_FROST_DOWNSAMPLE_STEPS]*wgpu.Buffer
	// TODO idk if its really necessary to keep a hold of res_x/y here, cuz we could just write this straight to bg_crop_bufs.
	res_x, res_y [WGPU_FROST_DOWNSAMPLE_STEPS]uint32
}

//go:embed shaders/frost/kawase_down.wgsl
var kawase_down_shader_src string

func (b *WgpuBackend) create_frost_pipelines() error {
	f := &b.frost
	var err error

	// Create first Kawase downsampling pipeline.

	if f.kawase_down_first_pipeline, err = b.NewPipeline("Frost (Kawase downsampling)", kawase_down_shader_src,
		[]wgpu.BindGroupLayoutEntry{
			{ // Background texture.
				Binding:    0,
				Visibility: wgpu.ShaderStageFragment,
				Texture: wgpu.TextureBindingLayout{
					Multisampled:  false,
					ViewDimension: wgpu.TextureViewDimension2D,
					SampleType:    wgpu.TextureSampleTypeFloat,
				},
			},
			{ // Background texture sampler.
				Binding:    1,
				Visibility: wgpu.ShaderStageFragment,
				Sampler: wgpu.SamplerBindingLayout{
					Type: wgpu.SamplerBindingTypeFiltering,
				},
			},
			{ // Background sampling position and size (crop).
				Binding:    2,
				Visibility: wgpu.ShaderStageFragment,
				Buffer: wgpu.BufferBindingLayout{
					Type: wgpu.BufferBindingTypeUniform,
				},
			},
		},
		[]wgpu.VertexBufferLayout{},
		&wgpu.BlendStatePremultipliedAlphaBlending,
	); err != nil {
		return err
	}

	return nil
}

func (d *WgpuBackendDivData) create_frost(b *WgpuBackend, w, h uint32) error {
	f := &d.frost
	var err error

	// Prepare render buffers.

	cur_w, cur_h := w, h

	for i := range WGPU_FROST_DOWNSAMPLE_STEPS { // XXX This number looks good for the radius we want, but this will have to be configurable in the future.
		cur_w /= 2
		cur_h /= 2

		if cur_w < 1 {
			cur_w = 1
		}
		if cur_h < 1 {
			cur_h = 1
		}

		f.res_x[i], f.res_y[i] = cur_w, cur_h

		if f.render_bufs[i], err = b.NewRenderBuf("Frost", cur_w, cur_h, b.format); err != nil {
			return err
		}

		if f.bg_crop_bufs[i], err = b.dev.CreateBuffer(&wgpu.BufferDescriptor{
			Size:  24,
			Usage: wgpu.BufferUsageUniform | wgpu.BufferUsageCopyDst,
		}); err != nil {
			println("Can't create background crop vector buffer.", err)
			return err
		}
	}

	return nil
}

func (b *WgpuBackend) encounter_frost(d *Div) {
	f := b.frost

	// We've encountered frost, end the current render pass.

	b.render_pass.End()
	b.render_pass.Release()

	// Then, swap the current and previous render buffers.

	tmp := b.render_buf
	b.render_buf = b.prev_render_buf
	b.prev_render_buf = tmp

	// Then, copy over the contents of the previous render buffer into the new one.

	b.cmd_enc.CopyTextureToTexture(b.prev_render_buf.tex.AsImageCopy(), b.render_buf.tex.AsImageCopy(), &wgpu.Extent3D{
		Width:              b.x_res,
		Height:             b.y_res,
		DepthOrArrayLayers: 1,
	})

	// Now, we move on to rendering the frost.
	// The first downsample buffer samples straight from prev_render_buf.

	data := d.backend_data.(WgpuBackendDivData)

	r := b.cmd_enc.BeginRenderPass(&wgpu.RenderPassDescriptor{
		Label: "First frost Kawase downsample render pass",
		ColorAttachments: []wgpu.RenderPassColorAttachment{
			{
				View:    data.frost.render_bufs[0].view,
				LoadOp:  wgpu.LoadOpClear,
				StoreOp: wgpu.StoreOpStore,
			},
		},
	})

	bind_group, err := b.dev.CreateBindGroup(&wgpu.BindGroupDescriptor{
		Layout: f.kawase_down_first_pipeline.bind_group_layout,
		Entries: []wgpu.BindGroupEntry{
			{
				Binding:     0,
				TextureView: b.prev_render_buf.view,
			},
			{
				Binding: 1,
				Sampler: b.prev_render_buf.sampler,
			},
			{
				Binding: 2,
				Buffer:  data.frost.bg_crop_bufs[0],
				Size:    wgpu.WholeSize,
			},
		},
	})
	if err != nil {
		println(err)
	}

	bg_crop := [6]float32{
		1. / float32(b.x_res) * float32(d.flow_x),
		1. / float32(b.y_res) * float32(d.flow_y),
		2. / float32(b.x_res), 2. / float32(b.y_res),
		float32(b.x_res), float32(b.y_res),
	}

	b.queue.WriteBuffer(data.frost.bg_crop_bufs[0], 0, wgpu.ToBytes(bg_crop[:]))
	b.frost.kawase_down_first_pipeline.Set(r, bind_group)
	r.Draw(6, 1, 0, 0)

	r.End()
	r.Release()

	// Then, the next ones.

	for i := range WGPU_FROST_DOWNSAMPLE_STEPS - 1 {
		r = b.cmd_enc.BeginRenderPass(&wgpu.RenderPassDescriptor{
			Label: "Intermediate frost Kawase downsample render pass",
			ColorAttachments: []wgpu.RenderPassColorAttachment{
				{
					View:    data.frost.render_bufs[i+1].view,
					LoadOp:  wgpu.LoadOpClear,
					StoreOp: wgpu.StoreOpStore,
				},
			},
		})

		bind_group, err = b.dev.CreateBindGroup(&wgpu.BindGroupDescriptor{
			Layout: f.kawase_down_first_pipeline.bind_group_layout,
			Entries: []wgpu.BindGroupEntry{
				{
					Binding:     0,
					TextureView: data.frost.render_bufs[i].view,
				},
				{
					Binding: 1,
					Sampler: data.frost.render_bufs[i].sampler,
				},
				{
					Binding: 2,
					Buffer:  data.frost.bg_crop_bufs[i+1],
					Size:    wgpu.WholeSize,
				},
			},
		})
		if err != nil {
			println(err)
		}

		res_x, res_y := float32(data.frost.res_x[i]), float32(data.frost.res_y[i])
		bg_crop = [6]float32{0, 0, 2. / res_x, 2. / res_y, res_x, res_y}

		b.queue.WriteBuffer(data.frost.bg_crop_bufs[i+1], 0, wgpu.ToBytes(bg_crop[:]))
		b.frost.kawase_down_first_pipeline.Set(r, bind_group)
		r.Draw(6, 1, 0, 0)

		r.End()
		r.Release()
	}

	// Then, we do the upsample chain until the last one, where our existing frost shader can take over (and do all the alpha blending with the div texture etc etc).
	// So we will need 2 different shaders for the frost upsampling then; one intermediate and one, the existing one, final.
	// That final shader's bind group will have the second to last upsampling render buffer as its texture.
	// Although actually I wonder if we should have the last shader do the last upsampling step, because then I don't know if we can actually do refraction.
	// Anyways I think we need 3 or 4 shaders total for this. Whew!
	// See https://blog.frost.kiwi/dual-kawase/#dual-kawase-blur

	// Finally, now that our frost is ready, start a new render pass with our new render buffer for the rest of UI rendering.

	b.render_pass = b.cmd_enc.BeginRenderPass(&wgpu.RenderPassDescriptor{
		Label: "Intermediate render pass",
		ColorAttachments: []wgpu.RenderPassColorAttachment{
			{
				View:    b.render_buf.view,
				LoadOp:  wgpu.LoadOpLoad,
				StoreOp: wgpu.StoreOpStore,
			},
		},
	})
}
