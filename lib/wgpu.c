// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2025 Aymeric Wibo

#include "wgpu.h"

#define __AQUA_LIB_COMPONENT__
#include "component.h"
#include "win_internal.h"

#include <stdio.h>
#include <string.h>

#define SPEC "aquabsd.black.wgpu"

struct wgpu_ctx_t {
	uint64_t hid;
	uint64_t vid;

	kos_cookie_t last_cookie;

	bool is_conn;
	uint64_t conn_id;

	struct {
	} consts;

	struct {
		uint32_t surface_from_win;
		// clang-format off
// FN_IDS:BEGIN
		uint32_t wgpuCreateInstance;
		uint32_t wgpuGetInstanceCapabilities;
		uint32_t wgpuGetProcAddress;
		uint32_t wgpuAdapterGetFeatures;
		uint32_t wgpuAdapterGetInfo;
		uint32_t wgpuAdapterGetLimits;
		uint32_t wgpuAdapterHasFeature;
		uint32_t wgpuAdapterRequestDevice;
		uint32_t wgpuAdapterAddRef;
		uint32_t wgpuAdapterRelease;
		uint32_t wgpuAdapterInfoFreeMembers;
		uint32_t wgpuBindGroupSetLabel;
		uint32_t wgpuBindGroupAddRef;
		uint32_t wgpuBindGroupRelease;
		uint32_t wgpuBindGroupLayoutSetLabel;
		uint32_t wgpuBindGroupLayoutAddRef;
		uint32_t wgpuBindGroupLayoutRelease;
		uint32_t wgpuBufferDestroy;
		uint32_t wgpuBufferGetConstMappedRange;
		uint32_t wgpuBufferGetMapState;
		uint32_t wgpuBufferGetMappedRange;
		uint32_t wgpuBufferGetSize;
		uint32_t wgpuBufferGetUsage;
		uint32_t wgpuBufferMapAsync;
		uint32_t wgpuBufferSetLabel;
		uint32_t wgpuBufferUnmap;
		uint32_t wgpuBufferAddRef;
		uint32_t wgpuBufferRelease;
		uint32_t wgpuCommandBufferSetLabel;
		uint32_t wgpuCommandBufferAddRef;
		uint32_t wgpuCommandBufferRelease;
		uint32_t wgpuCommandEncoderBeginComputePass;
		uint32_t wgpuCommandEncoderBeginRenderPass;
		uint32_t wgpuCommandEncoderClearBuffer;
		uint32_t wgpuCommandEncoderCopyBufferToBuffer;
		uint32_t wgpuCommandEncoderCopyBufferToTexture;
		uint32_t wgpuCommandEncoderCopyTextureToBuffer;
		uint32_t wgpuCommandEncoderCopyTextureToTexture;
		uint32_t wgpuCommandEncoderFinish;
		uint32_t wgpuCommandEncoderInsertDebugMarker;
		uint32_t wgpuCommandEncoderPopDebugGroup;
		uint32_t wgpuCommandEncoderPushDebugGroup;
		uint32_t wgpuCommandEncoderResolveQuerySet;
		uint32_t wgpuCommandEncoderSetLabel;
		uint32_t wgpuCommandEncoderWriteTimestamp;
		uint32_t wgpuCommandEncoderAddRef;
		uint32_t wgpuCommandEncoderRelease;
		uint32_t wgpuComputePassEncoderDispatchWorkgroups;
		uint32_t wgpuComputePassEncoderDispatchWorkgroupsIndirect;
		uint32_t wgpuComputePassEncoderEnd;
		uint32_t wgpuComputePassEncoderInsertDebugMarker;
		uint32_t wgpuComputePassEncoderPopDebugGroup;
		uint32_t wgpuComputePassEncoderPushDebugGroup;
		uint32_t wgpuComputePassEncoderSetBindGroup;
		uint32_t wgpuComputePassEncoderSetLabel;
		uint32_t wgpuComputePassEncoderSetPipeline;
		uint32_t wgpuComputePassEncoderAddRef;
		uint32_t wgpuComputePassEncoderRelease;
		uint32_t wgpuComputePipelineGetBindGroupLayout;
		uint32_t wgpuComputePipelineSetLabel;
		uint32_t wgpuComputePipelineAddRef;
		uint32_t wgpuComputePipelineRelease;
		uint32_t wgpuDeviceCreateBindGroup;
		uint32_t wgpuDeviceCreateBindGroupLayout;
		uint32_t wgpuDeviceCreateBuffer;
		uint32_t wgpuDeviceCreateCommandEncoder;
		uint32_t wgpuDeviceCreateComputePipeline;
		uint32_t wgpuDeviceCreateComputePipelineAsync;
		uint32_t wgpuDeviceCreatePipelineLayout;
		uint32_t wgpuDeviceCreateQuerySet;
		uint32_t wgpuDeviceCreateRenderBundleEncoder;
		uint32_t wgpuDeviceCreateRenderPipeline;
		uint32_t wgpuDeviceCreateRenderPipelineAsync;
		uint32_t wgpuDeviceCreateSampler;
		uint32_t wgpuDeviceCreateShaderModule;
		uint32_t wgpuDeviceCreateTexture;
		uint32_t wgpuDeviceDestroy;
		uint32_t wgpuDeviceGetAdapterInfo;
		uint32_t wgpuDeviceGetFeatures;
		uint32_t wgpuDeviceGetLimits;
		uint32_t wgpuDeviceGetLostFuture;
		uint32_t wgpuDeviceGetQueue;
		uint32_t wgpuDeviceHasFeature;
		uint32_t wgpuDevicePopErrorScope;
		uint32_t wgpuDevicePushErrorScope;
		uint32_t wgpuDeviceSetLabel;
		uint32_t wgpuDeviceAddRef;
		uint32_t wgpuDeviceRelease;
		uint32_t wgpuInstanceCreateSurface;
		uint32_t wgpuInstanceGetWGSLLanguageFeatures;
		uint32_t wgpuInstanceProcessEvents;
		uint32_t wgpuInstanceRequestAdapter;
		uint32_t wgpuInstanceWaitAny;
		uint32_t wgpuInstanceAddRef;
		uint32_t wgpuInstanceRelease;
		uint32_t wgpuPipelineLayoutSetLabel;
		uint32_t wgpuPipelineLayoutAddRef;
		uint32_t wgpuPipelineLayoutRelease;
		uint32_t wgpuQuerySetDestroy;
		uint32_t wgpuQuerySetGetCount;
		uint32_t wgpuQuerySetGetType;
		uint32_t wgpuQuerySetSetLabel;
		uint32_t wgpuQuerySetAddRef;
		uint32_t wgpuQuerySetRelease;
		uint32_t wgpuQueueOnSubmittedWorkDone;
		uint32_t wgpuQueueSetLabel;
		uint32_t wgpuQueueSubmit;
		uint32_t wgpuQueueWriteBuffer;
		uint32_t wgpuQueueWriteTexture;
		uint32_t wgpuQueueAddRef;
		uint32_t wgpuQueueRelease;
		uint32_t wgpuRenderBundleSetLabel;
		uint32_t wgpuRenderBundleAddRef;
		uint32_t wgpuRenderBundleRelease;
		uint32_t wgpuRenderBundleEncoderDraw;
		uint32_t wgpuRenderBundleEncoderDrawIndexed;
		uint32_t wgpuRenderBundleEncoderDrawIndexedIndirect;
		uint32_t wgpuRenderBundleEncoderDrawIndirect;
		uint32_t wgpuRenderBundleEncoderFinish;
		uint32_t wgpuRenderBundleEncoderInsertDebugMarker;
		uint32_t wgpuRenderBundleEncoderPopDebugGroup;
		uint32_t wgpuRenderBundleEncoderPushDebugGroup;
		uint32_t wgpuRenderBundleEncoderSetBindGroup;
		uint32_t wgpuRenderBundleEncoderSetIndexBuffer;
		uint32_t wgpuRenderBundleEncoderSetLabel;
		uint32_t wgpuRenderBundleEncoderSetPipeline;
		uint32_t wgpuRenderBundleEncoderSetVertexBuffer;
		uint32_t wgpuRenderBundleEncoderAddRef;
		uint32_t wgpuRenderBundleEncoderRelease;
		uint32_t wgpuRenderPassEncoderBeginOcclusionQuery;
		uint32_t wgpuRenderPassEncoderDraw;
		uint32_t wgpuRenderPassEncoderDrawIndexed;
		uint32_t wgpuRenderPassEncoderDrawIndexedIndirect;
		uint32_t wgpuRenderPassEncoderDrawIndirect;
		uint32_t wgpuRenderPassEncoderEnd;
		uint32_t wgpuRenderPassEncoderEndOcclusionQuery;
		uint32_t wgpuRenderPassEncoderExecuteBundles;
		uint32_t wgpuRenderPassEncoderInsertDebugMarker;
		uint32_t wgpuRenderPassEncoderPopDebugGroup;
		uint32_t wgpuRenderPassEncoderPushDebugGroup;
		uint32_t wgpuRenderPassEncoderSetBindGroup;
		uint32_t wgpuRenderPassEncoderSetBlendConstant;
		uint32_t wgpuRenderPassEncoderSetIndexBuffer;
		uint32_t wgpuRenderPassEncoderSetLabel;
		uint32_t wgpuRenderPassEncoderSetPipeline;
		uint32_t wgpuRenderPassEncoderSetScissorRect;
		uint32_t wgpuRenderPassEncoderSetStencilReference;
		uint32_t wgpuRenderPassEncoderSetVertexBuffer;
		uint32_t wgpuRenderPassEncoderSetViewport;
		uint32_t wgpuRenderPassEncoderAddRef;
		uint32_t wgpuRenderPassEncoderRelease;
		uint32_t wgpuRenderPipelineGetBindGroupLayout;
		uint32_t wgpuRenderPipelineSetLabel;
		uint32_t wgpuRenderPipelineAddRef;
		uint32_t wgpuRenderPipelineRelease;
		uint32_t wgpuSamplerSetLabel;
		uint32_t wgpuSamplerAddRef;
		uint32_t wgpuSamplerRelease;
		uint32_t wgpuShaderModuleGetCompilationInfo;
		uint32_t wgpuShaderModuleSetLabel;
		uint32_t wgpuShaderModuleAddRef;
		uint32_t wgpuShaderModuleRelease;
		uint32_t wgpuSupportedFeaturesFreeMembers;
		uint32_t wgpuSupportedWGSLLanguageFeaturesFreeMembers;
		uint32_t wgpuSurfaceConfigure;
		uint32_t wgpuSurfaceGetCapabilities;
		uint32_t wgpuSurfaceGetCurrentTexture;
		uint32_t wgpuSurfacePresent;
		uint32_t wgpuSurfaceUnconfigure;
		uint32_t wgpuSurfaceAddRef;
		uint32_t wgpuSurfaceRelease;
		uint32_t wgpuSurfaceCapabilitiesFreeMembers;
		uint32_t wgpuTextureCreateView;
		uint32_t wgpuTextureDestroy;
		uint32_t wgpuTextureGetDepthOrArrayLayers;
		uint32_t wgpuTextureGetDimension;
		uint32_t wgpuTextureGetFormat;
		uint32_t wgpuTextureGetHeight;
		uint32_t wgpuTextureGetMipLevelCount;
		uint32_t wgpuTextureGetSampleCount;
		uint32_t wgpuTextureGetUsage;
		uint32_t wgpuTextureGetWidth;
		uint32_t wgpuTextureSetLabel;
		uint32_t wgpuTextureAddRef;
		uint32_t wgpuTextureRelease;
		uint32_t wgpuTextureViewSetLabel;
		uint32_t wgpuTextureViewAddRef;
		uint32_t wgpuTextureViewRelease;
		uint32_t wgpuGenerateReport;
		uint32_t wgpuInstanceEnumerateAdapters;
		uint32_t wgpuQueueSubmitForIndex;
		uint32_t wgpuDevicePoll;
		uint32_t wgpuDeviceCreateShaderModuleSpirV;
		uint32_t wgpuSetLogCallback;
		uint32_t wgpuSetLogLevel;
		uint32_t wgpuGetVersion;
		uint32_t wgpuRenderPassEncoderSetPushConstants;
		uint32_t wgpuComputePassEncoderSetPushConstants;
		uint32_t wgpuRenderBundleEncoderSetPushConstants;
		uint32_t wgpuRenderPassEncoderMultiDrawIndirect;
		uint32_t wgpuRenderPassEncoderMultiDrawIndexedIndirect;
		uint32_t wgpuRenderPassEncoderMultiDrawIndirectCount;
		uint32_t wgpuRenderPassEncoderMultiDrawIndexedIndirectCount;
		uint32_t wgpuComputePassEncoderBeginPipelineStatisticsQuery;
		uint32_t wgpuComputePassEncoderEndPipelineStatisticsQuery;
		uint32_t wgpuRenderPassEncoderBeginPipelineStatisticsQuery;
		uint32_t wgpuRenderPassEncoderEndPipelineStatisticsQuery;
		uint32_t wgpuComputePassEncoderWriteTimestamp;
		uint32_t wgpuRenderPassEncoderWriteTimestamp;
// FN_IDS:END
		// clang-format on
	} fns;

	bool last_success;
	kos_val_t last_ret;
};

static component_t comp;

static bool probe(kos_vdev_descr_t const* vdev) {
	return strcmp((char*) vdev->spec, SPEC) == 0;
}

WGPUSurface wgpu_surface_from_win(wgpu_ctx_t ctx, WGPUInstance instance, win_t win) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = instance,
		},
		{
			.opaque_ptr = win->opaque_ptr,
		},
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.surface_from_win, args);
	kos_flush(true);

	return ctx->last_ret.opaque_ptr;
}

aqua_component_t wgpu_init(aqua_ctx_t ctx) {
	aqua_register_component(ctx, &comp);
	kos_req_vdev("aquabsd.black.wgpu");

	return &comp;
}

wgpu_ctx_t wgpu_conn(kos_vdev_descr_t const* vdev) {
	wgpu_ctx_t const ctx = calloc(1, sizeof *ctx);

	if (ctx == NULL) {
		return NULL;
	}

	ctx->hid = vdev->host_id;
	ctx->vid = vdev->vdev_id;

	ctx->is_conn = false;
	ctx->last_cookie = kos_vdev_conn(ctx->hid, ctx->vid);

	// Add pending connection.

	cookie_notif_conn_tuple_t tuple = {
		.cookie = ctx->last_cookie,
		.comp = &comp,
		.data = ctx,
	};

	aqua_add_pending_conn(comp.ctx, &tuple);

	// Finally, flush.

	kos_flush(true);

	return ctx;
}

void wgpu_disconn(wgpu_ctx_t ctx) {
	if (ctx == NULL) {
		return;
	}

	if (!ctx->is_conn) {
		free(ctx);
		return;
	}

	kos_vdev_disconn(ctx->conn_id);
	free(ctx);
}

uint64_t wgpu_get_hid(wgpu_ctx_t ctx) {
	return ctx->hid;
}

uint64_t wgpu_get_vid(wgpu_ctx_t ctx) {
	return ctx->vid;
}

static void notif_conn(kos_notif_t const* notif, void* data) {
	wgpu_ctx_t const ctx = data;

	if (ctx == NULL || notif->cookie != ctx->last_cookie) {
		return;
	}

	ctx->conn_id = notif->conn_id;
	ctx->is_conn = true;

	// Read constants.

	memset(&ctx->consts, 0xFF, sizeof ctx->consts);

	for (size_t i = 0; i < notif->conn.const_count; i++) {
		kos_const_t const* const c = &notif->conn.consts[i];
		char const* const name = (void*) c->name;

		printf("WebGPU const: %s\n", name);
	}

	for (size_t i = 0; i < sizeof ctx->consts / sizeof(uint32_t); i++) {
		if (((uint32_t*) &ctx->consts)[i] == -1u) {
			ctx->is_conn = false;
			break;
		}
	}

	// Read functions.

	memset(&ctx->fns, 0xFF, sizeof ctx->fns);

	for (size_t i = 0; i < notif->conn.fn_count; i++) {
		kos_fn_t const* const fn = &notif->conn.fns[i];
		char const* const name = (void*) fn->name;

		if (
			strcmp(name, "surface_from_win") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "instance") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "win") == 0
		) {
			ctx->fns.surface_from_win = i;
		}

		// clang-format off
