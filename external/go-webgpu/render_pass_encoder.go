//go:build !js

package wgpu

/*

#include <stdlib.h>
#include <aqua/wgpu.h>

extern void gowebgpu_error_callback_c(enum WGPUPopErrorScopeStatus status, WGPUErrorType type, WGPUStringView message, void * userdata, void * userdata2);

static inline void gowebgpu_render_pass_encoder_end(WGPURenderPassEncoder renderPassEncoder, WGPUDevice device, void * error_userdata) {
	wgpuDevicePushErrorScope(device, WGPUErrorFilter_Validation);
	wgpuRenderPassEncoderEnd(renderPassEncoder);

	WGPUPopErrorScopeCallbackInfo const err_cb = {
		.callback = gowebgpu_error_callback_c,
		.userdata1 = error_userdata,
	};

	wgpuDevicePopErrorScope(device, err_cb);
}

static inline void gowebgpu_render_pass_encoder_release(WGPURenderPassEncoder renderPassEncoder, WGPUDevice device) {
	wgpuDeviceRelease(device);
	wgpuRenderPassEncoderRelease(renderPassEncoder);
}

*/
import "C"
import (
	"errors"
	"runtime/cgo"
	"unsafe"
)

type RenderPassEncoder struct {
	deviceRef C.WGPUDevice
	ref       C.WGPURenderPassEncoder
}

func (p *RenderPassEncoder) BeginOcclusionQuery(queryIndex uint32) {
	C.aqua_wgpuRenderPassEncoderBeginOcclusionQuery(global_ctx.ctx, p.ref, C.uint32_t(queryIndex))
}

func (p *RenderPassEncoder) BeginPipelineStatisticsQuery(querySet *QuerySet, queryIndex uint32) {
	C.aqua_wgpuRenderPassEncoderBeginPipelineStatisticsQuery(global_ctx.ctx, p.ref, querySet.ref, C.uint32_t(queryIndex))
}

func (p *RenderPassEncoder) Draw(vertexCount, instanceCount, firstVertex, firstInstance uint32) {
	C.aqua_wgpuRenderPassEncoderDraw(global_ctx.ctx, p.ref,
		C.uint32_t(vertexCount),
		C.uint32_t(instanceCount),
		C.uint32_t(firstVertex),
		C.uint32_t(firstInstance),
	)
}

func (p *RenderPassEncoder) DrawIndexed(indexCount uint32, instanceCount uint32, firstIndex uint32, baseVertex int32, firstInstance uint32) {
	C.aqua_wgpuRenderPassEncoderDrawIndexed(global_ctx.ctx, p.ref,
		C.uint32_t(indexCount),
		C.uint32_t(instanceCount),
		C.uint32_t(firstIndex),
		C.int32_t(baseVertex),
		C.uint32_t(firstInstance),
	)
}

func (p *RenderPassEncoder) DrawIndexedIndirect(indirectBuffer *Buffer, indirectOffset uint64) {
	C.aqua_wgpuRenderPassEncoderDrawIndexedIndirect(global_ctx.ctx, p.ref, indirectBuffer.ref, C.uint64_t(indirectOffset))
}

func (p *RenderPassEncoder) DrawIndirect(indirectBuffer *Buffer, indirectOffset uint64) {
	C.aqua_wgpuRenderPassEncoderDrawIndirect(global_ctx.ctx, p.ref, indirectBuffer.ref, C.uint64_t(indirectOffset))
}

func (p *RenderPassEncoder) End() (err error) {
	var cb errorCallback = func(_ ErrorType, message string) {
		err = errors.New("wgpu.(*RenderPassEncoder).End(): " + message)
	}
	errorCallbackHandle := cgo.NewHandle(cb)
	defer errorCallbackHandle.Delete()

	C.gowebgpu_render_pass_encoder_end(
		p.ref,
		p.deviceRef,
		unsafe.Pointer(&errorCallbackHandle),
	)
	return
}

func (p *RenderPassEncoder) EndOcclusionQuery() {
	C.aqua_wgpuRenderPassEncoderEndOcclusionQuery(global_ctx.ctx, p.ref)
}

