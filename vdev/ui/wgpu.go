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

	x_res, y_res uint32

	title_font     *Font
	paragraph_font *Font

	regular_pipeline *TextPipeline
	solid_pipeline   *SolidPipeline
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

func (b *WgpuBackend) render(elem IElem, render_pass *wgpu.RenderPassEncoder) {
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
		b.regular_pipeline.Set(render_pass, data.bind_group)

		b.queue.WriteBuffer(data.mvp_buf, 0, wgpu.ToBytes(mvp[:]))

		data.model.draw(render_pass)
	case *Div:
		if e.backend_data == nil {
			b.gen_div_backend_data(e, e.flow_w, e.flow_h)
		} else {
			data := e.backend_data.(WgpuBackendDivData)
			data.model.gen_pane(b, float32(e.flow_w), float32(e.flow_h), 10)
		}

		data := e.backend_data.(WgpuBackendDivData)
		b.solid_pipeline.Set(render_pass, data.bind_group)

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

		b.queue.WriteBuffer(data.mvp_buf, 0, wgpu.ToBytes(mvp[:]))
		b.queue.WriteBuffer(data.colour_buf, 0, wgpu.ToBytes(colour[:]))

		data.model.draw(render_pass)

		// Render children.

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

	if x_res != ui.root.flow_w || y_res != ui.root.flow_h { // Make UI dirty on resize.
		ui.dirty = true
	}

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

	backend.x_res = x_res
	backend.y_res = y_res

	backend.render(&ui.root, render_pass)

	render_pass.End()
}
