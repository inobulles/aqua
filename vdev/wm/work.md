# Work on WM VDEV

Notes on current issues I'm having with the WM VDEV.

## DMA-BUF textures showing up blank

At the moment, I still have no idea what's up with this or how I can effectively debug this.

## Destroying textures

Currently, I'm not destroying any of the textures.

Normally, in wlroots, if we use a texture (`struct wlr_vk_texture`) in some command buffer, we set it's `last_used_cb` member to that command buffer.
This indicates to `vulkan_texture_destroy()` that the texture is in use by a command buffer, and thus we cannot actually destroy it just yet.
In this situation, we instead add it to the `last_used_cb->destroy_textures` list, which we then only go over in `release_command_buffer_resources()` when it's actually safe to release the command buffer and its resources.

Now, in .wm, we're not allowing wlroots to do it's own rendering and use it's own command buffer.
This is all handled by WebGPU instead.
So, we set a dummy `last_used_cb` so that the texture still doesn't get freed prematurely.
We then have a `wlr_vk_dummy_cb_destroy_textures()` function we can call after we know it's safe to free these textures from WebGPU's perspective.

Unfortunately, if I do try to call this function, I get a GPU hang.

I'm wondering if this is because we're assuming WebGPU took ownership of these textures, and so we should really be asking WebGPU to destroy them instead.
I'm just concerned about any extra tracking that wlroots does on it's side obviously.

Or: we're actually just not supposed to destroy these textures every frame.
They get unref'd on certain events, such as damage or actual destruction, but otherwise we shouldn't try to destroy them.
We can figure out when the textures are supposed to be destroyed by setting a breakpoint in lldb in `vulkan_texture_destroy()` while running `tinywl`:

```lldb
b set --file render/vulkan/texture.c --line 221
```

And looking at various backtraces.
It seems to show that, for Kitty at least, we aren't actually destroying textures on each commit, so if we are actually leaking memory somewhere, it's probably not here.

## Layout transitions

We get the following error from the validation layer:

```
[W aquabsd.black.wm.wlr wlroots:69] [render/vulkan/vulkan.c:66] vkCmdPipelineBarrier(): pImageMemoryBarriers[0].image (VkImage 0x6150000000615) cannot transition the layout of aspect=1, level=0, layer=0 from VK_IMAGE_LAYOUT_GENERAL when the previous known layout is VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL.
The Vulkan spec states: If layouts are not ignored, oldLayout must be VK_IMAGE_LAYOUT_UNDEFINED or the current layout of the image subresources affected by the barrier (https://docs.vulkan.org/spec/latest/chapters/synchronization.html#VUID-VkImageMemoryBarrier-oldLayout-01197) (VUID-VkImageMemoryBarrier-oldLayout-01197)
```

This is because WebGPU leaves the texture in `VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL` once it's done with it, while `vulkan_read_pixels()` (which we call through `wlr_texture_read_pixels()` to sync the SHM window textures... another story) expects it to be in `VK_IMAGE_LAYOUT_GENERAL` in the first `vulkan_change_layout()` call.
The simple hack is to just transition it *from* `VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL`, which is fine at the moment as calling `wlr_texture_read_pixels()` is very temporary.

The other layout transition-related issue is:

```
[W aquabsd.black.wm.wlr wlroots:69] [render/vulkan/vulkan.c:66] vkQueueSubmit(): pSubmits[0] command buffer VkCommandBuffer 0x8a7878cc0 expects VkImage 0x6150000000615 (subresource: aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, mipLevel = 0, arrayLayer = 0) to be in layout VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL--instead, current layout is VK_IMAGE_LAYOUT_GENERAL.
The Vulkan spec states: If a descriptor with type equal to any of VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM, VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, or VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT is accessed as a result of this command, all image subresources identified by that descriptor must be in the image layout identified when the descriptor was written (https://docs.vulkan.org/spec/latest/chapters/drawing.html#VUID-vkCmdDraw-None-09600) (VUID-vkCmdDraw-None-09600)
```

Similarly, this is because `vulkan_read_pixels()` leaves the texture in `VK_IMAGE_LAYOUT_GENERAL` once it's done with it.
The quick fix is to reflect what we just did, so just transition it to `VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL` instead in the last `vulkan_change_layout()` call.
Again, this is temporary anyway so no real use in finding a more permanent fix.
