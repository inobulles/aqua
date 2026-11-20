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
	uint32_t mods,
	void* data
);
extern void go_lib_bindings_wm_mouse_button_cb(wm_t wm, uint32_t time, uint8_t pressed, uint32_t button, uint32_t mods, void* data);
extern void go_lib_bindings_wm_key_cb(wm_t wm, uint32_t time, uint8_t pressed, uint32_t keycode, uint32_t keysym, uint32_t mods, void* data);
extern void go_lib_bindings_wm_mouse_axis_cb(
	wm_t wm,
	uint32_t time,
	wm_axis_source_t source,
	wm_axis_orientation_t orientation,
	wm_axis_direction_t direction,
	double delta,
	int32_t delta_discrete,
	uint32_t mods,
	void* data
);
extern void go_lib_bindings_wm_gesture_cb(
	wm_t wm,
	uint32_t time,
	wm_gesture_kind_t kind,
	wm_gesture_phase_t phase,
	uint32_t fingers,
	double dx,
	double dy,
	double scale,
	double rotation,
	uint8_t cancelled,
	uint32_t mods,
	void* data
);
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
	mouse_axis_cb_handle    cgo.Handle
	gesture_cb_handle       cgo.Handle
	key_cb_handle           cgo.Handle
}

type WmWin uint64

// Modifier key bitmask (see wm_mod_t).
type WmMod uint32

const (
	// MODS:BEGIN
	WM_MOD_SHIFT WmMod = C.WM_MOD_SHIFT
	WM_MOD_CAPS  WmMod = C.WM_MOD_CAPS
	WM_MOD_CTRL  WmMod = C.WM_MOD_CTRL
	WM_MOD_ALT   WmMod = C.WM_MOD_ALT
	WM_MOD_MOD2  WmMod = C.WM_MOD_MOD2
	WM_MOD_MOD3  WmMod = C.WM_MOD_MOD3
	WM_MOD_META  WmMod = C.WM_MOD_META
	WM_MOD_MOD5  WmMod = C.WM_MOD_MOD5
	// MODS:END
)

// Backend the WM is running on (see wm_backend_t).
type WmBackend uint32

const (
	// BACKENDS:BEGIN
	WM_BACKEND_DRM      WmBackend = C.WM_BACKEND_DRM
	WM_BACKEND_WAYLAND  WmBackend = C.WM_BACKEND_WAYLAND
	WM_BACKEND_X11      WmBackend = C.WM_BACKEND_X11
	WM_BACKEND_HEADLESS WmBackend = C.WM_BACKEND_HEADLESS
	// BACKENDS:END
	WM_BACKEND_UNKNOWN WmBackend = C.WM_BACKEND_UNKNOWN
)

// Source of a mouse axis (scroll) event (see wm_axis_source_t).
type WmAxisSource uint8

const (
	WM_AXIS_SOURCE_WHEEL      WmAxisSource = C.WM_AXIS_SOURCE_WHEEL
	WM_AXIS_SOURCE_FINGER     WmAxisSource = C.WM_AXIS_SOURCE_FINGER
	WM_AXIS_SOURCE_CONTINUOUS WmAxisSource = C.WM_AXIS_SOURCE_CONTINUOUS
	WM_AXIS_SOURCE_WHEEL_TILT WmAxisSource = C.WM_AXIS_SOURCE_WHEEL_TILT
)

// Orientation of a mouse axis (scroll) event (see wm_axis_orientation_t).
type WmAxisOrientation uint8

const (
	WM_AXIS_ORIENTATION_VERTICAL   WmAxisOrientation = C.WM_AXIS_ORIENTATION_VERTICAL
	WM_AXIS_ORIENTATION_HORIZONTAL WmAxisOrientation = C.WM_AXIS_ORIENTATION_HORIZONTAL
)

// Relative direction of a mouse axis (scroll) event (see wm_axis_direction_t).
type WmAxisDirection uint8

const (
	WM_AXIS_DIRECTION_IDENTICAL WmAxisDirection = C.WM_AXIS_DIRECTION_IDENTICAL
	WM_AXIS_DIRECTION_INVERTED  WmAxisDirection = C.WM_AXIS_DIRECTION_INVERTED
)

// Kind of touchpad gesture (see wm_gesture_kind_t).
type WmGestureKind uint8

const (
	WM_GESTURE_SWIPE WmGestureKind = C.WM_GESTURE_SWIPE
	WM_GESTURE_PINCH WmGestureKind = C.WM_GESTURE_PINCH
	WM_GESTURE_HOLD  WmGestureKind = C.WM_GESTURE_HOLD
)

