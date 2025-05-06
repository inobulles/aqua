//go:build !js

package wgpu

/*

#include <stdlib.h>
#include <aqua/wgpu.h>

*/
import "C"

type RenderPipeline struct {
	ref C.WGPURenderPipeline
}

func (p *RenderPipeline) GetBindGroupLayout(groupIndex uint32) *BindGroupLayout {
	ref := C.aqua_wgpuRenderPipelineGetBindGroupLayout(global_ctx.ctx, p.ref, C.uint32_t(groupIndex))
	if ref == nil {
		panic("Failed to accquire BindGroupLayout")
	}

	return &BindGroupLayout{ref}
}

func (p *RenderPipeline) Release() {
	C.aqua_wgpuRenderPipelineRelease(global_ctx.ctx, p.ref)
}
