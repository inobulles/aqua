// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package main

/*
#include <aqua/vdriver.h>
*/
import "C"

import (
	"fmt"
	"runtime/cgo"
	"unsafe"

	"obiw.ac/aqua/wgpu"
)

type WgpuBackend struct {
	Backend
	dev    wgpu.Device
	queue  *wgpu.Queue
	format wgpu.TextureFormat

	cmd_enc         wgpu.CommandEncoder
	render_buf      *WgpuTexture
	prev_render_buf *WgpuTexture
	render_pass     *wgpu.RenderPassEncoder

	x_res, y_res uint32

	title_font     *Font
	paragraph_font *Font

	regular_pipeline *TextPipeline
	solid_pipeline   *SolidPipeline
	texture_pipeline *TexturePipeline
	frost_pipeline   *FrostPipeline
}

type IWgpuBackendData interface {
	release()
}

func (b *WgpuBackend) free_elem(elem IElem) {
	switch e := elem.ElemBase().backend_data.(type) {
	case IWgpuBackendData:
		e.release()
	}

	elem.ElemBase().backend_data = nil
}

func (b *WgpuBackend) get_font(e *Text) *Font {
	switch e.semantic {
	case TextSemanticParagraph:
		return b.paragraph_font
	case TextSemanticTitle:
		return b.title_font
	default:
		panic(fmt.Sprintf("unexpected main.TextSemantic: %#v", e.semantic))
	}
}

