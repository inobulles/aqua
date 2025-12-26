// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#include "apple.h"

#include "../win/win.h"
#include "../wm/wm_public.h"

#include <aqua/vdriver.h>
#include <umber.h>

#include <webgpu/webgpu.h>
#include <webgpu/wgpu.h>

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#define SPEC "aquabsd.black.wgpu"
#define VERS 0
#define VDRIVER_HUMAN "WebGPU driver"

static umber_class_t const* cls = NULL;
static vid_t only_vid;

static void init(void) {
	cls = umber_class_new(SPEC, UMBER_LVL_WARN, "aqua.black.wgpu WebGPU VDRIVER.");
	assert(cls != NULL);
}

static void probe(void) {
	assert(VDRIVER.notif_cb != NULL);

	// TODO Report all the GPUs on the system as different VDEVs?

	only_vid = VDRIVER.vdev_id_lo;

	kos_notif_t const notif = {
		.kind = KOS_NOTIF_ATTACH,
		.attach.vdev = {
			.kind = KOS_VDEV_KIND_LOCAL,
			.spec = SPEC,
			.vers = VERS,
			.human = "WebGPU GPU", // TODO Should we get the actual GPU's name here?
			.vdriver_human = VDRIVER_HUMAN,

			.pref = 0,
			.host_id = VDRIVER.host_id,
			.vdev_id = only_vid,
		},
	};

	VDRIVER.notif_cb(&notif, VDRIVER.notif_data);
}

#include "fns.h"

static void conn(kos_cookie_t cookie, vid_t vid, uint64_t conn_id) {
	assert(VDRIVER.notif_cb != NULL);

	if (vid != only_vid) {
		kos_notif_t const notif = {
			.kind = KOS_NOTIF_CONN_FAIL,
			.cookie = cookie,
		};

		VDRIVER.notif_cb(&notif, VDRIVER.notif_data);
		return;
	}

	kos_notif_t const notif = {
		.kind = KOS_NOTIF_CONN,
		.conn_id = conn_id,
		.cookie = cookie,
		.conn = {
			.fn_count = sizeof(FNS) / sizeof(*FNS),
			.fns = FNS,
		},
	};

	VDRIVER.notif_cb(&notif, VDRIVER.notif_data);
}

static void call(kos_cookie_t cookie, vid_t vdev_id, uint64_t conn_id, uint64_t fn_id, kos_val_t const* args) {
	(void) vdev_id;

	assert(VDRIVER.notif_cb != NULL);

	kos_notif_t notif = {
		.kind = KOS_NOTIF_CALL_RET,
		.conn_id = conn_id,
		.cookie = cookie,
	};

	switch (fn_id) {
	case 0: {
		WGPUInstance const inst = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);

		if (inst == NULL) {
			LOG_E(cls, "'surface_from_win' called with non-local or NULL instance.");
			return; // TODO Return KOS_NOTIF_CALL_FAIL.
		}

		aqua_win_t* const win = vdriver_unwrap_local_opaque_ptr(args[1].opaque_ptr);

		if (win == NULL) {
			if (args[1].opaque_ptr.ptr == 0) {
				LOG_E(cls, "'surface_from_win' called with NULL window.");
			}

			else {
				LOG_E(cls, "'surface_from_win' called with non-local window (host_id=%" PRIx64 "). Creating a surface with a non-local window will be supported at some point in the future, but not right now.", args[1].opaque_ptr.host_id);
			}

			return; // TODO Return KOS_NOTIF_CALL_FAIL.
		}

		WGPUSurfaceDescriptor descr;
		WGPUSurface surf = NULL;

		switch (win->kind) {
		case AQUA_WIN_KIND_WAYLAND:;
			WGPUSurfaceSourceWaylandSurface const descr_from_wayland = {
				.chain = (WGPUChainedStruct const) {
					.sType = WGPUSType_SurfaceSourceWaylandSurface,
				},
				.display = win->detail.wayland.display,
				.surface = win->detail.wayland.surface,
			};

			descr.nextInChain = (void*) &descr_from_wayland;
			surf = wgpuInstanceCreateSurface(inst, &descr);

			break;
		case AQUA_WIN_KIND_XCB:;
			WGPUSurfaceSourceXCBWindow const descr_from_xcb = {
				.chain = (WGPUChainedStruct const) {
					.sType = WGPUSType_SurfaceSourceXCBWindow,
				},
				.connection = win->detail.xcb.connection,
				.window = win->detail.xcb.window,
			};

			descr.nextInChain = (void*) &descr_from_xcb;
			surf = wgpuInstanceCreateSurface(inst, &descr);

			break;
		case AQUA_WIN_KIND_XLIB:;
			WGPUSurfaceSourceXlibWindow const descr_from_xlib = {
				.chain = (WGPUChainedStruct const) {
					.sType = WGPUSType_SurfaceSourceXlibWindow,
				},
				.display = win->detail.xlib.display,
				.window = win->detail.xlib.window,
			};

			descr.nextInChain = (void*) &descr_from_xlib;
			surf = wgpuInstanceCreateSurface(inst, &descr);

			break;
		case AQUA_WIN_KIND_APPKIT:;
#if defined(__APPLE__)
			WGPUSurfaceSourceMetalLayer const descr_from_metal = wgpu_get_metal_layer_surface_source(win);

			descr.nextInChain = (void*) &descr_from_metal;
			surf = wgpuInstanceCreateSurface(inst, &descr);
#else
			assert(false);
#endif

			break;
		case AQUA_WIN_KIND_NONE:
		default:
			LOG_E(cls, "Unsupported window kind: %d. This can happen if you are trying to create a surface from a window whose main loop has not yet started.", win->kind);
			return; // TODO Return KOS_NOTIF_CALL_FAIL.
		}

		assert(surf != NULL);
		notif.call_ret.ret.opaque_ptr = vdriver_make_opaque_ptr(surf);

		break;
	}
	case 1: {
		WGPUInstance const inst = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);

		if (inst == NULL) {
			LOG_E(cls, "'device_from_wm' called with non-local or NULL instance.");
			return; // TODO Return KOS_NOTIF_CALL_FAIL.
		}

		aqua_wm_t* const wm = vdriver_unwrap_local_opaque_ptr(args[1].opaque_ptr);

		if (wm == NULL) {
			LOG_E(cls, "'device_from_wm' called with non-local or NULL WM.");
			return; // TODO Return KOS_NOTIF_CALL_FAIL.
		}

		WGPUDevice const dev = wgpuDeviceFromVk(inst, wm->vk_instance, wm->vk_phys_dev, wm->vk_dev, wm->vk_queue_family);
		assert(dev != NULL);
		notif.call_ret.ret.opaque_ptr = vdriver_make_opaque_ptr(dev);

		break;
	}

		// This code is automatically generated by 'vdev/wgpu/gen.py'
		// If you need to update this, read the 'vdev/wgpu/README.md' document.

		// clang-format off
