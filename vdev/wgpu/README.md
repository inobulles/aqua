# .wgpu

This device provides an interface to the [WebGPU API](https://developer.mozilla.org/en-US/docs/Web/API/WebGPU_API) for AQUA programs.
This implementation of the `.wgpu` device specifically uses the [`wgpu`](https://github.com/gfx-rs/wgpu) implementation of WebGPU through the [`wgpu-native`](https://github.com/gfx-rs/wgpu-native) library (which provides a C API).

The C API is defined by the [`webgpu.h`](https://github.com/webgpu-native/webgpu-headers) header (common between [Dawn](https://dawn.googlesource.com/dawn/) and wgpu projects), from which the device is generated.
Unofficial extensions to this API can be found `wgpu.h`.
This comes with the `wgpu-native` dependency and is installed in the include directory, so there's no need to download it separately.

The C library code for [aqua-c](https://github.com/inobulles/aqua-c) is outputted to `c-lib`.

## Generating the device

Run the `gen.py` script:

```console
python gen.py
```

This will generate a new `main.c` device source file from the `webgpu.h` header, provided it is placed in the default location for the Bob temporary prefix.

## Extensions

To facilitate the use of .wgpu in the .wm VDEV, a couple AQUA-specific unofficial extensions have been added on top of the WebGPU API.

### `wgpuInstanceDeviceFromEGL`

This function allows a device to be created from an instance, forcing the use of the EGL backend.
This is important as .wm creates the EGL context itself and needs to be able to use the context it created.

### `wgpuDeviceTextureFromRenderbuffer`

This takes in an OpenGL RBO (TODO: should this function be called `wgpuDeviceTextureFromOGLRenderbuffer` instead?) and creates a texture from that that can be used to draw on by the client.
A WebGPU texture isn't really a texture in the OpenGL sense in that it can either wrap around a native texture or a native renderbuffer.
In that sense it's more like the equivalent to an FBO.