// Phase of a touchpad gesture (see wm_gesture_phase_t).
type WmGesturePhase uint8

const (
	WM_GESTURE_PHASE_BEGIN  WmGesturePhase = C.WM_GESTURE_PHASE_BEGIN
	WM_GESTURE_PHASE_UPDATE WmGesturePhase = C.WM_GESTURE_PHASE_UPDATE
	WM_GESTURE_PHASE_END    WmGesturePhase = C.WM_GESTURE_PHASE_END
)

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
	if w.mouse_motion_cb_handle != 0 {
		w.mouse_motion_cb_handle.Delete()
	}
	if w.mouse_button_cb_handle != 0 {
		w.mouse_button_cb_handle.Delete()
	}
	if w.mouse_axis_cb_handle != 0 {
		w.mouse_axis_cb_handle.Delete()
	}
	if w.gesture_cb_handle != 0 {
		w.gesture_cb_handle.Delete()
	}
	if w.key_cb_handle != 0 {
		w.key_cb_handle.Delete()
	}
}

func (w *Wm) WinNotifyMouseMotion(win WmWin, time uint32, x, y uint32) {
	C.wm_win_notify_mouse_motion(w.wm, C.wm_win_t(win), C.uint32_t(time), C.uint32_t(x), C.uint32_t(y))
}

func (w *Wm) WinNotifyMouseButton(win WmWin, time uint32, pressed bool, button uint32) {
	C.wm_win_notify_mouse_button(w.wm, C.wm_win_t(win), C.uint32_t(time), C.bool(pressed), C.uint32_t(button))
}

func (w *Wm) WinNotifyKey(win WmWin, time uint32, pressed bool, keycode uint32) {
	C.wm_win_notify_key(w.wm, C.wm_win_t(win), C.uint32_t(time), C.bool(pressed), C.uint32_t(keycode))
}

func (w *Wm) WinNotifyMouseAxis(
	win WmWin,
	time uint32,
	source WmAxisSource,
	orientation WmAxisOrientation,
	direction WmAxisDirection,
	delta float64,
	deltaDiscrete int32,
) {
	C.wm_win_notify_mouse_axis(
		w.wm,
		C.wm_win_t(win),
		C.uint32_t(time),
		C.wm_axis_source_t(source),
		C.wm_axis_orientation_t(orientation),
		C.wm_axis_direction_t(direction),
		C.double(delta),
		C.int32_t(deltaDiscrete),
	)
}

func (w *Wm) WinNotifyGesture(
	win WmWin,
	time uint32,
	kind WmGestureKind,
	phase WmGesturePhase,
	fingers uint32,
	dx, dy float64,
	scale, rotation float64,
	cancelled bool,
) {
	C.wm_win_notify_gesture(
		w.wm,
		C.wm_win_t(win),
		C.uint32_t(time),
		C.wm_gesture_kind_t(kind),
		C.wm_gesture_phase_t(phase),
		C.uint32_t(fingers),
		C.double(dx),
		C.double(dy),
		C.double(scale),
		C.double(rotation),
		C.bool(cancelled),
	)
}

func (w *Wm) WinSetSize(win WmWin, x_res, y_res uint32) {
	C.wm_win_set_size(w.wm, C.wm_win_t(win), C.uint32_t(x_res), C.uint32_t(y_res))
}

func (w *Wm) WinSetResizing(win WmWin, resizing bool) {
	C.wm_win_set_resizing(w.wm, C.wm_win_t(win), C.bool(resizing))
}

func (w *Wm) GetBackend() WmBackend {
	return WmBackend(C.wm_get_backend(w.wm))
}

func (w *Wm) ScheduleFrame() {
	C.wm_schedule_frame(w.wm)
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

	// WebGPU texture format (wgpu.TextureFormat) the image should be imported as.
	Format uint32

	// Whether the image's alpha channel should be ignored (e.g. for XRGB8888).
	Opaque bool
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
			Format:   uint32(c_surf.format),
			Opaque:   bool(c_surf.opaque),
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
	mods WmMod,
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
	mods C.uint32_t,
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
			WmMod(mods),
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
	mods WmMod,
)

