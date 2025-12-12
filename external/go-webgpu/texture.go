//go:build !js

package wgpu

/*

#include <stdlib.h>
#include <aqua/wgpu.h>
extern wgpu_ctx_t gowebgpu_ctx;

extern void gowebgpu_error_callback_c(enum WGPUPopErrorScopeStatus status, WGPUErrorType type, WGPUStringView message, void * userdata, void * userdata2);

static inline WGPUTextureView gowebgpu_texture_create_view(WGPUTexture texture, WGPUTextureViewDescriptor const * descriptor, WGPUDevice device, void * error_userdata) {
	WGPUTextureView ref = NULL;
	aqua_wgpuDevicePushErrorScope(gowebgpu_ctx, device, WGPUErrorFilter_Validation);
	ref = aqua_wgpuTextureCreateView(gowebgpu_ctx, texture, descriptor);

	WGPUPopErrorScopeCallbackInfo const err_cb = {
		.callback = gowebgpu_error_callback_c,
		.userdata1 = error_userdata,
	};

	aqua_wgpuDevicePopErrorScope(gowebgpu_ctx, device, err_cb);

	return ref;
}

static inline void gowebgpu_texture_release(WGPUTexture texture, WGPUDevice device) {
	aqua_wgpuDeviceRelease(gowebgpu_ctx, device);
	aqua_wgpuTextureRelease(gowebgpu_ctx, texture);
}

*/
import "C"
import (
	"errors"
	"runtime/cgo"
	"unsafe"
)

type Texture struct {
	deviceRef C.WGPUDevice
	ref       C.WGPUTexture
}

func (p *Texture) CreateView(descriptor *TextureViewDescriptor) (*TextureView, error) {
	var desc *C.WGPUTextureViewDescriptor

	if descriptor != nil {
		desc = &C.WGPUTextureViewDescriptor{
			format:          C.WGPUTextureFormat(descriptor.Format),
			dimension:       C.WGPUTextureViewDimension(descriptor.Dimension),
			baseMipLevel:    C.uint32_t(descriptor.BaseMipLevel),
			mipLevelCount:   C.uint32_t(descriptor.MipLevelCount),
			baseArrayLayer:  C.uint32_t(descriptor.BaseArrayLayer),
			arrayLayerCount: C.uint32_t(descriptor.ArrayLayerCount),
			aspect:          C.WGPUTextureAspect(descriptor.Aspect),
		}

		if descriptor.Label != "" {
			label := C.CString(descriptor.Label)
			defer C.free(unsafe.Pointer(label))

			desc.label.data = label
			desc.label.length = C.WGPU_STRLEN
		}
	}

	var err error = nil
	var cb errorCallback = func(_ ErrorType, message string) {
		err = errors.New("wgpu.(*Texture).CreateView(): " + message)
	}
	errorCallbackHandle := cgo.NewHandle(cb)
	defer errorCallbackHandle.Delete()

	ref := C.gowebgpu_texture_create_view(
		p.ref,
		desc,
		p.deviceRef,
		unsafe.Pointer(&errorCallbackHandle),
	)
	if err != nil {
		C.aqua_wgpuTextureViewRelease(global_ctx.ctx, ref)
		return nil, err
	}

	return &TextureView{ref}, nil
}

func (p *Texture) Destroy() {
	C.aqua_wgpuTextureDestroy(global_ctx.ctx, p.ref)
}

func (p *Texture) GetDepthOrArrayLayers() uint32 {
	return uint32(C.aqua_wgpuTextureGetDepthOrArrayLayers(global_ctx.ctx, p.ref))
}

func (p *Texture) GetDimension() TextureDimension {
	return TextureDimension(C.aqua_wgpuTextureGetDimension(global_ctx.ctx, p.ref))
}

func (p *Texture) GetFormat() TextureFormat {
	return TextureFormat(C.aqua_wgpuTextureGetFormat(global_ctx.ctx, p.ref))
}

func (p *Texture) GetHeight() uint32 {
	return uint32(C.aqua_wgpuTextureGetHeight(global_ctx.ctx, p.ref))
}

func (p *Texture) GetMipLevelCount() uint32 {
	return uint32(C.aqua_wgpuTextureGetMipLevelCount(global_ctx.ctx, p.ref))
}

func (p *Texture) GetSampleCount() uint32 {
	return uint32(C.aqua_wgpuTextureGetSampleCount(global_ctx.ctx, p.ref))
}

func (p *Texture) GetUsage() TextureUsage {
	return TextureUsage(C.aqua_wgpuTextureGetUsage(global_ctx.ctx, p.ref))
}

func (p *Texture) GetWidth() uint32 {
	return uint32(C.aqua_wgpuTextureGetWidth(global_ctx.ctx, p.ref))
}

func (p *Texture) Release() {
	C.gowebgpu_texture_release(p.ref, p.deviceRef)
}