func (p *RenderPassEncoder) EndPipelineStatisticsQuery() {
	C.aqua_wgpuRenderPassEncoderEndPipelineStatisticsQuery(global_ctx.ctx, p.ref)
}

func (p *RenderPassEncoder) ExecuteBundles(bundles ...*RenderBundle) {
	bundlesCount := len(bundles)
	if bundlesCount == 0 {
		C.aqua_wgpuRenderPassEncoderExecuteBundles(global_ctx.ctx, p.ref, 0, nil)
		return
	}

	bundlesPtr := C.malloc(C.size_t(bundlesCount) * C.size_t(unsafe.Sizeof(C.WGPURenderBundle(nil))))
	defer C.free(bundlesPtr)

	bundlesSlice := unsafe.Slice((*C.WGPURenderBundle)(bundlesPtr), bundlesCount)
	for i, v := range bundles {
		bundlesSlice[i] = v.ref
	}

	C.aqua_wgpuRenderPassEncoderExecuteBundles(global_ctx.ctx, p.ref, C.size_t(bundlesCount), (*C.WGPURenderBundle)(bundlesPtr))
}

func (p *RenderPassEncoder) InsertDebugMarker(markerLabel string) {
	markerLabelStr := C.CString(markerLabel)
	defer C.free(unsafe.Pointer(markerLabelStr))

	C.aqua_wgpuRenderPassEncoderInsertDebugMarker(global_ctx.ctx, p.ref, C.WGPUStringView{
		data:   markerLabelStr,
		length: C.WGPU_STRLEN,
	})
}

func (p *RenderPassEncoder) PopDebugGroup() {
	C.aqua_wgpuRenderPassEncoderPopDebugGroup(global_ctx.ctx, p.ref)
}

func (p *RenderPassEncoder) PushDebugGroup(groupLabel string) {
	groupLabelStr := C.CString(groupLabel)
	defer C.free(unsafe.Pointer(groupLabelStr))

	C.aqua_wgpuRenderPassEncoderPushDebugGroup(global_ctx.ctx, p.ref, C.WGPUStringView{
		data:   groupLabelStr,
		length: C.WGPU_STRLEN,
	})
}

func (p *RenderPassEncoder) SetBindGroup(groupIndex uint32, group *BindGroup, dynamicOffsets []uint32) {
	dynamicOffsetCount := len(dynamicOffsets)
	if dynamicOffsetCount == 0 {
		C.aqua_wgpuRenderPassEncoderSetBindGroup(global_ctx.ctx, 
			p.ref,
			C.uint32_t(groupIndex),
			group.ref,
			0,
			nil,
		)
	} else {
		C.aqua_wgpuRenderPassEncoderSetBindGroup(global_ctx.ctx, 
			p.ref,
			C.uint32_t(groupIndex),
			group.ref,
			C.size_t(dynamicOffsetCount),
			(*C.uint32_t)(unsafe.Pointer(&dynamicOffsets[0])),
		)
	}
}

func (p *RenderPassEncoder) SetBlendConstant(color *Color) {
	c := C.WGPUColor{
		r: C.double(color.R),
		g: C.double(color.G),
		b: C.double(color.B),
		a: C.double(color.A),
	}
	C.aqua_wgpuRenderPassEncoderSetBlendConstant(global_ctx.ctx, p.ref, &c)
}

func (p *RenderPassEncoder) SetIndexBuffer(buffer *Buffer, format IndexFormat, offset uint64, size uint64) {
	C.aqua_wgpuRenderPassEncoderSetIndexBuffer(global_ctx.ctx, 
		p.ref,
		buffer.ref,
		C.WGPUIndexFormat(format),
		C.uint64_t(offset),
		C.uint64_t(size),
	)
}

func (p *RenderPassEncoder) SetPipeline(pipeline *RenderPipeline) {
	C.aqua_wgpuRenderPassEncoderSetPipeline(global_ctx.ctx, p.ref, pipeline.ref)
}

