//go:build !js

package wgpu

/*

#include <stdlib.h>
#include <aqua/wgpu.h>

*/
import "C"

type ComputePipeline struct {
	ref C.WGPUComputePipeline
}

func (p *ComputePipeline) GetBindGroupLayout(groupIndex uint32) *BindGroupLayout {
	ref := C.aqua_wgpuComputePipelineGetBindGroupLayout(global_ctx.ctx, p.ref, C.uint32_t(groupIndex))
	if ref == nil {
		panic("Failed to accquire BindGroupLayout")
	}

	return &BindGroupLayout{ref}
}

func (p *ComputePipeline) Release() {
	C.aqua_wgpuComputePipelineRelease(global_ctx.ctx, p.ref)
}
