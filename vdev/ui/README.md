# .ui

The UI device for AQUA.
This provides an interface for creating UIs in AQUA programs.

For the most part, there is just one "de facto" implementation, but this is made as a device so that UIs can be rendered on multiple different machines which could potentially implement this differently to fit their form factors better.
E.g. a mobile device might want to implement this using native UI components, or a VR headset could forgo the traditional layout of a UI and render the UI in 3D space instead.

## WebGPU backend

The primary (and currently only) backend for this implementation is WebGPU, done through the [`.wgpu`](../wgpu/README.md) device.
The VDRIVER links to the AQUA C library, and makes the same calls to the `.wgpu` device as any other AQUA program would (through `kos_vdev_call` and all).

It is still the client's responsibility to set up the `.wgpu` device connection and to create the WebGPU instance and device.
It must share its connection (host ID and connection ID) through the `backend_wgpu_init` function, and call the `backend_wgpu_render` function in its render loop to actually render the UI.

### Future optimization

If going through the KOS to access `.wgpu` ends up being too slow, the `wgpu` library could multiplex the KOS calls (`kos_vdev_call`) and direct WebGPU calls, if available and we are sure this is the same VDEV.

In fact, maybe it makes sense for the library itself to report a new VDEV kind as "direct" which, when the consumer of the library selects this, instructs the library to directly call the WebGPU API.
This is more a consideration for the library instead of this VDRIVER, just thought I'd put it here as it is relevant and will likely be the first place I do this if I do.
