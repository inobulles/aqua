// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#if defined(__APPLE__)
# include "../win/win.h"
# include ".bob/prefix/include/webgpu-headers/webgpu.h"

WGPUSurfaceDescriptorFromMetalLayer wgpu_get_surface_descriptor_from_metal_layer(aqua_win_t* win);
#endif
