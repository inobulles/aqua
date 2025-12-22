// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package aqua

/*
#cgo LDFLAGS: -laqua_ui

#include <aqua/ui.h>
#include <aqua/ui/wgpu.h>
*/
import "C"
import (
	"errors"
	"fmt"
	"unsafe"
)

type UiComponent struct {
	Component
}

type UiCtx struct {
	ctx C.ui_ctx_t
}

type Ui struct {
	ctx *UiCtx
	ui  C.ui_t
}

type UiElem struct {
	ui   *Ui
	elem C.ui_elem_t
}

type UiSupportedBackends int

const (
	UI_BACKEND_NONE UiSupportedBackends = C.UI_BACKEND_NONE
	UI_BACKEND_WGPU UiSupportedBackends = C.UI_BACKEND_WGPU
)

type UiDimUnits int

const (
	UI_DIM_UNITS_ZERO        UiDimUnits = C.UI_DIM_UNITS_ZERO
	UI_DIM_UNITS_PARENT_FRAC UiDimUnits = C.UI_DIM_UNITS_PARENT_FRAC
	UI_DIM_UNITS_PIXELS      UiDimUnits = C.UI_DIM_UNITS_PIXELS
)

type UiDim struct {
	kind UiDimUnits
	val  float32
}

func (UiDim) Zero() UiDim {
	return UiDim{
		kind: UI_DIM_UNITS_ZERO,
		val:  0,
	}
}

func (UiDim) Full() UiDim {
	return UiDim{
		kind: UI_DIM_UNITS_PARENT_FRAC,
		val:  1,
	}
}

func (UiDim) Pixels(pixels uint32) UiDim {
	return UiDim{
		kind: UI_DIM_UNITS_PIXELS,
		val:  float32(pixels),
	}
}

type UiRaster struct {
	XRes, YRes uint32
	Data       []byte
}

func (c *Context) UiInit() *UiComponent {
	comp := C.ui_init(c.internal)

	if comp == nil {
		return nil
	}

	return &UiComponent{
		Component{internal: comp},
	}
}

func (c *UiComponent) Conn(vdev *VdevDescr) *UiCtx {
	if vdev == nil {
		return nil
	}

	ctx := C.ui_conn(&vdev.internal)

	if ctx == nil {
		return nil
	}

	return &UiCtx{ctx}
}

func (c *UiCtx) GetSupportedBackends() UiSupportedBackends {
	return UiSupportedBackends(C.ui_get_supported_backends(c.ctx))
}

func (c *UiCtx) Create() *Ui {
	ui := C.ui_create(c.ctx)

	if ui == nil {
		return nil
	}

	return &Ui{
		ctx: c,
		ui:  ui,
	}
}

func (u *Ui) Destroy() {
	C.ui_destroy(u.ui)
}

func (u *Ui) GetRoot() *UiElem {
	return &UiElem{
		ui:   u,
		elem: C.ui_get_root(u.ui),
	}
}

func (e *UiElem) AddText(semantics, text string) *UiElem {
	c_semantics := C.CString(semantics)
	defer C.free(unsafe.Pointer(c_semantics))

	c_text := C.CString(text)
	defer C.free(unsafe.Pointer(c_text))

	return &UiElem{
		ui:   e.ui,
		elem: C.ui_add_text(e.elem, c_semantics, c_text),
	}
}

func (e *UiElem) AddDiv(semantics string) *UiElem {
	c_semantics := C.CString(semantics)
	defer C.free(unsafe.Pointer(c_semantics))

	return &UiElem{
		ui:   e.ui,
		elem: C.ui_add_div(e.elem, c_semantics),
	}
}

func (e *UiElem) SetAttr(key string, val any) {
	c_key := C.CString(key)
	defer C.free(unsafe.Pointer(c_key))

	switch v := val.(type) {
	case string:
		c_val := C.CString(v)
		defer C.free(unsafe.Pointer(c_val))

		C.ui_set_attr_str(e.elem, c_key, c_val)
	case float32:
		C.ui_set_attr_f32(e.elem, c_key, C.float(v))
	case uint32:
		C.ui_set_attr_u32(e.elem, c_key, C.uint32_t(v))
	case unsafe.Pointer:
		C.ui_set_attr_opaque_ptr(e.elem, c_key, v)
	case UiDim:
		C.ui_set_attr_dim(e.elem, c_key, C.ui_dim_t{
			units: C.ui_dim_units_t(v.kind),
			val:   C.float(v.val),
		})
	case UiRaster:
		if uint32(len(v.Data)) != v.XRes*v.YRes*4 {
			panic("wrong size for data")
		}

		C.ui_set_attr_raster(e.elem, c_key, C.ui_raster_t{
			x_res: C.uint32_t(v.XRes),
			y_res: C.uint32_t(v.YRes),
			data:  unsafe.Pointer(&v.Data[0]),
		})
	case float64:
		panic(fmt.Sprintf("unexpected value type for attribute: %T (did you mean to cast to float32 first?)", val))
	default:
		panic(fmt.Sprintf("unexpected value type for attribute: %T", val))
	}
}

// WebGPU backend stuff.

type UiWgpuEzState struct {
	internal   C.ui_wgpu_ez_state_t
	RawWgpuCtx unsafe.Pointer
}

func (u *Ui) WgpuEzSetup(win *Win, wgpu_ctx *WgpuCtx) (*UiWgpuEzState, error) {
	state := &UiWgpuEzState{}

	if C.ui_wgpu_ez_setup(&state.internal, u.ui, win.win, wgpu_ctx.ctx) != 0 {
		return nil, errors.New("ui_wgpu_ez_setup failed")
	}

	state.RawWgpuCtx = unsafe.Pointer(state.internal.wgpu_ctx)

	return state, nil
}

func (s *UiWgpuEzState) RawDevice() unsafe.Pointer {
	return unsafe.Pointer(s.internal.device)
}

func (s *UiWgpuEzState) Render() error {
	if C.ui_wgpu_ez_render(&s.internal) != 0 {
		return errors.New("ui_wgpu_ez_render failed")
	}

	return nil
}

func (s *UiWgpuEzState) Resize(x_res, y_res uint32) {
	C.ui_wgpu_ez_resize(&s.internal, C.uint32_t(x_res), C.uint32_t(y_res))
}
