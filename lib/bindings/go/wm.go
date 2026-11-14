// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package aqua

/*
#cgo LDFLAGS: -laqua_wm

#include <aqua/wm.h>

extern void go_lib_bindings_wm_redraw_cb(
	wm_t wm,
	void* raw_vk_image,
	uint32_t x_res,
	uint32_t y_res,
	void* data
);
extern void go_lib_bindings_wm_new_win_cb(wm_t wm, wm_win_t win, char* app_id, void* data);
extern void go_lib_bindings_wm_redraw_win_cb(
	wm_t wm,
	wm_win_t win,
	uint32_t x_res,
	uint32_t y_res,
	size_t surf_count,
	wm_surf_t* surfs,
	void* data
);
extern void go_lib_bindings_wm_destroy_image_cb(wm_t wm, void* raw_image, void* data);
extern void go_lib_bindings_wm_destroy_win_cb(wm_t wm, wm_win_t win, void* data);
extern void go_lib_bindings_wm_mouse_motion_cb(
	wm_t wm,
	uint32_t time,
	uint8_t is_abs,
	double dx,
	double dy,
	double x,
	double y,
	double unaccel_dx,
	double unaccel_dy,
	void* data
);
extern void go_lib_bindings_wm_mouse_button_cb(wm_t wm, uint32_t time, uint8_t pressed, uint32_t button, void* data);
*/
import "C"
import (
	"runtime/cgo"
	"unsafe"
)

type WmComponent struct {
	Component
}

type WmCtx struct {
	ctx C.wm_ctx_t
}

type Wm struct {
	ctx *WmCtx
	wm  C.wm_t

	redraw_cb_handle        cgo.Handle
	new_win_cb_handle       cgo.Handle
	redraw_win_cb_handle    cgo.Handle
	destroy_image_cb_handle cgo.Handle
	destroy_win_cb_handle   cgo.Handle
	mouse_motion_cb_handle  cgo.Handle
	mouse_button_cb_handle  cgo.Handle
}

type WmWin uint64

func (c *Context) WmInit() *WmComponent {
	comp := C.wm_init(c.internal)

	if comp == nil {
		return nil
	}

	return &WmComponent{
		Component{internal: comp},
	}
}

func (c *WmComponent) Conn(vdev *VdevDescr) *WmCtx {
	if vdev == nil {
		return nil
	}

	ctx := C.wm_conn(&vdev.internal)

	if ctx == nil {
		return nil
	}

	return &WmCtx{ctx}
}

func (c *WmCtx) Disconn() {
	C.wm_disconn(c.ctx)
}

func (c *WmCtx) Create() *Wm {
	wm := C.wm_create(c.ctx)

	if wm == nil {
		return nil
	}

	return &Wm{
		ctx: c,
		wm:  wm,
	}
}

func (w *Wm) Destroy() {
	C.wm_destroy(w.wm)

	if w.redraw_cb_handle != 0 {
		w.redraw_cb_handle.Delete()
	}
	if w.new_win_cb_handle != 0 {
		w.new_win_cb_handle.Delete()
	}
	if w.redraw_win_cb_handle != 0 {
		w.redraw_win_cb_handle.Delete()
	}
	if w.destroy_image_cb_handle != 0 {
		w.destroy_image_cb_handle.Delete()
	}
	if w.destroy_win_cb_handle != 0 {
		w.destroy_win_cb_handle.Delete()
	}
}

func (w *Wm) WinNotifyMouseMotion(win WmWin, time uint32, x, y uint32) {
	C.wm_win_notify_mouse_motion(w.wm, C.wm_win_t(win), C.uint32_t(time), C.uint32_t(x), C.uint32_t(y))
}

func (w *Wm) WinNotifyMouseButton(win WmWin, time uint32, pressed bool, button uint32) {
	C.wm_win_notify_mouse_button(w.wm, C.wm_win_t(win), C.uint32_t(time), C.bool(pressed), C.uint32_t(button))
}

type WmRedrawCb func(raw_vk_image unsafe.Pointer, x_res, y_res uint32)

