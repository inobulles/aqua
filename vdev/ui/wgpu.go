// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package main

/*
#include "../../kos/lib/vdriver.h"
*/
import "C"

import (
	"fmt"
	"image"
	"runtime/cgo"
	"unsafe"

	"obiw.ac/aqua/wgpu"
)

type WgpuBackend struct {
	Backend
	dev    wgpu.Device
	format wgpu.TextureFormat

	title_font *Font

	regular_pipeline *RegularPipeline
}

type WgpuBackendTextData struct {
	data *image.RGBA

	tex     *wgpu.Texture
	view    *wgpu.TextureView
	sampler *wgpu.Sampler

	mvp_buf    *wgpu.Buffer
	bind_group *wgpu.BindGroup

	// TODO Probably can be generically reused in a quad kinda thing.

	vbo         *wgpu.Buffer
	ibo         *wgpu.Buffer
	index_count uint32
}

type Vertex struct {
	X, Y float32
	U, V float32
}

func QuadVertices(w, h float32) []Vertex {
	w = 1
	h = 1

	return []Vertex{
		{0, 0, 0.0, 0.0},
		{w, 0, 1.0, 0.0},
		{w, h, 1.0, 1.0},
		{0, h, 0.0, 1.0},
	}
}

func QuadIndices() []uint16 {
	return []uint16{
		0, 1, 2,
		2, 3, 0,
	}
}

func (b *WgpuBackend) free_elem(elem IElem) {
	switch e := elem.(type) {
	case *Text:
		data := e.backend_data.(WgpuBackendTextData)

		if data.tex != nil {
			data.tex.Release()
		}
		if data.view != nil {
			data.view.Release()
		}
		if data.sampler != nil {
			data.sampler.Release()
		}
		if data.mvp_buf != nil {
			data.mvp_buf.Release()
		}
		if data.bind_group != nil {
			data.bind_group.Release()
		}
		if data.vbo != nil {
			data.vbo.Release()
		}
		if data.ibo != nil {
			data.ibo.Release()
		}

		e.backend_data = nil
	}
}

func (b *WgpuBackend) generate_text(e *Text) {
	// TODO Split into multiple functions when I make quad thing made only once for backend, because cleaning when error is kinda disgusting currently.

	// If we already have backend data, free everything.

	if e.backend_data != nil {
		b.free_elem(e)
	}

	// Generate new text.

	data := WgpuBackendTextData{
		data: b.title_font.Render(e.text, int(e.ElemBase().flow_w)),
	}

	w := uint32(data.data.Bounds().Dx())
	h := uint32(data.data.Bounds().Dy())

	tex_size := wgpu.Extent3D{
		Width:              w,
		Height:             h,
		DepthOrArrayLayers: 1,
	}

	var err error

	if data.tex, err = b.dev.CreateTexture(&wgpu.TextureDescriptor{
		Label:         fmt.Sprintf("Texture (%s)", e.text),
		Size:          tex_size,
		MipLevelCount: 1,
		SampleCount:   1,
		Dimension:     wgpu.TextureDimension2D,
		Format:        wgpu.TextureFormatRGBA8Unorm,
		Usage:         wgpu.TextureUsageTextureBinding | wgpu.TextureUsageCopyDst,
	}); err != nil {
		println("Can't create texture.")
		b.free_elem(e)
		return
	}

	if err = b.dev.GetQueue().WriteTexture(
		data.tex.AsImageCopy(),
		data.data.Pix,
		&wgpu.TexelCopyBufferLayout{
			Offset:       0,
			BytesPerRow:  4 * w,
			RowsPerImage: h,
		},
		&tex_size,
	); err != nil {
		println("Can't write texture.")
		b.free_elem(e)
		return
	}

	if data.view, err = data.tex.CreateView(nil); err != nil {
		println("Can't create texture view.")
		b.free_elem(e)
		return
	}

	if data.sampler, err = b.dev.CreateSampler(&wgpu.SamplerDescriptor{
		AddressModeU:  wgpu.AddressModeClampToEdge,
		AddressModeV:  wgpu.AddressModeClampToEdge,
		AddressModeW:  wgpu.AddressModeClampToEdge,
		MagFilter:     wgpu.FilterModeLinear,
		MinFilter:     wgpu.FilterModeLinear,
		MipmapFilter:  wgpu.MipmapFilterModeLinear,
		MaxAnisotropy: 1,
	}); err != nil {
		println("Can't create sampler.")
		b.free_elem(e)
		return
	}

	// Create MVP matrix buffer.

	if data.mvp_buf, err = b.dev.CreateBuffer(&wgpu.BufferDescriptor{
		Size:  64,
		Usage: wgpu.BufferUsageUniform | wgpu.BufferUsageCopyDst,
	}); err != nil {
		println("Can't create MVP matrix buffer.")
		b.free_elem(e)
		return
	}

	// Bind group shit.

	if data.bind_group, err = b.dev.CreateBindGroup(&wgpu.BindGroupDescriptor{
		Layout: b.regular_pipeline.bind_group_layout,
		Entries: []wgpu.BindGroupEntry{
			{
				Binding: 0,
				Buffer:  data.mvp_buf,
				Size:    wgpu.WholeSize,
			},
			{
				Binding:     1,
				TextureView: data.view,
			},
			{
				Binding: 2,
				Sampler: data.sampler,
			},
		},
	}); err != nil {
		println("Can't create bind group.")
		b.free_elem(e)
		return
	}

	// Generate VBO/IBO.
	// TODO This should probably be done only once for the WHOLE backend, not once for every text object.

	if data.vbo, err = b.dev.CreateBufferInit(&wgpu.BufferInitDescriptor{
		Label:    fmt.Sprintf("VBO (%s)", e.text),
		Contents: wgpu.ToBytes(QuadVertices(float32(w), float32(h))),
		Usage:    wgpu.BufferUsageVertex,
	}); err != nil {
		println("Can't create VBO.")
		b.free_elem(e)
		return
	}

	if data.ibo, err = b.dev.CreateBufferInit(&wgpu.BufferInitDescriptor{
		Label:    fmt.Sprintf("IBO (%s)", e.text),
		Contents: wgpu.ToBytes(QuadIndices()),
		Usage:    wgpu.BufferUsageIndex,
	}); err != nil {
		println("Can't create IBO.")
		b.free_elem(e)
		return
	}

	e.backend_data = data
}

