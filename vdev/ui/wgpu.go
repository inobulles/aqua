// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025-2026 Aymeric Wibo

package main

/*
#include "glue.h"

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

	regular_pipeline        *TextPipeline
	solid_pipeline          *SolidPipeline
	frost_pipeline          *FrostPipeline
	texture_pipeline        *TexturePipeline
	texture_opaque_pipeline *TexturePipeline

	unit_quad *Model

	swapchain_view *wgpu.TextureView
	last_frost     *Div
	frost          WgpuBackendFrost
}

type IWgpuBackendData interface {
	release()
}

func (b *WgpuBackend) set_attr(elem *Elem, key string, val any) {
	switch key {
	// If we change the background texture view, the previous backend data is invalid and must be regenerated.
	case "bg.wgpu_tex_view":
		prev, _ := elem.get_attr(key).(unsafe.Pointer)
		cur, _ := val.(unsafe.Pointer)

		if prev != nil && prev != cur {
			b.free_backend_data(elem)
		}
	// Same thing if we enable/disable frost.
	case "frost":
		if prev, _ := elem.get_attr(key).(bool); prev != val.(bool) {
			b.free_backend_data(elem)
		}
	// Same thing if we enable/disable opacity (different pipeline for opaque or non-opaque textures).
	case "bg.opaque":
		if prev, _ := elem.get_attr(key).(bool); prev != val.(bool) {
			b.free_backend_data(elem)
		}
	}
}

func (b *WgpuBackend) free_elem(elem IElem) {
	b.free_backend_data(elem.ElemBase())
}

func (b *WgpuBackend) free_backend_data(elem *Elem) {
	switch e := elem.backend_data.(type) {
	case WgpuBackendDivData:
		e.release()
	case WgpuBackendTextData:
		e.release()
	}

	elem.backend_data = nil
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

func (b *WgpuBackend) render(elem IElem, parent [4][4]float32) {
	// Create MVP matrix.

	base := elem.ElemBase()

	// Centre of the element in transform space.
	// The element's vertices are in pixels relative to this.

	cx := float32(base.flow_x) + float32(base.flow_w)/2
	cy := -(float32(base.flow_y) + float32(base.flow_h)/2)

	// The element's own transform happens around its centre.

	local := translate_mat(cx, cy)

	if base.rot.pos != 0 {
		local = mul_mat(local, rot_mat(float32(base.rot.pos)))
	}

	if base.scale.pos != 1 {
		local = mul_mat(local, scale_mat(float32(base.scale.pos)))
	}

	local = mul_mat(local, translate_mat(-cx, -cy))

	// Combine with the parent's transform (this is what gets passed on to our children), and then place the vertices at the element's centre.

	world := mul_mat(parent, local)

	proj := [4][4]float32{
		{2 / float32(b.x_res), 0, 0, 0},
		{0, 2 / float32(b.y_res), 0, 0},
		{0, 0, 1, 0},
		{-1, 1, 0, 1},
	}

	mvp := mul_mat(proj, mul_mat(world, translate_mat(cx, cy)))

	// Element specific rendering.

	switch e := elem.(type) {
	case *Text:
		if e.backend_data == nil || e.flow_w != e.backend_data.(WgpuBackendTextData).flow_w {
			if err := b.gen_text_backend_data(e); err != nil {
				return
			}
		}

		data := e.backend_data.(WgpuBackendTextData)
		b.regular_pipeline.Set(b.render_pass, data.bind_group)

		text_mvp := mul_mat(mvp, scale_xy_mat(float32(data.w), float32(data.h)))
		b.queue.WriteBuffer(data.mvp_buf, 0, wgpu.ToBytes(text_mvp[:]))

		b.unit_quad.draw(b.render_pass)
	case *Div:
		// Only frost depends on the size of the div; everything else just scales the unit quad.

		if e.backend_data == nil || e.bg_tex_view_changed() || (e.do_frost() && e.size_changed()) {
			if err := b.gen_div_backend_data(e, e.flow_w, e.flow_h); err != nil {
				return
			}
		}

		data := e.backend_data.(WgpuBackendDivData)

		if data.tex == nil {
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
			b.encounter_frost(e)
		}

		div_mvp := mul_mat(mvp, scale_xy_mat(float32(e.flow_w), float32(e.flow_h)))

		data.pipeline.Set(b.render_pass, data.bind_group)
		b.queue.WriteBuffer(data.mvp_buf, 0, wgpu.ToBytes(div_mvp[:]))
		b.unit_quad.draw(b.render_pass)

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

			b.render(child, world)
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
		println("Failed to load title font.", err)
		return
	}

	if backend.paragraph_font, err = NewFontFromFile("/home/obiwac/.local/share/fonts/Montserrat/Montserrat-Regular.ttf", 20); err != nil {
		println("Failed to load title font.", err)
		return
	}

	if backend.regular_pipeline, err = backend.NewTextPipeline(); err != nil {
		println("Failed to create regular pipeline.", err)
		return
	}

	if backend.solid_pipeline, err = backend.NewSolidPipeline(); err != nil {
		println("Failed to create solid pipeline.", err)
		return
	}

	if backend.texture_pipeline, err = backend.NewTexturePipeline(TextureAlphaTranslucent); err != nil {
		println("Failed to create texture pipeline.", err)
		return
	}

	if backend.texture_opaque_pipeline, err = backend.NewTexturePipeline(TextureAlphaOpaque); err != nil {
		println("Failed to create opaque texture pipeline.", err)
		return
	}

	if backend.frost_pipeline, err = backend.NewFrostPipeline(); err != nil {
		println("Failed to create frost pipeline.", err)
		return
	}

	if err = backend.create_frost_pipelines(); err != nil {
		println("Failed to create frost pipelines.", err)
		return
	}

	backend.unit_quad = &Model{}

	if err = backend.unit_quad.gen_quad(backend, 1, 1); err != nil {
		println("Failed to create unit quad.", err)
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
	frame_raw unsafe.Pointer, // XXX This cannot have wgpu.TextureUsageCopyDst on it!
	cmd_enc_raw unsafe.Pointer,
	x_res, y_res uint32,
) C.render_flags_t {
	ui := cgo.Handle(ui_raw).Value().(*Ui)
	b := ui.backend.(*WgpuBackend)

	// While we're rendering, we might do operations which ordinarily ask for new frames.
	// Obivously don't ask for any new frames while we're rendering.

	ui.idle = false

	// Make UI dirty on resize.
	// Also, recreate the render buffers.

	if x_res != ui.root.flow_w || y_res != ui.root.flow_h {
		ui.invalidate_layout()

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

	// Advance animations.

	needs_frame := ui.animate()

	// Do reflow if layout is dirty.

	if ui.layout_dirty {
		ui.reflow(x_res, y_res)
		ui.layout_dirty = false
	}

	// Get view for our frame (swapchain texture).

	frame_tex := b.dev.TextureFromRaw(frame_raw)
	frame_view, err := frame_tex.CreateView(nil)
	if err != nil {
		panic(err)
	}
	defer frame_view.Release()

	// Find last frost element.

	b.swapchain_view = frame_view
	b.last_frost = b.find_last_frost(&ui.root)

	// Create initial render pass.
	// All passes should use LoadOpLoad except for the first.
	// If there are no frost elements, we can render to the swapchain texture directly.

	initial_view := frame_view
	if b.last_frost != nil {
		initial_view = b.render_buf.view
	}

	b.cmd_enc = b.dev.CommandEncoderFromRaw(cmd_enc_raw)

	b.render_pass = b.cmd_enc.BeginRenderPass(&wgpu.RenderPassDescriptor{
		Label: "Initial render pass",
		ColorAttachments: []wgpu.RenderPassColorAttachment{
			{
				View:       initial_view,
				LoadOp:     wgpu.LoadOpClear,
				StoreOp:    wgpu.StoreOpStore,
				ClearValue: wgpu.Color{R: 0.0, G: 0.0, B: 0.0, A: 0.5},
			},
		},
	})

	// Actually render UI.

	b.render(&ui.root, identity_mat())

	// End last render pass.

	b.render_pass.End()
	b.render_pass.Release()

	var flags C.render_flags_t

	if needs_frame {
		flags |= C.RENDER_FLAG_NEEDS_FRAME
	}

	ui.idle = !needs_frame

	return flags
}