//export go_lib_bindings_wm_redraw_cb
func go_lib_bindings_wm_redraw_cb(
	_ C.wm_t,
	raw_vk_image unsafe.Pointer,
	x_res, y_res C.uint32_t,
	data unsafe.Pointer,
) {
	handle := (cgo.Handle)(data)

	if cb, ok := handle.Value().(WmRedrawCb); ok {
		cb(raw_vk_image, uint32(x_res), uint32(y_res))
	}
}

func (w *Wm) RegisterRedrawCb(cb WmRedrawCb) {
	if w.redraw_cb_handle != 0 {
		w.redraw_cb_handle.Delete()
	}

	w.redraw_cb_handle = cgo.NewHandle(cb)

	C.wm_register_redraw_cb(
		w.wm,
		C.wm_redraw_cb_t(C.go_lib_bindings_wm_redraw_cb),
		unsafe.Pointer(w.redraw_cb_handle),
	)
}

type WmDestroyImageCb func(raw_image unsafe.Pointer)

//export go_lib_bindings_wm_destroy_image_cb
func go_lib_bindings_wm_destroy_image_cb(
	_ C.wm_t,
	raw_image unsafe.Pointer,
	data unsafe.Pointer,
) {
	handle := (cgo.Handle)(data)

	if cb, ok := handle.Value().(WmDestroyImageCb); ok {
		cb(raw_image)
	}
}

func (w *Wm) RegisterDestroyImageCb(cb WmDestroyImageCb) {
	if w.destroy_image_cb_handle != 0 {
		w.destroy_image_cb_handle.Delete()
	}

	w.destroy_image_cb_handle = cgo.NewHandle(cb)

	C.wm_register_destroy_image_cb(
		w.wm,
		C.wm_destroy_image_cb_t(C.go_lib_bindings_wm_destroy_image_cb),
		unsafe.Pointer(w.destroy_image_cb_handle),
	)
}

type WmNewWinCb func(win WmWin, app_id string)

//export go_lib_bindings_wm_new_win_cb
func go_lib_bindings_wm_new_win_cb(
	_ C.wm_t,
	win C.wm_win_t,
	app_id *C.char,
	data unsafe.Pointer,
) {
	handle := (cgo.Handle)(data)

	if cb, ok := handle.Value().(WmNewWinCb); ok {
		cb(WmWin(win), C.GoString(app_id))
	}
}

func (w *Wm) RegisterNewWinCb(cb WmNewWinCb) {
	if w.new_win_cb_handle != 0 {
		w.new_win_cb_handle.Delete()
	}

	w.new_win_cb_handle = cgo.NewHandle(cb)

	C.wm_register_new_win_cb(
		w.wm,
		C.wm_new_win_cb_t(C.go_lib_bindings_wm_new_win_cb),
		unsafe.Pointer(w.new_win_cb_handle),
	)
}

type WmSurf struct {
	X, Y       int32
	XRes, YRes uint32
	RawImage   unsafe.Pointer
}

type WmRedrawWinCb func(win WmWin, x_res, y_res uint32, surfs []WmSurf)

//export go_lib_bindings_wm_redraw_win_cb
func go_lib_bindings_wm_redraw_win_cb(
	_ C.wm_t,
	win C.wm_win_t,
	x_res, y_res C.uint32_t,
	surf_count C.size_t,
	c_surfs *C.wm_surf_t,
	data unsafe.Pointer,
) {
	handle := (cgo.Handle)(data)

	cb, ok := handle.Value().(WmRedrawWinCb)

	if !ok {
		return
	}

	surfs := make([]WmSurf, surf_count)

	for i, c_surf := range unsafe.Slice(c_surfs, surf_count) {
		surfs[i] = WmSurf{
			X:        int32(c_surf.x),
			Y:        int32(c_surf.y),
			XRes:     uint32(c_surf.x_res),
			YRes:     uint32(c_surf.y_res),
			RawImage: c_surf.raw_image,
		}
	}

	cb(WmWin(win), uint32(x_res), uint32(y_res), surfs)
}