// FN_VALIDATORS:BEGIN
		if (
			strcmp(name, "wgpuCreateInstance") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[0].name, "descriptor") == 0
		) {
			ctx->fns.wgpuCreateInstance = i;
		}

		if (
			strcmp(name, "wgpuGetInstanceCapabilities") == 0 &&
			fn->ret_type == KOS_TYPE_U32 &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[0].name, "capabilities") == 0
		) {
			ctx->fns.wgpuGetInstanceCapabilities = i;
		}

		if (
			strcmp(name, "wgpuGetProcAddress") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "procName") == 0
		) {
			ctx->fns.wgpuGetProcAddress = i;
		}

		if (
			strcmp(name, "wgpuAdapterGetFeatures") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "adapter") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "features") == 0
		) {
			ctx->fns.wgpuAdapterGetFeatures = i;
		}

		if (
			strcmp(name, "wgpuAdapterGetInfo") == 0 &&
			fn->ret_type == KOS_TYPE_U32 &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "adapter") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "info") == 0
		) {
			ctx->fns.wgpuAdapterGetInfo = i;
		}

		if (
			strcmp(name, "wgpuAdapterGetLimits") == 0 &&
			fn->ret_type == KOS_TYPE_U32 &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "adapter") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "limits") == 0
		) {
			ctx->fns.wgpuAdapterGetLimits = i;
		}

		if (
			strcmp(name, "wgpuAdapterHasFeature") == 0 &&
			fn->ret_type == KOS_TYPE_BOOL &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "adapter") == 0 &&
			fn->params[1].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[1].name, "feature") == 0
		) {
			ctx->fns.wgpuAdapterHasFeature = i;
		}

		if (
			strcmp(name, "wgpuAdapterRequestDevice") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 3 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "adapter") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "descriptor") == 0 &&
			fn->params[2].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[2].name, "callbackInfo") == 0
		) {
			ctx->fns.wgpuAdapterRequestDevice = i;
		}

		if (
			strcmp(name, "wgpuAdapterAddRef") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "adapter") == 0
		) {
			ctx->fns.wgpuAdapterAddRef = i;
		}

		if (
			strcmp(name, "wgpuAdapterRelease") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "adapter") == 0
		) {
			ctx->fns.wgpuAdapterRelease = i;
		}

		if (
			strcmp(name, "wgpuAdapterInfoFreeMembers") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[0].name, "adapterInfo") == 0
		) {
			ctx->fns.wgpuAdapterInfoFreeMembers = i;
		}

		if (
			strcmp(name, "wgpuBindGroupSetLabel") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "bindGroup") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "label") == 0
		) {
			ctx->fns.wgpuBindGroupSetLabel = i;
		}

		if (
			strcmp(name, "wgpuBindGroupAddRef") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "bindGroup") == 0
		) {
			ctx->fns.wgpuBindGroupAddRef = i;
		}

		if (
			strcmp(name, "wgpuBindGroupRelease") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "bindGroup") == 0
		) {
			ctx->fns.wgpuBindGroupRelease = i;
		}

		if (
			strcmp(name, "wgpuBindGroupLayoutSetLabel") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "bindGroupLayout") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "label") == 0
		) {
			ctx->fns.wgpuBindGroupLayoutSetLabel = i;
		}

		if (
			strcmp(name, "wgpuBindGroupLayoutAddRef") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "bindGroupLayout") == 0
		) {
			ctx->fns.wgpuBindGroupLayoutAddRef = i;
		}

		if (
			strcmp(name, "wgpuBindGroupLayoutRelease") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "bindGroupLayout") == 0
		) {
			ctx->fns.wgpuBindGroupLayoutRelease = i;
		}

		if (
			strcmp(name, "wgpuBufferDestroy") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "buffer") == 0
		) {
			ctx->fns.wgpuBufferDestroy = i;
		}

		if (
			strcmp(name, "wgpuBufferGetConstMappedRange") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 3 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "buffer") == 0 &&
			fn->params[1].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[1].name, "offset") == 0 &&
			fn->params[2].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[2].name, "size") == 0
		) {
			ctx->fns.wgpuBufferGetConstMappedRange = i;
		}

		if (
			strcmp(name, "wgpuBufferGetMapState") == 0 &&
			fn->ret_type == KOS_TYPE_U32 &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "buffer") == 0
		) {
			ctx->fns.wgpuBufferGetMapState = i;
		}

		if (
			strcmp(name, "wgpuBufferGetMappedRange") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 3 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "buffer") == 0 &&
			fn->params[1].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[1].name, "offset") == 0 &&
			fn->params[2].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[2].name, "size") == 0
		) {
			ctx->fns.wgpuBufferGetMappedRange = i;
		}

		if (
			strcmp(name, "wgpuBufferGetSize") == 0 &&
			fn->ret_type == KOS_TYPE_U64 &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "buffer") == 0
		) {
			ctx->fns.wgpuBufferGetSize = i;
		}

		if (
			strcmp(name, "wgpuBufferGetUsage") == 0 &&
			fn->ret_type == KOS_TYPE_U64 &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "buffer") == 0
		) {
			ctx->fns.wgpuBufferGetUsage = i;
		}

		if (
			strcmp(name, "wgpuBufferMapAsync") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 5 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "buffer") == 0 &&
			fn->params[1].type == KOS_TYPE_U64 &&
			strcmp((char*) fn->params[1].name, "mode") == 0 &&
			fn->params[2].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[2].name, "offset") == 0 &&
			fn->params[3].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[3].name, "size") == 0 &&
			fn->params[4].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[4].name, "callbackInfo") == 0
		) {
			ctx->fns.wgpuBufferMapAsync = i;
		}

		if (
			strcmp(name, "wgpuBufferSetLabel") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "buffer") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "label") == 0
		) {
			ctx->fns.wgpuBufferSetLabel = i;
		}

		if (
			strcmp(name, "wgpuBufferUnmap") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "buffer") == 0
		) {
			ctx->fns.wgpuBufferUnmap = i;
		}

		if (
			strcmp(name, "wgpuBufferAddRef") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "buffer") == 0
		) {
			ctx->fns.wgpuBufferAddRef = i;
		}

		if (
			strcmp(name, "wgpuBufferRelease") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "buffer") == 0
		) {
			ctx->fns.wgpuBufferRelease = i;
		}

		if (
			strcmp(name, "wgpuCommandBufferSetLabel") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "commandBuffer") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "label") == 0
		) {
			ctx->fns.wgpuCommandBufferSetLabel = i;
		}

		if (
			strcmp(name, "wgpuCommandBufferAddRef") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "commandBuffer") == 0
		) {
			ctx->fns.wgpuCommandBufferAddRef = i;
		}

		if (
			strcmp(name, "wgpuCommandBufferRelease") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "commandBuffer") == 0
		) {
			ctx->fns.wgpuCommandBufferRelease = i;
		}

		if (
			strcmp(name, "wgpuCommandEncoderBeginComputePass") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "commandEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "descriptor") == 0
		) {
			ctx->fns.wgpuCommandEncoderBeginComputePass = i;
		}

		if (
			strcmp(name, "wgpuCommandEncoderBeginRenderPass") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "commandEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "descriptor") == 0
		) {
			ctx->fns.wgpuCommandEncoderBeginRenderPass = i;
		}

		if (
			strcmp(name, "wgpuCommandEncoderClearBuffer") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 4 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "commandEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "buffer") == 0 &&
			fn->params[2].type == KOS_TYPE_U64 &&
			strcmp((char*) fn->params[2].name, "offset") == 0 &&
			fn->params[3].type == KOS_TYPE_U64 &&
			strcmp((char*) fn->params[3].name, "size") == 0
		) {
			ctx->fns.wgpuCommandEncoderClearBuffer = i;
		}

		if (
			strcmp(name, "wgpuCommandEncoderCopyBufferToBuffer") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 6 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "commandEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "source") == 0 &&
			fn->params[2].type == KOS_TYPE_U64 &&
			strcmp((char*) fn->params[2].name, "sourceOffset") == 0 &&
			fn->params[3].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[3].name, "destination") == 0 &&
			fn->params[4].type == KOS_TYPE_U64 &&
			strcmp((char*) fn->params[4].name, "destinationOffset") == 0 &&
			fn->params[5].type == KOS_TYPE_U64 &&
			strcmp((char*) fn->params[5].name, "size") == 0
		) {
			ctx->fns.wgpuCommandEncoderCopyBufferToBuffer = i;
		}

		if (
			strcmp(name, "wgpuCommandEncoderCopyBufferToTexture") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 4 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "commandEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "source") == 0 &&
			fn->params[2].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[2].name, "destination") == 0 &&
			fn->params[3].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[3].name, "copySize") == 0
		) {
			ctx->fns.wgpuCommandEncoderCopyBufferToTexture = i;
		}

		if (
			strcmp(name, "wgpuCommandEncoderCopyTextureToBuffer") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 4 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "commandEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "source") == 0 &&
			fn->params[2].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[2].name, "destination") == 0 &&
			fn->params[3].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[3].name, "copySize") == 0
		) {
			ctx->fns.wgpuCommandEncoderCopyTextureToBuffer = i;
		}

		if (
			strcmp(name, "wgpuCommandEncoderCopyTextureToTexture") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 4 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "commandEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "source") == 0 &&
			fn->params[2].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[2].name, "destination") == 0 &&
			fn->params[3].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[3].name, "copySize") == 0
		) {
			ctx->fns.wgpuCommandEncoderCopyTextureToTexture = i;
		}

		if (
			strcmp(name, "wgpuCommandEncoderFinish") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "commandEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "descriptor") == 0
		) {
			ctx->fns.wgpuCommandEncoderFinish = i;
		}

		if (
			strcmp(name, "wgpuCommandEncoderInsertDebugMarker") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "commandEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "markerLabel") == 0
		) {
			ctx->fns.wgpuCommandEncoderInsertDebugMarker = i;
		}

		if (
			strcmp(name, "wgpuCommandEncoderPopDebugGroup") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "commandEncoder") == 0
		) {
			ctx->fns.wgpuCommandEncoderPopDebugGroup = i;
		}

		if (
			strcmp(name, "wgpuCommandEncoderPushDebugGroup") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "commandEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "groupLabel") == 0
		) {
			ctx->fns.wgpuCommandEncoderPushDebugGroup = i;
		}

		if (
			strcmp(name, "wgpuCommandEncoderResolveQuerySet") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 6 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "commandEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "querySet") == 0 &&
			fn->params[2].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[2].name, "firstQuery") == 0 &&
			fn->params[3].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[3].name, "queryCount") == 0 &&
			fn->params[4].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[4].name, "destination") == 0 &&
			fn->params[5].type == KOS_TYPE_U64 &&
			strcmp((char*) fn->params[5].name, "destinationOffset") == 0
		) {
			ctx->fns.wgpuCommandEncoderResolveQuerySet = i;
		}

		if (
			strcmp(name, "wgpuCommandEncoderSetLabel") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "commandEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "label") == 0
		) {
			ctx->fns.wgpuCommandEncoderSetLabel = i;
		}

		if (
			strcmp(name, "wgpuCommandEncoderWriteTimestamp") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 3 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "commandEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "querySet") == 0 &&
			fn->params[2].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[2].name, "queryIndex") == 0
		) {
			ctx->fns.wgpuCommandEncoderWriteTimestamp = i;
		}

		if (
			strcmp(name, "wgpuCommandEncoderAddRef") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "commandEncoder") == 0
		) {
			ctx->fns.wgpuCommandEncoderAddRef = i;
		}

		if (
			strcmp(name, "wgpuCommandEncoderRelease") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "commandEncoder") == 0
		) {
			ctx->fns.wgpuCommandEncoderRelease = i;
		}

		if (
			strcmp(name, "wgpuComputePassEncoderDispatchWorkgroups") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 4 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "computePassEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[1].name, "workgroupCountX") == 0 &&
			fn->params[2].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[2].name, "workgroupCountY") == 0 &&
			fn->params[3].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[3].name, "workgroupCountZ") == 0
		) {
			ctx->fns.wgpuComputePassEncoderDispatchWorkgroups = i;
		}

		if (
			strcmp(name, "wgpuComputePassEncoderDispatchWorkgroupsIndirect") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 3 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "computePassEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "indirectBuffer") == 0 &&
			fn->params[2].type == KOS_TYPE_U64 &&
			strcmp((char*) fn->params[2].name, "indirectOffset") == 0
		) {
			ctx->fns.wgpuComputePassEncoderDispatchWorkgroupsIndirect = i;
		}

		if (
			strcmp(name, "wgpuComputePassEncoderEnd") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "computePassEncoder") == 0
		) {
			ctx->fns.wgpuComputePassEncoderEnd = i;
		}

		if (
			strcmp(name, "wgpuComputePassEncoderInsertDebugMarker") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "computePassEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "markerLabel") == 0
		) {
			ctx->fns.wgpuComputePassEncoderInsertDebugMarker = i;
		}

		if (
			strcmp(name, "wgpuComputePassEncoderPopDebugGroup") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "computePassEncoder") == 0
		) {
			ctx->fns.wgpuComputePassEncoderPopDebugGroup = i;
		}

		if (
			strcmp(name, "wgpuComputePassEncoderPushDebugGroup") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "computePassEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "groupLabel") == 0
		) {
			ctx->fns.wgpuComputePassEncoderPushDebugGroup = i;
		}

		if (
			strcmp(name, "wgpuComputePassEncoderSetBindGroup") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 5 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "computePassEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[1].name, "groupIndex") == 0 &&
			fn->params[2].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[2].name, "group") == 0 &&
			fn->params[3].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[3].name, "dynamicOffsetCount") == 0 &&
			fn->params[4].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[4].name, "dynamicOffsets") == 0
		) {
			ctx->fns.wgpuComputePassEncoderSetBindGroup = i;
		}

		if (
			strcmp(name, "wgpuComputePassEncoderSetLabel") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "computePassEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "label") == 0
		) {
			ctx->fns.wgpuComputePassEncoderSetLabel = i;
		}

		if (
			strcmp(name, "wgpuComputePassEncoderSetPipeline") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "computePassEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "pipeline") == 0
		) {
			ctx->fns.wgpuComputePassEncoderSetPipeline = i;
		}

		if (
			strcmp(name, "wgpuComputePassEncoderAddRef") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "computePassEncoder") == 0
		) {
			ctx->fns.wgpuComputePassEncoderAddRef = i;
		}

		if (
			strcmp(name, "wgpuComputePassEncoderRelease") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "computePassEncoder") == 0
		) {
			ctx->fns.wgpuComputePassEncoderRelease = i;
		}

		if (
			strcmp(name, "wgpuComputePipelineGetBindGroupLayout") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "computePipeline") == 0 &&
			fn->params[1].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[1].name, "groupIndex") == 0
		) {
			ctx->fns.wgpuComputePipelineGetBindGroupLayout = i;
		}

		if (
			strcmp(name, "wgpuComputePipelineSetLabel") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "computePipeline") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "label") == 0
		) {
			ctx->fns.wgpuComputePipelineSetLabel = i;
		}

		if (
			strcmp(name, "wgpuComputePipelineAddRef") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "computePipeline") == 0
		) {
			ctx->fns.wgpuComputePipelineAddRef = i;
		}

		if (
			strcmp(name, "wgpuComputePipelineRelease") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "computePipeline") == 0
		) {
			ctx->fns.wgpuComputePipelineRelease = i;
		}

		if (
			strcmp(name, "wgpuDeviceCreateBindGroup") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "device") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "descriptor") == 0
		) {
			ctx->fns.wgpuDeviceCreateBindGroup = i;
		}

		if (
			strcmp(name, "wgpuDeviceCreateBindGroupLayout") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "device") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "descriptor") == 0
		) {
			ctx->fns.wgpuDeviceCreateBindGroupLayout = i;
		}

		if (
			strcmp(name, "wgpuDeviceCreateBuffer") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "device") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "descriptor") == 0
		) {
			ctx->fns.wgpuDeviceCreateBuffer = i;
		}

		if (
			strcmp(name, "wgpuDeviceCreateCommandEncoder") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "device") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "descriptor") == 0
		) {
			ctx->fns.wgpuDeviceCreateCommandEncoder = i;
		}

		if (
			strcmp(name, "wgpuDeviceCreateComputePipeline") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "device") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "descriptor") == 0
		) {
			ctx->fns.wgpuDeviceCreateComputePipeline = i;
		}

		if (
			strcmp(name, "wgpuDeviceCreateComputePipelineAsync") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 3 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "device") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "descriptor") == 0 &&
			fn->params[2].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[2].name, "callbackInfo") == 0
		) {
			ctx->fns.wgpuDeviceCreateComputePipelineAsync = i;
		}

		if (
			strcmp(name, "wgpuDeviceCreatePipelineLayout") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "device") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "descriptor") == 0
		) {
			ctx->fns.wgpuDeviceCreatePipelineLayout = i;
		}

		if (
			strcmp(name, "wgpuDeviceCreateQuerySet") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "device") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "descriptor") == 0
		) {
			ctx->fns.wgpuDeviceCreateQuerySet = i;
		}

		if (
			strcmp(name, "wgpuDeviceCreateRenderBundleEncoder") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "device") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "descriptor") == 0
		) {
			ctx->fns.wgpuDeviceCreateRenderBundleEncoder = i;
		}

		if (
			strcmp(name, "wgpuDeviceCreateRenderPipeline") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "device") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "descriptor") == 0
		) {
			ctx->fns.wgpuDeviceCreateRenderPipeline = i;
		}

		if (
			strcmp(name, "wgpuDeviceCreateRenderPipelineAsync") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 3 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "device") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "descriptor") == 0 &&
			fn->params[2].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[2].name, "callbackInfo") == 0
		) {
			ctx->fns.wgpuDeviceCreateRenderPipelineAsync = i;
		}

		if (
			strcmp(name, "wgpuDeviceCreateSampler") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "device") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "descriptor") == 0
		) {
			ctx->fns.wgpuDeviceCreateSampler = i;
		}

		if (
			strcmp(name, "wgpuDeviceCreateShaderModule") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "device") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "descriptor") == 0
		) {
			ctx->fns.wgpuDeviceCreateShaderModule = i;
		}

		if (
			strcmp(name, "wgpuDeviceCreateTexture") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "device") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "descriptor") == 0
		) {
			ctx->fns.wgpuDeviceCreateTexture = i;
		}

		if (
			strcmp(name, "wgpuDeviceDestroy") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "device") == 0
		) {
			ctx->fns.wgpuDeviceDestroy = i;
		}

		if (
			strcmp(name, "wgpuDeviceGetAdapterInfo") == 0 &&
			fn->ret_type == KOS_TYPE_BUF &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "device") == 0
		) {
			ctx->fns.wgpuDeviceGetAdapterInfo = i;
		}

		if (
			strcmp(name, "wgpuDeviceGetFeatures") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "device") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "features") == 0
		) {
			ctx->fns.wgpuDeviceGetFeatures = i;
		}

		if (
			strcmp(name, "wgpuDeviceGetLimits") == 0 &&
			fn->ret_type == KOS_TYPE_U32 &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "device") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "limits") == 0
		) {
			ctx->fns.wgpuDeviceGetLimits = i;
		}

		if (
			strcmp(name, "wgpuDeviceGetLostFuture") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "device") == 0
		) {
			ctx->fns.wgpuDeviceGetLostFuture = i;
		}

		if (
			strcmp(name, "wgpuDeviceGetQueue") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "device") == 0
		) {
			ctx->fns.wgpuDeviceGetQueue = i;
		}

		if (
			strcmp(name, "wgpuDeviceHasFeature") == 0 &&
			fn->ret_type == KOS_TYPE_BOOL &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "device") == 0 &&
			fn->params[1].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[1].name, "feature") == 0
		) {
			ctx->fns.wgpuDeviceHasFeature = i;
		}

		if (
			strcmp(name, "wgpuDevicePopErrorScope") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "device") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "callbackInfo") == 0
		) {
			ctx->fns.wgpuDevicePopErrorScope = i;
		}

		if (
			strcmp(name, "wgpuDevicePushErrorScope") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "device") == 0 &&
			fn->params[1].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[1].name, "filter") == 0
		) {
			ctx->fns.wgpuDevicePushErrorScope = i;
		}

		if (
			strcmp(name, "wgpuDeviceSetLabel") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "device") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "label") == 0
		) {
			ctx->fns.wgpuDeviceSetLabel = i;
		}

		if (
			strcmp(name, "wgpuDeviceAddRef") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "device") == 0
		) {
			ctx->fns.wgpuDeviceAddRef = i;
		}

		if (
			strcmp(name, "wgpuDeviceRelease") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "device") == 0
		) {
			ctx->fns.wgpuDeviceRelease = i;
		}

		if (
			strcmp(name, "wgpuInstanceCreateSurface") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "instance") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "descriptor") == 0
		) {
			ctx->fns.wgpuInstanceCreateSurface = i;
		}

		if (
			strcmp(name, "wgpuInstanceGetWGSLLanguageFeatures") == 0 &&
			fn->ret_type == KOS_TYPE_U32 &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "instance") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "features") == 0
		) {
			ctx->fns.wgpuInstanceGetWGSLLanguageFeatures = i;
		}

		if (
			strcmp(name, "wgpuInstanceProcessEvents") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "instance") == 0
		) {
			ctx->fns.wgpuInstanceProcessEvents = i;
		}

		if (
			strcmp(name, "wgpuInstanceRequestAdapter") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 3 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "instance") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "options") == 0 &&
			fn->params[2].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[2].name, "callbackInfo") == 0
		) {
			ctx->fns.wgpuInstanceRequestAdapter = i;
		}

		if (
			strcmp(name, "wgpuInstanceWaitAny") == 0 &&
			fn->ret_type == KOS_TYPE_U32 &&
			fn->param_count == 4 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "instance") == 0 &&
			fn->params[1].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[1].name, "futureCount") == 0 &&
			fn->params[2].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[2].name, "futures") == 0 &&
			fn->params[3].type == KOS_TYPE_U64 &&
			strcmp((char*) fn->params[3].name, "timeoutNS") == 0
		) {
			ctx->fns.wgpuInstanceWaitAny = i;
		}

		if (
			strcmp(name, "wgpuInstanceAddRef") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "instance") == 0
		) {
			ctx->fns.wgpuInstanceAddRef = i;
		}

		if (
			strcmp(name, "wgpuInstanceRelease") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "instance") == 0
		) {
			ctx->fns.wgpuInstanceRelease = i;
		}

		if (
			strcmp(name, "wgpuPipelineLayoutSetLabel") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "pipelineLayout") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "label") == 0
		) {
			ctx->fns.wgpuPipelineLayoutSetLabel = i;
		}

		if (
			strcmp(name, "wgpuPipelineLayoutAddRef") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "pipelineLayout") == 0
		) {
			ctx->fns.wgpuPipelineLayoutAddRef = i;
		}

		if (
			strcmp(name, "wgpuPipelineLayoutRelease") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "pipelineLayout") == 0
		) {
			ctx->fns.wgpuPipelineLayoutRelease = i;
		}

		if (
			strcmp(name, "wgpuQuerySetDestroy") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "querySet") == 0
		) {
			ctx->fns.wgpuQuerySetDestroy = i;
		}

		if (
			strcmp(name, "wgpuQuerySetGetCount") == 0 &&
			fn->ret_type == KOS_TYPE_U32 &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "querySet") == 0
		) {
			ctx->fns.wgpuQuerySetGetCount = i;
		}

		if (
			strcmp(name, "wgpuQuerySetGetType") == 0 &&
			fn->ret_type == KOS_TYPE_U32 &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "querySet") == 0
		) {
			ctx->fns.wgpuQuerySetGetType = i;
		}

		if (
			strcmp(name, "wgpuQuerySetSetLabel") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "querySet") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "label") == 0
		) {
			ctx->fns.wgpuQuerySetSetLabel = i;
		}

		if (
			strcmp(name, "wgpuQuerySetAddRef") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "querySet") == 0
		) {
			ctx->fns.wgpuQuerySetAddRef = i;
		}

		if (
			strcmp(name, "wgpuQuerySetRelease") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "querySet") == 0
		) {
			ctx->fns.wgpuQuerySetRelease = i;
		}

		if (
			strcmp(name, "wgpuQueueOnSubmittedWorkDone") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "queue") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "callbackInfo") == 0
		) {
			ctx->fns.wgpuQueueOnSubmittedWorkDone = i;
		}

		if (
			strcmp(name, "wgpuQueueSetLabel") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "queue") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "label") == 0
		) {
			ctx->fns.wgpuQueueSetLabel = i;
		}

		if (
			strcmp(name, "wgpuQueueSubmit") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 3 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "queue") == 0 &&
			fn->params[1].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[1].name, "commandCount") == 0 &&
			fn->params[2].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[2].name, "commands") == 0
		) {
			ctx->fns.wgpuQueueSubmit = i;
		}

		if (
			strcmp(name, "wgpuQueueWriteBuffer") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 5 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "queue") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "buffer") == 0 &&
			fn->params[2].type == KOS_TYPE_U64 &&
			strcmp((char*) fn->params[2].name, "bufferOffset") == 0 &&
			fn->params[3].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[3].name, "data") == 0 &&
			fn->params[4].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[4].name, "size") == 0
		) {
			ctx->fns.wgpuQueueWriteBuffer = i;
		}

		if (
			strcmp(name, "wgpuQueueWriteTexture") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 6 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "queue") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "destination") == 0 &&
			fn->params[2].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[2].name, "data") == 0 &&
			fn->params[3].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[3].name, "dataSize") == 0 &&
			fn->params[4].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[4].name, "dataLayout") == 0 &&
			fn->params[5].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[5].name, "writeSize") == 0
		) {
			ctx->fns.wgpuQueueWriteTexture = i;
		}

		if (
			strcmp(name, "wgpuQueueAddRef") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "queue") == 0
		) {
			ctx->fns.wgpuQueueAddRef = i;
		}

		if (
			strcmp(name, "wgpuQueueRelease") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "queue") == 0
		) {
			ctx->fns.wgpuQueueRelease = i;
		}

		if (
			strcmp(name, "wgpuRenderBundleSetLabel") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderBundle") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "label") == 0
		) {
			ctx->fns.wgpuRenderBundleSetLabel = i;
		}

		if (
			strcmp(name, "wgpuRenderBundleAddRef") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderBundle") == 0
		) {
			ctx->fns.wgpuRenderBundleAddRef = i;
		}

		if (
			strcmp(name, "wgpuRenderBundleRelease") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderBundle") == 0
		) {
			ctx->fns.wgpuRenderBundleRelease = i;
		}

		if (
			strcmp(name, "wgpuRenderBundleEncoderDraw") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 5 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderBundleEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[1].name, "vertexCount") == 0 &&
			fn->params[2].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[2].name, "instanceCount") == 0 &&
			fn->params[3].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[3].name, "firstVertex") == 0 &&
			fn->params[4].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[4].name, "firstInstance") == 0
		) {
			ctx->fns.wgpuRenderBundleEncoderDraw = i;
		}

		if (
			strcmp(name, "wgpuRenderBundleEncoderDrawIndexed") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 6 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderBundleEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[1].name, "indexCount") == 0 &&
			fn->params[2].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[2].name, "instanceCount") == 0 &&
			fn->params[3].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[3].name, "firstIndex") == 0 &&
			fn->params[4].type == KOS_TYPE_I32 &&
			strcmp((char*) fn->params[4].name, "baseVertex") == 0 &&
			fn->params[5].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[5].name, "firstInstance") == 0
		) {
			ctx->fns.wgpuRenderBundleEncoderDrawIndexed = i;
		}

		if (
			strcmp(name, "wgpuRenderBundleEncoderDrawIndexedIndirect") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 3 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderBundleEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "indirectBuffer") == 0 &&
			fn->params[2].type == KOS_TYPE_U64 &&
			strcmp((char*) fn->params[2].name, "indirectOffset") == 0
		) {
			ctx->fns.wgpuRenderBundleEncoderDrawIndexedIndirect = i;
		}

		if (
			strcmp(name, "wgpuRenderBundleEncoderDrawIndirect") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 3 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderBundleEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "indirectBuffer") == 0 &&
			fn->params[2].type == KOS_TYPE_U64 &&
			strcmp((char*) fn->params[2].name, "indirectOffset") == 0
		) {
			ctx->fns.wgpuRenderBundleEncoderDrawIndirect = i;
		}

		if (
			strcmp(name, "wgpuRenderBundleEncoderFinish") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderBundleEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "descriptor") == 0
		) {
			ctx->fns.wgpuRenderBundleEncoderFinish = i;
		}

		if (
			strcmp(name, "wgpuRenderBundleEncoderInsertDebugMarker") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderBundleEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "markerLabel") == 0
		) {
			ctx->fns.wgpuRenderBundleEncoderInsertDebugMarker = i;
		}

		if (
			strcmp(name, "wgpuRenderBundleEncoderPopDebugGroup") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderBundleEncoder") == 0
		) {
			ctx->fns.wgpuRenderBundleEncoderPopDebugGroup = i;
		}

		if (
			strcmp(name, "wgpuRenderBundleEncoderPushDebugGroup") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderBundleEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "groupLabel") == 0
		) {
			ctx->fns.wgpuRenderBundleEncoderPushDebugGroup = i;
		}

		if (
			strcmp(name, "wgpuRenderBundleEncoderSetBindGroup") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 5 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderBundleEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[1].name, "groupIndex") == 0 &&
			fn->params[2].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[2].name, "group") == 0 &&
			fn->params[3].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[3].name, "dynamicOffsetCount") == 0 &&
			fn->params[4].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[4].name, "dynamicOffsets") == 0
		) {
			ctx->fns.wgpuRenderBundleEncoderSetBindGroup = i;
		}

		if (
			strcmp(name, "wgpuRenderBundleEncoderSetIndexBuffer") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 5 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderBundleEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "buffer") == 0 &&
			fn->params[2].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[2].name, "format") == 0 &&
			fn->params[3].type == KOS_TYPE_U64 &&
			strcmp((char*) fn->params[3].name, "offset") == 0 &&
			fn->params[4].type == KOS_TYPE_U64 &&
			strcmp((char*) fn->params[4].name, "size") == 0
		) {
			ctx->fns.wgpuRenderBundleEncoderSetIndexBuffer = i;
		}

		if (
			strcmp(name, "wgpuRenderBundleEncoderSetLabel") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderBundleEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "label") == 0
		) {
			ctx->fns.wgpuRenderBundleEncoderSetLabel = i;
		}

		if (
			strcmp(name, "wgpuRenderBundleEncoderSetPipeline") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderBundleEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "pipeline") == 0
		) {
			ctx->fns.wgpuRenderBundleEncoderSetPipeline = i;
		}

		if (
			strcmp(name, "wgpuRenderBundleEncoderSetVertexBuffer") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 5 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderBundleEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[1].name, "slot") == 0 &&
			fn->params[2].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[2].name, "buffer") == 0 &&
			fn->params[3].type == KOS_TYPE_U64 &&
			strcmp((char*) fn->params[3].name, "offset") == 0 &&
			fn->params[4].type == KOS_TYPE_U64 &&
			strcmp((char*) fn->params[4].name, "size") == 0
		) {
			ctx->fns.wgpuRenderBundleEncoderSetVertexBuffer = i;
		}

		if (
			strcmp(name, "wgpuRenderBundleEncoderAddRef") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderBundleEncoder") == 0
		) {
			ctx->fns.wgpuRenderBundleEncoderAddRef = i;
		}

		if (
			strcmp(name, "wgpuRenderBundleEncoderRelease") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderBundleEncoder") == 0
		) {
			ctx->fns.wgpuRenderBundleEncoderRelease = i;
		}

		if (
			strcmp(name, "wgpuRenderPassEncoderBeginOcclusionQuery") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderPassEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[1].name, "queryIndex") == 0
		) {
			ctx->fns.wgpuRenderPassEncoderBeginOcclusionQuery = i;
		}

		if (
			strcmp(name, "wgpuRenderPassEncoderDraw") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 5 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderPassEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[1].name, "vertexCount") == 0 &&
			fn->params[2].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[2].name, "instanceCount") == 0 &&
			fn->params[3].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[3].name, "firstVertex") == 0 &&
			fn->params[4].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[4].name, "firstInstance") == 0
		) {
			ctx->fns.wgpuRenderPassEncoderDraw = i;
		}

		if (
			strcmp(name, "wgpuRenderPassEncoderDrawIndexed") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 6 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderPassEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[1].name, "indexCount") == 0 &&
			fn->params[2].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[2].name, "instanceCount") == 0 &&
			fn->params[3].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[3].name, "firstIndex") == 0 &&
			fn->params[4].type == KOS_TYPE_I32 &&
			strcmp((char*) fn->params[4].name, "baseVertex") == 0 &&
			fn->params[5].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[5].name, "firstInstance") == 0
		) {
			ctx->fns.wgpuRenderPassEncoderDrawIndexed = i;
		}

		if (
			strcmp(name, "wgpuRenderPassEncoderDrawIndexedIndirect") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 3 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderPassEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "indirectBuffer") == 0 &&
			fn->params[2].type == KOS_TYPE_U64 &&
			strcmp((char*) fn->params[2].name, "indirectOffset") == 0
		) {
			ctx->fns.wgpuRenderPassEncoderDrawIndexedIndirect = i;
		}

		if (
			strcmp(name, "wgpuRenderPassEncoderDrawIndirect") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 3 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderPassEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "indirectBuffer") == 0 &&
			fn->params[2].type == KOS_TYPE_U64 &&
			strcmp((char*) fn->params[2].name, "indirectOffset") == 0
		) {
			ctx->fns.wgpuRenderPassEncoderDrawIndirect = i;
		}

		if (
			strcmp(name, "wgpuRenderPassEncoderEnd") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderPassEncoder") == 0
		) {
			ctx->fns.wgpuRenderPassEncoderEnd = i;
		}

		if (
			strcmp(name, "wgpuRenderPassEncoderEndOcclusionQuery") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderPassEncoder") == 0
		) {
			ctx->fns.wgpuRenderPassEncoderEndOcclusionQuery = i;
		}

		if (
			strcmp(name, "wgpuRenderPassEncoderExecuteBundles") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 3 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderPassEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[1].name, "bundleCount") == 0 &&
			fn->params[2].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[2].name, "bundles") == 0
		) {
			ctx->fns.wgpuRenderPassEncoderExecuteBundles = i;
		}

		if (
			strcmp(name, "wgpuRenderPassEncoderInsertDebugMarker") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderPassEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "markerLabel") == 0
		) {
			ctx->fns.wgpuRenderPassEncoderInsertDebugMarker = i;
		}

		if (
			strcmp(name, "wgpuRenderPassEncoderPopDebugGroup") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderPassEncoder") == 0
		) {
			ctx->fns.wgpuRenderPassEncoderPopDebugGroup = i;
		}

		if (
			strcmp(name, "wgpuRenderPassEncoderPushDebugGroup") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderPassEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "groupLabel") == 0
		) {
			ctx->fns.wgpuRenderPassEncoderPushDebugGroup = i;
		}

		if (
			strcmp(name, "wgpuRenderPassEncoderSetBindGroup") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 5 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderPassEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[1].name, "groupIndex") == 0 &&
			fn->params[2].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[2].name, "group") == 0 &&
			fn->params[3].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[3].name, "dynamicOffsetCount") == 0 &&
			fn->params[4].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[4].name, "dynamicOffsets") == 0
		) {
			ctx->fns.wgpuRenderPassEncoderSetBindGroup = i;
		}

		if (
			strcmp(name, "wgpuRenderPassEncoderSetBlendConstant") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderPassEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "color") == 0
		) {
			ctx->fns.wgpuRenderPassEncoderSetBlendConstant = i;
		}

		if (
			strcmp(name, "wgpuRenderPassEncoderSetIndexBuffer") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 5 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderPassEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "buffer") == 0 &&
			fn->params[2].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[2].name, "format") == 0 &&
			fn->params[3].type == KOS_TYPE_U64 &&
			strcmp((char*) fn->params[3].name, "offset") == 0 &&
			fn->params[4].type == KOS_TYPE_U64 &&
			strcmp((char*) fn->params[4].name, "size") == 0
		) {
			ctx->fns.wgpuRenderPassEncoderSetIndexBuffer = i;
		}

		if (
			strcmp(name, "wgpuRenderPassEncoderSetLabel") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderPassEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "label") == 0
		) {
			ctx->fns.wgpuRenderPassEncoderSetLabel = i;
		}

		if (
			strcmp(name, "wgpuRenderPassEncoderSetPipeline") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderPassEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "pipeline") == 0
		) {
			ctx->fns.wgpuRenderPassEncoderSetPipeline = i;
		}

		if (
			strcmp(name, "wgpuRenderPassEncoderSetScissorRect") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 5 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderPassEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[1].name, "x") == 0 &&
			fn->params[2].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[2].name, "y") == 0 &&
			fn->params[3].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[3].name, "width") == 0 &&
			fn->params[4].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[4].name, "height") == 0
		) {
			ctx->fns.wgpuRenderPassEncoderSetScissorRect = i;
		}

		if (
			strcmp(name, "wgpuRenderPassEncoderSetStencilReference") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderPassEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[1].name, "reference") == 0
		) {
			ctx->fns.wgpuRenderPassEncoderSetStencilReference = i;
		}

		if (
			strcmp(name, "wgpuRenderPassEncoderSetVertexBuffer") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 5 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderPassEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[1].name, "slot") == 0 &&
			fn->params[2].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[2].name, "buffer") == 0 &&
			fn->params[3].type == KOS_TYPE_U64 &&
			strcmp((char*) fn->params[3].name, "offset") == 0 &&
			fn->params[4].type == KOS_TYPE_U64 &&
			strcmp((char*) fn->params[4].name, "size") == 0
		) {
			ctx->fns.wgpuRenderPassEncoderSetVertexBuffer = i;
		}

		if (
			strcmp(name, "wgpuRenderPassEncoderSetViewport") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 7 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderPassEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_F32 &&
			strcmp((char*) fn->params[1].name, "x") == 0 &&
			fn->params[2].type == KOS_TYPE_F32 &&
			strcmp((char*) fn->params[2].name, "y") == 0 &&
			fn->params[3].type == KOS_TYPE_F32 &&
			strcmp((char*) fn->params[3].name, "width") == 0 &&
			fn->params[4].type == KOS_TYPE_F32 &&
			strcmp((char*) fn->params[4].name, "height") == 0 &&
			fn->params[5].type == KOS_TYPE_F32 &&
			strcmp((char*) fn->params[5].name, "minDepth") == 0 &&
			fn->params[6].type == KOS_TYPE_F32 &&
			strcmp((char*) fn->params[6].name, "maxDepth") == 0
		) {
			ctx->fns.wgpuRenderPassEncoderSetViewport = i;
		}

		if (
			strcmp(name, "wgpuRenderPassEncoderAddRef") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderPassEncoder") == 0
		) {
			ctx->fns.wgpuRenderPassEncoderAddRef = i;
		}

		if (
			strcmp(name, "wgpuRenderPassEncoderRelease") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderPassEncoder") == 0
		) {
			ctx->fns.wgpuRenderPassEncoderRelease = i;
		}

		if (
			strcmp(name, "wgpuRenderPipelineGetBindGroupLayout") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderPipeline") == 0 &&
			fn->params[1].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[1].name, "groupIndex") == 0
		) {
			ctx->fns.wgpuRenderPipelineGetBindGroupLayout = i;
		}

		if (
			strcmp(name, "wgpuRenderPipelineSetLabel") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderPipeline") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "label") == 0
		) {
			ctx->fns.wgpuRenderPipelineSetLabel = i;
		}

		if (
			strcmp(name, "wgpuRenderPipelineAddRef") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderPipeline") == 0
		) {
			ctx->fns.wgpuRenderPipelineAddRef = i;
		}

		if (
			strcmp(name, "wgpuRenderPipelineRelease") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderPipeline") == 0
		) {
			ctx->fns.wgpuRenderPipelineRelease = i;
		}

		if (
			strcmp(name, "wgpuSamplerSetLabel") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "sampler") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "label") == 0
		) {
			ctx->fns.wgpuSamplerSetLabel = i;
		}

		if (
			strcmp(name, "wgpuSamplerAddRef") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "sampler") == 0
		) {
			ctx->fns.wgpuSamplerAddRef = i;
		}

		if (
			strcmp(name, "wgpuSamplerRelease") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "sampler") == 0
		) {
			ctx->fns.wgpuSamplerRelease = i;
		}

		if (
			strcmp(name, "wgpuShaderModuleGetCompilationInfo") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "shaderModule") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "callbackInfo") == 0
		) {
			ctx->fns.wgpuShaderModuleGetCompilationInfo = i;
		}

		if (
			strcmp(name, "wgpuShaderModuleSetLabel") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "shaderModule") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "label") == 0
		) {
			ctx->fns.wgpuShaderModuleSetLabel = i;
		}

		if (
			strcmp(name, "wgpuShaderModuleAddRef") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "shaderModule") == 0
		) {
			ctx->fns.wgpuShaderModuleAddRef = i;
		}

		if (
			strcmp(name, "wgpuShaderModuleRelease") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "shaderModule") == 0
		) {
			ctx->fns.wgpuShaderModuleRelease = i;
		}

		if (
			strcmp(name, "wgpuSupportedFeaturesFreeMembers") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[0].name, "supportedFeatures") == 0
		) {
			ctx->fns.wgpuSupportedFeaturesFreeMembers = i;
		}

		if (
			strcmp(name, "wgpuSupportedWGSLLanguageFeaturesFreeMembers") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[0].name, "supportedWGSLLanguageFeatures") == 0
		) {
			ctx->fns.wgpuSupportedWGSLLanguageFeaturesFreeMembers = i;
		}

		if (
			strcmp(name, "wgpuSurfaceConfigure") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "surface") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "config") == 0
		) {
			ctx->fns.wgpuSurfaceConfigure = i;
		}

		if (
			strcmp(name, "wgpuSurfaceGetCapabilities") == 0 &&
			fn->ret_type == KOS_TYPE_U32 &&
			fn->param_count == 3 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "surface") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "adapter") == 0 &&
			fn->params[2].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[2].name, "capabilities") == 0
		) {
			ctx->fns.wgpuSurfaceGetCapabilities = i;
		}

		if (
			strcmp(name, "wgpuSurfaceGetCurrentTexture") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "surface") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "surfaceTexture") == 0
		) {
			ctx->fns.wgpuSurfaceGetCurrentTexture = i;
		}

		if (
			strcmp(name, "wgpuSurfacePresent") == 0 &&
			fn->ret_type == KOS_TYPE_U32 &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "surface") == 0
		) {
			ctx->fns.wgpuSurfacePresent = i;
		}

		if (
			strcmp(name, "wgpuSurfaceUnconfigure") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "surface") == 0
		) {
			ctx->fns.wgpuSurfaceUnconfigure = i;
		}

		if (
			strcmp(name, "wgpuSurfaceAddRef") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "surface") == 0
		) {
			ctx->fns.wgpuSurfaceAddRef = i;
		}

		if (
			strcmp(name, "wgpuSurfaceRelease") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "surface") == 0
		) {
			ctx->fns.wgpuSurfaceRelease = i;
		}

		if (
			strcmp(name, "wgpuSurfaceCapabilitiesFreeMembers") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[0].name, "surfaceCapabilities") == 0
		) {
			ctx->fns.wgpuSurfaceCapabilitiesFreeMembers = i;
		}

		if (
			strcmp(name, "wgpuTextureCreateView") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "texture") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "descriptor") == 0
		) {
			ctx->fns.wgpuTextureCreateView = i;
		}

		if (
			strcmp(name, "wgpuTextureDestroy") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "texture") == 0
		) {
			ctx->fns.wgpuTextureDestroy = i;
		}

		if (
			strcmp(name, "wgpuTextureGetDepthOrArrayLayers") == 0 &&
			fn->ret_type == KOS_TYPE_U32 &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "texture") == 0
		) {
			ctx->fns.wgpuTextureGetDepthOrArrayLayers = i;
		}

		if (
			strcmp(name, "wgpuTextureGetDimension") == 0 &&
			fn->ret_type == KOS_TYPE_U32 &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "texture") == 0
		) {
			ctx->fns.wgpuTextureGetDimension = i;
		}

		if (
			strcmp(name, "wgpuTextureGetFormat") == 0 &&
			fn->ret_type == KOS_TYPE_U32 &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "texture") == 0
		) {
			ctx->fns.wgpuTextureGetFormat = i;
		}

		if (
			strcmp(name, "wgpuTextureGetHeight") == 0 &&
			fn->ret_type == KOS_TYPE_U32 &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "texture") == 0
		) {
			ctx->fns.wgpuTextureGetHeight = i;
		}

		if (
			strcmp(name, "wgpuTextureGetMipLevelCount") == 0 &&
			fn->ret_type == KOS_TYPE_U32 &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "texture") == 0
		) {
			ctx->fns.wgpuTextureGetMipLevelCount = i;
		}

		if (
			strcmp(name, "wgpuTextureGetSampleCount") == 0 &&
			fn->ret_type == KOS_TYPE_U32 &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "texture") == 0
		) {
			ctx->fns.wgpuTextureGetSampleCount = i;
		}

		if (
			strcmp(name, "wgpuTextureGetUsage") == 0 &&
			fn->ret_type == KOS_TYPE_U64 &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "texture") == 0
		) {
			ctx->fns.wgpuTextureGetUsage = i;
		}

		if (
			strcmp(name, "wgpuTextureGetWidth") == 0 &&
			fn->ret_type == KOS_TYPE_U32 &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "texture") == 0
		) {
			ctx->fns.wgpuTextureGetWidth = i;
		}

		if (
			strcmp(name, "wgpuTextureSetLabel") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "texture") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "label") == 0
		) {
			ctx->fns.wgpuTextureSetLabel = i;
		}

		if (
			strcmp(name, "wgpuTextureAddRef") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "texture") == 0
		) {
			ctx->fns.wgpuTextureAddRef = i;
		}

		if (
			strcmp(name, "wgpuTextureRelease") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "texture") == 0
		) {
			ctx->fns.wgpuTextureRelease = i;
		}

		if (
			strcmp(name, "wgpuTextureViewSetLabel") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "textureView") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "label") == 0
		) {
			ctx->fns.wgpuTextureViewSetLabel = i;
		}

		if (
			strcmp(name, "wgpuTextureViewAddRef") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "textureView") == 0
		) {
			ctx->fns.wgpuTextureViewAddRef = i;
		}

		if (
			strcmp(name, "wgpuTextureViewRelease") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "textureView") == 0
		) {
			ctx->fns.wgpuTextureViewRelease = i;
		}

		if (
			strcmp(name, "wgpuGenerateReport") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "instance") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "report") == 0
		) {
			ctx->fns.wgpuGenerateReport = i;
		}

		if (
			strcmp(name, "wgpuInstanceEnumerateAdapters") == 0 &&
			fn->ret_type == KOS_TYPE_U32 &&
			fn->param_count == 3 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "instance") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "options") == 0 &&
			fn->params[2].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[2].name, "adapters") == 0
		) {
			ctx->fns.wgpuInstanceEnumerateAdapters = i;
		}

		if (
			strcmp(name, "wgpuQueueSubmitForIndex") == 0 &&
			fn->ret_type == KOS_TYPE_U64 &&
			fn->param_count == 3 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "queue") == 0 &&
			fn->params[1].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[1].name, "commandCount") == 0 &&
			fn->params[2].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[2].name, "commands") == 0
		) {
			ctx->fns.wgpuQueueSubmitForIndex = i;
		}

		if (
			strcmp(name, "wgpuDevicePoll") == 0 &&
			fn->ret_type == KOS_TYPE_BOOL &&
			fn->param_count == 3 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "device") == 0 &&
			fn->params[1].type == KOS_TYPE_BOOL &&
			strcmp((char*) fn->params[1].name, "wait") == 0 &&
			fn->params[2].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[2].name, "submissionIndex") == 0
		) {
			ctx->fns.wgpuDevicePoll = i;
		}

		if (
			strcmp(name, "wgpuDeviceCreateShaderModuleSpirV") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "device") == 0 &&
			fn->params[1].type == KOS_TYPE_BUF &&
			strcmp((char*) fn->params[1].name, "descriptor") == 0
		) {
			ctx->fns.wgpuDeviceCreateShaderModuleSpirV = i;
		}

		if (
			strcmp(name, "wgpuSetLogCallback") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 2 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "callback") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "userdata") == 0
		) {
			ctx->fns.wgpuSetLogCallback = i;
		}

		if (
			strcmp(name, "wgpuSetLogLevel") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[0].name, "level") == 0
		) {
			ctx->fns.wgpuSetLogLevel = i;
		}

		if (
			strcmp(name, "wgpuGetVersion") == 0 &&
			fn->ret_type == KOS_TYPE_U32 &&
			fn->param_count == 0
		) {
			ctx->fns.wgpuGetVersion = i;
		}

		if (
			strcmp(name, "wgpuRenderPassEncoderSetPushConstants") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 5 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "encoder") == 0 &&
			fn->params[1].type == KOS_TYPE_U64 &&
			strcmp((char*) fn->params[1].name, "stages") == 0 &&
			fn->params[2].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[2].name, "offset") == 0 &&
			fn->params[3].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[3].name, "sizeBytes") == 0 &&
			fn->params[4].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[4].name, "data") == 0
		) {
			ctx->fns.wgpuRenderPassEncoderSetPushConstants = i;
		}

		if (
			strcmp(name, "wgpuComputePassEncoderSetPushConstants") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 4 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "encoder") == 0 &&
			fn->params[1].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[1].name, "offset") == 0 &&
			fn->params[2].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[2].name, "sizeBytes") == 0 &&
			fn->params[3].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[3].name, "data") == 0
		) {
			ctx->fns.wgpuComputePassEncoderSetPushConstants = i;
		}

		if (
			strcmp(name, "wgpuRenderBundleEncoderSetPushConstants") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 5 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "encoder") == 0 &&
			fn->params[1].type == KOS_TYPE_U64 &&
			strcmp((char*) fn->params[1].name, "stages") == 0 &&
			fn->params[2].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[2].name, "offset") == 0 &&
			fn->params[3].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[3].name, "sizeBytes") == 0 &&
			fn->params[4].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[4].name, "data") == 0
		) {
			ctx->fns.wgpuRenderBundleEncoderSetPushConstants = i;
		}

		if (
			strcmp(name, "wgpuRenderPassEncoderMultiDrawIndirect") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 4 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "encoder") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "buffer") == 0 &&
			fn->params[2].type == KOS_TYPE_U64 &&
			strcmp((char*) fn->params[2].name, "offset") == 0 &&
			fn->params[3].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[3].name, "count") == 0
		) {
			ctx->fns.wgpuRenderPassEncoderMultiDrawIndirect = i;
		}

		if (
			strcmp(name, "wgpuRenderPassEncoderMultiDrawIndexedIndirect") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 4 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "encoder") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "buffer") == 0 &&
			fn->params[2].type == KOS_TYPE_U64 &&
			strcmp((char*) fn->params[2].name, "offset") == 0 &&
			fn->params[3].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[3].name, "count") == 0
		) {
			ctx->fns.wgpuRenderPassEncoderMultiDrawIndexedIndirect = i;
		}

		if (
			strcmp(name, "wgpuRenderPassEncoderMultiDrawIndirectCount") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 6 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "encoder") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "buffer") == 0 &&
			fn->params[2].type == KOS_TYPE_U64 &&
			strcmp((char*) fn->params[2].name, "offset") == 0 &&
			fn->params[3].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[3].name, "count_buffer") == 0 &&
			fn->params[4].type == KOS_TYPE_U64 &&
			strcmp((char*) fn->params[4].name, "count_buffer_offset") == 0 &&
			fn->params[5].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[5].name, "max_count") == 0
		) {
			ctx->fns.wgpuRenderPassEncoderMultiDrawIndirectCount = i;
		}

		if (
			strcmp(name, "wgpuRenderPassEncoderMultiDrawIndexedIndirectCount") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 6 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "encoder") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "buffer") == 0 &&
			fn->params[2].type == KOS_TYPE_U64 &&
			strcmp((char*) fn->params[2].name, "offset") == 0 &&
			fn->params[3].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[3].name, "count_buffer") == 0 &&
			fn->params[4].type == KOS_TYPE_U64 &&
			strcmp((char*) fn->params[4].name, "count_buffer_offset") == 0 &&
			fn->params[5].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[5].name, "max_count") == 0
		) {
			ctx->fns.wgpuRenderPassEncoderMultiDrawIndexedIndirectCount = i;
		}

		if (
			strcmp(name, "wgpuComputePassEncoderBeginPipelineStatisticsQuery") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 3 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "computePassEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "querySet") == 0 &&
			fn->params[2].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[2].name, "queryIndex") == 0
		) {
			ctx->fns.wgpuComputePassEncoderBeginPipelineStatisticsQuery = i;
		}

		if (
			strcmp(name, "wgpuComputePassEncoderEndPipelineStatisticsQuery") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "computePassEncoder") == 0
		) {
			ctx->fns.wgpuComputePassEncoderEndPipelineStatisticsQuery = i;
		}

		if (
			strcmp(name, "wgpuRenderPassEncoderBeginPipelineStatisticsQuery") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 3 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderPassEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "querySet") == 0 &&
			fn->params[2].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[2].name, "queryIndex") == 0
		) {
			ctx->fns.wgpuRenderPassEncoderBeginPipelineStatisticsQuery = i;
		}

		if (
			strcmp(name, "wgpuRenderPassEncoderEndPipelineStatisticsQuery") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderPassEncoder") == 0
		) {
			ctx->fns.wgpuRenderPassEncoderEndPipelineStatisticsQuery = i;
		}

		if (
			strcmp(name, "wgpuComputePassEncoderWriteTimestamp") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 3 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "computePassEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "querySet") == 0 &&
			fn->params[2].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[2].name, "queryIndex") == 0
		) {
			ctx->fns.wgpuComputePassEncoderWriteTimestamp = i;
		}

		if (
			strcmp(name, "wgpuRenderPassEncoderWriteTimestamp") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 3 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "renderPassEncoder") == 0 &&
			fn->params[1].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[1].name, "querySet") == 0 &&
			fn->params[2].type == KOS_TYPE_U32 &&
			strcmp((char*) fn->params[2].name, "queryIndex") == 0
		) {
			ctx->fns.wgpuRenderPassEncoderWriteTimestamp = i;
		}
