//go:build !js

package wgpu

/*

#include <stdlib.h>
#include <aqua/wgpu.h>
extern wgpu_ctx_t gowebgpu_ctx;

extern void gowebgpu_error_callback_c(enum WGPUPopErrorScopeStatus status, WGPUErrorType type, WGPUStringView message, void * userdata, void * userdata2);
extern void gowebgpu_buffer_map_callback_c(WGPUMapAsyncStatus status, void *userdata);

static inline void gowebgpu_buffer_map_async(WGPUBuffer buffer, WGPUMapMode mode, size_t offset, size_t size, WGPUBufferMapCallbackInfo callback, WGPUDevice device, void * error_userdata) {
	aqua_wgpuDevicePushErrorScope(gowebgpu_ctx, device, WGPUErrorFilter_Validation);
	aqua_wgpuBufferMapAsync(gowebgpu_ctx, buffer, mode, offset, size, callback);

	WGPUPopErrorScopeCallbackInfo const err_cb = {
		.callback = gowebgpu_error_callback_c,
		.userdata1 = error_userdata,
	};

	aqua_wgpuDevicePopErrorScope(gowebgpu_ctx, device, err_cb);
}

static inline void gowebgpu_buffer_unmap(WGPUBuffer buffer, WGPUDevice device, void * error_userdata) {
	aqua_wgpuDevicePushErrorScope(gowebgpu_ctx, device, WGPUErrorFilter_Validation);
	aqua_wgpuBufferUnmap(gowebgpu_ctx, buffer);

	WGPUPopErrorScopeCallbackInfo const err_cb = {
		.callback = gowebgpu_error_callback_c,
		.userdata1 = error_userdata,
	};

	aqua_wgpuDevicePopErrorScope(gowebgpu_ctx, device, err_cb);
}

static inline void gowebgpu_buffer_release(WGPUBuffer buffer, WGPUDevice device) {
	aqua_wgpuDeviceRelease(gowebgpu_ctx, device);
	aqua_wgpuBufferRelease(gowebgpu_ctx, buffer);
}

*/
import "C"
import (
	"errors"
	"runtime/cgo"
	"unsafe"
)

type Buffer struct {
	deviceRef C.WGPUDevice
	ref       C.WGPUBuffer
}

func (p *Buffer) Destroy() {
	C.aqua_wgpuBufferDestroy(global_ctx.ctx, p.ref)
}

func (p *Buffer) GetMappedRange(offset, size uint) []byte {
	buf := C.aqua_wgpuBufferGetMappedRange(global_ctx.ctx, p.ref, C.size_t(offset), C.size_t(size))
	return unsafe.Slice((*byte)(buf), size)
}

func (p *Buffer) GetSize() uint64 {
	return uint64(C.aqua_wgpuBufferGetSize(global_ctx.ctx, p.ref))
}

func (p *Buffer) GetUsage() BufferUsage {
	return BufferUsage(C.aqua_wgpuBufferGetUsage(global_ctx.ctx, p.ref))
}

//export gowebgpu_buffer_map_callback_go
func gowebgpu_buffer_map_callback_go(status C.WGPUMapAsyncStatus, userdata unsafe.Pointer) {
	handle := *(*cgo.Handle)(userdata)
	defer handle.Delete()

	cb, ok := handle.Value().(BufferMapCallback)
	if ok {
		cb(MapAsyncStatus(status))
	}
}

func (p *Buffer) MapAsync(mode MapMode, offset uint64, size uint64, callback BufferMapCallback) (err error) {
	callbackHandle := cgo.NewHandle(callback)

	var cb errorCallback = func(_ ErrorType, message string) {
		err = errors.New("wgpu.(*Buffer).MapAsync(): " + message)
	}
	errorCallbackHandle := cgo.NewHandle(cb)
	defer errorCallbackHandle.Delete()

	C.gowebgpu_buffer_map_async(
		p.ref,
		C.WGPUMapMode(mode),
		C.size_t(offset),
		C.size_t(size),
		C.WGPUBufferMapCallbackInfo{
			callback:  C.WGPUBufferMapCallback(C.gowebgpu_buffer_map_callback_c),
			userdata1: unsafe.Pointer(&callbackHandle),
		},
		p.deviceRef,
		unsafe.Pointer(&errorCallbackHandle),
	)
	return
}

func (p *Buffer) Unmap() (err error) {
	var cb errorCallback = func(_ ErrorType, message string) {
		err = errors.New("wgpu.(*Buffer).Unmap(): " + message)
	}
	errorCallbackHandle := cgo.NewHandle(cb)
	defer errorCallbackHandle.Delete()

	C.gowebgpu_buffer_unmap(
		p.ref,
		p.deviceRef,
		unsafe.Pointer(&errorCallbackHandle),
	)
	return
}

func (p *Buffer) Release() {
	C.gowebgpu_buffer_release(p.ref, p.deviceRef)
}
