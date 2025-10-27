# .wm

This device provides functionality for managing windows and compositing them on screen.
This implementation does this exclusively through [Wayland](https://wayland.freedesktop.org/), using [wlroots](https://gitlab.freedesktop.org/wlroots/wlroots/).

A lot of this code comes from the (never finished) [`aquabsd.black.wm`](https://github.com/inobulles/aqua-devices/tree/main/src/aquabsd.black/wm) device in the previous version of AQUA.

## Building

Currently this is a little hacky as Bob the Builder has some problems dealing with this situation.
But basically, starting from a clean build (`bob clean`), you've got to disable the `PkgConfig.*` calls in `build.fl`:

```fl
let wlroots_cflags = [] # PkgConfig.cflags("wlroots-0.19")
let wlroots_libs = [] # PkgConfig.libs("wlroots-0.19")
```

When you run `bob build`, this will fail but the dependencies will be fetched, built, and installed to the Bob prefix.
Then you can uncomment the `PkgConfig.*` calls again and building should work.

See [inobulles/bob#108](https://github.com/inobulles/bob/pull/108) for details.

Alternatively, you could just remove wlroots from the `deps` vector in `build.fl` and just install wlroots globally.
This is provided through the `x11-toolkits/wlroots019` port on FreeBSD.

### Building XDG shell code

Eventually this will be integrated into the buildsystem, but in the meantime you have to do this manually.
With `wayland-scanner`:

```sh
wayland-scanner private-code < /usr/local/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml > xdg-shell-protocol.c
wayland-scanner client-header < /usr/local/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml > xdg-shell-protocol.h
mv xdg-shell-protocol.* .bob/prefix/include/wlroots-0.19/wlr/types
```

On your OS distribution, this `xdg-shell.xml` file may be found elsewhere (e.g. at `/usr/share` instead of `/usr/local/share`).

## Note on X11 support

X11 is only supported on previous versions of AQUA (see [`aquabsd.alps.wm`](https://github.com/inobulles/aqua-devices/tree/main/src/aquabsd.alps/wm)).
.wm will never support X11, but you can still create X11 windows with `aquabsd.black.win` in current versions of AQUA.

## Other (personal) notes

I'd (obviously) like for .wm to be the best compositor it can be, so here are a list of ideas I'd like to explore to improve latency and general perceived smoothness.
[This great article](https://raphlinus.github.io/ui/graphics/2020/09/13/compositor-is-evil.html) by Raph Levien highlights issues with compositors and provides some pistes to solve them.

### Repaint delays

Delay compositing a bit to allow for more inputs before the screen is flipped.
This is something I was already doing with `aquabsd.alps.ftime`.

It seems like [Weston](https://www.collabora.com/about-us/blog/2015/02/12/weston-repaint-scheduling/) and Sway already do this, but only with a fixed delay.
There is an [issue](https://github.com/swaywm/sway/issues/4734) which describes some techniques for dynamically changing this delay.

`aquabsd.alps.ftime` used a rather crude dynamic delay by taking a rolling average of the past few frametimes.
The main issues with this is that it was very resistant to change (especially tending downwards) and thus has a tendency to skip multiple frames.

### Partial redrawing

Something similar to Apple's [CALayers](https://developer.apple.com/documentation/quartzcore/calayer?language=objc) could be explored.
Check out [this blog post](https://mozillagfx.wordpress.com/2019/10/22/dramatically-reduced-power-usage-in-firefox-70-on-macos-with-core-animation/) from Mozilla on using Core Animation on macOS to improve battery life.

### Hardware overlays

Some overlays could skip the compositor entirely by using hardware planes.
`libliftoff` is a library which does this and helps wrangle all the differences between various hardware implementations of hardware planes.
Here's a [talk from FOSDEM](https://archive.fosdem.org/2020/schedule/event/kms_planes/) by Simon Ser, the creator of `libliftoff`, introducing it.

There is also a compositor which uses `libliftoff` also by Simon called [glider](https://github.com/emersion/glider).

This looks like a great way to increase battery life.
In [this experiment](https://octodon.social/@emersion/103300395120210509), power usage drops by over 50%.

### Opaque regions

Like with macOS, Wayland provides a way to define ["opaque" regions](https://wayland-book.com/surfaces-in-depth/surface-regions.html) to localize blurring (I had already thought of doing this through the DWD protocol in `aquabsd.alps.wm` back when I wrote that).
For now though, I think I'll stick to an effect similar to what I had in AQUA 2.X by sampling a blurred version of the wallpaper in the UI device which will utilize WebGPU, and keeping proper blur strictly within the bounds of an application window.

## Findings w.r.t. how we should go about creating WebGPU surfaces and adapters

Just as `aquabsd.black.wgpu` currently has `CMD_SURFACE_FROM_WIN`, it should have `CMD_SURFACE_FROM_WM`, which returns a `WGPUSurface` created atop the WM's DRM device (which should already be determined when creating the WM I think, perhaps with a flag if I can imagine a case where you wouldn't want to do such).

Here is what wlroot's GLES2 renderer does:

- `open_preferred_drm_fd`: Quite simple, find the file descriptor of the preferred DRM device.
- `wlr_gles2_renderer_create_with_drm_fd` is then called to create the GLES2 renderer on that DRM device.
- That then calls `wlr_egl_create_with_drm_fd`, which creates the EGL context atop that DRM device.
- `egl_create` checks for a bunch of extensions then calls `eglBindAPI(EGL_OPENGL_ES_API)`.
- Back in `wlr_egl_create_with_drm_fd`, we check for `EXT_platform_device`. If it's not available, we fall back on `KHR_platform_gbm`, but on my system it is, so we'll just ignore that for now.
- `get_egl_device_from_drm_fd` is then called which uses `eglQueryDevicesEXT` (`EGL_EXT_device_enumeration`, returns `EGLDeviceEXT[]`) to enumerate all the devices.
- `drmGetDevice` is used to get a `drmDevice` from the DRM fd.
- The device list gotten from `eglQueryDevicesEXT` is traversed to try to match the `drmDevice`.
- For each `EGLDeviceEXT`, the `drmDevice` is traversed to see if a render node matches it's name.
- Now we have our `EGLDeviceEXT`, `egl_init` is called.
- The main thing here is the `eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT, remote_display, display_attribs)` call (where `remote_display` is the `EGLDeviceEXT` we found in the previous step), which simply returns an `EGLDisplay`, which we can continue setting up like we normally would with EGL.
- The rest is completely standard EGL setup (with `eglInitialize(EGLDisplay)` etc etc).
- Back in `wlr_gles2_renderer_create_with_drm_fd`, we make the EGL context current and are off to the races with OpenGL setup :)

Here is what the wlroot's Vulkan renderer does:

- `open_preferred_drm_fd` is in common with the GLES2 renderer.
- The rest I don't know :P

On the side of `wgpu`, ~I'm a little confused because it seems like `eglInitialize` is being called upon creation of a `WGPUInstance` (in `Instance::init` in `wgpu-hal/src/gles/egl.rs`).
I would've expected this to be done when creating the `WGPUSurface`.~
Actually it's being called in `Instance::create_surface` which makes a little more sense.

I think we'll have to create a new member on `RawWindowHandle`/`RawDisplayHandle`, something like `RawDisplayHandle::PlatformDevice`, which would either correspond to a `EGLDeviceEXT` in the context of the HAL, and would correspond to the DRM device fd in the generic WebGPU context (i.e. when calling `wgpuInstanceCreateSurface`).

In `wgpu-native`, we'd have a new `WGPUSType_SurfaceDescriptorFromDrmFd`, corresponding to `native::WGPUSurfaceDescriptorFromDrmFd`.
The request would look like this:

```c
int const drm_fd = open_preferred_drm_fd(...);

WGPUSurfaceDescriptorFromDrmFd const descr_from_drm_fd = {
	.chain = (WGPUChainedStruct const) {
		.sType = WGPUSType_SurfaceDescriptorFromDrmFd,
	},
	.fd = drm_fd,
};

WGPUSurfaceDescriptor const descr = {
	.nextInChain = (WGPUChainedStruct const*) &descr_from_drm_fd,
};

WGPUSurface const surface = wgpuInstanceCreateSurface(instance, &descr);
```

## Findings w.r.t. how to get textures working and whatnot

Let's cave and purely use the OpenGL backend for WebGPU for now.
I already know how textures work with EGL and if anything goes wrong with Vulkan well it's gonna be a lot more work.

Probably a WebGPU extension for this is needed and doesn't exist yet?
Making this play nicely with non-specific shader code is going to be a nightmare 🙂

The two reference functions of interest in wlroots are `gles2_texture_from_buffer` and `vulkan_texture_from_buffer`, which both quite simply return a `struct wlr_texture` to be used elsewhere in the renderer.
`struct wlr_texture` looks like this:

```c
struct wlr_texture {
	const struct wlr_texture_impl *impl;
	uint32_t width, height;

	struct wlr_renderer *renderer;
};
```

The `impl` member contains the following struct which defines the functions we can call on this implementation of `wlr_texture`.
In the case of the GLES2 renderer:

```c
static const struct wlr_texture_impl texture_impl = {
	.update_from_buffer = gles2_texture_update_from_buffer,
	.read_pixels = gles2_texture_read_pixels,
	.preferred_read_format = gles2_texture_preferred_read_format,
	.destroy = handle_gles2_texture_destroy,
};
```

And for Vulkan:

```c
static const struct wlr_texture_impl texture_impl = {
	.update_from_buffer = vulkan_texture_update_from_buffer,
	.read_pixels = vulkan_texture_read_pixels,
	.preferred_read_format = vulkan_texture_preferred_read_format,
	.destroy = vulkan_texture_unref,
};
```

These functions then access custom data for each texture type by getting the container of them with `wl_container_of`.
For GLES2 this container is `struct wlr_gles2_texture`:

```c
struct wlr_gles2_texture {
	struct wlr_texture wlr_texture;
	struct wlr_gles2_renderer *renderer;
	struct wl_list link; // wlr_gles2_renderer.textures

	GLenum target;

	// If this texture is imported from a buffer, the texture is does not own
	// these states. These cannot be destroyed along with the texture in this
	// case.
	GLuint tex;
	GLuint fbo;

	bool has_alpha;

	uint32_t drm_format; // for mutable textures only, used to interpret upload data
	struct wlr_gles2_buffer *buffer; // for DMA-BUF imports only
};
```

And for Vulkan it is `struct wlr_vk_texture`:

```c
// State (e.g. image texture) associated with a surface.
struct wlr_vk_texture {
	struct wlr_texture wlr_texture;
	struct wlr_vk_renderer *renderer;
	uint32_t mem_count;
	VkDeviceMemory memories[WLR_DMABUF_MAX_PLANES];
	VkImage image;
	const struct wlr_vk_format *format;
	enum wlr_vk_texture_transform transform;
	struct wlr_vk_command_buffer *last_used_cb; // to track when it can be destroyed
	bool dmabuf_imported;
	bool owned; // if dmabuf_imported: whether we have ownership of the image
	bool transitioned; // if dma_imported: whether we transitioned it away from preinit
	bool has_alpha; // whether the image is has alpha channel
	bool using_mutable_srgb; // is this accessed through _SRGB format view
	struct wl_list foreign_link; // wlr_vk_renderer.foreign_textures
	struct wl_list destroy_link; // wlr_vk_command_buffer.destroy_textures
	struct wl_list link; // wlr_vk_renderer.textures

	// If imported from a wlr_buffer
	struct wlr_buffer *buffer;
	struct wlr_addon buffer_addon;
	// For DMA-BUF implicit sync interop
	VkSemaphore foreign_semaphores[WLR_DMABUF_MAX_PLANES];

	struct wl_list views; // struct wlr_vk_texture_ds.link
};
```

(Maybe now it's clear why I'm starting by forcing the OpenGL backend to WebGPU 🙂)

For GLES2, once we have our `struct wlr_gles2_texture`, we can just get its `tex` member and do as in `aquabsd.alps.ogl` with the `GL_OES_EGL_image_external` extension.

Now "all" that's left to do is design the WebGPU extension 🥲
And also make Naga support `texture_external`: <https://github.com/gfx-rs/wgpu/issues/4528>.

Here seems to be more information <https://github.com/gfx-rs/wgpu/issues/2320>.

### Update as of 10/06/2024

All is working now and with only two functions added to WebGPU: first, `wgpuInstanceDeviceFromEGL`, which creates a device from an instance forcing the use of the EGL backend.
It simply takes in a `getProcAddress` function (as that's what's needed for the EGL backend in `wgpu`).

Second, `wgpuDeviceTextureFromRenderbuffer`, which takes in an OpenGL RBO (TODO: should this function be called `wgpuDeviceTextureFromOGLRenderbuffer` instead?) and creates a texture from that that can be used to draw on by the client.
A WebGPU texture isn't really a texture in the OpenGL sense in that it can either wrap around a native texture or a native renderbuffer.
In that sense it's more like the equivalent to an FBO.

What's left now is to understand how the swapchain works and how wlroots does all this while creating only 3 buffers (as of when I'm writing this, .wm just creates a new buffer for every frame, which is obviously not ideal).

My guess is that these 3 buffers are part of the swapchain and they are shifted around somehow, but so far I don't really understand the wlroots code that does this.
What I have gotten so far is this:

- `gles2_begin_buffer_pass` is the entry to the renderer. It starts by setting the EGL context current, and then "gets or creates" a `struct wlr_gles2_buffer ` with `gles2_buffer_get_or_create`. Then, it actually begins the pass by calling `begin_gles2_buffer_pass`.
- `gles2_buffer_get_or_create` uses this `wlr_addon` system that I haven't yet taken the time to understand and that doesn't seem to be documented anywhere. Anyway, if a buffer is "found" (?) in there, it's returned straight away, and otherwise, the whole getting of a DMA-BUF and `EGLImage` creation process is run. Once that's done, it's added to a list of buffers on the renderer and some addon thing is "initialized" idk.
- `begin_gles2_buffer_pass` calls `gles2_buffer_get_fbo`, which just returns the current FBO if the buffer object has one and otherwise creates the RBO/FBO and attaches the RBO to the RBO.

Now I really don't understand where the swapchain thing fits into here, but the first step would be to understand the whole addon thing :)