// FN_VALIDATORS:END
		// clang-format on
	}

	for (size_t i = 0; i < sizeof ctx->fns / sizeof(uint32_t); i++) {
		if (((uint32_t*) &ctx->fns)[i] == -1u) {
			ctx->is_conn = false;
			break;
		}
	}
}

static void notif_conn_fail(kos_notif_t const* notif, void* data) {
	(void) notif;
	(void) data;

	fprintf(stderr, "TODO Connection failed, but how do we handle this?\n");
}

static void notif_call_ret(kos_notif_t const* notif, void* data) {
	wgpu_ctx_t const ctx = data;

	if (ctx == NULL || !ctx->is_conn || notif->cookie != ctx->last_cookie) {
		return;
	}

	ctx->last_success = true;
	ctx->last_ret = notif->call_ret.ret;
}

static void notif_call_fail(kos_notif_t const* notif, void* data) {
	wgpu_ctx_t const ctx = data;
	ctx->last_success = false;

	(void) notif;

	fprintf(stderr, "TODO Call failed, but how do we handle this?\n");
}

static component_t comp = {
	.probe = probe,
	.notif_conn = notif_conn,
	.notif_conn_fail = notif_conn_fail,
	.notif_call_ret = notif_call_ret,
	.notif_call_fail = notif_call_fail,
	.vdev_count = 0,
	.vdevs = NULL,
};

