# .wgpu

This device provides an interface to the [WebGPU API](https://developer.mozilla.org/en-US/docs/Web/API/WebGPU_API) for AQUA programs.
This implementation of the `.wgpu` device specifically uses the [`wgpu`](https://github.com/gfx-rs/wgpu) implementation of WebGPU through the [`wgpu-native`](https://github.com/gfx-rs/wgpu-native) library (which provides a C API).

The C API is defined by the [`webgpu.h`](https://github.com/webgpu-native/webgpu-headers) header (common between [Dawn](https://dawn.googlesource.com/dawn/) and wgpu projects), from which the device is generated.
Unofficial extensions to this API can be found `wgpu.h`.
These must be kept up to date in the `ffi` directory.

The C library code for [aqua-c](https://github.com/inobulles/aqua-c) is outputted to `c-lib`.

## Generating the device

Run the `gen.py` script:

```console
python gen.py
```

This will generate a new `main.c` device source file from the `webgpu.h` header.

## Extensions

TODO Talk about `WGPUEGLGetProcAddress` and `wgpuInstanceDeviceFromEGL` here.
