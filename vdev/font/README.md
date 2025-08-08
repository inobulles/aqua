# .font

This device provides functionality for rendering text in AQUA applications.

It uses the awesome [Pango](https://www.gtk.org/docs/architecture/pango) library.

## Installing Pango

In the future I'd like this to be entirely handled by Bob the Builder, but in the meantime you'll need to install Pango yourself.

### FreeBSD

```console
pkg install pango
```

### Ubuntu

```console
apt install libpango1.0-dev
```

### Arch Linux

```console
pacman -S cairo harfbuzz pango
```

### macOS (with Brew)

```console
brew install pango
```