// clang-format off
// FNS:BEGIN
WGPUInstance aqua_wgpuCreateInstance(wgpu_ctx_t ctx,WGPU_NULLABLE WGPUInstanceDescriptor const * descriptor) {
	kos_val_t const args[] = {
		{
			.buf.size = sizeof *descriptor,
			.buf.ptr = (void*) descriptor,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuCreateInstance, args);
	kos_flush(true);
	
	return ctx->last_ret.opaque_ptr;
}

WGPUStatus aqua_wgpuGetInstanceCapabilities(wgpu_ctx_t ctx,WGPUInstanceCapabilities * capabilities) {
	kos_val_t const args[] = {
		{
			.buf.size = sizeof *capabilities,
			.buf.ptr = (void*) capabilities,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuGetInstanceCapabilities, args);
	kos_flush(true);
	
	return ctx->last_ret.u32;
}

WGPUProc aqua_wgpuGetProcAddress(wgpu_ctx_t ctx,WGPUStringView procName) {
	kos_val_t const args[] = {
		{
			.buf.size = procName.length,
			.buf.ptr = (void*) procName.data,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuGetProcAddress, args);
	kos_flush(true);
	
	return ctx->last_ret.opaque_ptr;
}

void aqua_wgpuAdapterGetFeatures(wgpu_ctx_t ctx,WGPUAdapter adapter,WGPUSupportedFeatures * features) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) adapter,
		},
		{
			.buf.size = sizeof *features,
			.buf.ptr = (void*) features,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuAdapterGetFeatures, args);
	kos_flush(true);
	
}

