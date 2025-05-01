# webgpu.h

This header must be kept up to date from the [webgpu-native/webgpu-headers](https://github.com/webgpu-native/webgpu-headers) repo (same ref as the one used in `wgpu-native`'s submodule for this).
We can't depend on the repo directly because nothing defines where the header should be installed, so we have a Bob project here to install it to the right place.
