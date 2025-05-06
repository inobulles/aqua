//go:build !js

package wgpu

/*

// Android
#cgo android,amd64 LDFLAGS: -L${SRCDIR}/lib/android/amd64 -lwgpu_native
#cgo android,386 LDFLAGS: -L${SRCDIR}/lib/android/386 -lwgpu_native
#cgo android,arm64 LDFLAGS: -L${SRCDIR}/lib/android/arm64 -lwgpu_native
#cgo android,arm LDFLAGS: -L${SRCDIR}/lib/android/arm -lwgpu_native

#cgo android LDFLAGS: -landroid -lm -llog

// Linux
#cgo linux,!android,amd64 LDFLAGS: -L${SRCDIR}/lib/linux/amd64 -lwgpu_native
#cgo linux,!android,arm64 LDFLAGS: -L${SRCDIR}/lib/linux/arm64 -lwgpu_native

#cgo linux,!android LDFLAGS: -lm -ldl

// iOS
#cgo ios,amd64 LDFLAGS: -L${SRCDIR}/lib/ios/amd64 -lwgpu_native
#cgo ios,arm64 LDFLAGS: -L${SRCDIR}/lib/ios/arm64 -lwgpu_native

// Darwin
#cgo darwin,!ios,amd64 LDFLAGS: -L${SRCDIR}/lib/darwin/amd64 -lwgpu_native
#cgo darwin,!ios,arm64 LDFLAGS: -L${SRCDIR}/lib/darwin/arm64 -lwgpu_native

#cgo darwin LDFLAGS: -framework QuartzCore -framework Metal

// Windows
#cgo windows,amd64 LDFLAGS: -L${SRCDIR}/lib/windows/amd64 -lwgpu_native
#cgo windows,arm64 LDFLAGS: -L${SRCDIR}/lib/windows/arm64 -lwgpu_native

#cgo windows LDFLAGS: -lopengl32 -lgdi32 -ld3dcompiler_47 -lws2_32 -luserenv -lbcrypt -lntdll

#include <stdio.h>
#include <aqua/wgpu.h>

#ifdef __ANDROID__
#include <android/log.h>
void logCallback_cgo(WGPULogLevel level, char const *msg) {
	switch (level) {
	case WGPULogLevel_Error:
		__android_log_write(ANDROID_LOG_ERROR, "GoLogWGPU", msg);
		break;
	case WGPULogLevel_Warn:
		__android_log_write(ANDROID_LOG_WARN, "GoLogWGPU", msg);
		break;
	default:
		__android_log_write(ANDROID_LOG_INFO, "GoLogWGPU", msg);
		break;
	}
}
#else
void logCallback_cgo(WGPULogLevel level, char const *msg) {
	char const *level_str;
	switch (level) {
	case WGPULogLevel_Error:
		level_str = "Error";
		break;
	case WGPULogLevel_Warn:
		level_str = "Warn";
		break;
	case WGPULogLevel_Info:
		level_str = "Info";
		break;
	case WGPULogLevel_Debug:
		level_str = "Debug";
		break;
	case WGPULogLevel_Trace:
		level_str = "Trace";
		break;
	default:
		level_str = "Unknown Level";
	}
	fprintf(stderr, "[wgpu] [%s] %s\n", level_str, msg);
}
#endif


*/
import "C"

func init() {
	C.aqua_wgpuSetLogCallback(global_ctx.ctx, C.WGPULogCallback(C.logCallback_cgo), nil)
}

func SetLogLevel(level LogLevel) {
	C.aqua_wgpuSetLogLevel(global_ctx.ctx, C.WGPULogLevel(level))
}

func GetVersion() Version {
	return Version(C.aqua_wgpuGetVersion(global_ctx.ctx, ))
}

type (
	BindGroup       struct{ ref C.WGPUBindGroup }
	BindGroupLayout struct{ ref C.WGPUBindGroupLayout }
	CommandBuffer   struct{ ref C.WGPUCommandBuffer }
	PipelineLayout  struct{ ref C.WGPUPipelineLayout }
	QuerySet        struct{ ref C.WGPUQuerySet }
	RenderBundle    struct{ ref C.WGPURenderBundle }
	Sampler         struct{ ref C.WGPUSampler }
	ShaderModule    struct{ ref C.WGPUShaderModule }
	TextureView     struct{ ref C.WGPUTextureView }
)

func (p *BindGroup) Release()       { C.aqua_wgpuBindGroupRelease(global_ctx.ctx, p.ref) }
func (p *BindGroupLayout) Release() { C.aqua_wgpuBindGroupLayoutRelease(global_ctx.ctx, p.ref) }
func (p *CommandBuffer) Release()   { C.aqua_wgpuCommandBufferRelease(global_ctx.ctx, p.ref) }
func (p *PipelineLayout) Release()  { C.aqua_wgpuPipelineLayoutRelease(global_ctx.ctx, p.ref) }
func (p *QuerySet) Release()        { C.aqua_wgpuQuerySetRelease(global_ctx.ctx, p.ref) }
func (p *RenderBundle) Release()    { C.aqua_wgpuRenderBundleRelease(global_ctx.ctx, p.ref) }
func (p *Sampler) Release()         { C.aqua_wgpuSamplerRelease(global_ctx.ctx, p.ref) }
func (p *ShaderModule) Release()    { C.aqua_wgpuShaderModuleRelease(global_ctx.ctx, p.ref) }
func (p *TextureView) Release()     { C.aqua_wgpuTextureViewRelease(global_ctx.ctx, p.ref) }

// cBool converts the given Go bool to a C.WGPUBool.
func cBool(b bool) C.WGPUBool {
	if b {
		return 1
	}
	return 0
}

// goBool converts the given C.WGPUBool to a Go bool.
func goBool(b C.WGPUBool) bool {
	return b != 0
}