func (w *Wm) RegisterRedrawWinCb(cb WmRedrawWinCb) {
	if w.redraw_win_cb_handle != 0 {
		w.redraw_win_cb_handle.Delete()
	}

	w.redraw_win_cb_handle = cgo.NewHandle(cb)

	C.wm_register_redraw_win_cb(
		w.wm,
		C.wm_redraw_win_cb_t(C.go_lib_bindings_wm_redraw_win_cb),
		unsafe.Pointer(w.redraw_win_cb_handle),
	)
}

type WmDestroyWinCb func(win WmWin)

//export go_lib_bindings_wm_destroy_win_cb
func go_lib_bindings_wm_destroy_win_cb(
	_ C.wm_t,
	win C.wm_win_t,
	data unsafe.Pointer,
) {
	handle := (cgo.Handle)(data)

	if cb, ok := handle.Value().(WmDestroyWinCb); ok {
		cb(WmWin(win))
	}
}

func (w *Wm) RegisterDestroyWinCb(cb WmDestroyWinCb) {
	if w.destroy_win_cb_handle != 0 {
		w.destroy_win_cb_handle.Delete()
	}

	w.destroy_win_cb_handle = cgo.NewHandle(cb)

	C.wm_register_destroy_win_cb(
		w.wm,
		C.wm_destroy_win_cb_t(C.go_lib_bindings_wm_destroy_win_cb),
		unsafe.Pointer(w.destroy_win_cb_handle),
	)
}

type WmMouseMotionCb func(
	time uint32,
	isAbs bool,
	dx, dy float64,
	x, y float64,
	unaccelDx, unaccelDy float64,
)

//export go_lib_bindings_wm_mouse_motion_cb
func go_lib_bindings_wm_mouse_motion_cb(
	_ C.wm_t,
	time C.uint32_t,
	is_abs C.uint8_t,
	dx C.double,
	dy C.double,
	x C.double,
	y C.double,
	unaccel_dx C.double,
	unaccel_dy C.double,
	data unsafe.Pointer,
) {
	handle := (cgo.Handle)(data)

	if cb, ok := handle.Value().(WmMouseMotionCb); ok {
		cb(
			uint32(time),
			is_abs != 0,
			float64(dx),
			float64(dy),
			float64(x),
			float64(y),
			float64(unaccel_dx),
			float64(unaccel_dy),
		)
	}
}

func (w *Wm) RegisterMouseMotionCb(cb WmMouseMotionCb) {
	if w.mouse_motion_cb_handle != 0 {
		w.mouse_motion_cb_handle.Delete()
	}

	w.mouse_motion_cb_handle = cgo.NewHandle(cb)

	C.wm_register_mouse_motion_cb(
		w.wm,
		C.wm_mouse_motion_cb_t(C.go_lib_bindings_wm_mouse_motion_cb),
		unsafe.Pointer(w.mouse_motion_cb_handle),
	)
}

type WmMouseButtonCb func(
	time uint32,
	pressed bool,
	button uint32,
)

//export go_lib_bindings_wm_mouse_button_cb
func go_lib_bindings_wm_mouse_button_cb(
	_ C.wm_t,
	time C.uint32_t,
	pressed C.uint8_t,
	button C.uint32_t,
	data unsafe.Pointer,
) {
	handle := (cgo.Handle)(data)

	if cb, ok := handle.Value().(WmMouseButtonCb); ok {
		cb(uint32(time), pressed != 0, uint32(button))
	}
}

func (w *Wm) RegisterMouseButtonCb(cb WmMouseButtonCb) {
	if w.mouse_button_cb_handle != 0 {
		w.mouse_button_cb_handle.Delete()
	}

	w.mouse_button_cb_handle = cgo.NewHandle(cb)

	C.wm_register_mouse_button_cb(
		w.wm,
		C.wm_mouse_button_cb_t(C.go_lib_bindings_wm_mouse_button_cb),
		unsafe.Pointer(w.mouse_button_cb_handle),
	)
}

func (w *Wm) Loop() {
	C.wm_loop(w.wm)
}

func (w *Wm) GetInternalYesIKnowWhatImDoing() unsafe.Pointer {
	return unsafe.Pointer(w.wm)
}
