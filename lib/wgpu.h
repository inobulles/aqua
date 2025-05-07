// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2025 Aymeric Wibo

#pragma once

#include "root.h"
#include "win.h"

#include <aqua/kos.h>

#include <webgpu/webgpu.h>
#include <webgpu/wgpu.h>

/**
 * WebGPU library component context.
 */
typedef struct wgpu_ctx_t* wgpu_ctx_t;

/**
 * Initialize the WebGPU library component.
 *
 * @param ctx The AQUA library context.
 * @return The WebGPU library component handle.
 */
aqua_component_t wgpu_init(aqua_ctx_t ctx);

/**
 * Connect to a WebGPU VDEV.
 *
 * {@link wgpu_disconn} must be called to disconnect from the VDEV and to free the context.
 *
 * @param vdev The descriptor of the WebGPU VDEV to connect to.
 * @return The WebGPU library component context or `NULL` if allocation failed.
 */
wgpu_ctx_t wgpu_conn(kos_vdev_descr_t const* vdev);

/**
 * Disconnect from a WebGPU VDEV.
 *
 * This function disconnects from the WebGPU VDEV and frees the context.
 *
 * @param ctx The WebGPU library component context.
 */
void wgpu_disconn(wgpu_ctx_t ctx);

/**
 * Get the WebGPU connection's host ID.
 *
 * @param ctx The WebGPU library component context.
 * @return The host ID.
 */
uint64_t wgpu_get_hid(wgpu_ctx_t ctx);

/**
 * Get the WebGPU connection's VDEV ID.
 *
 * @param ctx The WebGPU library component context.
 * @return The VDEV ID.
 */
uint64_t wgpu_get_vid(wgpu_ctx_t ctx);

WGPUSurface wgpu_surface_from_win(wgpu_ctx_t ctx, WGPUInstance instance, win_t win);

