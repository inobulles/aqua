# .audio

This device provides functionality for audio output.

## Choice of crate for audio

The two crates I could realistically choose between were [CPAL](https://github.com/RustAudio/cpal) (see [Rust Audio](https://rust.audio/)) and [TinyAudio](https://github.com/mrDIMAS/tinyaudio).

CPAL seems best supported and supports FreeBSD out of the box (or, at least, is tested on it), though only through ALSA over OSS.
It would be nice to eventually add a proper OSS backend to CPAL.

Also, for some reason, CPAL doesn't use tags.
It looks to me like commit 98918e4 is version 0.16.0, which is what we're using currently.

## Installing ALSA development packages

### Ubuntu/Debian

```console
apt install libasound2-dev
```

### Fedora

```console
dnf install alsa-lib-devel
```