WGPUStatus aqua_wgpuAdapterGetInfo(wgpu_ctx_t ctx,WGPUAdapter adapter,WGPUAdapterInfo * info) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) adapter,
		},
		{
			.buf.size = sizeof *info,
			.buf.ptr = (void*) info,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuAdapterGetInfo, args);
	kos_flush(true);
	
	return ctx->last_ret.u32;
}

WGPUStatus aqua_wgpuAdapterGetLimits(wgpu_ctx_t ctx,WGPUAdapter adapter,WGPULimits * limits) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) adapter,
		},
		{
			.buf.size = sizeof *limits,
			.buf.ptr = (void*) limits,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuAdapterGetLimits, args);
	kos_flush(true);
	
	return ctx->last_ret.u32;
}

WGPUBool aqua_wgpuAdapterHasFeature(wgpu_ctx_t ctx,WGPUAdapter adapter,WGPUFeatureName feature) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) adapter,
		},
		{
			.u32 = feature,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuAdapterHasFeature, args);
	kos_flush(true);
	
	return ctx->last_ret.b;
}

WGPUFuture aqua_wgpuAdapterRequestDevice(wgpu_ctx_t ctx,WGPUAdapter adapter,WGPU_NULLABLE WGPUDeviceDescriptor const * descriptor,WGPURequestDeviceCallbackInfo callbackInfo) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) adapter,
		},
		{
			.buf.size = sizeof *descriptor,
			.buf.ptr = (void*) descriptor,
		},
		{
			.buf.size = sizeof callbackInfo,
			.buf.ptr = &callbackInfo,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuAdapterRequestDevice, args);
	kos_flush(true);
	
	return (WGPUFuture) {.id = ctx->last_ret.u64};
}

void aqua_wgpuAdapterAddRef(wgpu_ctx_t ctx,WGPUAdapter adapter) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) adapter,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuAdapterAddRef, args);
	kos_flush(true);
	
}

void aqua_wgpuAdapterRelease(wgpu_ctx_t ctx,WGPUAdapter adapter) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) adapter,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuAdapterRelease, args);
	kos_flush(true);
	
}

void aqua_wgpuAdapterInfoFreeMembers(wgpu_ctx_t ctx,WGPUAdapterInfo adapterInfo) {
	kos_val_t const args[] = {
		{
			.buf.size = sizeof adapterInfo,
			.buf.ptr = &adapterInfo,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuAdapterInfoFreeMembers, args);
	kos_flush(true);
	
}

void aqua_wgpuBindGroupSetLabel(wgpu_ctx_t ctx,WGPUBindGroup bindGroup,WGPUStringView label) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) bindGroup,
		},
		{
			.buf.size = label.length,
			.buf.ptr = (void*) label.data,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuBindGroupSetLabel, args);
	kos_flush(true);
	
}

void aqua_wgpuBindGroupAddRef(wgpu_ctx_t ctx,WGPUBindGroup bindGroup) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) bindGroup,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuBindGroupAddRef, args);
	kos_flush(true);
	
}

void aqua_wgpuBindGroupRelease(wgpu_ctx_t ctx,WGPUBindGroup bindGroup) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) bindGroup,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuBindGroupRelease, args);
	kos_flush(true);
	
}

void aqua_wgpuBindGroupLayoutSetLabel(wgpu_ctx_t ctx,WGPUBindGroupLayout bindGroupLayout,WGPUStringView label) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) bindGroupLayout,
		},
		{
			.buf.size = label.length,
			.buf.ptr = (void*) label.data,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuBindGroupLayoutSetLabel, args);
	kos_flush(true);
	
}

void aqua_wgpuBindGroupLayoutAddRef(wgpu_ctx_t ctx,WGPUBindGroupLayout bindGroupLayout) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) bindGroupLayout,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuBindGroupLayoutAddRef, args);
	kos_flush(true);
	
}

void aqua_wgpuBindGroupLayoutRelease(wgpu_ctx_t ctx,WGPUBindGroupLayout bindGroupLayout) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) bindGroupLayout,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuBindGroupLayoutRelease, args);
	kos_flush(true);
	
}

void aqua_wgpuBufferDestroy(wgpu_ctx_t ctx,WGPUBuffer buffer) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) buffer,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuBufferDestroy, args);
	kos_flush(true);
	
}

void const * aqua_wgpuBufferGetConstMappedRange(wgpu_ctx_t ctx,WGPUBuffer buffer,size_t offset,size_t size) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) buffer,
		},
		{
			.u32 = offset,
		},
		{
			.u32 = size,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuBufferGetConstMappedRange, args);
	kos_flush(true);
	
	return ctx->last_ret.opaque_ptr;
}

WGPUBufferMapState aqua_wgpuBufferGetMapState(wgpu_ctx_t ctx,WGPUBuffer buffer) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) buffer,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuBufferGetMapState, args);
	kos_flush(true);
	
	return ctx->last_ret.u32;
}

void * aqua_wgpuBufferGetMappedRange(wgpu_ctx_t ctx,WGPUBuffer buffer,size_t offset,size_t size) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) buffer,
		},
		{
			.u32 = offset,
		},
		{
			.u32 = size,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuBufferGetMappedRange, args);
	kos_flush(true);
	
	return ctx->last_ret.opaque_ptr;
}

uint64_t aqua_wgpuBufferGetSize(wgpu_ctx_t ctx,WGPUBuffer buffer) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) buffer,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuBufferGetSize, args);
	kos_flush(true);
	
	return ctx->last_ret.u64;
}

WGPUBufferUsage aqua_wgpuBufferGetUsage(wgpu_ctx_t ctx,WGPUBuffer buffer) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) buffer,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuBufferGetUsage, args);
	kos_flush(true);
	
	return ctx->last_ret.u64;
}

WGPUFuture aqua_wgpuBufferMapAsync(wgpu_ctx_t ctx,WGPUBuffer buffer,WGPUMapMode mode,size_t offset,size_t size,WGPUBufferMapCallbackInfo callbackInfo) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) buffer,
		},
		{
			.u64 = mode,
		},
		{
			.u32 = offset,
		},
		{
			.u32 = size,
		},
		{
			.buf.size = sizeof callbackInfo,
			.buf.ptr = &callbackInfo,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuBufferMapAsync, args);
	kos_flush(true);
	
	return (WGPUFuture) {.id = ctx->last_ret.u64};
}

void aqua_wgpuBufferSetLabel(wgpu_ctx_t ctx,WGPUBuffer buffer,WGPUStringView label) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) buffer,
		},
		{
			.buf.size = label.length,
			.buf.ptr = (void*) label.data,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuBufferSetLabel, args);
	kos_flush(true);
	
}

void aqua_wgpuBufferUnmap(wgpu_ctx_t ctx,WGPUBuffer buffer) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) buffer,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuBufferUnmap, args);
	kos_flush(true);
	
}

void aqua_wgpuBufferAddRef(wgpu_ctx_t ctx,WGPUBuffer buffer) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) buffer,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuBufferAddRef, args);
	kos_flush(true);
	
}

void aqua_wgpuBufferRelease(wgpu_ctx_t ctx,WGPUBuffer buffer) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) buffer,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuBufferRelease, args);
	kos_flush(true);
	
}

void aqua_wgpuCommandBufferSetLabel(wgpu_ctx_t ctx,WGPUCommandBuffer commandBuffer,WGPUStringView label) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) commandBuffer,
		},
		{
			.buf.size = label.length,
			.buf.ptr = (void*) label.data,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuCommandBufferSetLabel, args);
	kos_flush(true);
	
}

void aqua_wgpuCommandBufferAddRef(wgpu_ctx_t ctx,WGPUCommandBuffer commandBuffer) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) commandBuffer,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuCommandBufferAddRef, args);
	kos_flush(true);
	
}

void aqua_wgpuCommandBufferRelease(wgpu_ctx_t ctx,WGPUCommandBuffer commandBuffer) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) commandBuffer,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuCommandBufferRelease, args);
	kos_flush(true);
	
}

WGPUComputePassEncoder aqua_wgpuCommandEncoderBeginComputePass(wgpu_ctx_t ctx,WGPUCommandEncoder commandEncoder,WGPU_NULLABLE WGPUComputePassDescriptor const * descriptor) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) commandEncoder,
		},
		{
			.buf.size = sizeof *descriptor,
			.buf.ptr = (void*) descriptor,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuCommandEncoderBeginComputePass, args);
	kos_flush(true);
	
	return ctx->last_ret.opaque_ptr;
}

WGPURenderPassEncoder aqua_wgpuCommandEncoderBeginRenderPass(wgpu_ctx_t ctx,WGPUCommandEncoder commandEncoder,WGPURenderPassDescriptor const * descriptor) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) commandEncoder,
		},
		{
			.buf.size = sizeof *descriptor,
			.buf.ptr = (void*) descriptor,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuCommandEncoderBeginRenderPass, args);
	kos_flush(true);
	
	return ctx->last_ret.opaque_ptr;
}

void aqua_wgpuCommandEncoderClearBuffer(wgpu_ctx_t ctx,WGPUCommandEncoder commandEncoder,WGPUBuffer buffer,uint64_t offset,uint64_t size) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) commandEncoder,
		},
		{
			.opaque_ptr = (void*) buffer,
		},
		{
			.u64 = offset,
		},
		{
			.u64 = size,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuCommandEncoderClearBuffer, args);
	kos_flush(true);
	
}

void aqua_wgpuCommandEncoderCopyBufferToBuffer(wgpu_ctx_t ctx,WGPUCommandEncoder commandEncoder,WGPUBuffer source,uint64_t sourceOffset,WGPUBuffer destination,uint64_t destinationOffset,uint64_t size) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) commandEncoder,
		},
		{
			.opaque_ptr = (void*) source,
		},
		{
			.u64 = sourceOffset,
		},
		{
			.opaque_ptr = (void*) destination,
		},
		{
			.u64 = destinationOffset,
		},
		{
			.u64 = size,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuCommandEncoderCopyBufferToBuffer, args);
	kos_flush(true);
	
}

void aqua_wgpuCommandEncoderCopyBufferToTexture(wgpu_ctx_t ctx,WGPUCommandEncoder commandEncoder,WGPUTexelCopyBufferInfo const * source,WGPUTexelCopyTextureInfo const * destination,WGPUExtent3D const * copySize) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) commandEncoder,
		},
		{
			.buf.size = sizeof *source,
			.buf.ptr = (void*) source,
		},
		{
			.buf.size = sizeof *destination,
			.buf.ptr = (void*) destination,
		},
		{
			.buf.size = sizeof *copySize,
			.buf.ptr = (void*) copySize,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuCommandEncoderCopyBufferToTexture, args);
	kos_flush(true);
	
}

void aqua_wgpuCommandEncoderCopyTextureToBuffer(wgpu_ctx_t ctx,WGPUCommandEncoder commandEncoder,WGPUTexelCopyTextureInfo const * source,WGPUTexelCopyBufferInfo const * destination,WGPUExtent3D const * copySize) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) commandEncoder,
		},
		{
			.buf.size = sizeof *source,
			.buf.ptr = (void*) source,
		},
		{
			.buf.size = sizeof *destination,
			.buf.ptr = (void*) destination,
		},
		{
			.buf.size = sizeof *copySize,
			.buf.ptr = (void*) copySize,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuCommandEncoderCopyTextureToBuffer, args);
	kos_flush(true);
	
}

void aqua_wgpuCommandEncoderCopyTextureToTexture(wgpu_ctx_t ctx,WGPUCommandEncoder commandEncoder,WGPUTexelCopyTextureInfo const * source,WGPUTexelCopyTextureInfo const * destination,WGPUExtent3D const * copySize) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) commandEncoder,
		},
		{
			.buf.size = sizeof *source,
			.buf.ptr = (void*) source,
		},
		{
			.buf.size = sizeof *destination,
			.buf.ptr = (void*) destination,
		},
		{
			.buf.size = sizeof *copySize,
			.buf.ptr = (void*) copySize,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuCommandEncoderCopyTextureToTexture, args);
	kos_flush(true);
	
}

WGPUCommandBuffer aqua_wgpuCommandEncoderFinish(wgpu_ctx_t ctx,WGPUCommandEncoder commandEncoder,WGPU_NULLABLE WGPUCommandBufferDescriptor const * descriptor) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) commandEncoder,
		},
		{
			.buf.size = sizeof *descriptor,
			.buf.ptr = (void*) descriptor,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuCommandEncoderFinish, args);
	kos_flush(true);
	
	return ctx->last_ret.opaque_ptr;
}

void aqua_wgpuCommandEncoderInsertDebugMarker(wgpu_ctx_t ctx,WGPUCommandEncoder commandEncoder,WGPUStringView markerLabel) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) commandEncoder,
		},
		{
			.buf.size = markerLabel.length,
			.buf.ptr = (void*) markerLabel.data,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuCommandEncoderInsertDebugMarker, args);
	kos_flush(true);
	
}

void aqua_wgpuCommandEncoderPopDebugGroup(wgpu_ctx_t ctx,WGPUCommandEncoder commandEncoder) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) commandEncoder,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuCommandEncoderPopDebugGroup, args);
	kos_flush(true);
	
}

void aqua_wgpuCommandEncoderPushDebugGroup(wgpu_ctx_t ctx,WGPUCommandEncoder commandEncoder,WGPUStringView groupLabel) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) commandEncoder,
		},
		{
			.buf.size = groupLabel.length,
			.buf.ptr = (void*) groupLabel.data,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuCommandEncoderPushDebugGroup, args);
	kos_flush(true);
	
}

void aqua_wgpuCommandEncoderResolveQuerySet(wgpu_ctx_t ctx,WGPUCommandEncoder commandEncoder,WGPUQuerySet querySet,uint32_t firstQuery,uint32_t queryCount,WGPUBuffer destination,uint64_t destinationOffset) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) commandEncoder,
		},
		{
			.opaque_ptr = (void*) querySet,
		},
		{
			.u32 = firstQuery,
		},
		{
			.u32 = queryCount,
		},
		{
			.opaque_ptr = (void*) destination,
		},
		{
			.u64 = destinationOffset,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuCommandEncoderResolveQuerySet, args);
	kos_flush(true);
	
}

void aqua_wgpuCommandEncoderSetLabel(wgpu_ctx_t ctx,WGPUCommandEncoder commandEncoder,WGPUStringView label) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) commandEncoder,
		},
		{
			.buf.size = label.length,
			.buf.ptr = (void*) label.data,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuCommandEncoderSetLabel, args);
	kos_flush(true);
	
}

void aqua_wgpuCommandEncoderWriteTimestamp(wgpu_ctx_t ctx,WGPUCommandEncoder commandEncoder,WGPUQuerySet querySet,uint32_t queryIndex) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) commandEncoder,
		},
		{
			.opaque_ptr = (void*) querySet,
		},
		{
			.u32 = queryIndex,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuCommandEncoderWriteTimestamp, args);
	kos_flush(true);
	
}

void aqua_wgpuCommandEncoderAddRef(wgpu_ctx_t ctx,WGPUCommandEncoder commandEncoder) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) commandEncoder,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuCommandEncoderAddRef, args);
	kos_flush(true);
	
}

void aqua_wgpuCommandEncoderRelease(wgpu_ctx_t ctx,WGPUCommandEncoder commandEncoder) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) commandEncoder,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuCommandEncoderRelease, args);
	kos_flush(true);
	
}

