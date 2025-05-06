//go:build !js

package wgpu

/*

#include <stdlib.h>
#include <aqua/wgpu.h>

*/
import "C"
import "unsafe"

type RenderBundleEncoder struct {
	ref C.WGPURenderBundleEncoder
}

func (p *RenderBundleEncoder) Draw(vertexCount, instanceCount, firstVertex, firstInstance uint32) {
	C.aqua_wgpuRenderBundleEncoderDraw(global_ctx.ctx, 
		p.ref,
		C.uint32_t(vertexCount),
		C.uint32_t(instanceCount),
		C.uint32_t(firstVertex),
		C.uint32_t(firstInstance),
	)
}

func (p *RenderBundleEncoder) DrawIndexed(indexCount, instanceCount, firstIndex, baseVertex, firstInstance uint32) {
	C.aqua_wgpuRenderBundleEncoderDrawIndexed(global_ctx.ctx, 
		p.ref,
		C.uint32_t(indexCount),
		C.uint32_t(instanceCount),
		C.uint32_t(firstIndex),
		C.int32_t(baseVertex),
		C.uint32_t(firstInstance),
	)
}

func (p *RenderBundleEncoder) DrawIndexedIndirect(indirectBuffer *Buffer, indirectOffset uint64) {
	C.aqua_wgpuRenderBundleEncoderDrawIndexedIndirect(global_ctx.ctx, 
		p.ref,
		indirectBuffer.ref,
		C.uint64_t(indirectOffset),
	)
}

func (p *RenderBundleEncoder) DrawIndirect(indirectBuffer *Buffer, indirectOffset uint64) {
	C.aqua_wgpuRenderBundleEncoderDrawIndirect(global_ctx.ctx, 
		p.ref,
		indirectBuffer.ref,
		C.uint64_t(indirectOffset),
	)
}

type RenderBundleDescriptor struct {
	Label string
}

func (p *RenderBundleEncoder) Finish(descriptor *RenderBundleDescriptor) *RenderBundle {
	var desc *C.WGPURenderBundleDescriptor

	if descriptor != nil {
		label := C.CString(descriptor.Label)
		defer C.free(unsafe.Pointer(label))

		desc = &C.WGPURenderBundleDescriptor{
			label: C.WGPUStringView{data: label, length: C.WGPU_STRLEN},
		}
	}

	ref := C.aqua_wgpuRenderBundleEncoderFinish(global_ctx.ctx, p.ref, desc)
	if ref == nil {
		panic("Failed to accquire RenderBundle")
	}
	return &RenderBundle{ref}
}

func (p *RenderBundleEncoder) InsertDebugMarker(markerLabel string) {
	markerLabelStr := C.CString(markerLabel)
	defer C.free(unsafe.Pointer(markerLabelStr))

	C.aqua_wgpuRenderBundleEncoderInsertDebugMarker(global_ctx.ctx, p.ref, C.WGPUStringView{
		data:   markerLabelStr,
		length: C.WGPU_STRLEN,
	})
}

func (p *RenderBundleEncoder) PopDebugGroup() {
	C.aqua_wgpuRenderBundleEncoderPopDebugGroup(global_ctx.ctx, p.ref)
}

func (p *RenderBundleEncoder) PushDebugGroup(groupLabel string) {
	groupLabelStr := C.CString(groupLabel)
	defer C.free(unsafe.Pointer(groupLabelStr))

	C.aqua_wgpuRenderBundleEncoderPushDebugGroup(global_ctx.ctx, p.ref, C.WGPUStringView{
		data:   groupLabelStr,
		length: C.WGPU_STRLEN,
	})
}

func (p *RenderBundleEncoder) SetBindGroup(groupIndex uint32, group *BindGroup, dynamicOffsets []uint32) {
	dynamicOffsetCount := len(dynamicOffsets)
	if dynamicOffsetCount == 0 {
		C.aqua_wgpuRenderBundleEncoderSetBindGroup(global_ctx.ctx, p.ref, C.uint32_t(groupIndex), group.ref, 0, nil)
	} else {
		C.aqua_wgpuRenderBundleEncoderSetBindGroup(global_ctx.ctx, 
			p.ref, C.uint32_t(groupIndex), group.ref,
			C.size_t(dynamicOffsetCount), (*C.uint32_t)(unsafe.Pointer(&dynamicOffsets[0])),
		)
	}
}

func (p *RenderBundleEncoder) SetIndexBuffer(buffer *Buffer, format IndexFormat, offset uint64, size uint64) {
	C.aqua_wgpuRenderBundleEncoderSetIndexBuffer(global_ctx.ctx, 
		p.ref,
		buffer.ref,
		C.WGPUIndexFormat(format),
		C.uint64_t(offset),
		C.uint64_t(size),
	)
}

func (p *RenderBundleEncoder) SetPipeline(pipeline *RenderPipeline) {
	C.aqua_wgpuRenderBundleEncoderSetPipeline(global_ctx.ctx, p.ref, pipeline.ref)
}

func (p *RenderBundleEncoder) SetVertexBuffer(slot uint32, buffer *Buffer, offset uint64, size uint64) {
	C.aqua_wgpuRenderBundleEncoderSetVertexBuffer(global_ctx.ctx, 
		p.ref,
		C.uint32_t(slot),
		buffer.ref,
		C.uint64_t(offset),
		C.uint64_t(size),
	)
}

func (p *RenderBundleEncoder) Release() {
	C.aqua_wgpuRenderBundleEncoderRelease(global_ctx.ctx, p.ref)
}
