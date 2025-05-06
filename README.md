# AQUA

Mono-repo for all things AQUA 🏄

## What is AQUA?

TBD. Short description with a link to more info.

## Basic build instructions

AQUA requires the [Bob the Builder buildsystem](https://github.com/inobulles/bob) to build.

To build all components, you can simply run:

```console
bob build
```

Bob will create a temporary installation prefix for you, in which you can enter a shell with `bob sh`.

Running a demo can be done by passing the path to the demo's directory to `bob run`'s `-C` option:

```console
bob run -C demos/cam
```

(In fact, you don't need to have run a build step first to run a demo; it'll do this for you.)

If you wish to install AQUA to your system, run:

```console
bob install
```

## Using GrapeVine

GrapeVine is automatically built when running `bob build` from the root directory.
You can run it by running `gvd` (the GrapeVine daemon), either as-is if you've installed it or with `bob run`, e.g.:

```console
bob run gvd -i wg0 &
```

In the example, it's being run on the `wg0` network interface, which represents a WireGuard VPN interface.
It is recommended that you use a VPN for communication between devices on the GrapeVine, and in fact this is the only possible way to communicate between devices on different networks.

You can run `gvd` on a physical interface, but do note that all traffic is unencrypted and unauthenticated, and thus anyone on the same local network will be able to participate in and spy on the GrapeVine.

## Guide du routard

Here is a list of the components and their respective subdirectories held within this repo:

|Path|Description|
|-|-|
|`gv`|GrapeVine sources for `gvd` (GrapeVine daemon) and `libgv`.|
|`kos`|KOS sources, i.e. platform-specific "glue" to run AQUA apps and interact with VDEVs and GrapeVine.|
|`vdev`|VDEV driver sources. VDEVs are the virtual drivers that allow AQUA apps to interact with hardware or other some software.|
|`demos`|Demo apps that showcase AQUA's capabilities and serve as simple examples.|
|`lib`|C library AQUA apps can use to facilitate communication with the KOS and VDEVs.|
|`lib/bindings`|Language specific bindings to the C library.|
|`external`|Any external dependencies which can't directly be depended on.|

More detailed information may be found in the READMEs of each subdirectory respectively.

## Older versions

At some point in the future, I'd also like to clean up all previous versions of AQUA and bring their history into this repo.
What's in here currently would then be rebased on top of the old history.

This could be done either by keeping this repo, either by hijacking `aqua-kos` (as it's been around the longest) and deleting this one.

The histories between `AQUA-3.x-SDL-window-KOS` and `aqua-kos` don't overlap so they are fine, but `SDL2-AQUA-KOS` was developed at some point in parallel to `aqua-kos` for some reason.
It seems like this turned into something similar to what would become `aqua-unix` however so maybe that could count towards its history.