//export go_lib_bindings_wm_mouse_button_cb
func go_lib_bindings_wm_mouse_button_cb(
	_ C.wm_t,
	time C.uint32_t,
	pressed C.uint8_t,
	button C.uint32_t,
	mods C.uint32_t,
	data unsafe.Pointer,
) {
	handle := (cgo.Handle)(data)

	if cb, ok := handle.Value().(WmMouseButtonCb); ok {
		cb(uint32(time), pressed != 0, uint32(button), WmMod(mods))
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

type WmMouseAxisCb func(
	time uint32,
	source WmAxisSource,
	orientation WmAxisOrientation,
	direction WmAxisDirection,
	delta float64,
	deltaDiscrete int32,
	mods WmMod,
)

//export go_lib_bindings_wm_mouse_axis_cb
func go_lib_bindings_wm_mouse_axis_cb(
	_ C.wm_t,
	time C.uint32_t,
	source C.wm_axis_source_t,
	orientation C.wm_axis_orientation_t,
	direction C.wm_axis_direction_t,
	delta C.double,
	delta_discrete C.int32_t,
	mods C.uint32_t,
	data unsafe.Pointer,
) {
	handle := (cgo.Handle)(data)

	if cb, ok := handle.Value().(WmMouseAxisCb); ok {
		cb(
			uint32(time),
			WmAxisSource(source),
			WmAxisOrientation(orientation),
			WmAxisDirection(direction),
			float64(delta),
			int32(delta_discrete),
			WmMod(mods),
		)
	}
}

func (w *Wm) RegisterMouseAxisCb(cb WmMouseAxisCb) {
	if w.mouse_axis_cb_handle != 0 {
		w.mouse_axis_cb_handle.Delete()
	}

	w.mouse_axis_cb_handle = cgo.NewHandle(cb)

	C.wm_register_mouse_axis_cb(
		w.wm,
		C.wm_mouse_axis_cb_t(C.go_lib_bindings_wm_mouse_axis_cb),
		unsafe.Pointer(w.mouse_axis_cb_handle),
	)
}

type WmGestureCb func(
	time uint32,
	kind WmGestureKind,
	phase WmGesturePhase,
	fingers uint32,
	dx, dy float64,
	scale, rotation float64,
	cancelled bool,
	mods WmMod,
)

//export go_lib_bindings_wm_gesture_cb
func go_lib_bindings_wm_gesture_cb(
	_ C.wm_t,
	time C.uint32_t,
	kind C.wm_gesture_kind_t,
	phase C.wm_gesture_phase_t,
	fingers C.uint32_t,
	dx C.double,
	dy C.double,
	scale C.double,
	rotation C.double,
	cancelled C.uint8_t,
	mods C.uint32_t,
	data unsafe.Pointer,
) {
	handle := (cgo.Handle)(data)

	if cb, ok := handle.Value().(WmGestureCb); ok {
		cb(
			uint32(time),
			WmGestureKind(kind),
			WmGesturePhase(phase),
			uint32(fingers),
			float64(dx),
			float64(dy),
			float64(scale),
			float64(rotation),
			cancelled != 0,
			WmMod(mods),
		)
	}
}

func (w *Wm) RegisterGestureCb(cb WmGestureCb) {
	if w.gesture_cb_handle != 0 {
		w.gesture_cb_handle.Delete()
	}

	w.gesture_cb_handle = cgo.NewHandle(cb)

	C.wm_register_gesture_cb(
		w.wm,
		C.wm_gesture_cb_t(C.go_lib_bindings_wm_gesture_cb),
		unsafe.Pointer(w.gesture_cb_handle),
	)
}

// keysym is an XKB keysym (see XKB_KEY_* in <aqua/xkbcommon-keysyms.h> (C header)), or 0 if the key doesn't resolve to a single keysym.
type WmKeyCb func(
	time uint32,
	pressed bool,
	keycode uint32,
	keysym uint32,
	mods WmMod,
)

//export go_lib_bindings_wm_key_cb
func go_lib_bindings_wm_key_cb(
	_ C.wm_t,
	time C.uint32_t,
	pressed C.uint8_t,
	keycode C.uint32_t,
	keysym C.uint32_t,
	mods C.uint32_t,
	data unsafe.Pointer,
) {
	handle := (cgo.Handle)(data)

	if cb, ok := handle.Value().(WmKeyCb); ok {
		cb(uint32(time), pressed != 0, uint32(keycode), uint32(keysym), WmMod(mods))
	}
}

func (w *Wm) RegisterKeyCb(cb WmKeyCb) {
	if w.key_cb_handle != 0 {
		w.key_cb_handle.Delete()
	}

	w.key_cb_handle = cgo.NewHandle(cb)

	C.wm_register_key_cb(
		w.wm,
		C.wm_key_cb_t(C.go_lib_bindings_wm_key_cb),
		unsafe.Pointer(w.key_cb_handle),
	)
}

func (w *Wm) Loop() {
	C.wm_loop(w.wm)
}

func (w *Wm) GetInternalYesIKnowWhatImDoing() unsafe.Pointer {
	return unsafe.Pointer(w.wm)
}
