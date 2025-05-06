# Go WebGPU bindings for AQUA

This Go module provides bindings to the AQUA WebGPU library (`libaqua_wgpu`).

## Usage

Since it is a library component like any other, you must first initialize it and then create a connection.
To make things simpler, connecting to the device will only be done once and the connection context will be held globally.
If you need to use more than one WebGPU device, you will have to use the C library directly.

## Generating the bindings

The bindings are pulled from the (I guess de facto) [`webgpu bindings for Go`](https://github.com/cogentcore/webgpu/) (originally [here](https://github.com/rajveermalviya/go-webgpu) but that project seems to be abandoned and the author is unreachable).
They are cleaned up and adapted to call the AQUA C library instead by a simple script.
You can run the script in the following directory as such:

```sh
python gen.py
```

These bindings need to target the same version that the `libaqua_wgpu` library was generated from.
If you need to update the `libaqua_wgpu` library, you must first ensure the headers in `external/webgpu-headers` are up to date, and then you must run the wgpu VDEV's `gen.py` script (which generates both the device source and the accompanying library).
More information about this can be found in its [README file](/vdev/wgpu/README.md).
