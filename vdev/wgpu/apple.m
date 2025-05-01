// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#if defined(__APPLE__)
#if !defined(__OBJC__)
#error "This file must be compiled as Objective-C"
#endif

#include "apple.h"

# include <Cocoa/Cocoa.h>
# include <QuartzCore/CAMetalLayer.h>

WGPUSurfaceSourceMetalLayer wgpu_get_metal_layer_surface_source(aqua_win_t* win) {
	NSView* const view = win->detail.appkit.ns_view;
	[view setWantsLayer:YES];
	CAMetalLayer* const layer = [CAMetalLayer layer];
	[view setLayer:layer];

	WGPUSurfaceSourceMetalLayer  const descr_from_metal = {
		.chain = (WGPUChainedStruct const) {
			.sType = WGPUSType_SurfaceSourceMetalLayer,
		},
		.layer = layer,
	};

	return descr_from_metal;
}
#endif