void aqua_wgpuComputePassEncoderDispatchWorkgroups(wgpu_ctx_t ctx,WGPUComputePassEncoder computePassEncoder,uint32_t workgroupCountX,uint32_t workgroupCountY,uint32_t workgroupCountZ) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) computePassEncoder,
		},
		{
			.u32 = workgroupCountX,
		},
		{
			.u32 = workgroupCountY,
		},
		{
			.u32 = workgroupCountZ,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuComputePassEncoderDispatchWorkgroups, args);
	kos_flush(true);
	
}

void aqua_wgpuComputePassEncoderDispatchWorkgroupsIndirect(wgpu_ctx_t ctx,WGPUComputePassEncoder computePassEncoder,WGPUBuffer indirectBuffer,uint64_t indirectOffset) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) computePassEncoder,
		},
		{
			.opaque_ptr = (void*) indirectBuffer,
		},
		{
			.u64 = indirectOffset,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuComputePassEncoderDispatchWorkgroupsIndirect, args);
	kos_flush(true);
	
}

void aqua_wgpuComputePassEncoderEnd(wgpu_ctx_t ctx,WGPUComputePassEncoder computePassEncoder) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) computePassEncoder,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuComputePassEncoderEnd, args);
	kos_flush(true);
	
}

void aqua_wgpuComputePassEncoderInsertDebugMarker(wgpu_ctx_t ctx,WGPUComputePassEncoder computePassEncoder,WGPUStringView markerLabel) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) computePassEncoder,
		},
		{
			.buf.size = markerLabel.length,
			.buf.ptr = (void*) markerLabel.data,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuComputePassEncoderInsertDebugMarker, args);
	kos_flush(true);
	
}

void aqua_wgpuComputePassEncoderPopDebugGroup(wgpu_ctx_t ctx,WGPUComputePassEncoder computePassEncoder) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) computePassEncoder,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuComputePassEncoderPopDebugGroup, args);
	kos_flush(true);
	
}

void aqua_wgpuComputePassEncoderPushDebugGroup(wgpu_ctx_t ctx,WGPUComputePassEncoder computePassEncoder,WGPUStringView groupLabel) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) computePassEncoder,
		},
		{
			.buf.size = groupLabel.length,
			.buf.ptr = (void*) groupLabel.data,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuComputePassEncoderPushDebugGroup, args);
	kos_flush(true);
	
}

void aqua_wgpuComputePassEncoderSetBindGroup(wgpu_ctx_t ctx,WGPUComputePassEncoder computePassEncoder,uint32_t groupIndex,WGPU_NULLABLE WGPUBindGroup group,size_t dynamicOffsetCount,uint32_t const * dynamicOffsets) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) computePassEncoder,
		},
		{
			.u32 = groupIndex,
		},
		{
			.opaque_ptr = (void*) group,
		},
		{
			.u32 = dynamicOffsetCount,
		},
		{
			.buf.size = sizeof *dynamicOffsets,
			.buf.ptr = (void*) dynamicOffsets,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuComputePassEncoderSetBindGroup, args);
	kos_flush(true);
	
}

void aqua_wgpuComputePassEncoderSetLabel(wgpu_ctx_t ctx,WGPUComputePassEncoder computePassEncoder,WGPUStringView label) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) computePassEncoder,
		},
		{
			.buf.size = label.length,
			.buf.ptr = (void*) label.data,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuComputePassEncoderSetLabel, args);
	kos_flush(true);
	
}

void aqua_wgpuComputePassEncoderSetPipeline(wgpu_ctx_t ctx,WGPUComputePassEncoder computePassEncoder,WGPUComputePipeline pipeline) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) computePassEncoder,
		},
		{
			.opaque_ptr = (void*) pipeline,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuComputePassEncoderSetPipeline, args);
	kos_flush(true);
	
}

void aqua_wgpuComputePassEncoderAddRef(wgpu_ctx_t ctx,WGPUComputePassEncoder computePassEncoder) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) computePassEncoder,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuComputePassEncoderAddRef, args);
	kos_flush(true);
	
}

void aqua_wgpuComputePassEncoderRelease(wgpu_ctx_t ctx,WGPUComputePassEncoder computePassEncoder) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) computePassEncoder,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuComputePassEncoderRelease, args);
	kos_flush(true);
	
}

WGPUBindGroupLayout aqua_wgpuComputePipelineGetBindGroupLayout(wgpu_ctx_t ctx,WGPUComputePipeline computePipeline,uint32_t groupIndex) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) computePipeline,
		},
		{
			.u32 = groupIndex,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuComputePipelineGetBindGroupLayout, args);
	kos_flush(true);
	
	return ctx->last_ret.opaque_ptr;
}

void aqua_wgpuComputePipelineSetLabel(wgpu_ctx_t ctx,WGPUComputePipeline computePipeline,WGPUStringView label) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) computePipeline,
		},
		{
			.buf.size = label.length,
			.buf.ptr = (void*) label.data,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuComputePipelineSetLabel, args);
	kos_flush(true);
	
}

void aqua_wgpuComputePipelineAddRef(wgpu_ctx_t ctx,WGPUComputePipeline computePipeline) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) computePipeline,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuComputePipelineAddRef, args);
	kos_flush(true);
	
}

void aqua_wgpuComputePipelineRelease(wgpu_ctx_t ctx,WGPUComputePipeline computePipeline) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) computePipeline,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuComputePipelineRelease, args);
	kos_flush(true);
	
}

WGPUBindGroup aqua_wgpuDeviceCreateBindGroup(wgpu_ctx_t ctx,WGPUDevice device,WGPUBindGroupDescriptor const * descriptor) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) device,
		},
		{
			.buf.size = sizeof *descriptor,
			.buf.ptr = (void*) descriptor,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuDeviceCreateBindGroup, args);
	kos_flush(true);
	
	return ctx->last_ret.opaque_ptr;
}

WGPUBindGroupLayout aqua_wgpuDeviceCreateBindGroupLayout(wgpu_ctx_t ctx,WGPUDevice device,WGPUBindGroupLayoutDescriptor const * descriptor) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) device,
		},
		{
			.buf.size = sizeof *descriptor,
			.buf.ptr = (void*) descriptor,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuDeviceCreateBindGroupLayout, args);
	kos_flush(true);
	
	return ctx->last_ret.opaque_ptr;
}

WGPUBuffer aqua_wgpuDeviceCreateBuffer(wgpu_ctx_t ctx,WGPUDevice device,WGPUBufferDescriptor const * descriptor) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) device,
		},
		{
			.buf.size = sizeof *descriptor,
			.buf.ptr = (void*) descriptor,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuDeviceCreateBuffer, args);
	kos_flush(true);
	
	return ctx->last_ret.opaque_ptr;
}

WGPUCommandEncoder aqua_wgpuDeviceCreateCommandEncoder(wgpu_ctx_t ctx,WGPUDevice device,WGPU_NULLABLE WGPUCommandEncoderDescriptor const * descriptor) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) device,
		},
		{
			.buf.size = sizeof *descriptor,
			.buf.ptr = (void*) descriptor,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuDeviceCreateCommandEncoder, args);
	kos_flush(true);
	
	return ctx->last_ret.opaque_ptr;
}

WGPUComputePipeline aqua_wgpuDeviceCreateComputePipeline(wgpu_ctx_t ctx,WGPUDevice device,WGPUComputePipelineDescriptor const * descriptor) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) device,
		},
		{
			.buf.size = sizeof *descriptor,
			.buf.ptr = (void*) descriptor,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuDeviceCreateComputePipeline, args);
	kos_flush(true);
	
	return ctx->last_ret.opaque_ptr;
}

WGPUFuture aqua_wgpuDeviceCreateComputePipelineAsync(wgpu_ctx_t ctx,WGPUDevice device,WGPUComputePipelineDescriptor const * descriptor,WGPUCreateComputePipelineAsyncCallbackInfo callbackInfo) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) device,
		},
		{
			.buf.size = sizeof *descriptor,
			.buf.ptr = (void*) descriptor,
		},
		{
			.buf.size = sizeof callbackInfo,
			.buf.ptr = &callbackInfo,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuDeviceCreateComputePipelineAsync, args);
	kos_flush(true);
	
	return (WGPUFuture) {.id = ctx->last_ret.u64};
}

WGPUPipelineLayout aqua_wgpuDeviceCreatePipelineLayout(wgpu_ctx_t ctx,WGPUDevice device,WGPUPipelineLayoutDescriptor const * descriptor) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) device,
		},
		{
			.buf.size = sizeof *descriptor,
			.buf.ptr = (void*) descriptor,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuDeviceCreatePipelineLayout, args);
	kos_flush(true);
	
	return ctx->last_ret.opaque_ptr;
}

WGPUQuerySet aqua_wgpuDeviceCreateQuerySet(wgpu_ctx_t ctx,WGPUDevice device,WGPUQuerySetDescriptor const * descriptor) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) device,
		},
		{
			.buf.size = sizeof *descriptor,
			.buf.ptr = (void*) descriptor,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuDeviceCreateQuerySet, args);
	kos_flush(true);
	
	return ctx->last_ret.opaque_ptr;
}

WGPURenderBundleEncoder aqua_wgpuDeviceCreateRenderBundleEncoder(wgpu_ctx_t ctx,WGPUDevice device,WGPURenderBundleEncoderDescriptor const * descriptor) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) device,
		},
		{
			.buf.size = sizeof *descriptor,
			.buf.ptr = (void*) descriptor,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuDeviceCreateRenderBundleEncoder, args);
	kos_flush(true);
	
	return ctx->last_ret.opaque_ptr;
}

WGPURenderPipeline aqua_wgpuDeviceCreateRenderPipeline(wgpu_ctx_t ctx,WGPUDevice device,WGPURenderPipelineDescriptor const * descriptor) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) device,
		},
		{
			.buf.size = sizeof *descriptor,
			.buf.ptr = (void*) descriptor,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuDeviceCreateRenderPipeline, args);
	kos_flush(true);
	
	return ctx->last_ret.opaque_ptr;
}

WGPUFuture aqua_wgpuDeviceCreateRenderPipelineAsync(wgpu_ctx_t ctx,WGPUDevice device,WGPURenderPipelineDescriptor const * descriptor,WGPUCreateRenderPipelineAsyncCallbackInfo callbackInfo) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) device,
		},
		{
			.buf.size = sizeof *descriptor,
			.buf.ptr = (void*) descriptor,
		},
		{
			.buf.size = sizeof callbackInfo,
			.buf.ptr = &callbackInfo,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuDeviceCreateRenderPipelineAsync, args);
	kos_flush(true);
	
	return (WGPUFuture) {.id = ctx->last_ret.u64};
}

WGPUSampler aqua_wgpuDeviceCreateSampler(wgpu_ctx_t ctx,WGPUDevice device,WGPU_NULLABLE WGPUSamplerDescriptor const * descriptor) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) device,
		},
		{
			.buf.size = sizeof *descriptor,
			.buf.ptr = (void*) descriptor,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuDeviceCreateSampler, args);
	kos_flush(true);
	
	return ctx->last_ret.opaque_ptr;
}

WGPUShaderModule aqua_wgpuDeviceCreateShaderModule(wgpu_ctx_t ctx,WGPUDevice device,WGPUShaderModuleDescriptor const * descriptor) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) device,
		},
		{
			.buf.size = sizeof *descriptor,
			.buf.ptr = (void*) descriptor,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuDeviceCreateShaderModule, args);
	kos_flush(true);
	
	return ctx->last_ret.opaque_ptr;
}

WGPUTexture aqua_wgpuDeviceCreateTexture(wgpu_ctx_t ctx,WGPUDevice device,WGPUTextureDescriptor const * descriptor) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) device,
		},
		{
			.buf.size = sizeof *descriptor,
			.buf.ptr = (void*) descriptor,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuDeviceCreateTexture, args);
	kos_flush(true);
	
	return ctx->last_ret.opaque_ptr;
}

void aqua_wgpuDeviceDestroy(wgpu_ctx_t ctx,WGPUDevice device) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) device,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuDeviceDestroy, args);
	kos_flush(true);
	
}

WGPUAdapterInfo aqua_wgpuDeviceGetAdapterInfo(wgpu_ctx_t ctx,WGPUDevice device) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) device,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuDeviceGetAdapterInfo, args);
	kos_flush(true);
	
	return *(WGPUAdapterInfo*) ctx->last_ret.buf.ptr;
}

void aqua_wgpuDeviceGetFeatures(wgpu_ctx_t ctx,WGPUDevice device,WGPUSupportedFeatures * features) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) device,
		},
		{
			.buf.size = sizeof *features,
			.buf.ptr = (void*) features,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuDeviceGetFeatures, args);
	kos_flush(true);
	
}

WGPUStatus aqua_wgpuDeviceGetLimits(wgpu_ctx_t ctx,WGPUDevice device,WGPULimits * limits) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) device,
		},
		{
			.buf.size = sizeof *limits,
			.buf.ptr = (void*) limits,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuDeviceGetLimits, args);
	kos_flush(true);
	
	return ctx->last_ret.u32;
}

WGPUFuture aqua_wgpuDeviceGetLostFuture(wgpu_ctx_t ctx,WGPUDevice device) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) device,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuDeviceGetLostFuture, args);
	kos_flush(true);
	
	return (WGPUFuture) {.id = ctx->last_ret.u64};
}

WGPUQueue aqua_wgpuDeviceGetQueue(wgpu_ctx_t ctx,WGPUDevice device) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) device,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuDeviceGetQueue, args);
	kos_flush(true);
	
	return ctx->last_ret.opaque_ptr;
}

WGPUBool aqua_wgpuDeviceHasFeature(wgpu_ctx_t ctx,WGPUDevice device,WGPUFeatureName feature) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) device,
		},
		{
			.u32 = feature,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuDeviceHasFeature, args);
	kos_flush(true);
	
	return ctx->last_ret.b;
}

WGPUFuture aqua_wgpuDevicePopErrorScope(wgpu_ctx_t ctx,WGPUDevice device,WGPUPopErrorScopeCallbackInfo callbackInfo) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) device,
		},
		{
			.buf.size = sizeof callbackInfo,
			.buf.ptr = &callbackInfo,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuDevicePopErrorScope, args);
	kos_flush(true);
	
	return (WGPUFuture) {.id = ctx->last_ret.u64};
}

void aqua_wgpuDevicePushErrorScope(wgpu_ctx_t ctx,WGPUDevice device,WGPUErrorFilter filter) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) device,
		},
		{
			.u32 = filter,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuDevicePushErrorScope, args);
	kos_flush(true);
	
}

void aqua_wgpuDeviceSetLabel(wgpu_ctx_t ctx,WGPUDevice device,WGPUStringView label) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) device,
		},
		{
			.buf.size = label.length,
			.buf.ptr = (void*) label.data,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuDeviceSetLabel, args);
	kos_flush(true);
	
}

void aqua_wgpuDeviceAddRef(wgpu_ctx_t ctx,WGPUDevice device) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) device,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuDeviceAddRef, args);
	kos_flush(true);
	
}

void aqua_wgpuDeviceRelease(wgpu_ctx_t ctx,WGPUDevice device) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) device,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuDeviceRelease, args);
	kos_flush(true);
	
}

WGPUSurface aqua_wgpuInstanceCreateSurface(wgpu_ctx_t ctx,WGPUInstance instance,WGPUSurfaceDescriptor const * descriptor) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) instance,
		},
		{
			.buf.size = sizeof *descriptor,
			.buf.ptr = (void*) descriptor,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuInstanceCreateSurface, args);
	kos_flush(true);
	
	return ctx->last_ret.opaque_ptr;
}

WGPUStatus aqua_wgpuInstanceGetWGSLLanguageFeatures(wgpu_ctx_t ctx,WGPUInstance instance,WGPUSupportedWGSLLanguageFeatures * features) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) instance,
		},
		{
			.buf.size = sizeof *features,
			.buf.ptr = (void*) features,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuInstanceGetWGSLLanguageFeatures, args);
	kos_flush(true);
	
	return ctx->last_ret.u32;
}

void aqua_wgpuInstanceProcessEvents(wgpu_ctx_t ctx,WGPUInstance instance) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) instance,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuInstanceProcessEvents, args);
	kos_flush(true);
	
}

WGPUFuture aqua_wgpuInstanceRequestAdapter(wgpu_ctx_t ctx,WGPUInstance instance,WGPU_NULLABLE WGPURequestAdapterOptions const * options,WGPURequestAdapterCallbackInfo callbackInfo) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) instance,
		},
		{
			.buf.size = sizeof *options,
			.buf.ptr = (void*) options,
		},
		{
			.buf.size = sizeof callbackInfo,
			.buf.ptr = &callbackInfo,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuInstanceRequestAdapter, args);
	kos_flush(true);
	
	return (WGPUFuture) {.id = ctx->last_ret.u64};
}

WGPUWaitStatus aqua_wgpuInstanceWaitAny(wgpu_ctx_t ctx,WGPUInstance instance,size_t futureCount,WGPU_NULLABLE WGPUFutureWaitInfo * futures,uint64_t timeoutNS) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) instance,
		},
		{
			.u32 = futureCount,
		},
		{
			.buf.size = sizeof *futures,
			.buf.ptr = (void*) futures,
		},
		{
			.u64 = timeoutNS,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuInstanceWaitAny, args);
	kos_flush(true);
	
	return ctx->last_ret.u32;
}

void aqua_wgpuInstanceAddRef(wgpu_ctx_t ctx,WGPUInstance instance) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) instance,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuInstanceAddRef, args);
	kos_flush(true);
	
}

void aqua_wgpuInstanceRelease(wgpu_ctx_t ctx,WGPUInstance instance) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) instance,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuInstanceRelease, args);
	kos_flush(true);
	
}

