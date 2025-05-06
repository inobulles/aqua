//go:build !js

package wgpu

/*

#include <stdlib.h>
#include <aqua/wgpu.h>

extern void gowebgpu_error_callback_c(enum WGPUPopErrorScopeStatus status, WGPUErrorType type, WGPUStringView message, void * userdata, void * userdata2);

static inline void gowebgpu_compute_pass_encoder_end(WGPUComputePassEncoder computePassEncoder, WGPUDevice device, void * error_userdata) {
	wgpuDevicePushErrorScope(device, WGPUErrorFilter_Validation);
	wgpuComputePassEncoderEnd(computePassEncoder);

	WGPUPopErrorScopeCallbackInfo const err_cb = {
		.callback = gowebgpu_error_callback_c,
		.userdata1 = error_userdata,
	};

	wgpuDevicePopErrorScope(device, err_cb);
}

static inline void gowebgpu_compute_pass_encoder_release(WGPUComputePassEncoder computePassEncoder, WGPUDevice device) {
	wgpuDeviceRelease(device);
	wgpuComputePassEncoderRelease(computePassEncoder);
}

*/
import "C"
import (
	"errors"
	"runtime/cgo"
	"unsafe"
)

type ComputePassEncoder struct {
	deviceRef C.WGPUDevice
	ref       C.WGPUComputePassEncoder
}

func (p *ComputePassEncoder) BeginPipelineStatisticsQuery(querySet *QuerySet, queryIndex uint32) {
	C.aqua_wgpuComputePassEncoderBeginPipelineStatisticsQuery(global_ctx.ctx, p.ref, querySet.ref, C.uint32_t(queryIndex))
}

func (p *ComputePassEncoder) DispatchWorkgroups(workgroupCountX, workgroupCountY, workgroupCountZ uint32) {
	C.aqua_wgpuComputePassEncoderDispatchWorkgroups(global_ctx.ctx, p.ref, C.uint32_t(workgroupCountX), C.uint32_t(workgroupCountY), C.uint32_t(workgroupCountZ))
}

func (p *ComputePassEncoder) DispatchWorkgroupsIndirect(indirectBuffer *Buffer, indirectOffset uint64) {
	C.aqua_wgpuComputePassEncoderDispatchWorkgroupsIndirect(global_ctx.ctx, p.ref, indirectBuffer.ref, C.uint64_t(indirectOffset))
}

func (p *ComputePassEncoder) End() (err error) {
	var cb errorCallback = func(_ ErrorType, message string) {
		err = errors.New("wgpu.(*ComputePassEncoder).End(): " + message)
	}
	errorCallbackHandle := cgo.NewHandle(cb)
	defer errorCallbackHandle.Delete()

	C.gowebgpu_compute_pass_encoder_end(p.ref, p.deviceRef, unsafe.Pointer(&errorCallbackHandle))
	return
}

func (p *ComputePassEncoder) EndPipelineStatisticsQuery() {
	C.aqua_wgpuComputePassEncoderEndPipelineStatisticsQuery(global_ctx.ctx, p.ref)
}

func (p *ComputePassEncoder) InsertDebugMarker(markerLabel string) {
	markerLabelStr := C.CString(markerLabel)
	defer C.free(unsafe.Pointer(markerLabelStr))

	C.aqua_wgpuComputePassEncoderInsertDebugMarker(global_ctx.ctx, p.ref, C.WGPUStringView{
		data: markerLabelStr,
		length: C.WGPU_STRLEN,
	})
}

func (p *ComputePassEncoder) PopDebugGroup() {
	C.aqua_wgpuComputePassEncoderPopDebugGroup(global_ctx.ctx, p.ref)
}

func (p *ComputePassEncoder) PushDebugGroup(groupLabel string) {
	groupLabelStr := C.CString(groupLabel)
	defer C.free(unsafe.Pointer(groupLabelStr))

	C.aqua_wgpuComputePassEncoderPushDebugGroup(global_ctx.ctx, p.ref, C.WGPUStringView{
		data: groupLabelStr,
		length: C.WGPU_STRLEN,
	})
}

func (p *ComputePassEncoder) SetBindGroup(groupIndex uint32, group *BindGroup, dynamicOffsets []uint32) {
	dynamicOffsetCount := len(dynamicOffsets)
	if dynamicOffsetCount == 0 {
		C.aqua_wgpuComputePassEncoderSetBindGroup(global_ctx.ctx, p.ref, C.uint32_t(groupIndex), group.ref, 0, nil)
	} else {
		C.aqua_wgpuComputePassEncoderSetBindGroup(global_ctx.ctx, 
			p.ref, C.uint32_t(groupIndex), group.ref,
			C.size_t(dynamicOffsetCount), (*C.uint32_t)(unsafe.Pointer(&dynamicOffsets[0])),
		)
	}
}

func (p *ComputePassEncoder) SetPipeline(pipeline *ComputePipeline) {
	C.aqua_wgpuComputePassEncoderSetPipeline(global_ctx.ctx, p.ref, pipeline.ref)
}

func (p *ComputePassEncoder) Release() {
	C.gowebgpu_compute_pass_encoder_release(p.ref, p.deviceRef)
}