// clang-format off
// PROTOS:BEGIN
WGPUInstance aqua_wgpuCreateInstance(wgpu_ctx_t ctx,WGPU_NULLABLE WGPUInstanceDescriptor const * descriptor);
WGPUStatus aqua_wgpuGetInstanceCapabilities(wgpu_ctx_t ctx,WGPUInstanceCapabilities * capabilities);
WGPUProc aqua_wgpuGetProcAddress(wgpu_ctx_t ctx,WGPUStringView procName);
void aqua_wgpuAdapterGetFeatures(wgpu_ctx_t ctx,WGPUAdapter adapter,WGPUSupportedFeatures * features);
WGPUStatus aqua_wgpuAdapterGetInfo(wgpu_ctx_t ctx,WGPUAdapter adapter,WGPUAdapterInfo * info);
WGPUStatus aqua_wgpuAdapterGetLimits(wgpu_ctx_t ctx,WGPUAdapter adapter,WGPULimits * limits);
WGPUBool aqua_wgpuAdapterHasFeature(wgpu_ctx_t ctx,WGPUAdapter adapter,WGPUFeatureName feature);
WGPUFuture aqua_wgpuAdapterRequestDevice(wgpu_ctx_t ctx,WGPUAdapter adapter,WGPU_NULLABLE WGPUDeviceDescriptor const * descriptor,WGPURequestDeviceCallbackInfo callbackInfo);
void aqua_wgpuAdapterAddRef(wgpu_ctx_t ctx,WGPUAdapter adapter);
void aqua_wgpuAdapterRelease(wgpu_ctx_t ctx,WGPUAdapter adapter);
void aqua_wgpuAdapterInfoFreeMembers(wgpu_ctx_t ctx,WGPUAdapterInfo adapterInfo);
void aqua_wgpuBindGroupSetLabel(wgpu_ctx_t ctx,WGPUBindGroup bindGroup,WGPUStringView label);
void aqua_wgpuBindGroupAddRef(wgpu_ctx_t ctx,WGPUBindGroup bindGroup);
void aqua_wgpuBindGroupRelease(wgpu_ctx_t ctx,WGPUBindGroup bindGroup);
void aqua_wgpuBindGroupLayoutSetLabel(wgpu_ctx_t ctx,WGPUBindGroupLayout bindGroupLayout,WGPUStringView label);
void aqua_wgpuBindGroupLayoutAddRef(wgpu_ctx_t ctx,WGPUBindGroupLayout bindGroupLayout);
void aqua_wgpuBindGroupLayoutRelease(wgpu_ctx_t ctx,WGPUBindGroupLayout bindGroupLayout);
void aqua_wgpuBufferDestroy(wgpu_ctx_t ctx,WGPUBuffer buffer);
void const * aqua_wgpuBufferGetConstMappedRange(wgpu_ctx_t ctx,WGPUBuffer buffer,size_t offset,size_t size);
WGPUBufferMapState aqua_wgpuBufferGetMapState(wgpu_ctx_t ctx,WGPUBuffer buffer);
void * aqua_wgpuBufferGetMappedRange(wgpu_ctx_t ctx,WGPUBuffer buffer,size_t offset,size_t size);
uint64_t aqua_wgpuBufferGetSize(wgpu_ctx_t ctx,WGPUBuffer buffer);
WGPUBufferUsage aqua_wgpuBufferGetUsage(wgpu_ctx_t ctx,WGPUBuffer buffer);
WGPUFuture aqua_wgpuBufferMapAsync(wgpu_ctx_t ctx,WGPUBuffer buffer,WGPUMapMode mode,size_t offset,size_t size,WGPUBufferMapCallbackInfo callbackInfo);
void aqua_wgpuBufferSetLabel(wgpu_ctx_t ctx,WGPUBuffer buffer,WGPUStringView label);
void aqua_wgpuBufferUnmap(wgpu_ctx_t ctx,WGPUBuffer buffer);
void aqua_wgpuBufferAddRef(wgpu_ctx_t ctx,WGPUBuffer buffer);
void aqua_wgpuBufferRelease(wgpu_ctx_t ctx,WGPUBuffer buffer);
void aqua_wgpuCommandBufferSetLabel(wgpu_ctx_t ctx,WGPUCommandBuffer commandBuffer,WGPUStringView label);
void aqua_wgpuCommandBufferAddRef(wgpu_ctx_t ctx,WGPUCommandBuffer commandBuffer);
void aqua_wgpuCommandBufferRelease(wgpu_ctx_t ctx,WGPUCommandBuffer commandBuffer);
WGPUComputePassEncoder aqua_wgpuCommandEncoderBeginComputePass(wgpu_ctx_t ctx,WGPUCommandEncoder commandEncoder,WGPU_NULLABLE WGPUComputePassDescriptor const * descriptor);
WGPURenderPassEncoder aqua_wgpuCommandEncoderBeginRenderPass(wgpu_ctx_t ctx,WGPUCommandEncoder commandEncoder,WGPURenderPassDescriptor const * descriptor);
void aqua_wgpuCommandEncoderClearBuffer(wgpu_ctx_t ctx,WGPUCommandEncoder commandEncoder,WGPUBuffer buffer,uint64_t offset,uint64_t size);
void aqua_wgpuCommandEncoderCopyBufferToBuffer(wgpu_ctx_t ctx,WGPUCommandEncoder commandEncoder,WGPUBuffer source,uint64_t sourceOffset,WGPUBuffer destination,uint64_t destinationOffset,uint64_t size);
void aqua_wgpuCommandEncoderCopyBufferToTexture(wgpu_ctx_t ctx,WGPUCommandEncoder commandEncoder,WGPUTexelCopyBufferInfo const * source,WGPUTexelCopyTextureInfo const * destination,WGPUExtent3D const * copySize);
void aqua_wgpuCommandEncoderCopyTextureToBuffer(wgpu_ctx_t ctx,WGPUCommandEncoder commandEncoder,WGPUTexelCopyTextureInfo const * source,WGPUTexelCopyBufferInfo const * destination,WGPUExtent3D const * copySize);
void aqua_wgpuCommandEncoderCopyTextureToTexture(wgpu_ctx_t ctx,WGPUCommandEncoder commandEncoder,WGPUTexelCopyTextureInfo const * source,WGPUTexelCopyTextureInfo const * destination,WGPUExtent3D const * copySize);
WGPUCommandBuffer aqua_wgpuCommandEncoderFinish(wgpu_ctx_t ctx,WGPUCommandEncoder commandEncoder,WGPU_NULLABLE WGPUCommandBufferDescriptor const * descriptor);
void aqua_wgpuCommandEncoderInsertDebugMarker(wgpu_ctx_t ctx,WGPUCommandEncoder commandEncoder,WGPUStringView markerLabel);
void aqua_wgpuCommandEncoderPopDebugGroup(wgpu_ctx_t ctx,WGPUCommandEncoder commandEncoder);
void aqua_wgpuCommandEncoderPushDebugGroup(wgpu_ctx_t ctx,WGPUCommandEncoder commandEncoder,WGPUStringView groupLabel);
void aqua_wgpuCommandEncoderResolveQuerySet(wgpu_ctx_t ctx,WGPUCommandEncoder commandEncoder,WGPUQuerySet querySet,uint32_t firstQuery,uint32_t queryCount,WGPUBuffer destination,uint64_t destinationOffset);
void aqua_wgpuCommandEncoderSetLabel(wgpu_ctx_t ctx,WGPUCommandEncoder commandEncoder,WGPUStringView label);
void aqua_wgpuCommandEncoderWriteTimestamp(wgpu_ctx_t ctx,WGPUCommandEncoder commandEncoder,WGPUQuerySet querySet,uint32_t queryIndex);
void aqua_wgpuCommandEncoderAddRef(wgpu_ctx_t ctx,WGPUCommandEncoder commandEncoder);
void aqua_wgpuCommandEncoderRelease(wgpu_ctx_t ctx,WGPUCommandEncoder commandEncoder);
void aqua_wgpuComputePassEncoderDispatchWorkgroups(wgpu_ctx_t ctx,WGPUComputePassEncoder computePassEncoder,uint32_t workgroupCountX,uint32_t workgroupCountY,uint32_t workgroupCountZ);
void aqua_wgpuComputePassEncoderDispatchWorkgroupsIndirect(wgpu_ctx_t ctx,WGPUComputePassEncoder computePassEncoder,WGPUBuffer indirectBuffer,uint64_t indirectOffset);
void aqua_wgpuComputePassEncoderEnd(wgpu_ctx_t ctx,WGPUComputePassEncoder computePassEncoder);
void aqua_wgpuComputePassEncoderInsertDebugMarker(wgpu_ctx_t ctx,WGPUComputePassEncoder computePassEncoder,WGPUStringView markerLabel);
void aqua_wgpuComputePassEncoderPopDebugGroup(wgpu_ctx_t ctx,WGPUComputePassEncoder computePassEncoder);
void aqua_wgpuComputePassEncoderPushDebugGroup(wgpu_ctx_t ctx,WGPUComputePassEncoder computePassEncoder,WGPUStringView groupLabel);
void aqua_wgpuComputePassEncoderSetBindGroup(wgpu_ctx_t ctx,WGPUComputePassEncoder computePassEncoder,uint32_t groupIndex,WGPU_NULLABLE WGPUBindGroup group,size_t dynamicOffsetCount,uint32_t const * dynamicOffsets);
void aqua_wgpuComputePassEncoderSetLabel(wgpu_ctx_t ctx,WGPUComputePassEncoder computePassEncoder,WGPUStringView label);
void aqua_wgpuComputePassEncoderSetPipeline(wgpu_ctx_t ctx,WGPUComputePassEncoder computePassEncoder,WGPUComputePipeline pipeline);
void aqua_wgpuComputePassEncoderAddRef(wgpu_ctx_t ctx,WGPUComputePassEncoder computePassEncoder);
void aqua_wgpuComputePassEncoderRelease(wgpu_ctx_t ctx,WGPUComputePassEncoder computePassEncoder);
WGPUBindGroupLayout aqua_wgpuComputePipelineGetBindGroupLayout(wgpu_ctx_t ctx,WGPUComputePipeline computePipeline,uint32_t groupIndex);
void aqua_wgpuComputePipelineSetLabel(wgpu_ctx_t ctx,WGPUComputePipeline computePipeline,WGPUStringView label);
void aqua_wgpuComputePipelineAddRef(wgpu_ctx_t ctx,WGPUComputePipeline computePipeline);
void aqua_wgpuComputePipelineRelease(wgpu_ctx_t ctx,WGPUComputePipeline computePipeline);
WGPUBindGroup aqua_wgpuDeviceCreateBindGroup(wgpu_ctx_t ctx,WGPUDevice device,WGPUBindGroupDescriptor const * descriptor);
WGPUBindGroupLayout aqua_wgpuDeviceCreateBindGroupLayout(wgpu_ctx_t ctx,WGPUDevice device,WGPUBindGroupLayoutDescriptor const * descriptor);
WGPUBuffer aqua_wgpuDeviceCreateBuffer(wgpu_ctx_t ctx,WGPUDevice device,WGPUBufferDescriptor const * descriptor);
WGPUCommandEncoder aqua_wgpuDeviceCreateCommandEncoder(wgpu_ctx_t ctx,WGPUDevice device,WGPU_NULLABLE WGPUCommandEncoderDescriptor const * descriptor);
WGPUComputePipeline aqua_wgpuDeviceCreateComputePipeline(wgpu_ctx_t ctx,WGPUDevice device,WGPUComputePipelineDescriptor const * descriptor);
WGPUFuture aqua_wgpuDeviceCreateComputePipelineAsync(wgpu_ctx_t ctx,WGPUDevice device,WGPUComputePipelineDescriptor const * descriptor,WGPUCreateComputePipelineAsyncCallbackInfo callbackInfo);
WGPUPipelineLayout aqua_wgpuDeviceCreatePipelineLayout(wgpu_ctx_t ctx,WGPUDevice device,WGPUPipelineLayoutDescriptor const * descriptor);
WGPUQuerySet aqua_wgpuDeviceCreateQuerySet(wgpu_ctx_t ctx,WGPUDevice device,WGPUQuerySetDescriptor const * descriptor);
WGPURenderBundleEncoder aqua_wgpuDeviceCreateRenderBundleEncoder(wgpu_ctx_t ctx,WGPUDevice device,WGPURenderBundleEncoderDescriptor const * descriptor);
WGPURenderPipeline aqua_wgpuDeviceCreateRenderPipeline(wgpu_ctx_t ctx,WGPUDevice device,WGPURenderPipelineDescriptor const * descriptor);
WGPUFuture aqua_wgpuDeviceCreateRenderPipelineAsync(wgpu_ctx_t ctx,WGPUDevice device,WGPURenderPipelineDescriptor const * descriptor,WGPUCreateRenderPipelineAsyncCallbackInfo callbackInfo);
WGPUSampler aqua_wgpuDeviceCreateSampler(wgpu_ctx_t ctx,WGPUDevice device,WGPU_NULLABLE WGPUSamplerDescriptor const * descriptor);
WGPUShaderModule aqua_wgpuDeviceCreateShaderModule(wgpu_ctx_t ctx,WGPUDevice device,WGPUShaderModuleDescriptor const * descriptor);
WGPUTexture aqua_wgpuDeviceCreateTexture(wgpu_ctx_t ctx,WGPUDevice device,WGPUTextureDescriptor const * descriptor);
void aqua_wgpuDeviceDestroy(wgpu_ctx_t ctx,WGPUDevice device);
WGPUAdapterInfo aqua_wgpuDeviceGetAdapterInfo(wgpu_ctx_t ctx,WGPUDevice device);
void aqua_wgpuDeviceGetFeatures(wgpu_ctx_t ctx,WGPUDevice device,WGPUSupportedFeatures * features);
WGPUStatus aqua_wgpuDeviceGetLimits(wgpu_ctx_t ctx,WGPUDevice device,WGPULimits * limits);
WGPUFuture aqua_wgpuDeviceGetLostFuture(wgpu_ctx_t ctx,WGPUDevice device);
WGPUQueue aqua_wgpuDeviceGetQueue(wgpu_ctx_t ctx,WGPUDevice device);
WGPUBool aqua_wgpuDeviceHasFeature(wgpu_ctx_t ctx,WGPUDevice device,WGPUFeatureName feature);
WGPUFuture aqua_wgpuDevicePopErrorScope(wgpu_ctx_t ctx,WGPUDevice device,WGPUPopErrorScopeCallbackInfo callbackInfo);
void aqua_wgpuDevicePushErrorScope(wgpu_ctx_t ctx,WGPUDevice device,WGPUErrorFilter filter);
void aqua_wgpuDeviceSetLabel(wgpu_ctx_t ctx,WGPUDevice device,WGPUStringView label);
void aqua_wgpuDeviceAddRef(wgpu_ctx_t ctx,WGPUDevice device);
void aqua_wgpuDeviceRelease(wgpu_ctx_t ctx,WGPUDevice device);
WGPUSurface aqua_wgpuInstanceCreateSurface(wgpu_ctx_t ctx,WGPUInstance instance,WGPUSurfaceDescriptor const * descriptor);
WGPUStatus aqua_wgpuInstanceGetWGSLLanguageFeatures(wgpu_ctx_t ctx,WGPUInstance instance,WGPUSupportedWGSLLanguageFeatures * features);
void aqua_wgpuInstanceProcessEvents(wgpu_ctx_t ctx,WGPUInstance instance);
WGPUFuture aqua_wgpuInstanceRequestAdapter(wgpu_ctx_t ctx,WGPUInstance instance,WGPU_NULLABLE WGPURequestAdapterOptions const * options,WGPURequestAdapterCallbackInfo callbackInfo);
WGPUWaitStatus aqua_wgpuInstanceWaitAny(wgpu_ctx_t ctx,WGPUInstance instance,size_t futureCount,WGPU_NULLABLE WGPUFutureWaitInfo * futures,uint64_t timeoutNS);
void aqua_wgpuInstanceAddRef(wgpu_ctx_t ctx,WGPUInstance instance);
void aqua_wgpuInstanceRelease(wgpu_ctx_t ctx,WGPUInstance instance);
void aqua_wgpuPipelineLayoutSetLabel(wgpu_ctx_t ctx,WGPUPipelineLayout pipelineLayout,WGPUStringView label);
void aqua_wgpuPipelineLayoutAddRef(wgpu_ctx_t ctx,WGPUPipelineLayout pipelineLayout);
void aqua_wgpuPipelineLayoutRelease(wgpu_ctx_t ctx,WGPUPipelineLayout pipelineLayout);
void aqua_wgpuQuerySetDestroy(wgpu_ctx_t ctx,WGPUQuerySet querySet);
uint32_t aqua_wgpuQuerySetGetCount(wgpu_ctx_t ctx,WGPUQuerySet querySet);
WGPUQueryType aqua_wgpuQuerySetGetType(wgpu_ctx_t ctx,WGPUQuerySet querySet);
void aqua_wgpuQuerySetSetLabel(wgpu_ctx_t ctx,WGPUQuerySet querySet,WGPUStringView label);
void aqua_wgpuQuerySetAddRef(wgpu_ctx_t ctx,WGPUQuerySet querySet);
void aqua_wgpuQuerySetRelease(wgpu_ctx_t ctx,WGPUQuerySet querySet);
WGPUFuture aqua_wgpuQueueOnSubmittedWorkDone(wgpu_ctx_t ctx,WGPUQueue queue,WGPUQueueWorkDoneCallbackInfo callbackInfo);
void aqua_wgpuQueueSetLabel(wgpu_ctx_t ctx,WGPUQueue queue,WGPUStringView label);
void aqua_wgpuQueueSubmit(wgpu_ctx_t ctx,WGPUQueue queue,size_t commandCount,WGPUCommandBuffer const * commands);
void aqua_wgpuQueueWriteBuffer(wgpu_ctx_t ctx,WGPUQueue queue,WGPUBuffer buffer,uint64_t bufferOffset,void const * data,size_t size);
void aqua_wgpuQueueWriteTexture(wgpu_ctx_t ctx,WGPUQueue queue,WGPUTexelCopyTextureInfo const * destination,void const * data,size_t dataSize,WGPUTexelCopyBufferLayout const * dataLayout,WGPUExtent3D const * writeSize);
void aqua_wgpuQueueAddRef(wgpu_ctx_t ctx,WGPUQueue queue);
void aqua_wgpuQueueRelease(wgpu_ctx_t ctx,WGPUQueue queue);
void aqua_wgpuRenderBundleSetLabel(wgpu_ctx_t ctx,WGPURenderBundle renderBundle,WGPUStringView label);
void aqua_wgpuRenderBundleAddRef(wgpu_ctx_t ctx,WGPURenderBundle renderBundle);
void aqua_wgpuRenderBundleRelease(wgpu_ctx_t ctx,WGPURenderBundle renderBundle);
void aqua_wgpuRenderBundleEncoderDraw(wgpu_ctx_t ctx,WGPURenderBundleEncoder renderBundleEncoder,uint32_t vertexCount,uint32_t instanceCount,uint32_t firstVertex,uint32_t firstInstance);
void aqua_wgpuRenderBundleEncoderDrawIndexed(wgpu_ctx_t ctx,WGPURenderBundleEncoder renderBundleEncoder,uint32_t indexCount,uint32_t instanceCount,uint32_t firstIndex,int32_t baseVertex,uint32_t firstInstance);
void aqua_wgpuRenderBundleEncoderDrawIndexedIndirect(wgpu_ctx_t ctx,WGPURenderBundleEncoder renderBundleEncoder,WGPUBuffer indirectBuffer,uint64_t indirectOffset);
void aqua_wgpuRenderBundleEncoderDrawIndirect(wgpu_ctx_t ctx,WGPURenderBundleEncoder renderBundleEncoder,WGPUBuffer indirectBuffer,uint64_t indirectOffset);
WGPURenderBundle aqua_wgpuRenderBundleEncoderFinish(wgpu_ctx_t ctx,WGPURenderBundleEncoder renderBundleEncoder,WGPU_NULLABLE WGPURenderBundleDescriptor const * descriptor);
void aqua_wgpuRenderBundleEncoderInsertDebugMarker(wgpu_ctx_t ctx,WGPURenderBundleEncoder renderBundleEncoder,WGPUStringView markerLabel);
void aqua_wgpuRenderBundleEncoderPopDebugGroup(wgpu_ctx_t ctx,WGPURenderBundleEncoder renderBundleEncoder);
void aqua_wgpuRenderBundleEncoderPushDebugGroup(wgpu_ctx_t ctx,WGPURenderBundleEncoder renderBundleEncoder,WGPUStringView groupLabel);
void aqua_wgpuRenderBundleEncoderSetBindGroup(wgpu_ctx_t ctx,WGPURenderBundleEncoder renderBundleEncoder,uint32_t groupIndex,WGPU_NULLABLE WGPUBindGroup group,size_t dynamicOffsetCount,uint32_t const * dynamicOffsets);
void aqua_wgpuRenderBundleEncoderSetIndexBuffer(wgpu_ctx_t ctx,WGPURenderBundleEncoder renderBundleEncoder,WGPUBuffer buffer,WGPUIndexFormat format,uint64_t offset,uint64_t size);
void aqua_wgpuRenderBundleEncoderSetLabel(wgpu_ctx_t ctx,WGPURenderBundleEncoder renderBundleEncoder,WGPUStringView label);
void aqua_wgpuRenderBundleEncoderSetPipeline(wgpu_ctx_t ctx,WGPURenderBundleEncoder renderBundleEncoder,WGPURenderPipeline pipeline);
void aqua_wgpuRenderBundleEncoderSetVertexBuffer(wgpu_ctx_t ctx,WGPURenderBundleEncoder renderBundleEncoder,uint32_t slot,WGPU_NULLABLE WGPUBuffer buffer,uint64_t offset,uint64_t size);
void aqua_wgpuRenderBundleEncoderAddRef(wgpu_ctx_t ctx,WGPURenderBundleEncoder renderBundleEncoder);
void aqua_wgpuRenderBundleEncoderRelease(wgpu_ctx_t ctx,WGPURenderBundleEncoder renderBundleEncoder);
void aqua_wgpuRenderPassEncoderBeginOcclusionQuery(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder,uint32_t queryIndex);
void aqua_wgpuRenderPassEncoderDraw(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder,uint32_t vertexCount,uint32_t instanceCount,uint32_t firstVertex,uint32_t firstInstance);
void aqua_wgpuRenderPassEncoderDrawIndexed(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder,uint32_t indexCount,uint32_t instanceCount,uint32_t firstIndex,int32_t baseVertex,uint32_t firstInstance);
void aqua_wgpuRenderPassEncoderDrawIndexedIndirect(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder,WGPUBuffer indirectBuffer,uint64_t indirectOffset);
void aqua_wgpuRenderPassEncoderDrawIndirect(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder,WGPUBuffer indirectBuffer,uint64_t indirectOffset);
void aqua_wgpuRenderPassEncoderEnd(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder);
void aqua_wgpuRenderPassEncoderEndOcclusionQuery(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder);
void aqua_wgpuRenderPassEncoderExecuteBundles(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder,size_t bundleCount,WGPURenderBundle const * bundles);
void aqua_wgpuRenderPassEncoderInsertDebugMarker(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder,WGPUStringView markerLabel);
void aqua_wgpuRenderPassEncoderPopDebugGroup(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder);
void aqua_wgpuRenderPassEncoderPushDebugGroup(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder,WGPUStringView groupLabel);
void aqua_wgpuRenderPassEncoderSetBindGroup(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder,uint32_t groupIndex,WGPU_NULLABLE WGPUBindGroup group,size_t dynamicOffsetCount,uint32_t const * dynamicOffsets);
void aqua_wgpuRenderPassEncoderSetBlendConstant(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder,WGPUColor const * color);
void aqua_wgpuRenderPassEncoderSetIndexBuffer(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder,WGPUBuffer buffer,WGPUIndexFormat format,uint64_t offset,uint64_t size);
void aqua_wgpuRenderPassEncoderSetLabel(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder,WGPUStringView label);
void aqua_wgpuRenderPassEncoderSetPipeline(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder,WGPURenderPipeline pipeline);
void aqua_wgpuRenderPassEncoderSetScissorRect(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder,uint32_t x,uint32_t y,uint32_t width,uint32_t height);
void aqua_wgpuRenderPassEncoderSetStencilReference(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder,uint32_t reference);
void aqua_wgpuRenderPassEncoderSetVertexBuffer(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder,uint32_t slot,WGPU_NULLABLE WGPUBuffer buffer,uint64_t offset,uint64_t size);
void aqua_wgpuRenderPassEncoderSetViewport(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder,float x,float y,float width,float height,float minDepth,float maxDepth);
void aqua_wgpuRenderPassEncoderAddRef(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder);
void aqua_wgpuRenderPassEncoderRelease(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder);
WGPUBindGroupLayout aqua_wgpuRenderPipelineGetBindGroupLayout(wgpu_ctx_t ctx,WGPURenderPipeline renderPipeline,uint32_t groupIndex);
void aqua_wgpuRenderPipelineSetLabel(wgpu_ctx_t ctx,WGPURenderPipeline renderPipeline,WGPUStringView label);
void aqua_wgpuRenderPipelineAddRef(wgpu_ctx_t ctx,WGPURenderPipeline renderPipeline);
void aqua_wgpuRenderPipelineRelease(wgpu_ctx_t ctx,WGPURenderPipeline renderPipeline);
void aqua_wgpuSamplerSetLabel(wgpu_ctx_t ctx,WGPUSampler sampler,WGPUStringView label);
void aqua_wgpuSamplerAddRef(wgpu_ctx_t ctx,WGPUSampler sampler);
void aqua_wgpuSamplerRelease(wgpu_ctx_t ctx,WGPUSampler sampler);
WGPUFuture aqua_wgpuShaderModuleGetCompilationInfo(wgpu_ctx_t ctx,WGPUShaderModule shaderModule,WGPUCompilationInfoCallbackInfo callbackInfo);
void aqua_wgpuShaderModuleSetLabel(wgpu_ctx_t ctx,WGPUShaderModule shaderModule,WGPUStringView label);
void aqua_wgpuShaderModuleAddRef(wgpu_ctx_t ctx,WGPUShaderModule shaderModule);
void aqua_wgpuShaderModuleRelease(wgpu_ctx_t ctx,WGPUShaderModule shaderModule);
void aqua_wgpuSupportedFeaturesFreeMembers(wgpu_ctx_t ctx,WGPUSupportedFeatures supportedFeatures);
void aqua_wgpuSupportedWGSLLanguageFeaturesFreeMembers(wgpu_ctx_t ctx,WGPUSupportedWGSLLanguageFeatures supportedWGSLLanguageFeatures);
void aqua_wgpuSurfaceConfigure(wgpu_ctx_t ctx,WGPUSurface surface,WGPUSurfaceConfiguration const * config);
WGPUStatus aqua_wgpuSurfaceGetCapabilities(wgpu_ctx_t ctx,WGPUSurface surface,WGPUAdapter adapter,WGPUSurfaceCapabilities * capabilities);
void aqua_wgpuSurfaceGetCurrentTexture(wgpu_ctx_t ctx,WGPUSurface surface,WGPUSurfaceTexture * surfaceTexture);
WGPUStatus aqua_wgpuSurfacePresent(wgpu_ctx_t ctx,WGPUSurface surface);
void aqua_wgpuSurfaceUnconfigure(wgpu_ctx_t ctx,WGPUSurface surface);
void aqua_wgpuSurfaceAddRef(wgpu_ctx_t ctx,WGPUSurface surface);
void aqua_wgpuSurfaceRelease(wgpu_ctx_t ctx,WGPUSurface surface);
void aqua_wgpuSurfaceCapabilitiesFreeMembers(wgpu_ctx_t ctx,WGPUSurfaceCapabilities surfaceCapabilities);
WGPUTextureView aqua_wgpuTextureCreateView(wgpu_ctx_t ctx,WGPUTexture texture,WGPU_NULLABLE WGPUTextureViewDescriptor const * descriptor);
void aqua_wgpuTextureDestroy(wgpu_ctx_t ctx,WGPUTexture texture);
uint32_t aqua_wgpuTextureGetDepthOrArrayLayers(wgpu_ctx_t ctx,WGPUTexture texture);
WGPUTextureDimension aqua_wgpuTextureGetDimension(wgpu_ctx_t ctx,WGPUTexture texture);
WGPUTextureFormat aqua_wgpuTextureGetFormat(wgpu_ctx_t ctx,WGPUTexture texture);
uint32_t aqua_wgpuTextureGetHeight(wgpu_ctx_t ctx,WGPUTexture texture);
uint32_t aqua_wgpuTextureGetMipLevelCount(wgpu_ctx_t ctx,WGPUTexture texture);
uint32_t aqua_wgpuTextureGetSampleCount(wgpu_ctx_t ctx,WGPUTexture texture);
WGPUTextureUsage aqua_wgpuTextureGetUsage(wgpu_ctx_t ctx,WGPUTexture texture);
uint32_t aqua_wgpuTextureGetWidth(wgpu_ctx_t ctx,WGPUTexture texture);
void aqua_wgpuTextureSetLabel(wgpu_ctx_t ctx,WGPUTexture texture,WGPUStringView label);
void aqua_wgpuTextureAddRef(wgpu_ctx_t ctx,WGPUTexture texture);
void aqua_wgpuTextureRelease(wgpu_ctx_t ctx,WGPUTexture texture);
void aqua_wgpuTextureViewSetLabel(wgpu_ctx_t ctx,WGPUTextureView textureView,WGPUStringView label);
void aqua_wgpuTextureViewAddRef(wgpu_ctx_t ctx,WGPUTextureView textureView);
void aqua_wgpuTextureViewRelease(wgpu_ctx_t ctx,WGPUTextureView textureView);
void aqua_wgpuGenerateReport(wgpu_ctx_t ctx,WGPUInstance instance,WGPUGlobalReport * report);
size_t aqua_wgpuInstanceEnumerateAdapters(wgpu_ctx_t ctx,WGPUInstance instance,WGPU_NULLABLE WGPUInstanceEnumerateAdapterOptions const * options,WGPUAdapter * adapters);
WGPUSubmissionIndex aqua_wgpuQueueSubmitForIndex(wgpu_ctx_t ctx,WGPUQueue queue,size_t commandCount,WGPUCommandBuffer const * commands);
WGPUBool aqua_wgpuDevicePoll(wgpu_ctx_t ctx,WGPUDevice device,WGPUBool wait,WGPU_NULLABLE WGPUSubmissionIndex const * submissionIndex);
WGPUShaderModule aqua_wgpuDeviceCreateShaderModuleSpirV(wgpu_ctx_t ctx,WGPUDevice device,WGPUShaderModuleDescriptorSpirV const * descriptor);
void aqua_wgpuSetLogCallback(wgpu_ctx_t ctx,WGPULogCallback callback,void * userdata);
void aqua_wgpuSetLogLevel(wgpu_ctx_t ctx,WGPULogLevel level);
uint32_t aqua_wgpuGetVersion(wgpu_ctx_t ctx);
void aqua_wgpuRenderPassEncoderSetPushConstants(wgpu_ctx_t ctx,WGPURenderPassEncoder encoder,WGPUShaderStage stages,uint32_t offset,uint32_t sizeBytes,void const * data);
void aqua_wgpuComputePassEncoderSetPushConstants(wgpu_ctx_t ctx,WGPUComputePassEncoder encoder,uint32_t offset,uint32_t sizeBytes,void const * data);
void aqua_wgpuRenderBundleEncoderSetPushConstants(wgpu_ctx_t ctx,WGPURenderBundleEncoder encoder,WGPUShaderStage stages,uint32_t offset,uint32_t sizeBytes,void const * data);
void aqua_wgpuRenderPassEncoderMultiDrawIndirect(wgpu_ctx_t ctx,WGPURenderPassEncoder encoder,WGPUBuffer buffer,uint64_t offset,uint32_t count);
void aqua_wgpuRenderPassEncoderMultiDrawIndexedIndirect(wgpu_ctx_t ctx,WGPURenderPassEncoder encoder,WGPUBuffer buffer,uint64_t offset,uint32_t count);
void aqua_wgpuRenderPassEncoderMultiDrawIndirectCount(wgpu_ctx_t ctx,WGPURenderPassEncoder encoder,WGPUBuffer buffer,uint64_t offset,WGPUBuffer count_buffer,uint64_t count_buffer_offset,uint32_t max_count);
void aqua_wgpuRenderPassEncoderMultiDrawIndexedIndirectCount(wgpu_ctx_t ctx,WGPURenderPassEncoder encoder,WGPUBuffer buffer,uint64_t offset,WGPUBuffer count_buffer,uint64_t count_buffer_offset,uint32_t max_count);
void aqua_wgpuComputePassEncoderBeginPipelineStatisticsQuery(wgpu_ctx_t ctx,WGPUComputePassEncoder computePassEncoder,WGPUQuerySet querySet,uint32_t queryIndex);
void aqua_wgpuComputePassEncoderEndPipelineStatisticsQuery(wgpu_ctx_t ctx,WGPUComputePassEncoder computePassEncoder);
void aqua_wgpuRenderPassEncoderBeginPipelineStatisticsQuery(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder,WGPUQuerySet querySet,uint32_t queryIndex);
void aqua_wgpuRenderPassEncoderEndPipelineStatisticsQuery(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder);
void aqua_wgpuComputePassEncoderWriteTimestamp(wgpu_ctx_t ctx,WGPUComputePassEncoder computePassEncoder,WGPUQuerySet querySet,uint32_t queryIndex);
void aqua_wgpuRenderPassEncoderWriteTimestamp(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder,WGPUQuerySet querySet,uint32_t queryIndex);
// PROTOS:END
// clang-format on
