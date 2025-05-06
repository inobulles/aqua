# webgpu.h

This header must be kept up to date from the [webgpu-native/webgpu-headers](https://github.com/webgpu-native/webgpu-headers) repo (same ref as the one used in `wgpu-native`'s submodule for this).
We can't depend on the repo directly because nothing defines where the header should be installed, so we have a Bob project here to install it to the right place.

We could rely on the headers installed by the `wgpu-native` dependency used in a few places (`webgpu-headers/webgpu.h`), but these are installed in a non-standard place and mean we need a lot more dependencies on platforms which might want to use the library without necessarily building the `wgpu` VDEV.

TODO Where goes wgpu.h come from?
TODO Also, we should update or at least ask about wgpu.h upstream.
