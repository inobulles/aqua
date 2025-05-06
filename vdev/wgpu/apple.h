// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#if defined(__APPLE__)
# include "../win/win.h"
# include <webgpu/webgpu.h>

WGPUSurfaceSourceMetalLayer wgpu_get_metal_layer_surface_source(aqua_win_t* win);
#endif