func (b *WgpuBackend) encounter_frost() {
	// We've encountered frost, end the current render pass.

	b.render_pass.End()
	b.render_pass.Release()

	// Then, swap the current and previous render buffers.

	tmp := b.render_buf
	b.render_buf = b.prev_render_buf
	b.prev_render_buf = tmp

	// Finally, start a new render pass with our new render buffer.

	b.cmd_enc.CopyTextureToTexture(b.prev_render_buf.tex.AsImageCopy(), b.render_buf.tex.AsImageCopy(), &wgpu.Extent3D{
		Width:              b.x_res,
		Height:             b.y_res,
		DepthOrArrayLayers: 1,
	})

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

func (b *WgpuBackend) render(elem IElem) {
	// Create MVP matrix.

	base := elem.ElemBase()

	mvp := [4][4]float32{
		{2 / float32(b.x_res), 0, 0, 0},
		{0, 2 / float32(b.y_res), 0, 0},
		{0, 0, 1, 0},
		{
			-1 + 2*(float32(base.flow_x)+float32(base.flow_w)/2)/float32(b.x_res),
			1 - 2*(float32(base.flow_y)+float32(base.flow_h)/2)/float32(b.y_res),
			0, 1,
		},
	}

	if rot := elem.ElemBase().get_attr("rot"); rot != nil {
		mvp = mul_mat(mvp, rot_mat(rot.(float32)))
	}

	if scale := elem.ElemBase().get_attr("scale"); scale != nil {
		mvp = mul_mat(mvp, scale_mat(scale.(float32)))
	}

	// Element specific rendering.

	switch e := elem.(type) {
	case *Text:
		if e.backend_data == nil || e.flow_w != e.backend_data.(WgpuBackendTextData).flow_w {
			b.gen_text_backend_data(e)
		}

		data := e.backend_data.(WgpuBackendTextData)
		b.regular_pipeline.Set(b.render_pass, data.bind_group)

		b.queue.WriteBuffer(data.mvp_buf, 0, wgpu.ToBytes(mvp[:]))

		data.model.draw(b.render_pass)
	case *Div:
		if e.do_frost() {
			b.encounter_frost()

			if e.backend_data != nil {
				data := e.backend_data.(WgpuBackendDivData)
				data.release()
				e.backend_data = nil
			}
		}

		if e.backend_data == nil {
			b.gen_div_backend_data(e, e.flow_w, e.flow_h)
		} else {
			data := e.backend_data.(WgpuBackendDivData)
			data.model.gen_pane(b, float32(e.flow_w), float32(e.flow_h), 10)
		}

		data := e.backend_data.(WgpuBackendDivData)

		if data.tex == nil {
			b.solid_pipeline.Set(b.render_pass, data.bind_group)

			colour := [4]float32{0, 0, 0, 0}

			if r := elem.ElemBase().get_attr("bg.r"); r != nil {
				colour[0] = r.(float32)
			}
			if g := elem.ElemBase().get_attr("bg.g"); g != nil {
				colour[1] = g.(float32)
			}
			if b := elem.ElemBase().get_attr("bg.b"); b != nil {
				colour[2] = b.(float32)
			}
			if a := elem.ElemBase().get_attr("bg.a"); a != nil {
				colour[3] = a.(float32)
			}

			b.queue.WriteBuffer(data.colour_buf, 0, wgpu.ToBytes(colour[:]))
		} else if e.do_frost() {
			b.frost_pipeline.Set(b.render_pass, data.bind_group)
		} else {
			b.texture_pipeline.Set(b.render_pass, data.bind_group)
		}

		b.queue.WriteBuffer(data.mvp_buf, 0, wgpu.ToBytes(mvp[:]))

		data.model.draw(b.render_pass)

		// Render children.

		for _, child := range e.children {
			// Recompute position if absolutely positioned.
			// We need to do this on each frame, because changing the position of an absolutely positioned won't necessarily trigger a reflow (nor would we want it to, for performance reasons).

			if child.ElemBase().is_abs {
				abs := child.ElemBase().abs

				child.ElemBase().flow_x = e.dimension_to_px_x(abs.x) -
					int32(abs.anchor_x*float32(child.ElemBase().flow_w))
				child.ElemBase().flow_y = e.dimension_to_px_y(abs.y) -
					int32(abs.anchor_y*float32(child.ElemBase().flow_h))
			}

			// Actually render child.

			b.render(child)
		}
	default:
		panic("Unknown element kind.")
	}
}

func (b *WgpuBackend) calculate_size(elem IElem, max_w, max_h uint32) (w, h uint32) {
	switch e := elem.(type) {
	case *Text:
		return b.get_font(e).Measure(e.text, int(max_w))
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

	wgpu.SetGlobalCtx(unsafe.Pointer(uintptr(cid)))
	dev := wgpu.CreateDeviceFromRaw(dev_raw)

	backend := &WgpuBackend{
		dev:    dev,
		queue:  dev.GetQueue(),
		format: wgpu.TextureFormat(format),
	}

	var err error

	if backend.title_font, err = NewFontFromFile("/home/obiwac/.local/share/fonts/Montserrat/Montserrat-Black.ttf", 70); err != nil {
		println("Failed to load title font.")
		return
	}

	if backend.paragraph_font, err = NewFontFromFile("/home/obiwac/.local/share/fonts/Montserrat/Montserrat-Regular.ttf", 20); err != nil {
		println("Failed to load title font.")
		return
	}

	if backend.regular_pipeline, err = backend.NewTextPipeline(); err != nil {
		println("Failed to create regular pipeline.")
		return
	}

	if backend.solid_pipeline, err = backend.NewSolidPipeline(); err != nil {
		println("Failed to create solid pipeline.")
		return
	}

	if backend.texture_pipeline, err = backend.NewTexturePipeline(); err != nil {
		println("Failed to create texture pipeline.")
		return
	}

	if backend.frost_pipeline, err = backend.NewFrostPipeline(); err != nil {
		println("Failed to create frost pipeline.")
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
	b := ui.backend.(*WgpuBackend)

	// Make UI dirty on resize.
	// Also, recreate the render buffers.

	if x_res != ui.root.flow_w || y_res != ui.root.flow_h {
		ui.dirty = true

		b.x_res, b.y_res = x_res, y_res

		if b.prev_render_buf != nil {
			b.prev_render_buf.Release()
		}

		if b.render_buf != nil {
			b.render_buf.Release()
		}

		var err error

		if b.prev_render_buf, err = b.NewRenderBuf("Render buffer (A)", x_res, y_res, b.format); err != nil {
			panic(err)
		}

		if b.render_buf, err = b.NewRenderBuf("Render buffer (B)", x_res, y_res, b.format); err != nil {
			panic(err)
		}
	}

	// Do reflow if UI is dirty.

	if ui.dirty {
		ui.reflow(x_res, y_res)
		ui.dirty = false
	}

	// Create initial render pass.
	// All passes should use LoadOpLoad except for the first.

	b.cmd_enc = b.dev.CommandEncoderFromRaw(cmd_enc_raw)

	b.render_pass = b.cmd_enc.BeginRenderPass(&wgpu.RenderPassDescriptor{
		Label: "Initial render pass",
		ColorAttachments: []wgpu.RenderPassColorAttachment{
			{
				View:    b.render_buf.view,
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
	})

	// Actually render UI.

	b.render(&ui.root)

	// End last render pass.

	b.render_pass.End()
	b.render_pass.Release()

	// Write last render buffer texture to the final texture view.
	// This is kind of inefficient for the last render, because we are doing one extra copy for no reason.
	// I guess a solution would be to look forward to see if there are any other frost elements, and just use the swapchain buffer as the new render buffer if there are none left.
	// Also, we could fold frost elements together by checking if they are overlapping or not - if not, we render them all in the same render pass.

	final_tex := b.dev.TextureFromRaw(frame_raw)

	b.cmd_enc.CopyTextureToTexture(b.render_buf.tex.AsImageCopy(), final_tex.AsImageCopy(), &wgpu.Extent3D{
		Width:              x_res,
		Height:             y_res,
		DepthOrArrayLayers: 1,
	})
}