void aqua_wgpuPipelineLayoutSetLabel(wgpu_ctx_t ctx,WGPUPipelineLayout pipelineLayout,WGPUStringView label) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) pipelineLayout,
		},
		{
			.buf.size = label.length,
			.buf.ptr = (void*) label.data,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuPipelineLayoutSetLabel, args);
	kos_flush(true);
	
}

void aqua_wgpuPipelineLayoutAddRef(wgpu_ctx_t ctx,WGPUPipelineLayout pipelineLayout) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) pipelineLayout,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuPipelineLayoutAddRef, args);
	kos_flush(true);
	
}

void aqua_wgpuPipelineLayoutRelease(wgpu_ctx_t ctx,WGPUPipelineLayout pipelineLayout) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) pipelineLayout,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuPipelineLayoutRelease, args);
	kos_flush(true);
	
}

void aqua_wgpuQuerySetDestroy(wgpu_ctx_t ctx,WGPUQuerySet querySet) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) querySet,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuQuerySetDestroy, args);
	kos_flush(true);
	
}

uint32_t aqua_wgpuQuerySetGetCount(wgpu_ctx_t ctx,WGPUQuerySet querySet) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) querySet,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuQuerySetGetCount, args);
	kos_flush(true);
	
	return ctx->last_ret.u32;
}

WGPUQueryType aqua_wgpuQuerySetGetType(wgpu_ctx_t ctx,WGPUQuerySet querySet) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) querySet,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuQuerySetGetType, args);
	kos_flush(true);
	
	return ctx->last_ret.u32;
}

void aqua_wgpuQuerySetSetLabel(wgpu_ctx_t ctx,WGPUQuerySet querySet,WGPUStringView label) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) querySet,
		},
		{
			.buf.size = label.length,
			.buf.ptr = (void*) label.data,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuQuerySetSetLabel, args);
	kos_flush(true);
	
}

void aqua_wgpuQuerySetAddRef(wgpu_ctx_t ctx,WGPUQuerySet querySet) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) querySet,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuQuerySetAddRef, args);
	kos_flush(true);
	
}

void aqua_wgpuQuerySetRelease(wgpu_ctx_t ctx,WGPUQuerySet querySet) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) querySet,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuQuerySetRelease, args);
	kos_flush(true);
	
}

WGPUFuture aqua_wgpuQueueOnSubmittedWorkDone(wgpu_ctx_t ctx,WGPUQueue queue,WGPUQueueWorkDoneCallbackInfo callbackInfo) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) queue,
		},
		{
			.buf.size = sizeof callbackInfo,
			.buf.ptr = &callbackInfo,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuQueueOnSubmittedWorkDone, args);
	kos_flush(true);
	
	return (WGPUFuture) {.id = ctx->last_ret.u64};
}

void aqua_wgpuQueueSetLabel(wgpu_ctx_t ctx,WGPUQueue queue,WGPUStringView label) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) queue,
		},
		{
			.buf.size = label.length,
			.buf.ptr = (void*) label.data,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuQueueSetLabel, args);
	kos_flush(true);
	
}

void aqua_wgpuQueueSubmit(wgpu_ctx_t ctx,WGPUQueue queue,size_t commandCount,WGPUCommandBuffer const * commands) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) queue,
		},
		{
			.u32 = commandCount,
		},
		{
			.buf.size = sizeof *commands,
			.buf.ptr = (void*) commands,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuQueueSubmit, args);
	kos_flush(true);
	
}

void aqua_wgpuQueueWriteBuffer(wgpu_ctx_t ctx,WGPUQueue queue,WGPUBuffer buffer,uint64_t bufferOffset,void const * data,size_t size) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) queue,
		},
		{
			.opaque_ptr = (void*) buffer,
		},
		{
			.u64 = bufferOffset,
		},
		{
			.opaque_ptr = (void*) data,
		},
		{
			.u32 = size,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuQueueWriteBuffer, args);
	kos_flush(true);
	
}

void aqua_wgpuQueueWriteTexture(wgpu_ctx_t ctx,WGPUQueue queue,WGPUTexelCopyTextureInfo const * destination,void const * data,size_t dataSize,WGPUTexelCopyBufferLayout const * dataLayout,WGPUExtent3D const * writeSize) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) queue,
		},
		{
			.buf.size = sizeof *destination,
			.buf.ptr = (void*) destination,
		},
		{
			.opaque_ptr = (void*) data,
		},
		{
			.u32 = dataSize,
		},
		{
			.buf.size = sizeof *dataLayout,
			.buf.ptr = (void*) dataLayout,
		},
		{
			.buf.size = sizeof *writeSize,
			.buf.ptr = (void*) writeSize,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuQueueWriteTexture, args);
	kos_flush(true);
	
}

void aqua_wgpuQueueAddRef(wgpu_ctx_t ctx,WGPUQueue queue) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) queue,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuQueueAddRef, args);
	kos_flush(true);
	
}

void aqua_wgpuQueueRelease(wgpu_ctx_t ctx,WGPUQueue queue) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) queue,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuQueueRelease, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderBundleSetLabel(wgpu_ctx_t ctx,WGPURenderBundle renderBundle,WGPUStringView label) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderBundle,
		},
		{
			.buf.size = label.length,
			.buf.ptr = (void*) label.data,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderBundleSetLabel, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderBundleAddRef(wgpu_ctx_t ctx,WGPURenderBundle renderBundle) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderBundle,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderBundleAddRef, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderBundleRelease(wgpu_ctx_t ctx,WGPURenderBundle renderBundle) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderBundle,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderBundleRelease, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderBundleEncoderDraw(wgpu_ctx_t ctx,WGPURenderBundleEncoder renderBundleEncoder,uint32_t vertexCount,uint32_t instanceCount,uint32_t firstVertex,uint32_t firstInstance) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderBundleEncoder,
		},
		{
			.u32 = vertexCount,
		},
		{
			.u32 = instanceCount,
		},
		{
			.u32 = firstVertex,
		},
		{
			.u32 = firstInstance,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderBundleEncoderDraw, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderBundleEncoderDrawIndexed(wgpu_ctx_t ctx,WGPURenderBundleEncoder renderBundleEncoder,uint32_t indexCount,uint32_t instanceCount,uint32_t firstIndex,int32_t baseVertex,uint32_t firstInstance) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderBundleEncoder,
		},
		{
			.u32 = indexCount,
		},
		{
			.u32 = instanceCount,
		},
		{
			.u32 = firstIndex,
		},
		{
			.i32 = baseVertex,
		},
		{
			.u32 = firstInstance,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderBundleEncoderDrawIndexed, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderBundleEncoderDrawIndexedIndirect(wgpu_ctx_t ctx,WGPURenderBundleEncoder renderBundleEncoder,WGPUBuffer indirectBuffer,uint64_t indirectOffset) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderBundleEncoder,
		},
		{
			.opaque_ptr = (void*) indirectBuffer,
		},
		{
			.u64 = indirectOffset,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderBundleEncoderDrawIndexedIndirect, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderBundleEncoderDrawIndirect(wgpu_ctx_t ctx,WGPURenderBundleEncoder renderBundleEncoder,WGPUBuffer indirectBuffer,uint64_t indirectOffset) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderBundleEncoder,
		},
		{
			.opaque_ptr = (void*) indirectBuffer,
		},
		{
			.u64 = indirectOffset,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderBundleEncoderDrawIndirect, args);
	kos_flush(true);
	
}

WGPURenderBundle aqua_wgpuRenderBundleEncoderFinish(wgpu_ctx_t ctx,WGPURenderBundleEncoder renderBundleEncoder,WGPU_NULLABLE WGPURenderBundleDescriptor const * descriptor) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderBundleEncoder,
		},
		{
			.buf.size = sizeof *descriptor,
			.buf.ptr = (void*) descriptor,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderBundleEncoderFinish, args);
	kos_flush(true);
	
	return ctx->last_ret.opaque_ptr;
}

void aqua_wgpuRenderBundleEncoderInsertDebugMarker(wgpu_ctx_t ctx,WGPURenderBundleEncoder renderBundleEncoder,WGPUStringView markerLabel) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderBundleEncoder,
		},
		{
			.buf.size = markerLabel.length,
			.buf.ptr = (void*) markerLabel.data,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderBundleEncoderInsertDebugMarker, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderBundleEncoderPopDebugGroup(wgpu_ctx_t ctx,WGPURenderBundleEncoder renderBundleEncoder) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderBundleEncoder,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderBundleEncoderPopDebugGroup, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderBundleEncoderPushDebugGroup(wgpu_ctx_t ctx,WGPURenderBundleEncoder renderBundleEncoder,WGPUStringView groupLabel) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderBundleEncoder,
		},
		{
			.buf.size = groupLabel.length,
			.buf.ptr = (void*) groupLabel.data,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderBundleEncoderPushDebugGroup, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderBundleEncoderSetBindGroup(wgpu_ctx_t ctx,WGPURenderBundleEncoder renderBundleEncoder,uint32_t groupIndex,WGPU_NULLABLE WGPUBindGroup group,size_t dynamicOffsetCount,uint32_t const * dynamicOffsets) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderBundleEncoder,
		},
		{
			.u32 = groupIndex,
		},
		{
			.opaque_ptr = (void*) group,
		},
		{
			.u32 = dynamicOffsetCount,
		},
		{
			.buf.size = sizeof *dynamicOffsets,
			.buf.ptr = (void*) dynamicOffsets,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderBundleEncoderSetBindGroup, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderBundleEncoderSetIndexBuffer(wgpu_ctx_t ctx,WGPURenderBundleEncoder renderBundleEncoder,WGPUBuffer buffer,WGPUIndexFormat format,uint64_t offset,uint64_t size) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderBundleEncoder,
		},
		{
			.opaque_ptr = (void*) buffer,
		},
		{
			.u32 = format,
		},
		{
			.u64 = offset,
		},
		{
			.u64 = size,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderBundleEncoderSetIndexBuffer, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderBundleEncoderSetLabel(wgpu_ctx_t ctx,WGPURenderBundleEncoder renderBundleEncoder,WGPUStringView label) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderBundleEncoder,
		},
		{
			.buf.size = label.length,
			.buf.ptr = (void*) label.data,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderBundleEncoderSetLabel, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderBundleEncoderSetPipeline(wgpu_ctx_t ctx,WGPURenderBundleEncoder renderBundleEncoder,WGPURenderPipeline pipeline) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderBundleEncoder,
		},
		{
			.opaque_ptr = (void*) pipeline,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderBundleEncoderSetPipeline, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderBundleEncoderSetVertexBuffer(wgpu_ctx_t ctx,WGPURenderBundleEncoder renderBundleEncoder,uint32_t slot,WGPU_NULLABLE WGPUBuffer buffer,uint64_t offset,uint64_t size) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderBundleEncoder,
		},
		{
			.u32 = slot,
		},
		{
			.opaque_ptr = (void*) buffer,
		},
		{
			.u64 = offset,
		},
		{
			.u64 = size,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderBundleEncoderSetVertexBuffer, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderBundleEncoderAddRef(wgpu_ctx_t ctx,WGPURenderBundleEncoder renderBundleEncoder) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderBundleEncoder,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderBundleEncoderAddRef, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderBundleEncoderRelease(wgpu_ctx_t ctx,WGPURenderBundleEncoder renderBundleEncoder) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderBundleEncoder,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderBundleEncoderRelease, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderPassEncoderBeginOcclusionQuery(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder,uint32_t queryIndex) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderPassEncoder,
		},
		{
			.u32 = queryIndex,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderPassEncoderBeginOcclusionQuery, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderPassEncoderDraw(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder,uint32_t vertexCount,uint32_t instanceCount,uint32_t firstVertex,uint32_t firstInstance) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderPassEncoder,
		},
		{
			.u32 = vertexCount,
		},
		{
			.u32 = instanceCount,
		},
		{
			.u32 = firstVertex,
		},
		{
			.u32 = firstInstance,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderPassEncoderDraw, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderPassEncoderDrawIndexed(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder,uint32_t indexCount,uint32_t instanceCount,uint32_t firstIndex,int32_t baseVertex,uint32_t firstInstance) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderPassEncoder,
		},
		{
			.u32 = indexCount,
		},
		{
			.u32 = instanceCount,
		},
		{
			.u32 = firstIndex,
		},
		{
			.i32 = baseVertex,
		},
		{
			.u32 = firstInstance,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderPassEncoderDrawIndexed, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderPassEncoderDrawIndexedIndirect(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder,WGPUBuffer indirectBuffer,uint64_t indirectOffset) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderPassEncoder,
		},
		{
			.opaque_ptr = (void*) indirectBuffer,
		},
		{
			.u64 = indirectOffset,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderPassEncoderDrawIndexedIndirect, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderPassEncoderDrawIndirect(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder,WGPUBuffer indirectBuffer,uint64_t indirectOffset) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderPassEncoder,
		},
		{
			.opaque_ptr = (void*) indirectBuffer,
		},
		{
			.u64 = indirectOffset,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderPassEncoderDrawIndirect, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderPassEncoderEnd(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderPassEncoder,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderPassEncoderEnd, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderPassEncoderEndOcclusionQuery(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderPassEncoder,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderPassEncoderEndOcclusionQuery, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderPassEncoderExecuteBundles(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder,size_t bundleCount,WGPURenderBundle const * bundles) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderPassEncoder,
		},
		{
			.u32 = bundleCount,
		},
		{
			.buf.size = sizeof *bundles,
			.buf.ptr = (void*) bundles,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderPassEncoderExecuteBundles, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderPassEncoderInsertDebugMarker(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder,WGPUStringView markerLabel) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderPassEncoder,
		},
		{
			.buf.size = markerLabel.length,
			.buf.ptr = (void*) markerLabel.data,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderPassEncoderInsertDebugMarker, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderPassEncoderPopDebugGroup(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderPassEncoder,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderPassEncoderPopDebugGroup, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderPassEncoderPushDebugGroup(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder,WGPUStringView groupLabel) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderPassEncoder,
		},
		{
			.buf.size = groupLabel.length,
			.buf.ptr = (void*) groupLabel.data,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderPassEncoderPushDebugGroup, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderPassEncoderSetBindGroup(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder,uint32_t groupIndex,WGPU_NULLABLE WGPUBindGroup group,size_t dynamicOffsetCount,uint32_t const * dynamicOffsets) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderPassEncoder,
		},
		{
			.u32 = groupIndex,
		},
		{
			.opaque_ptr = (void*) group,
		},
		{
			.u32 = dynamicOffsetCount,
		},
		{
			.buf.size = sizeof *dynamicOffsets,
			.buf.ptr = (void*) dynamicOffsets,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderPassEncoderSetBindGroup, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderPassEncoderSetBlendConstant(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder,WGPUColor const * color) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderPassEncoder,
		},
		{
			.buf.size = sizeof *color,
			.buf.ptr = (void*) color,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderPassEncoderSetBlendConstant, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderPassEncoderSetIndexBuffer(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder,WGPUBuffer buffer,WGPUIndexFormat format,uint64_t offset,uint64_t size) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderPassEncoder,
		},
		{
			.opaque_ptr = (void*) buffer,
		},
		{
			.u32 = format,
		},
		{
			.u64 = offset,
		},
		{
			.u64 = size,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderPassEncoderSetIndexBuffer, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderPassEncoderSetLabel(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder,WGPUStringView label) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderPassEncoder,
		},
		{
			.buf.size = label.length,
			.buf.ptr = (void*) label.data,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderPassEncoderSetLabel, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderPassEncoderSetPipeline(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder,WGPURenderPipeline pipeline) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderPassEncoder,
		},
		{
			.opaque_ptr = (void*) pipeline,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderPassEncoderSetPipeline, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderPassEncoderSetScissorRect(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder,uint32_t x,uint32_t y,uint32_t width,uint32_t height) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderPassEncoder,
		},
		{
			.u32 = x,
		},
		{
			.u32 = y,
		},
		{
			.u32 = width,
		},
		{
			.u32 = height,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderPassEncoderSetScissorRect, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderPassEncoderSetStencilReference(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder,uint32_t reference) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderPassEncoder,
		},
		{
			.u32 = reference,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderPassEncoderSetStencilReference, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderPassEncoderSetVertexBuffer(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder,uint32_t slot,WGPU_NULLABLE WGPUBuffer buffer,uint64_t offset,uint64_t size) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderPassEncoder,
		},
		{
			.u32 = slot,
		},
		{
			.opaque_ptr = (void*) buffer,
		},
		{
			.u64 = offset,
		},
		{
			.u64 = size,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderPassEncoderSetVertexBuffer, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderPassEncoderSetViewport(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder,float x,float y,float width,float height,float minDepth,float maxDepth) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderPassEncoder,
		},
		{
			.f32 = x,
		},
		{
			.f32 = y,
		},
		{
			.f32 = width,
		},
		{
			.f32 = height,
		},
		{
			.f32 = minDepth,
		},
		{
			.f32 = maxDepth,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderPassEncoderSetViewport, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderPassEncoderAddRef(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderPassEncoder,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderPassEncoderAddRef, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderPassEncoderRelease(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderPassEncoder,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderPassEncoderRelease, args);
	kos_flush(true);
	
}

WGPUBindGroupLayout aqua_wgpuRenderPipelineGetBindGroupLayout(wgpu_ctx_t ctx,WGPURenderPipeline renderPipeline,uint32_t groupIndex) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderPipeline,
		},
		{
			.u32 = groupIndex,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderPipelineGetBindGroupLayout, args);
	kos_flush(true);
	
	return ctx->last_ret.opaque_ptr;
}

void aqua_wgpuRenderPipelineSetLabel(wgpu_ctx_t ctx,WGPURenderPipeline renderPipeline,WGPUStringView label) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderPipeline,
		},
		{
			.buf.size = label.length,
			.buf.ptr = (void*) label.data,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderPipelineSetLabel, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderPipelineAddRef(wgpu_ctx_t ctx,WGPURenderPipeline renderPipeline) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderPipeline,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderPipelineAddRef, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderPipelineRelease(wgpu_ctx_t ctx,WGPURenderPipeline renderPipeline) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderPipeline,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderPipelineRelease, args);
	kos_flush(true);
	
}