func (p *RenderPassEncoder) SetScissorRect(x, y, width, height uint32) {
	C.aqua_wgpuRenderPassEncoderSetScissorRect(global_ctx.ctx, 
		p.ref,
		C.uint32_t(x),
		C.uint32_t(y),
		C.uint32_t(width),
		C.uint32_t(height),
	)
}

func (p *RenderPassEncoder) SetStencilReference(reference uint32) {
	C.aqua_wgpuRenderPassEncoderSetStencilReference(global_ctx.ctx, p.ref, C.uint32_t(reference))
}

func (p *RenderPassEncoder) SetVertexBuffer(slot uint32, buffer *Buffer, offset uint64, size uint64) {
	C.aqua_wgpuRenderPassEncoderSetVertexBuffer(global_ctx.ctx, 
		p.ref,
		C.uint32_t(slot),
		buffer.ref,
		C.uint64_t(offset),
		C.uint64_t(size),
	)
}

func (p *RenderPassEncoder) SetViewport(x, y, width, height, minDepth, maxDepth float32) {
	C.aqua_wgpuRenderPassEncoderSetViewport(global_ctx.ctx, 
		p.ref,
		C.float(x),
		C.float(y),
		C.float(width),
		C.float(height),
		C.float(minDepth),
		C.float(maxDepth),
	)
}

func (p *RenderPassEncoder) SetPushConstants(stages ShaderStage, offset uint32, data []byte) {
	size := len(data)
	if size == 0 {
		C.aqua_wgpuRenderPassEncoderSetPushConstants(global_ctx.ctx, 
			p.ref,
			C.WGPUShaderStage(stages),
			C.uint32_t(offset),
			0,
			nil,
		)
		return
	}

	C.aqua_wgpuRenderPassEncoderSetPushConstants(global_ctx.ctx, 
		p.ref,
		C.WGPUShaderStage(stages),
		C.uint32_t(offset),
		C.uint32_t(size),
		unsafe.Pointer(&data[0]),
	)
}

func (p *RenderPassEncoder) MultiDrawIndirect(encoder *RenderPassEncoder, buffer Buffer, offset uint64, count uint32) {
	C.aqua_wgpuRenderPassEncoderMultiDrawIndirect(global_ctx.ctx, 
		encoder.ref,
		buffer.ref,
		C.uint64_t(offset),
		C.uint32_t(count),
	)
}

func (p *RenderPassEncoder) MultiDrawIndexedIndirect(encoder *RenderPassEncoder, buffer Buffer, offset uint64, count uint32) {
	C.aqua_wgpuRenderPassEncoderMultiDrawIndexedIndirect(global_ctx.ctx, 
		encoder.ref,
		buffer.ref,
		C.uint64_t(offset),
		C.uint32_t(count),
	)
}

func (p *RenderPassEncoder) MultiDrawIndirectCount(encoder *RenderPassEncoder, buffer Buffer, offset uint64, countBuffer Buffer, countBufferOffset uint64, maxCount uint32) {
	C.aqua_wgpuRenderPassEncoderMultiDrawIndirectCount(global_ctx.ctx, 
		encoder.ref,
		buffer.ref,
		C.uint64_t(offset),
		countBuffer.ref,
		C.uint64_t(countBufferOffset),
		C.uint32_t(maxCount),
	)
}

func (p *RenderPassEncoder) MultiDrawIndexedIndirectCount(encoder *RenderPassEncoder, buffer Buffer, offset uint64, countBuffer Buffer, countBufferOffset uint64, maxCount uint32) {
	C.aqua_wgpuRenderPassEncoderMultiDrawIndexedIndirectCount(global_ctx.ctx, 
		encoder.ref,
		buffer.ref,
		C.uint64_t(offset),
		countBuffer.ref,
		C.uint64_t(countBufferOffset),
		C.uint32_t(maxCount),
	)
}

func (p *RenderPassEncoder) Release() {
	C.gowebgpu_render_pass_encoder_release(p.ref, p.deviceRef)
}
