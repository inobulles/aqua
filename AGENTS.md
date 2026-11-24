# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What This Is

AQUA is a distributed virtual device (VDEV) runtime. Applications link against `libaqua.so` (KOS), which discovers and loads VDRIVERs — both locally from `VDRIVER_PATH` and remotely via the GrapeVine mesh network. All VDEV access is location-transparent.

## Build System

Uses [**Bob the Builder**](https://github.com/inobulles/bob) with `.fl` (Flamingo) build scripts — no Makefile.

```sh
bob build                    # build everything
bob run -C demos/<name>      # run a demo
bob install                  # install to system
bob sh                       # shell with tmp install prefix
bob clean                    # clean artifacts
```

**System deps:**
- FreeBSD: `pkg install -y meson rust go124`
- macOS: `brew install meson` (plus Rust + Go 1.24)
- Other VDRIVERs may have other dependencies.

## Tests

E2E tests use aquaBSD aquariums:

```sh
poetry run doas python tests/simple.py
```

## Formatting / Lint

Checked on every push via `.github/workflows/formatting.yml`.

## Architecture

Four major components:

### 1. KOS (`kos/`) — Runtime library (`libaqua.so`)

Linked by every AQUA app. Handles:
- VDEV discovery (local: scans `VDRIVER_PATH`; remote: reads `GV_NODES_PATH` written by gvd)
- Dynamic VDRIVER loading via `kos_req_vdev()`
- Notifies app via `KOS_NOTIF_ATTACH_VDEV` when a matching VDEV appears
- Also produces `libvdriver_loader.so` and `libvdriver.a` for VDRIVER authors

Public API: `kos/header/kos.h`. VDRIVER interface: `kos/lib/vdriver.h`.

### 2. GrapeVine (`gv/`) — Network discovery & remote proxy

- **`gvd`** daemon: broadcasts ELPs (echolocation packets), responds to QUERY packets, writes known hosts/VDEVs to `GV_NODES_PATH`
- **`gv-agent`** / `libgv_agent.so`: forked by gvd when a remote KOS connects; proxies all VDEV calls over TCP
- **`gv/proto/`**: serialization library (`libgv_proto.a`) for GV packet types (ELP, QUERY, QUERY_RES, CONN_VDEV, KOS_CALL, etc.)

Run daemon: `bob run gvd -i wg0`, or other network interface.

### 3. VDRIVERs (`vdev/`) — Device implementations

Installed as `lib/vdriver/<spec>.vdriver` (shared libraries). Loaded dynamically by KOS.

| VDRIVER | Language | Backend |
|---|---|---|
| `.win` | Rust | windowing support, uses winit |
| `.wgpu` | C (generated) | WebGPU support, uses wgpu-native |
| `.wm` | C | window management, uses wlroots (Wayland compositor, FreeBSD/Linux only) |
| `.ui` | Go | UI framework, with currently just a WebGPU backend |
| `.audio` | Rust | audio support, uses CPAL (ALSA on FreeBSD/Linux) |
| `.cam` | C + ObjC | (incomplete) camera support, uses AVFoundation (macOS) / v4l2 |
| `.test` | C | dummy device used for testing |

**`.wgpu` is code-generated:** `vdev/wgpu/main.c` is produced by `vdev/wgpu/gen.py` from `webgpu.h`.

### 4. C Library (`lib/`) — App-facing wrappers

Higher-level wrappers over KOS calls: `libaqua_win.so`, `libaqua_wgpu.so`, `libaqua_wm.so`, `libaqua_audio.so`, `libaqua_ui.so`, `libaqua_font.so`, `libaqua_vr.so`, `libaqua_test.so`.

Go bindings: `lib/bindings/go/` (module `obiw.ac/aqua`).

### Build dependency order

```
gv/proto  →  kos  →  gv (gvd + agent)
                  →  lib  →  vdev/*
```

## Key Concepts

- **VDEV ID (`vid_t`)**: `uint64_t` allocated in slices by KOS to each VDRIVER
- **Logging**: [`umber`](https://github.com/inobulles/umber) library used across all C components

## Vendored dependencies

### `external/webgpu-headers/`

Upstream WebGPU C headers (`webgpu.h`, `wgpu.h`). Do not modify.

### `external/go-webgpu/`

Go bindings for wgpu-native, forked from upstream. Has local patches on top — do not blindly reformat or clobber with upstream. Local additions include:

- AQUA-specific extensions for Vulkan interop: `DeviceFromWm`, `CommandEncoderFromVk`, `TextureFromVkImage`, raw to/from helpers
- Wires up AQUA wgpu C functions

When rebasing against upstream go-webgpu, preserve these local commits — they are not upstream and will never be.