void aqua_wgpuSamplerSetLabel(wgpu_ctx_t ctx,WGPUSampler sampler,WGPUStringView label) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) sampler,
		},
		{
			.buf.size = label.length,
			.buf.ptr = (void*) label.data,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuSamplerSetLabel, args);
	kos_flush(true);
	
}

void aqua_wgpuSamplerAddRef(wgpu_ctx_t ctx,WGPUSampler sampler) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) sampler,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuSamplerAddRef, args);
	kos_flush(true);
	
}

void aqua_wgpuSamplerRelease(wgpu_ctx_t ctx,WGPUSampler sampler) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) sampler,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuSamplerRelease, args);
	kos_flush(true);
	
}

WGPUFuture aqua_wgpuShaderModuleGetCompilationInfo(wgpu_ctx_t ctx,WGPUShaderModule shaderModule,WGPUCompilationInfoCallbackInfo callbackInfo) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) shaderModule,
		},
		{
			.buf.size = sizeof callbackInfo,
			.buf.ptr = &callbackInfo,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuShaderModuleGetCompilationInfo, args);
	kos_flush(true);
	
	return (WGPUFuture) {.id = ctx->last_ret.u64};
}

void aqua_wgpuShaderModuleSetLabel(wgpu_ctx_t ctx,WGPUShaderModule shaderModule,WGPUStringView label) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) shaderModule,
		},
		{
			.buf.size = label.length,
			.buf.ptr = (void*) label.data,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuShaderModuleSetLabel, args);
	kos_flush(true);
	
}

void aqua_wgpuShaderModuleAddRef(wgpu_ctx_t ctx,WGPUShaderModule shaderModule) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) shaderModule,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuShaderModuleAddRef, args);
	kos_flush(true);
	
}

void aqua_wgpuShaderModuleRelease(wgpu_ctx_t ctx,WGPUShaderModule shaderModule) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) shaderModule,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuShaderModuleRelease, args);
	kos_flush(true);
	
}

void aqua_wgpuSupportedFeaturesFreeMembers(wgpu_ctx_t ctx,WGPUSupportedFeatures supportedFeatures) {
	kos_val_t const args[] = {
		{
			.buf.size = sizeof supportedFeatures,
			.buf.ptr = &supportedFeatures,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuSupportedFeaturesFreeMembers, args);
	kos_flush(true);
	
}

void aqua_wgpuSupportedWGSLLanguageFeaturesFreeMembers(wgpu_ctx_t ctx,WGPUSupportedWGSLLanguageFeatures supportedWGSLLanguageFeatures) {
	kos_val_t const args[] = {
		{
			.buf.size = sizeof supportedWGSLLanguageFeatures,
			.buf.ptr = &supportedWGSLLanguageFeatures,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuSupportedWGSLLanguageFeaturesFreeMembers, args);
	kos_flush(true);
	
}

void aqua_wgpuSurfaceConfigure(wgpu_ctx_t ctx,WGPUSurface surface,WGPUSurfaceConfiguration const * config) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) surface,
		},
		{
			.buf.size = sizeof *config,
			.buf.ptr = (void*) config,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuSurfaceConfigure, args);
	kos_flush(true);
	
}

WGPUStatus aqua_wgpuSurfaceGetCapabilities(wgpu_ctx_t ctx,WGPUSurface surface,WGPUAdapter adapter,WGPUSurfaceCapabilities * capabilities) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) surface,
		},
		{
			.opaque_ptr = (void*) adapter,
		},
		{
			.buf.size = sizeof *capabilities,
			.buf.ptr = (void*) capabilities,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuSurfaceGetCapabilities, args);
	kos_flush(true);
	
	return ctx->last_ret.u32;
}

void aqua_wgpuSurfaceGetCurrentTexture(wgpu_ctx_t ctx,WGPUSurface surface,WGPUSurfaceTexture * surfaceTexture) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) surface,
		},
		{
			.buf.size = sizeof *surfaceTexture,
			.buf.ptr = (void*) surfaceTexture,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuSurfaceGetCurrentTexture, args);
	kos_flush(true);
	
}

WGPUStatus aqua_wgpuSurfacePresent(wgpu_ctx_t ctx,WGPUSurface surface) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) surface,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuSurfacePresent, args);
	kos_flush(true);
	
	return ctx->last_ret.u32;
}

void aqua_wgpuSurfaceUnconfigure(wgpu_ctx_t ctx,WGPUSurface surface) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) surface,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuSurfaceUnconfigure, args);
	kos_flush(true);
	
}

void aqua_wgpuSurfaceAddRef(wgpu_ctx_t ctx,WGPUSurface surface) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) surface,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuSurfaceAddRef, args);
	kos_flush(true);
	
}

void aqua_wgpuSurfaceRelease(wgpu_ctx_t ctx,WGPUSurface surface) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) surface,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuSurfaceRelease, args);
	kos_flush(true);
	
}

void aqua_wgpuSurfaceCapabilitiesFreeMembers(wgpu_ctx_t ctx,WGPUSurfaceCapabilities surfaceCapabilities) {
	kos_val_t const args[] = {
		{
			.buf.size = sizeof surfaceCapabilities,
			.buf.ptr = &surfaceCapabilities,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuSurfaceCapabilitiesFreeMembers, args);
	kos_flush(true);
	
}

WGPUTextureView aqua_wgpuTextureCreateView(wgpu_ctx_t ctx,WGPUTexture texture,WGPU_NULLABLE WGPUTextureViewDescriptor const * descriptor) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) texture,
		},
		{
			.buf.size = sizeof *descriptor,
			.buf.ptr = (void*) descriptor,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuTextureCreateView, args);
	kos_flush(true);
	
	return ctx->last_ret.opaque_ptr;
}

void aqua_wgpuTextureDestroy(wgpu_ctx_t ctx,WGPUTexture texture) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) texture,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuTextureDestroy, args);
	kos_flush(true);
	
}

uint32_t aqua_wgpuTextureGetDepthOrArrayLayers(wgpu_ctx_t ctx,WGPUTexture texture) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) texture,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuTextureGetDepthOrArrayLayers, args);
	kos_flush(true);
	
	return ctx->last_ret.u32;
}

WGPUTextureDimension aqua_wgpuTextureGetDimension(wgpu_ctx_t ctx,WGPUTexture texture) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) texture,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuTextureGetDimension, args);
	kos_flush(true);
	
	return ctx->last_ret.u32;
}

WGPUTextureFormat aqua_wgpuTextureGetFormat(wgpu_ctx_t ctx,WGPUTexture texture) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) texture,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuTextureGetFormat, args);
	kos_flush(true);
	
	return ctx->last_ret.u32;
}

uint32_t aqua_wgpuTextureGetHeight(wgpu_ctx_t ctx,WGPUTexture texture) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) texture,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuTextureGetHeight, args);
	kos_flush(true);
	
	return ctx->last_ret.u32;
}

uint32_t aqua_wgpuTextureGetMipLevelCount(wgpu_ctx_t ctx,WGPUTexture texture) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) texture,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuTextureGetMipLevelCount, args);
	kos_flush(true);
	
	return ctx->last_ret.u32;
}

uint32_t aqua_wgpuTextureGetSampleCount(wgpu_ctx_t ctx,WGPUTexture texture) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) texture,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuTextureGetSampleCount, args);
	kos_flush(true);
	
	return ctx->last_ret.u32;
}

WGPUTextureUsage aqua_wgpuTextureGetUsage(wgpu_ctx_t ctx,WGPUTexture texture) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) texture,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuTextureGetUsage, args);
	kos_flush(true);
	
	return ctx->last_ret.u64;
}

uint32_t aqua_wgpuTextureGetWidth(wgpu_ctx_t ctx,WGPUTexture texture) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) texture,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuTextureGetWidth, args);
	kos_flush(true);
	
	return ctx->last_ret.u32;
}

void aqua_wgpuTextureSetLabel(wgpu_ctx_t ctx,WGPUTexture texture,WGPUStringView label) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) texture,
		},
		{
			.buf.size = label.length,
			.buf.ptr = (void*) label.data,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuTextureSetLabel, args);
	kos_flush(true);
	
}

void aqua_wgpuTextureAddRef(wgpu_ctx_t ctx,WGPUTexture texture) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) texture,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuTextureAddRef, args);
	kos_flush(true);
	
}

void aqua_wgpuTextureRelease(wgpu_ctx_t ctx,WGPUTexture texture) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) texture,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuTextureRelease, args);
	kos_flush(true);
	
}

void aqua_wgpuTextureViewSetLabel(wgpu_ctx_t ctx,WGPUTextureView textureView,WGPUStringView label) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) textureView,
		},
		{
			.buf.size = label.length,
			.buf.ptr = (void*) label.data,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuTextureViewSetLabel, args);
	kos_flush(true);
	
}

void aqua_wgpuTextureViewAddRef(wgpu_ctx_t ctx,WGPUTextureView textureView) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) textureView,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuTextureViewAddRef, args);
	kos_flush(true);
	
}

void aqua_wgpuTextureViewRelease(wgpu_ctx_t ctx,WGPUTextureView textureView) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) textureView,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuTextureViewRelease, args);
	kos_flush(true);
	
}

void aqua_wgpuGenerateReport(wgpu_ctx_t ctx,WGPUInstance instance,WGPUGlobalReport * report) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) instance,
		},
		{
			.buf.size = sizeof *report,
			.buf.ptr = (void*) report,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuGenerateReport, args);
	kos_flush(true);
	
}

size_t aqua_wgpuInstanceEnumerateAdapters(wgpu_ctx_t ctx,WGPUInstance instance,WGPU_NULLABLE WGPUInstanceEnumerateAdapterOptions const * options,WGPUAdapter * adapters) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) instance,
		},
		{
			.buf.size = sizeof *options,
			.buf.ptr = (void*) options,
		},
		{
			.buf.size = sizeof *adapters,
			.buf.ptr = (void*) adapters,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuInstanceEnumerateAdapters, args);
	kos_flush(true);
	
	return ctx->last_ret.u32;
}

WGPUSubmissionIndex aqua_wgpuQueueSubmitForIndex(wgpu_ctx_t ctx,WGPUQueue queue,size_t commandCount,WGPUCommandBuffer const * commands) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) queue,
		},
		{
			.u32 = commandCount,
		},
		{
			.buf.size = sizeof *commands,
			.buf.ptr = (void*) commands,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuQueueSubmitForIndex, args);
	kos_flush(true);
	
	return ctx->last_ret.u64;
}

WGPUBool aqua_wgpuDevicePoll(wgpu_ctx_t ctx,WGPUDevice device,WGPUBool wait,WGPU_NULLABLE WGPUSubmissionIndex const * submissionIndex) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) device,
		},
		{
			.b = wait,
		},
		{
			.buf.size = sizeof *submissionIndex,
			.buf.ptr = (void*) submissionIndex,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuDevicePoll, args);
	kos_flush(true);
	
	return ctx->last_ret.b;
}

WGPUShaderModule aqua_wgpuDeviceCreateShaderModuleSpirV(wgpu_ctx_t ctx,WGPUDevice device,WGPUShaderModuleDescriptorSpirV const * descriptor) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) device,
		},
		{
			.buf.size = sizeof *descriptor,
			.buf.ptr = (void*) descriptor,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuDeviceCreateShaderModuleSpirV, args);
	kos_flush(true);
	
	return ctx->last_ret.opaque_ptr;
}

void aqua_wgpuSetLogCallback(wgpu_ctx_t ctx,WGPULogCallback callback,void * userdata) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) callback,
		},
		{
			.opaque_ptr = (void*) userdata,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuSetLogCallback, args);
	kos_flush(true);
	
}

void aqua_wgpuSetLogLevel(wgpu_ctx_t ctx,WGPULogLevel level) {
	kos_val_t const args[] = {
		{
			.u32 = level,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuSetLogLevel, args);
	kos_flush(true);
	
}

uint32_t aqua_wgpuGetVersion(wgpu_ctx_t ctx) {
	kos_val_t const args[] = {
		
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuGetVersion, args);
	kos_flush(true);
	
	return ctx->last_ret.u32;
}

void aqua_wgpuRenderPassEncoderSetPushConstants(wgpu_ctx_t ctx,WGPURenderPassEncoder encoder,WGPUShaderStage stages,uint32_t offset,uint32_t sizeBytes,void const * data) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) encoder,
		},
		{
			.u64 = stages,
		},
		{
			.u32 = offset,
		},
		{
			.u32 = sizeBytes,
		},
		{
			.opaque_ptr = (void*) data,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderPassEncoderSetPushConstants, args);
	kos_flush(true);
	
}

void aqua_wgpuComputePassEncoderSetPushConstants(wgpu_ctx_t ctx,WGPUComputePassEncoder encoder,uint32_t offset,uint32_t sizeBytes,void const * data) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) encoder,
		},
		{
			.u32 = offset,
		},
		{
			.u32 = sizeBytes,
		},
		{
			.opaque_ptr = (void*) data,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuComputePassEncoderSetPushConstants, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderBundleEncoderSetPushConstants(wgpu_ctx_t ctx,WGPURenderBundleEncoder encoder,WGPUShaderStage stages,uint32_t offset,uint32_t sizeBytes,void const * data) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) encoder,
		},
		{
			.u64 = stages,
		},
		{
			.u32 = offset,
		},
		{
			.u32 = sizeBytes,
		},
		{
			.opaque_ptr = (void*) data,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderBundleEncoderSetPushConstants, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderPassEncoderMultiDrawIndirect(wgpu_ctx_t ctx,WGPURenderPassEncoder encoder,WGPUBuffer buffer,uint64_t offset,uint32_t count) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) encoder,
		},
		{
			.opaque_ptr = (void*) buffer,
		},
		{
			.u64 = offset,
		},
		{
			.u32 = count,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderPassEncoderMultiDrawIndirect, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderPassEncoderMultiDrawIndexedIndirect(wgpu_ctx_t ctx,WGPURenderPassEncoder encoder,WGPUBuffer buffer,uint64_t offset,uint32_t count) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) encoder,
		},
		{
			.opaque_ptr = (void*) buffer,
		},
		{
			.u64 = offset,
		},
		{
			.u32 = count,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderPassEncoderMultiDrawIndexedIndirect, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderPassEncoderMultiDrawIndirectCount(wgpu_ctx_t ctx,WGPURenderPassEncoder encoder,WGPUBuffer buffer,uint64_t offset,WGPUBuffer count_buffer,uint64_t count_buffer_offset,uint32_t max_count) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) encoder,
		},
		{
			.opaque_ptr = (void*) buffer,
		},
		{
			.u64 = offset,
		},
		{
			.opaque_ptr = (void*) count_buffer,
		},
		{
			.u64 = count_buffer_offset,
		},
		{
			.u32 = max_count,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderPassEncoderMultiDrawIndirectCount, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderPassEncoderMultiDrawIndexedIndirectCount(wgpu_ctx_t ctx,WGPURenderPassEncoder encoder,WGPUBuffer buffer,uint64_t offset,WGPUBuffer count_buffer,uint64_t count_buffer_offset,uint32_t max_count) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) encoder,
		},
		{
			.opaque_ptr = (void*) buffer,
		},
		{
			.u64 = offset,
		},
		{
			.opaque_ptr = (void*) count_buffer,
		},
		{
			.u64 = count_buffer_offset,
		},
		{
			.u32 = max_count,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderPassEncoderMultiDrawIndexedIndirectCount, args);
	kos_flush(true);
	
}

void aqua_wgpuComputePassEncoderBeginPipelineStatisticsQuery(wgpu_ctx_t ctx,WGPUComputePassEncoder computePassEncoder,WGPUQuerySet querySet,uint32_t queryIndex) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) computePassEncoder,
		},
		{
			.opaque_ptr = (void*) querySet,
		},
		{
			.u32 = queryIndex,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuComputePassEncoderBeginPipelineStatisticsQuery, args);
	kos_flush(true);
	
}

void aqua_wgpuComputePassEncoderEndPipelineStatisticsQuery(wgpu_ctx_t ctx,WGPUComputePassEncoder computePassEncoder) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) computePassEncoder,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuComputePassEncoderEndPipelineStatisticsQuery, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderPassEncoderBeginPipelineStatisticsQuery(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder,WGPUQuerySet querySet,uint32_t queryIndex) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderPassEncoder,
		},
		{
			.opaque_ptr = (void*) querySet,
		},
		{
			.u32 = queryIndex,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderPassEncoderBeginPipelineStatisticsQuery, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderPassEncoderEndPipelineStatisticsQuery(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderPassEncoder,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderPassEncoderEndPipelineStatisticsQuery, args);
	kos_flush(true);
	
}

void aqua_wgpuComputePassEncoderWriteTimestamp(wgpu_ctx_t ctx,WGPUComputePassEncoder computePassEncoder,WGPUQuerySet querySet,uint32_t queryIndex) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) computePassEncoder,
		},
		{
			.opaque_ptr = (void*) querySet,
		},
		{
			.u32 = queryIndex,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuComputePassEncoderWriteTimestamp, args);
	kos_flush(true);
	
}

void aqua_wgpuRenderPassEncoderWriteTimestamp(wgpu_ctx_t ctx,WGPURenderPassEncoder renderPassEncoder,WGPUQuerySet querySet,uint32_t queryIndex) {
	kos_val_t const args[] = {
		{
			.opaque_ptr = (void*) renderPassEncoder,
		},
		{
			.opaque_ptr = (void*) querySet,
		},
		{
			.u32 = queryIndex,
		}
	};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.wgpuRenderPassEncoderWriteTimestamp, args);
	kos_flush(true);
	
}
// FNS:END
// clang-format on