func (b *WgpuBackend) render(elem IElem, render_pass *wgpu.RenderPassEncoder) {
	switch e := elem.(type) {
	case *Text:
		if e.backend_data == nil {
			b.generate_text(e)
		}

		data := e.backend_data.(WgpuBackendTextData)
		b.regular_pipeline.Set(render_pass, data.bind_group)

		render_pass.SetVertexBuffer(0, data.vbo, 0, wgpu.WholeSize)
		render_pass.SetIndexBuffer(data.ibo, wgpu.IndexFormatUint16, 0, wgpu.WholeSize)
		render_pass.DrawIndexed(6, 1, 0, 0, 0)
	case *Div:
		for _, child := range e.children {
			b.render(child, render_pass)
		}
	default:
		panic("Unknown element kind.")
	}
}

func (b *WgpuBackend) calculate_size(elem IElem, max_w, max_h uint32) (w, h uint32) {
	switch e := elem.(type) {
	case *Text:
		return b.title_font.Measure(e.text, int(max_w))
	case *Div:
		panic("Shouldn't be asking backend to calculate size of Div.")
	default:
		panic("Unknown element kind.")
	}
}

//export GoUiBackendWgpuInit
func GoUiBackendWgpuInit(
	ui_raw C.uintptr_t,
	hid C.uint64_t,
	cid C.uint64_t,
	dev_raw unsafe.Pointer,
	format C.uint32_t,
) {
	ui := cgo.Handle(ui_raw).Value().(*Ui)

	title_font, err := NewFontFromFile("/home/obiwac/.local/share/fonts/Montserrat/Montserrat-Black.ttf", 70)

	if err != nil {
		println("Failed to load font.")
		return
	}

	wgpu.SetGlobalCtx(unsafe.Pointer(uintptr(cid)))

	backend := &WgpuBackend{
		dev:        wgpu.CreateDeviceFromRaw(dev_raw),
		format:     wgpu.TextureFormat(format),
		title_font: title_font,
	}

	if backend.regular_pipeline, err = backend.NewRegularPipeline(); err != nil {
		println("Failed to create regular pipeline.")
		return
	}

	ui.backend = backend

	// TODO Have to set global context somehow.
	// Oof, this is gonna be complicated actually.
	// Okay, so this is how I'm gonna approach this:
	// - First, we need the client's connection to be passed on to us so that we can use it.
	// - Or, we could still need to connect, but be able to "inherit" an existing connection somehow? But the connection ID entirely depends on the KOS and the VDEV we're trying to share doesn't know of this number.
	// - This is probably going to require quite a bit of reworking of stuff.
	// - I can use this opportunity to figure out how the .wgpu device's surface-from-window creation is going to work. We must reject creation on a window which is not on the same host. But even then being able to do this is not a guarantee cuz the devices could be in different processes. What's the solution to this? Do we use the hid:vid to figure out if the device is local or UDS to us?

	// - Should we figure out a way to do .wgpu and .win on different devices?
	// - This means we gotta render .wgpu on an offscreen buffer and then transfer it to .win -> not too bad a solution actually.
	// - Same solution for UDS devices.
}

//export GoUiBackendWgpuRender
func GoUiBackendWgpuRender(
	ui_raw C.uintptr_t,
	frame_raw unsafe.Pointer,
	cmd_enc_raw unsafe.Pointer,
	x_res, y_res uint32,
) {
	ui := cgo.Handle(ui_raw).Value().(*Ui)
	backend := ui.backend.(*WgpuBackend)

	if ui.dirty {
		ui.reflow(x_res, y_res)
		ui.dirty = false
	}

	cmd_enc := backend.dev.CommandEncoderFromRaw(cmd_enc_raw)
	frame := wgpu.TextureViewFromRaw(frame_raw)

	render_pass_descr := wgpu.RenderPassDescriptor{
		Label: "render_pass",
		ColorAttachments: []wgpu.RenderPassColorAttachment{
			{
				View:    &frame,
				LoadOp:  wgpu.LoadOpClear,
				StoreOp: wgpu.StoreOpStore,
				ClearValue: wgpu.Color{
					R: 0.0,
					G: 0.0,
					B: 0.0,
					A: 0.5,
				},
			},
		},
	}

	render_pass := cmd_enc.BeginRenderPass(&render_pass_descr)
	defer render_pass.Release()

	backend.render(&ui.root, render_pass)

	render_pass.End()
}