// CALL_HANDLERS:BEGIN
	case 2: {
		WGPUInstanceDescriptor const * const descriptor = (void*) args[0].buf.ptr;
		assert(args[0].buf.size == sizeof *descriptor);
		notif.call_ret.ret.opaque_ptr = vdriver_make_opaque_ptr(wgpuCreateInstance(descriptor));
		break;
	}
	case 3: {
		WGPUInstanceCapabilities * const capabilities = (void*) args[0].buf.ptr;
		assert(args[0].buf.size == sizeof *capabilities);
		notif.call_ret.ret.u32 = wgpuGetInstanceCapabilities(capabilities);
		break;
	}
	case 4: {
		WGPUStringView const procName = {
			.data = args[0].buf.ptr,
			.length = args[0].buf.size,
		};
		notif.call_ret.ret.opaque_ptr = vdriver_make_opaque_ptr(wgpuGetProcAddress(procName));
		break;
	}
	case 5: {
		WGPUAdapter const adapter = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUSupportedFeatures * const features = (void*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof *features);
		wgpuAdapterGetFeatures(adapter, features);
		break;
	}
	case 6: {
		WGPUAdapter const adapter = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUAdapterInfo * const info = (void*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof *info);
		notif.call_ret.ret.u32 = wgpuAdapterGetInfo(adapter, info);
		break;
	}
	case 7: {
		WGPUAdapter const adapter = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPULimits * const limits = (void*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof *limits);
		notif.call_ret.ret.u32 = wgpuAdapterGetLimits(adapter, limits);
		break;
	}
	case 8: {
		WGPUAdapter const adapter = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUFeatureName const feature = args[1].u32;
		notif.call_ret.ret.b = wgpuAdapterHasFeature(adapter, feature);
		break;
	}
	case 9: {
		WGPUAdapter const adapter = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUDeviceDescriptor const * const descriptor = (void*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof *descriptor);
		WGPURequestDeviceCallbackInfo const callbackInfo = *(WGPURequestDeviceCallbackInfo*) args[2].buf.ptr;
		assert(args[2].buf.size == sizeof callbackInfo);
		notif.call_ret.ret.u64 = wgpuAdapterRequestDevice(adapter, descriptor, callbackInfo).id;
		break;
	}
	case 10: {
		WGPUAdapter const adapter = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuAdapterAddRef(adapter);
		break;
	}
	case 11: {
		WGPUAdapter const adapter = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuAdapterRelease(adapter);
		break;
	}
	case 12: {
		WGPUAdapterInfo const adapterInfo = *(WGPUAdapterInfo*) vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuAdapterInfoFreeMembers(adapterInfo);
		break;
	}
	case 13: {
		WGPUBindGroup const bindGroup = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUStringView const label = {
			.data = args[1].buf.ptr,
			.length = args[1].buf.size,
		};
		wgpuBindGroupSetLabel(bindGroup, label);
		break;
	}
	case 14: {
		WGPUBindGroup const bindGroup = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuBindGroupAddRef(bindGroup);
		break;
	}
	case 15: {
		WGPUBindGroup const bindGroup = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuBindGroupRelease(bindGroup);
		break;
	}
	case 16: {
		WGPUBindGroupLayout const bindGroupLayout = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUStringView const label = {
			.data = args[1].buf.ptr,
			.length = args[1].buf.size,
		};
		wgpuBindGroupLayoutSetLabel(bindGroupLayout, label);
		break;
	}
	case 17: {
		WGPUBindGroupLayout const bindGroupLayout = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuBindGroupLayoutAddRef(bindGroupLayout);
		break;
	}
	case 18: {
		WGPUBindGroupLayout const bindGroupLayout = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuBindGroupLayoutRelease(bindGroupLayout);
		break;
	}
	case 19: {
		WGPUBuffer const buffer = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuBufferDestroy(buffer);
		break;
	}
	case 20: {
		WGPUBuffer const buffer = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		size_t const offset = args[1].u32;
		size_t const size = args[2].u32;
		notif.call_ret.ret.opaque_ptr = vdriver_make_opaque_ptr(wgpuBufferGetConstMappedRange(buffer, offset, size));
		break;
	}
	case 21: {
		WGPUBuffer const buffer = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		notif.call_ret.ret.u32 = wgpuBufferGetMapState(buffer);
		break;
	}
	case 22: {
		WGPUBuffer const buffer = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		size_t const offset = args[1].u32;
		size_t const size = args[2].u32;
		notif.call_ret.ret.opaque_ptr = vdriver_make_opaque_ptr(wgpuBufferGetMappedRange(buffer, offset, size));
		break;
	}
	case 23: {
		WGPUBuffer const buffer = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		notif.call_ret.ret.u64 = wgpuBufferGetSize(buffer);
		break;
	}
	case 24: {
		WGPUBuffer const buffer = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		notif.call_ret.ret.u64 = wgpuBufferGetUsage(buffer);
		break;
	}
	case 25: {
		WGPUBuffer const buffer = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUMapMode const mode = args[1].u64;
		size_t const offset = args[2].u32;
		size_t const size = args[3].u32;
		WGPUBufferMapCallbackInfo const callbackInfo = *(WGPUBufferMapCallbackInfo*) args[4].buf.ptr;
		assert(args[4].buf.size == sizeof callbackInfo);
		notif.call_ret.ret.u64 = wgpuBufferMapAsync(buffer, mode, offset, size, callbackInfo).id;
		break;
	}
	case 26: {
		WGPUBuffer const buffer = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUStringView const label = {
			.data = args[1].buf.ptr,
			.length = args[1].buf.size,
		};
		wgpuBufferSetLabel(buffer, label);
		break;
	}
	case 27: {
		WGPUBuffer const buffer = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuBufferUnmap(buffer);
		break;
	}
	case 28: {
		WGPUBuffer const buffer = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuBufferAddRef(buffer);
		break;
	}
	case 29: {
		WGPUBuffer const buffer = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuBufferRelease(buffer);
		break;
	}
	case 30: {
		WGPUCommandBuffer const commandBuffer = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUStringView const label = {
			.data = args[1].buf.ptr,
			.length = args[1].buf.size,
		};
		wgpuCommandBufferSetLabel(commandBuffer, label);
		break;
	}
	case 31: {
		WGPUCommandBuffer const commandBuffer = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuCommandBufferAddRef(commandBuffer);
		break;
	}
	case 32: {
		WGPUCommandBuffer const commandBuffer = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuCommandBufferRelease(commandBuffer);
		break;
	}
	case 33: {
		WGPUCommandEncoder const commandEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUComputePassDescriptor const * const descriptor = (void*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof *descriptor);
		notif.call_ret.ret.opaque_ptr = vdriver_make_opaque_ptr(wgpuCommandEncoderBeginComputePass(commandEncoder, descriptor));
		break;
	}
	case 34: {
		WGPUCommandEncoder const commandEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPURenderPassDescriptor const * const descriptor = (void*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof *descriptor);
		notif.call_ret.ret.opaque_ptr = vdriver_make_opaque_ptr(wgpuCommandEncoderBeginRenderPass(commandEncoder, descriptor));
		break;
	}
	case 35: {
		WGPUCommandEncoder const commandEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUBuffer const buffer = vdriver_unwrap_local_opaque_ptr(args[1].opaque_ptr);
		uint64_t const offset = args[2].u64;
		uint64_t const size = args[3].u64;
		wgpuCommandEncoderClearBuffer(commandEncoder, buffer, offset, size);
		break;
	}
	case 36: {
		WGPUCommandEncoder const commandEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUBuffer const source = vdriver_unwrap_local_opaque_ptr(args[1].opaque_ptr);
		uint64_t const sourceOffset = args[2].u64;
		WGPUBuffer const destination = vdriver_unwrap_local_opaque_ptr(args[3].opaque_ptr);
		uint64_t const destinationOffset = args[4].u64;
		uint64_t const size = args[5].u64;
		wgpuCommandEncoderCopyBufferToBuffer(commandEncoder, source, sourceOffset, destination, destinationOffset, size);
		break;
	}
	case 37: {
		WGPUCommandEncoder const commandEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUTexelCopyBufferInfo const * const source = (void*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof *source);
		WGPUTexelCopyTextureInfo const * const destination = (void*) args[2].buf.ptr;
		assert(args[2].buf.size == sizeof *destination);
		WGPUExtent3D const * const copySize = (void*) args[3].buf.ptr;
		assert(args[3].buf.size == sizeof *copySize);
		wgpuCommandEncoderCopyBufferToTexture(commandEncoder, source, destination, copySize);
		break;
	}
	case 38: {
		WGPUCommandEncoder const commandEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUTexelCopyTextureInfo const * const source = (void*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof *source);
		WGPUTexelCopyBufferInfo const * const destination = (void*) args[2].buf.ptr;
		assert(args[2].buf.size == sizeof *destination);
		WGPUExtent3D const * const copySize = (void*) args[3].buf.ptr;
		assert(args[3].buf.size == sizeof *copySize);
		wgpuCommandEncoderCopyTextureToBuffer(commandEncoder, source, destination, copySize);
		break;
	}
	case 39: {
		WGPUCommandEncoder const commandEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUTexelCopyTextureInfo const * const source = (void*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof *source);
		WGPUTexelCopyTextureInfo const * const destination = (void*) args[2].buf.ptr;
		assert(args[2].buf.size == sizeof *destination);
		WGPUExtent3D const * const copySize = (void*) args[3].buf.ptr;
		assert(args[3].buf.size == sizeof *copySize);
		wgpuCommandEncoderCopyTextureToTexture(commandEncoder, source, destination, copySize);
		break;
	}
	case 40: {
		WGPUCommandEncoder const commandEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUCommandBufferDescriptor const * const descriptor = (void*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof *descriptor);
		notif.call_ret.ret.opaque_ptr = vdriver_make_opaque_ptr(wgpuCommandEncoderFinish(commandEncoder, descriptor));
		break;
	}
	case 41: {
		WGPUCommandEncoder const commandEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUStringView const markerLabel = {
			.data = args[1].buf.ptr,
			.length = args[1].buf.size,
		};
		wgpuCommandEncoderInsertDebugMarker(commandEncoder, markerLabel);
		break;
	}
	case 42: {
		WGPUCommandEncoder const commandEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuCommandEncoderPopDebugGroup(commandEncoder);
		break;
	}
	case 43: {
		WGPUCommandEncoder const commandEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUStringView const groupLabel = {
			.data = args[1].buf.ptr,
			.length = args[1].buf.size,
		};
		wgpuCommandEncoderPushDebugGroup(commandEncoder, groupLabel);
		break;
	}
	case 44: {
		WGPUCommandEncoder const commandEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUQuerySet const querySet = vdriver_unwrap_local_opaque_ptr(args[1].opaque_ptr);
		uint32_t const firstQuery = args[2].u32;
		uint32_t const queryCount = args[3].u32;
		WGPUBuffer const destination = vdriver_unwrap_local_opaque_ptr(args[4].opaque_ptr);
		uint64_t const destinationOffset = args[5].u64;
		wgpuCommandEncoderResolveQuerySet(commandEncoder, querySet, firstQuery, queryCount, destination, destinationOffset);
		break;
	}
	case 45: {
		WGPUCommandEncoder const commandEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUStringView const label = {
			.data = args[1].buf.ptr,
			.length = args[1].buf.size,
		};
		wgpuCommandEncoderSetLabel(commandEncoder, label);
		break;
	}
	case 46: {
		WGPUCommandEncoder const commandEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUQuerySet const querySet = vdriver_unwrap_local_opaque_ptr(args[1].opaque_ptr);
		uint32_t const queryIndex = args[2].u32;
		wgpuCommandEncoderWriteTimestamp(commandEncoder, querySet, queryIndex);
		break;
	}
	case 47: {
		WGPUCommandEncoder const commandEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuCommandEncoderAddRef(commandEncoder);
		break;
	}
	case 48: {
		WGPUCommandEncoder const commandEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuCommandEncoderRelease(commandEncoder);
		break;
	}
	case 49: {
		WGPUComputePassEncoder const computePassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		uint32_t const workgroupCountX = args[1].u32;
		uint32_t const workgroupCountY = args[2].u32;
		uint32_t const workgroupCountZ = args[3].u32;
		wgpuComputePassEncoderDispatchWorkgroups(computePassEncoder, workgroupCountX, workgroupCountY, workgroupCountZ);
		break;
	}
	case 50: {
		WGPUComputePassEncoder const computePassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUBuffer const indirectBuffer = vdriver_unwrap_local_opaque_ptr(args[1].opaque_ptr);
		uint64_t const indirectOffset = args[2].u64;
		wgpuComputePassEncoderDispatchWorkgroupsIndirect(computePassEncoder, indirectBuffer, indirectOffset);
		break;
	}
	case 51: {
		WGPUComputePassEncoder const computePassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuComputePassEncoderEnd(computePassEncoder);
		break;
	}
	case 52: {
		WGPUComputePassEncoder const computePassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUStringView const markerLabel = {
			.data = args[1].buf.ptr,
			.length = args[1].buf.size,
		};
		wgpuComputePassEncoderInsertDebugMarker(computePassEncoder, markerLabel);
		break;
	}
	case 53: {
		WGPUComputePassEncoder const computePassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuComputePassEncoderPopDebugGroup(computePassEncoder);
		break;
	}
	case 54: {
		WGPUComputePassEncoder const computePassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUStringView const groupLabel = {
			.data = args[1].buf.ptr,
			.length = args[1].buf.size,
		};
		wgpuComputePassEncoderPushDebugGroup(computePassEncoder, groupLabel);
		break;
	}
	case 55: {
		WGPUComputePassEncoder const computePassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		uint32_t const groupIndex = args[1].u32;
		WGPUBindGroup const group = vdriver_unwrap_local_opaque_ptr(args[2].opaque_ptr);
		size_t const dynamicOffsetCount = args[3].u32;
		uint32_t const * const dynamicOffsets = (void*) args[4].buf.ptr;
		assert(args[4].buf.size == sizeof *dynamicOffsets);
		wgpuComputePassEncoderSetBindGroup(computePassEncoder, groupIndex, group, dynamicOffsetCount, dynamicOffsets);
		break;
	}
	case 56: {
		WGPUComputePassEncoder const computePassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUStringView const label = {
			.data = args[1].buf.ptr,
			.length = args[1].buf.size,
		};
		wgpuComputePassEncoderSetLabel(computePassEncoder, label);
		break;
	}
	case 57: {
		WGPUComputePassEncoder const computePassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUComputePipeline const pipeline = vdriver_unwrap_local_opaque_ptr(args[1].opaque_ptr);
		wgpuComputePassEncoderSetPipeline(computePassEncoder, pipeline);
		break;
	}
	case 58: {
		WGPUComputePassEncoder const computePassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuComputePassEncoderAddRef(computePassEncoder);
		break;
	}
	case 59: {
		WGPUComputePassEncoder const computePassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuComputePassEncoderRelease(computePassEncoder);
		break;
	}
	case 60: {
		WGPUComputePipeline const computePipeline = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		uint32_t const groupIndex = args[1].u32;
		notif.call_ret.ret.opaque_ptr = vdriver_make_opaque_ptr(wgpuComputePipelineGetBindGroupLayout(computePipeline, groupIndex));
		break;
	}
	case 61: {
		WGPUComputePipeline const computePipeline = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUStringView const label = {
			.data = args[1].buf.ptr,
			.length = args[1].buf.size,
		};
		wgpuComputePipelineSetLabel(computePipeline, label);
		break;
	}
	case 62: {
		WGPUComputePipeline const computePipeline = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuComputePipelineAddRef(computePipeline);
		break;
	}
	case 63: {
		WGPUComputePipeline const computePipeline = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuComputePipelineRelease(computePipeline);
		break;
	}
	case 64: {
		WGPUDevice const device = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUBindGroupDescriptor const * const descriptor = (void*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof *descriptor);
		notif.call_ret.ret.opaque_ptr = vdriver_make_opaque_ptr(wgpuDeviceCreateBindGroup(device, descriptor));
		break;
	}
	case 65: {
		WGPUDevice const device = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUBindGroupLayoutDescriptor const * const descriptor = (void*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof *descriptor);
		notif.call_ret.ret.opaque_ptr = vdriver_make_opaque_ptr(wgpuDeviceCreateBindGroupLayout(device, descriptor));
		break;
	}
	case 66: {
		WGPUDevice const device = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUBufferDescriptor const * const descriptor = (void*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof *descriptor);
		notif.call_ret.ret.opaque_ptr = vdriver_make_opaque_ptr(wgpuDeviceCreateBuffer(device, descriptor));
		break;
	}
	case 67: {
		WGPUDevice const device = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUCommandEncoderDescriptor const * const descriptor = (void*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof *descriptor);
		notif.call_ret.ret.opaque_ptr = vdriver_make_opaque_ptr(wgpuDeviceCreateCommandEncoder(device, descriptor));
		break;
	}
	case 68: {
		WGPUDevice const device = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUComputePipelineDescriptor const * const descriptor = (void*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof *descriptor);
		notif.call_ret.ret.opaque_ptr = vdriver_make_opaque_ptr(wgpuDeviceCreateComputePipeline(device, descriptor));
		break;
	}
	case 69: {
		WGPUDevice const device = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUComputePipelineDescriptor const * const descriptor = (void*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof *descriptor);
		WGPUCreateComputePipelineAsyncCallbackInfo const callbackInfo = *(WGPUCreateComputePipelineAsyncCallbackInfo*) args[2].buf.ptr;
		assert(args[2].buf.size == sizeof callbackInfo);
		notif.call_ret.ret.u64 = wgpuDeviceCreateComputePipelineAsync(device, descriptor, callbackInfo).id;
		break;
	}
	case 70: {
		WGPUDevice const device = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUPipelineLayoutDescriptor const * const descriptor = (void*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof *descriptor);
		notif.call_ret.ret.opaque_ptr = vdriver_make_opaque_ptr(wgpuDeviceCreatePipelineLayout(device, descriptor));
		break;
	}
	case 71: {
		WGPUDevice const device = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUQuerySetDescriptor const * const descriptor = (void*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof *descriptor);
		notif.call_ret.ret.opaque_ptr = vdriver_make_opaque_ptr(wgpuDeviceCreateQuerySet(device, descriptor));
		break;
	}
	case 72: {
		WGPUDevice const device = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPURenderBundleEncoderDescriptor const * const descriptor = (void*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof *descriptor);
		notif.call_ret.ret.opaque_ptr = vdriver_make_opaque_ptr(wgpuDeviceCreateRenderBundleEncoder(device, descriptor));
		break;
	}
	case 73: {
		WGPUDevice const device = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPURenderPipelineDescriptor const * const descriptor = (void*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof *descriptor);
		notif.call_ret.ret.opaque_ptr = vdriver_make_opaque_ptr(wgpuDeviceCreateRenderPipeline(device, descriptor));
		break;
	}
	case 74: {
		WGPUDevice const device = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPURenderPipelineDescriptor const * const descriptor = (void*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof *descriptor);
		WGPUCreateRenderPipelineAsyncCallbackInfo const callbackInfo = *(WGPUCreateRenderPipelineAsyncCallbackInfo*) args[2].buf.ptr;
		assert(args[2].buf.size == sizeof callbackInfo);
		notif.call_ret.ret.u64 = wgpuDeviceCreateRenderPipelineAsync(device, descriptor, callbackInfo).id;
		break;
	}
	case 75: {
		WGPUDevice const device = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUSamplerDescriptor const * const descriptor = (void*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof *descriptor);
		notif.call_ret.ret.opaque_ptr = vdriver_make_opaque_ptr(wgpuDeviceCreateSampler(device, descriptor));
		break;
	}
	case 76: {
		WGPUDevice const device = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUShaderModuleDescriptor const * const descriptor = (void*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof *descriptor);
		notif.call_ret.ret.opaque_ptr = vdriver_make_opaque_ptr(wgpuDeviceCreateShaderModule(device, descriptor));
		break;
	}
	case 77: {
		WGPUDevice const device = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUTextureDescriptor const * const descriptor = (void*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof *descriptor);
		notif.call_ret.ret.opaque_ptr = vdriver_make_opaque_ptr(wgpuDeviceCreateTexture(device, descriptor));
		break;
	}
	case 78: {
		WGPUDevice const device = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuDeviceDestroy(device);
		break;
	}
	case 79: {
		WGPUDevice const device = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUAdapterInfo* const ptr = malloc(sizeof(WGPUAdapterInfo));
		assert(ptr != NULL);
		notif.call_ret.ret.buf.ptr = ptr;
		notif.call_ret.ret.buf.size = sizeof(WGPUAdapterInfo);
		*ptr = wgpuDeviceGetAdapterInfo(device);;
		break;
	}
	case 80: {
		WGPUDevice const device = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUSupportedFeatures * const features = (void*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof *features);
		wgpuDeviceGetFeatures(device, features);
		break;
	}
	case 81: {
		WGPUDevice const device = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPULimits * const limits = (void*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof *limits);
		notif.call_ret.ret.u32 = wgpuDeviceGetLimits(device, limits);
		break;
	}
	case 82: {
		WGPUDevice const device = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		notif.call_ret.ret.u64 = wgpuDeviceGetLostFuture(device).id;
		break;
	}
	case 83: {
		WGPUDevice const device = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		notif.call_ret.ret.opaque_ptr = vdriver_make_opaque_ptr(wgpuDeviceGetQueue(device));
		break;
	}
	case 84: {
		WGPUDevice const device = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUFeatureName const feature = args[1].u32;
		notif.call_ret.ret.b = wgpuDeviceHasFeature(device, feature);
		break;
	}
	case 85: {
		WGPUDevice const device = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUPopErrorScopeCallbackInfo const callbackInfo = *(WGPUPopErrorScopeCallbackInfo*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof callbackInfo);
		notif.call_ret.ret.u64 = wgpuDevicePopErrorScope(device, callbackInfo).id;
		break;
	}
	case 86: {
		WGPUDevice const device = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUErrorFilter const filter = args[1].u32;
		wgpuDevicePushErrorScope(device, filter);
		break;
	}
	case 87: {
		WGPUDevice const device = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUStringView const label = {
			.data = args[1].buf.ptr,
			.length = args[1].buf.size,
		};
		wgpuDeviceSetLabel(device, label);
		break;
	}
	case 88: {
		WGPUDevice const device = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuDeviceAddRef(device);
		break;
	}
	case 89: {
		WGPUDevice const device = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuDeviceRelease(device);
		break;
	}
	case 90: {
		WGPUInstance const instance = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUSurfaceDescriptor const * const descriptor = (void*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof *descriptor);
		notif.call_ret.ret.opaque_ptr = vdriver_make_opaque_ptr(wgpuInstanceCreateSurface(instance, descriptor));
		break;
	}
	case 91: {
		WGPUInstance const instance = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUSupportedWGSLLanguageFeatures * const features = (void*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof *features);
		notif.call_ret.ret.u32 = wgpuInstanceGetWGSLLanguageFeatures(instance, features);
		break;
	}
	case 92: {
		WGPUInstance const instance = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuInstanceProcessEvents(instance);
		break;
	}
	case 93: {
		WGPUInstance const instance = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPURequestAdapterOptions const * const options = (void*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof *options);
		WGPURequestAdapterCallbackInfo const callbackInfo = *(WGPURequestAdapterCallbackInfo*) args[2].buf.ptr;
		assert(args[2].buf.size == sizeof callbackInfo);
		notif.call_ret.ret.u64 = wgpuInstanceRequestAdapter(instance, options, callbackInfo).id;
		break;
	}
	case 94: {
		WGPUInstance const instance = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		size_t const futureCount = args[1].u32;
		WGPUFutureWaitInfo * const futures = (void*) args[2].buf.ptr;
		assert(args[2].buf.size == sizeof *futures);
		uint64_t const timeoutNS = args[3].u64;
		notif.call_ret.ret.u32 = wgpuInstanceWaitAny(instance, futureCount, futures, timeoutNS);
		break;
	}
	case 95: {
		WGPUInstance const instance = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuInstanceAddRef(instance);
		break;
	}
	case 96: {
		WGPUInstance const instance = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuInstanceRelease(instance);
		break;
	}
	case 97: {
		WGPUPipelineLayout const pipelineLayout = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUStringView const label = {
			.data = args[1].buf.ptr,
			.length = args[1].buf.size,
		};
		wgpuPipelineLayoutSetLabel(pipelineLayout, label);
		break;
	}
	case 98: {
		WGPUPipelineLayout const pipelineLayout = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuPipelineLayoutAddRef(pipelineLayout);
		break;
	}
	case 99: {
		WGPUPipelineLayout const pipelineLayout = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuPipelineLayoutRelease(pipelineLayout);
		break;
	}
	case 100: {
		WGPUQuerySet const querySet = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuQuerySetDestroy(querySet);
		break;
	}
	case 101: {
		WGPUQuerySet const querySet = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		notif.call_ret.ret.u32 = wgpuQuerySetGetCount(querySet);
		break;
	}
	case 102: {
		WGPUQuerySet const querySet = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		notif.call_ret.ret.u32 = wgpuQuerySetGetType(querySet);
		break;
	}
	case 103: {
		WGPUQuerySet const querySet = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUStringView const label = {
			.data = args[1].buf.ptr,
			.length = args[1].buf.size,
		};
		wgpuQuerySetSetLabel(querySet, label);
		break;
	}
	case 104: {
		WGPUQuerySet const querySet = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuQuerySetAddRef(querySet);
		break;
	}
	case 105: {
		WGPUQuerySet const querySet = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuQuerySetRelease(querySet);
		break;
	}
	case 106: {
		WGPUQueue const queue = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUQueueWorkDoneCallbackInfo const callbackInfo = *(WGPUQueueWorkDoneCallbackInfo*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof callbackInfo);
		notif.call_ret.ret.u64 = wgpuQueueOnSubmittedWorkDone(queue, callbackInfo).id;
		break;
	}
	case 107: {
		WGPUQueue const queue = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUStringView const label = {
			.data = args[1].buf.ptr,
			.length = args[1].buf.size,
		};
		wgpuQueueSetLabel(queue, label);
		break;
	}
	case 108: {
		WGPUQueue const queue = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		size_t const commandCount = args[1].u32;
		WGPUCommandBuffer const * const commands = (void*) args[2].buf.ptr;
		assert(args[2].buf.size == sizeof *commands);
		wgpuQueueSubmit(queue, commandCount, commands);
		break;
	}
	case 109: {
		WGPUQueue const queue = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUBuffer const buffer = vdriver_unwrap_local_opaque_ptr(args[1].opaque_ptr);
		uint64_t const bufferOffset = args[2].u64;
		void const * const data = vdriver_unwrap_local_opaque_ptr(args[3].opaque_ptr);
		size_t const size = args[4].u32;
		wgpuQueueWriteBuffer(queue, buffer, bufferOffset, data, size);
		break;
	}
	case 110: {
		WGPUQueue const queue = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUTexelCopyTextureInfo const * const destination = (void*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof *destination);
		void const * const data = vdriver_unwrap_local_opaque_ptr(args[2].opaque_ptr);
		size_t const dataSize = args[3].u32;
		WGPUTexelCopyBufferLayout const * const dataLayout = (void*) args[4].buf.ptr;
		assert(args[4].buf.size == sizeof *dataLayout);
		WGPUExtent3D const * const writeSize = (void*) args[5].buf.ptr;
		assert(args[5].buf.size == sizeof *writeSize);
		wgpuQueueWriteTexture(queue, destination, data, dataSize, dataLayout, writeSize);
		break;
	}
	case 111: {
		WGPUQueue const queue = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuQueueAddRef(queue);
		break;
	}
	case 112: {
		WGPUQueue const queue = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuQueueRelease(queue);
		break;
	}
	case 113: {
		WGPURenderBundle const renderBundle = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUStringView const label = {
			.data = args[1].buf.ptr,
			.length = args[1].buf.size,
		};
		wgpuRenderBundleSetLabel(renderBundle, label);
		break;
	}
	case 114: {
		WGPURenderBundle const renderBundle = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuRenderBundleAddRef(renderBundle);
		break;
	}
	case 115: {
		WGPURenderBundle const renderBundle = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuRenderBundleRelease(renderBundle);
		break;
	}
	case 116: {
		WGPURenderBundleEncoder const renderBundleEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		uint32_t const vertexCount = args[1].u32;
		uint32_t const instanceCount = args[2].u32;
		uint32_t const firstVertex = args[3].u32;
		uint32_t const firstInstance = args[4].u32;
		wgpuRenderBundleEncoderDraw(renderBundleEncoder, vertexCount, instanceCount, firstVertex, firstInstance);
		break;
	}
	case 117: {
		WGPURenderBundleEncoder const renderBundleEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		uint32_t const indexCount = args[1].u32;
		uint32_t const instanceCount = args[2].u32;
		uint32_t const firstIndex = args[3].u32;
		int32_t const baseVertex = args[4].i32;
		uint32_t const firstInstance = args[5].u32;
		wgpuRenderBundleEncoderDrawIndexed(renderBundleEncoder, indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
		break;
	}
	case 118: {
		WGPURenderBundleEncoder const renderBundleEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUBuffer const indirectBuffer = vdriver_unwrap_local_opaque_ptr(args[1].opaque_ptr);
		uint64_t const indirectOffset = args[2].u64;
		wgpuRenderBundleEncoderDrawIndexedIndirect(renderBundleEncoder, indirectBuffer, indirectOffset);
		break;
	}
	case 119: {
		WGPURenderBundleEncoder const renderBundleEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUBuffer const indirectBuffer = vdriver_unwrap_local_opaque_ptr(args[1].opaque_ptr);
		uint64_t const indirectOffset = args[2].u64;
		wgpuRenderBundleEncoderDrawIndirect(renderBundleEncoder, indirectBuffer, indirectOffset);
		break;
	}
	case 120: {
		WGPURenderBundleEncoder const renderBundleEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPURenderBundleDescriptor const * const descriptor = (void*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof *descriptor);
		notif.call_ret.ret.opaque_ptr = vdriver_make_opaque_ptr(wgpuRenderBundleEncoderFinish(renderBundleEncoder, descriptor));
		break;
	}
	case 121: {
		WGPURenderBundleEncoder const renderBundleEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUStringView const markerLabel = {
			.data = args[1].buf.ptr,
			.length = args[1].buf.size,
		};
		wgpuRenderBundleEncoderInsertDebugMarker(renderBundleEncoder, markerLabel);
		break;
	}
	case 122: {
		WGPURenderBundleEncoder const renderBundleEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuRenderBundleEncoderPopDebugGroup(renderBundleEncoder);
		break;
	}
	case 123: {
		WGPURenderBundleEncoder const renderBundleEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUStringView const groupLabel = {
			.data = args[1].buf.ptr,
			.length = args[1].buf.size,
		};
		wgpuRenderBundleEncoderPushDebugGroup(renderBundleEncoder, groupLabel);
		break;
	}
	case 124: {
		WGPURenderBundleEncoder const renderBundleEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		uint32_t const groupIndex = args[1].u32;
		WGPUBindGroup const group = vdriver_unwrap_local_opaque_ptr(args[2].opaque_ptr);
		size_t const dynamicOffsetCount = args[3].u32;
		uint32_t const * const dynamicOffsets = (void*) args[4].buf.ptr;
		assert(args[4].buf.size == sizeof *dynamicOffsets);
		wgpuRenderBundleEncoderSetBindGroup(renderBundleEncoder, groupIndex, group, dynamicOffsetCount, dynamicOffsets);
		break;
	}
	case 125: {
		WGPURenderBundleEncoder const renderBundleEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUBuffer const buffer = vdriver_unwrap_local_opaque_ptr(args[1].opaque_ptr);
		WGPUIndexFormat const format = args[2].u32;
		uint64_t const offset = args[3].u64;
		uint64_t const size = args[4].u64;
		wgpuRenderBundleEncoderSetIndexBuffer(renderBundleEncoder, buffer, format, offset, size);
		break;
	}
	case 126: {
		WGPURenderBundleEncoder const renderBundleEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUStringView const label = {
			.data = args[1].buf.ptr,
			.length = args[1].buf.size,
		};
		wgpuRenderBundleEncoderSetLabel(renderBundleEncoder, label);
		break;
	}
	case 127: {
		WGPURenderBundleEncoder const renderBundleEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPURenderPipeline const pipeline = vdriver_unwrap_local_opaque_ptr(args[1].opaque_ptr);
		wgpuRenderBundleEncoderSetPipeline(renderBundleEncoder, pipeline);
		break;
	}
	case 128: {
		WGPURenderBundleEncoder const renderBundleEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		uint32_t const slot = args[1].u32;
		WGPUBuffer const buffer = vdriver_unwrap_local_opaque_ptr(args[2].opaque_ptr);
		uint64_t const offset = args[3].u64;
		uint64_t const size = args[4].u64;
		wgpuRenderBundleEncoderSetVertexBuffer(renderBundleEncoder, slot, buffer, offset, size);
		break;
	}
	case 129: {
		WGPURenderBundleEncoder const renderBundleEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuRenderBundleEncoderAddRef(renderBundleEncoder);
		break;
	}
	case 130: {
		WGPURenderBundleEncoder const renderBundleEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuRenderBundleEncoderRelease(renderBundleEncoder);
		break;
	}
	case 131: {
		WGPURenderPassEncoder const renderPassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		uint32_t const queryIndex = args[1].u32;
		wgpuRenderPassEncoderBeginOcclusionQuery(renderPassEncoder, queryIndex);
		break;
	}
	case 132: {
		WGPURenderPassEncoder const renderPassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		uint32_t const vertexCount = args[1].u32;
		uint32_t const instanceCount = args[2].u32;
		uint32_t const firstVertex = args[3].u32;
		uint32_t const firstInstance = args[4].u32;
		wgpuRenderPassEncoderDraw(renderPassEncoder, vertexCount, instanceCount, firstVertex, firstInstance);
		break;
	}
	case 133: {
		WGPURenderPassEncoder const renderPassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		uint32_t const indexCount = args[1].u32;
		uint32_t const instanceCount = args[2].u32;
		uint32_t const firstIndex = args[3].u32;
		int32_t const baseVertex = args[4].i32;
		uint32_t const firstInstance = args[5].u32;
		wgpuRenderPassEncoderDrawIndexed(renderPassEncoder, indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
		break;
	}
	case 134: {
		WGPURenderPassEncoder const renderPassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUBuffer const indirectBuffer = vdriver_unwrap_local_opaque_ptr(args[1].opaque_ptr);
		uint64_t const indirectOffset = args[2].u64;
		wgpuRenderPassEncoderDrawIndexedIndirect(renderPassEncoder, indirectBuffer, indirectOffset);
		break;
	}
	case 135: {
		WGPURenderPassEncoder const renderPassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUBuffer const indirectBuffer = vdriver_unwrap_local_opaque_ptr(args[1].opaque_ptr);
		uint64_t const indirectOffset = args[2].u64;
		wgpuRenderPassEncoderDrawIndirect(renderPassEncoder, indirectBuffer, indirectOffset);
		break;
	}
	case 136: {
		WGPURenderPassEncoder const renderPassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuRenderPassEncoderEnd(renderPassEncoder);
		break;
	}
	case 137: {
		WGPURenderPassEncoder const renderPassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuRenderPassEncoderEndOcclusionQuery(renderPassEncoder);
		break;
	}
	case 138: {
		WGPURenderPassEncoder const renderPassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		size_t const bundleCount = args[1].u32;
		WGPURenderBundle const * const bundles = (void*) args[2].buf.ptr;
		assert(args[2].buf.size == sizeof *bundles);
		wgpuRenderPassEncoderExecuteBundles(renderPassEncoder, bundleCount, bundles);
		break;
	}
	case 139: {
		WGPURenderPassEncoder const renderPassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUStringView const markerLabel = {
			.data = args[1].buf.ptr,
			.length = args[1].buf.size,
		};
		wgpuRenderPassEncoderInsertDebugMarker(renderPassEncoder, markerLabel);
		break;
	}
	case 140: {
		WGPURenderPassEncoder const renderPassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuRenderPassEncoderPopDebugGroup(renderPassEncoder);
		break;
	}
	case 141: {
		WGPURenderPassEncoder const renderPassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUStringView const groupLabel = {
			.data = args[1].buf.ptr,
			.length = args[1].buf.size,
		};
		wgpuRenderPassEncoderPushDebugGroup(renderPassEncoder, groupLabel);
		break;
	}
	case 142: {
		WGPURenderPassEncoder const renderPassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		uint32_t const groupIndex = args[1].u32;
		WGPUBindGroup const group = vdriver_unwrap_local_opaque_ptr(args[2].opaque_ptr);
		size_t const dynamicOffsetCount = args[3].u32;
		uint32_t const * const dynamicOffsets = (void*) args[4].buf.ptr;
		assert(args[4].buf.size == sizeof *dynamicOffsets);
		wgpuRenderPassEncoderSetBindGroup(renderPassEncoder, groupIndex, group, dynamicOffsetCount, dynamicOffsets);
		break;
	}
	case 143: {
		WGPURenderPassEncoder const renderPassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUColor const * const color = (void*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof *color);
		wgpuRenderPassEncoderSetBlendConstant(renderPassEncoder, color);
		break;
	}
	case 144: {
		WGPURenderPassEncoder const renderPassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUBuffer const buffer = vdriver_unwrap_local_opaque_ptr(args[1].opaque_ptr);
		WGPUIndexFormat const format = args[2].u32;
		uint64_t const offset = args[3].u64;
		uint64_t const size = args[4].u64;
		wgpuRenderPassEncoderSetIndexBuffer(renderPassEncoder, buffer, format, offset, size);
		break;
	}
	case 145: {
		WGPURenderPassEncoder const renderPassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUStringView const label = {
			.data = args[1].buf.ptr,
			.length = args[1].buf.size,
		};
		wgpuRenderPassEncoderSetLabel(renderPassEncoder, label);
		break;
	}
	case 146: {
		WGPURenderPassEncoder const renderPassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPURenderPipeline const pipeline = vdriver_unwrap_local_opaque_ptr(args[1].opaque_ptr);
		wgpuRenderPassEncoderSetPipeline(renderPassEncoder, pipeline);
		break;
	}
	case 147: {
		WGPURenderPassEncoder const renderPassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		uint32_t const x = args[1].u32;
		uint32_t const y = args[2].u32;
		uint32_t const width = args[3].u32;
		uint32_t const height = args[4].u32;
		wgpuRenderPassEncoderSetScissorRect(renderPassEncoder, x, y, width, height);
		break;
	}
	case 148: {
		WGPURenderPassEncoder const renderPassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		uint32_t const reference = args[1].u32;
		wgpuRenderPassEncoderSetStencilReference(renderPassEncoder, reference);
		break;
	}
	case 149: {
		WGPURenderPassEncoder const renderPassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		uint32_t const slot = args[1].u32;
		WGPUBuffer const buffer = vdriver_unwrap_local_opaque_ptr(args[2].opaque_ptr);
		uint64_t const offset = args[3].u64;
		uint64_t const size = args[4].u64;
		wgpuRenderPassEncoderSetVertexBuffer(renderPassEncoder, slot, buffer, offset, size);
		break;
	}
	case 150: {
		WGPURenderPassEncoder const renderPassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		float const x = args[1].f32;
		float const y = args[2].f32;
		float const width = args[3].f32;
		float const height = args[4].f32;
		float const minDepth = args[5].f32;
		float const maxDepth = args[6].f32;
		wgpuRenderPassEncoderSetViewport(renderPassEncoder, x, y, width, height, minDepth, maxDepth);
		break;
	}
	case 151: {
		WGPURenderPassEncoder const renderPassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuRenderPassEncoderAddRef(renderPassEncoder);
		break;
	}
	case 152: {
		WGPURenderPassEncoder const renderPassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuRenderPassEncoderRelease(renderPassEncoder);
		break;
	}
	case 153: {
		WGPURenderPipeline const renderPipeline = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		uint32_t const groupIndex = args[1].u32;
		notif.call_ret.ret.opaque_ptr = vdriver_make_opaque_ptr(wgpuRenderPipelineGetBindGroupLayout(renderPipeline, groupIndex));
		break;
	}
	case 154: {
		WGPURenderPipeline const renderPipeline = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUStringView const label = {
			.data = args[1].buf.ptr,
			.length = args[1].buf.size,
		};
		wgpuRenderPipelineSetLabel(renderPipeline, label);
		break;
	}
	case 155: {
		WGPURenderPipeline const renderPipeline = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuRenderPipelineAddRef(renderPipeline);
		break;
	}
	case 156: {
		WGPURenderPipeline const renderPipeline = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuRenderPipelineRelease(renderPipeline);
		break;
	}
	case 157: {
		WGPUSampler const sampler = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUStringView const label = {
			.data = args[1].buf.ptr,
			.length = args[1].buf.size,
		};
		wgpuSamplerSetLabel(sampler, label);
		break;
	}
	case 158: {
		WGPUSampler const sampler = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuSamplerAddRef(sampler);
		break;
	}
	case 159: {
		WGPUSampler const sampler = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuSamplerRelease(sampler);
		break;
	}
	case 160: {
		WGPUShaderModule const shaderModule = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUCompilationInfoCallbackInfo const callbackInfo = *(WGPUCompilationInfoCallbackInfo*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof callbackInfo);
		notif.call_ret.ret.u64 = wgpuShaderModuleGetCompilationInfo(shaderModule, callbackInfo).id;
		break;
	}
	case 161: {
		WGPUShaderModule const shaderModule = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUStringView const label = {
			.data = args[1].buf.ptr,
			.length = args[1].buf.size,
		};
		wgpuShaderModuleSetLabel(shaderModule, label);
		break;
	}
	case 162: {
		WGPUShaderModule const shaderModule = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuShaderModuleAddRef(shaderModule);
		break;
	}
	case 163: {
		WGPUShaderModule const shaderModule = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuShaderModuleRelease(shaderModule);
		break;
	}
	case 164: {
		WGPUSupportedFeatures const supportedFeatures = *(WGPUSupportedFeatures*) args[0].buf.ptr;
		assert(args[0].buf.size == sizeof supportedFeatures);
		wgpuSupportedFeaturesFreeMembers(supportedFeatures);
		break;
	}
	case 165: {
		WGPUSupportedWGSLLanguageFeatures const supportedWGSLLanguageFeatures = *(WGPUSupportedWGSLLanguageFeatures*) args[0].buf.ptr;
		assert(args[0].buf.size == sizeof supportedWGSLLanguageFeatures);
		wgpuSupportedWGSLLanguageFeaturesFreeMembers(supportedWGSLLanguageFeatures);
		break;
	}
	case 166: {
		WGPUSurface const surface = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUSurfaceConfiguration const * const config = (void*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof *config);
		wgpuSurfaceConfigure(surface, config);
		break;
	}
	case 167: {
		WGPUSurface const surface = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUAdapter const adapter = vdriver_unwrap_local_opaque_ptr(args[1].opaque_ptr);
		WGPUSurfaceCapabilities * const capabilities = (void*) args[2].buf.ptr;
		assert(args[2].buf.size == sizeof *capabilities);
		notif.call_ret.ret.u32 = wgpuSurfaceGetCapabilities(surface, adapter, capabilities);
		break;
	}
	case 168: {
		WGPUSurface const surface = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUSurfaceTexture * const surfaceTexture = (void*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof *surfaceTexture);
		wgpuSurfaceGetCurrentTexture(surface, surfaceTexture);
		break;
	}
	case 169: {
		WGPUSurface const surface = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		notif.call_ret.ret.u32 = wgpuSurfacePresent(surface);
		break;
	}
	case 170: {
		WGPUSurface const surface = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuSurfaceUnconfigure(surface);
		break;
	}
	case 171: {
		WGPUSurface const surface = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuSurfaceAddRef(surface);
		break;
	}
	case 172: {
		WGPUSurface const surface = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuSurfaceRelease(surface);
		break;
	}
	case 173: {
		WGPUSurfaceCapabilities const surfaceCapabilities = *(WGPUSurfaceCapabilities*) vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuSurfaceCapabilitiesFreeMembers(surfaceCapabilities);
		break;
	}
	case 174: {
		WGPUTexture const texture = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUTextureViewDescriptor const * const descriptor = (void*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof *descriptor);
		notif.call_ret.ret.opaque_ptr = vdriver_make_opaque_ptr(wgpuTextureCreateView(texture, descriptor));
		break;
	}
	case 175: {
		WGPUTexture const texture = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuTextureDestroy(texture);
		break;
	}
	case 176: {
		WGPUTexture const texture = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		notif.call_ret.ret.u32 = wgpuTextureGetDepthOrArrayLayers(texture);
		break;
	}
	case 177: {
		WGPUTexture const texture = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		notif.call_ret.ret.u32 = wgpuTextureGetDimension(texture);
		break;
	}
	case 178: {
		WGPUTexture const texture = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		notif.call_ret.ret.u32 = wgpuTextureGetFormat(texture);
		break;
	}
	case 179: {
		WGPUTexture const texture = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		notif.call_ret.ret.u32 = wgpuTextureGetHeight(texture);
		break;
	}
	case 180: {
		WGPUTexture const texture = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		notif.call_ret.ret.u32 = wgpuTextureGetMipLevelCount(texture);
		break;
	}
	case 181: {
		WGPUTexture const texture = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		notif.call_ret.ret.u32 = wgpuTextureGetSampleCount(texture);
		break;
	}
	case 182: {
		WGPUTexture const texture = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		notif.call_ret.ret.u64 = wgpuTextureGetUsage(texture);
		break;
	}
	case 183: {
		WGPUTexture const texture = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		notif.call_ret.ret.u32 = wgpuTextureGetWidth(texture);
		break;
	}
	case 184: {
		WGPUTexture const texture = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUStringView const label = {
			.data = args[1].buf.ptr,
			.length = args[1].buf.size,
		};
		wgpuTextureSetLabel(texture, label);
		break;
	}
	case 185: {
		WGPUTexture const texture = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuTextureAddRef(texture);
		break;
	}
	case 186: {
		WGPUTexture const texture = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuTextureRelease(texture);
		break;
	}
	case 187: {
		WGPUTextureView const textureView = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUStringView const label = {
			.data = args[1].buf.ptr,
			.length = args[1].buf.size,
		};
		wgpuTextureViewSetLabel(textureView, label);
		break;
	}
	case 188: {
		WGPUTextureView const textureView = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuTextureViewAddRef(textureView);
		break;
	}
	case 189: {
		WGPUTextureView const textureView = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuTextureViewRelease(textureView);
		break;
	}
	case 190: {
		WGPUInstance const instance = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUGlobalReport * const report = (void*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof *report);
		wgpuGenerateReport(instance, report);
		break;
	}
	case 191: {
		WGPUInstance const instance = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUInstanceEnumerateAdapterOptions const * const options = (void*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof *options);
		WGPUAdapter * const adapters = (void*) args[2].buf.ptr;
		assert(args[2].buf.size == sizeof *adapters);
		notif.call_ret.ret.u32 = wgpuInstanceEnumerateAdapters(instance, options, adapters);
		break;
	}
	case 192: {
		WGPUQueue const queue = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		size_t const commandCount = args[1].u32;
		WGPUCommandBuffer const * const commands = (void*) args[2].buf.ptr;
		assert(args[2].buf.size == sizeof *commands);
		notif.call_ret.ret.u64 = wgpuQueueSubmitForIndex(queue, commandCount, commands);
		break;
	}
	case 193: {
		WGPUDevice const device = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUBool const wait = args[1].b;
		WGPUSubmissionIndex const * const submissionIndex = (void*) args[2].buf.ptr;
		assert(args[2].buf.size == sizeof *submissionIndex);
		notif.call_ret.ret.b = wgpuDevicePoll(device, wait, submissionIndex);
		break;
	}
	case 194: {
		WGPUDevice const device = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUShaderModuleDescriptorSpirV const * const descriptor = (void*) args[1].buf.ptr;
		assert(args[1].buf.size == sizeof *descriptor);
		notif.call_ret.ret.opaque_ptr = vdriver_make_opaque_ptr(wgpuDeviceCreateShaderModuleSpirV(device, descriptor));
		break;
	}
	case 195: {
		WGPULogCallback const callback = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		void * const userdata = vdriver_unwrap_local_opaque_ptr(args[1].opaque_ptr);
		wgpuSetLogCallback(callback, userdata);
		break;
	}
	case 196: {
		WGPULogLevel const level = args[0].u32;
		wgpuSetLogLevel(level);
		break;
	}
	case 197: {
		notif.call_ret.ret.u32 = wgpuGetVersion();
		break;
	}
	case 198: {
		WGPURenderPassEncoder const encoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUShaderStage const stages = args[1].u64;
		uint32_t const offset = args[2].u32;
		uint32_t const sizeBytes = args[3].u32;
		void const * const data = vdriver_unwrap_local_opaque_ptr(args[4].opaque_ptr);
		wgpuRenderPassEncoderSetPushConstants(encoder, stages, offset, sizeBytes, data);
		break;
	}
	case 199: {
		WGPUComputePassEncoder const encoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		uint32_t const offset = args[1].u32;
		uint32_t const sizeBytes = args[2].u32;
		void const * const data = vdriver_unwrap_local_opaque_ptr(args[3].opaque_ptr);
		wgpuComputePassEncoderSetPushConstants(encoder, offset, sizeBytes, data);
		break;
	}
	case 200: {
		WGPURenderBundleEncoder const encoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUShaderStage const stages = args[1].u64;
		uint32_t const offset = args[2].u32;
		uint32_t const sizeBytes = args[3].u32;
		void const * const data = vdriver_unwrap_local_opaque_ptr(args[4].opaque_ptr);
		wgpuRenderBundleEncoderSetPushConstants(encoder, stages, offset, sizeBytes, data);
		break;
	}
	case 201: {
		WGPURenderPassEncoder const encoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUBuffer const buffer = vdriver_unwrap_local_opaque_ptr(args[1].opaque_ptr);
		uint64_t const offset = args[2].u64;
		uint32_t const count = args[3].u32;
		wgpuRenderPassEncoderMultiDrawIndirect(encoder, buffer, offset, count);
		break;
	}
	case 202: {
		WGPURenderPassEncoder const encoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUBuffer const buffer = vdriver_unwrap_local_opaque_ptr(args[1].opaque_ptr);
		uint64_t const offset = args[2].u64;
		uint32_t const count = args[3].u32;
		wgpuRenderPassEncoderMultiDrawIndexedIndirect(encoder, buffer, offset, count);
		break;
	}
	case 203: {
		WGPURenderPassEncoder const encoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUBuffer const buffer = vdriver_unwrap_local_opaque_ptr(args[1].opaque_ptr);
		uint64_t const offset = args[2].u64;
		WGPUBuffer const count_buffer = vdriver_unwrap_local_opaque_ptr(args[3].opaque_ptr);
		uint64_t const count_buffer_offset = args[4].u64;
		uint32_t const max_count = args[5].u32;
		wgpuRenderPassEncoderMultiDrawIndirectCount(encoder, buffer, offset, count_buffer, count_buffer_offset, max_count);
		break;
	}
	case 204: {
		WGPURenderPassEncoder const encoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUBuffer const buffer = vdriver_unwrap_local_opaque_ptr(args[1].opaque_ptr);
		uint64_t const offset = args[2].u64;
		WGPUBuffer const count_buffer = vdriver_unwrap_local_opaque_ptr(args[3].opaque_ptr);
		uint64_t const count_buffer_offset = args[4].u64;
		uint32_t const max_count = args[5].u32;
		wgpuRenderPassEncoderMultiDrawIndexedIndirectCount(encoder, buffer, offset, count_buffer, count_buffer_offset, max_count);
		break;
	}
	case 205: {
		WGPUComputePassEncoder const computePassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUQuerySet const querySet = vdriver_unwrap_local_opaque_ptr(args[1].opaque_ptr);
		uint32_t const queryIndex = args[2].u32;
		wgpuComputePassEncoderBeginPipelineStatisticsQuery(computePassEncoder, querySet, queryIndex);
		break;
	}
	case 206: {
		WGPUComputePassEncoder const computePassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuComputePassEncoderEndPipelineStatisticsQuery(computePassEncoder);
		break;
	}
	case 207: {
		WGPURenderPassEncoder const renderPassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUQuerySet const querySet = vdriver_unwrap_local_opaque_ptr(args[1].opaque_ptr);
		uint32_t const queryIndex = args[2].u32;
		wgpuRenderPassEncoderBeginPipelineStatisticsQuery(renderPassEncoder, querySet, queryIndex);
		break;
	}
	case 208: {
		WGPURenderPassEncoder const renderPassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		wgpuRenderPassEncoderEndPipelineStatisticsQuery(renderPassEncoder);
		break;
	}
	case 209: {
		WGPUComputePassEncoder const computePassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUQuerySet const querySet = vdriver_unwrap_local_opaque_ptr(args[1].opaque_ptr);
		uint32_t const queryIndex = args[2].u32;
		wgpuComputePassEncoderWriteTimestamp(computePassEncoder, querySet, queryIndex);
		break;
	}
	case 210: {
		WGPURenderPassEncoder const renderPassEncoder = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);
		WGPUQuerySet const querySet = vdriver_unwrap_local_opaque_ptr(args[1].opaque_ptr);
		uint32_t const queryIndex = args[2].u32;
		wgpuRenderPassEncoderWriteTimestamp(renderPassEncoder, querySet, queryIndex);
		break;
	}
// CALL_HANDLERS:END
		// clang-format on
	default:
		assert(false); // We should never receive a function ID that we don't know how to handle.
	}

	VDRIVER.notif_cb(&notif, VDRIVER.notif_data);
}

vdriver_t VDRIVER = {
	.spec = SPEC,
	.human = VDRIVER_HUMAN,
	.vers = VERS,
	.init = init,
	.probe = probe,
	.conn = conn,
	.call = call,
};
