# The KOS

The KOS is _the_ thing that makes an AQUA application an AQUA application.
It is what provides the interface for interacting with VDEVs and is responsible for figuring out what VDEVs are available to the application, loading relevant VDRIVERs, and establishing connections to them, either locally or over the network through GrapeVine.

It takes the form of a library that can be linked to the application, though in theory the idea could be extended.
E.g., one could write a KOS that is a VM running some special bytecode (similar to the ZVM in AQUA 3.X).

## VDEV discovery

The KOS discovers VDEVs in two ways:

- Locally, by reading the VDRIVERs in `VDRIVER_PATH`. It does not automatically load these VDRIVERs, but waits for the application to request a specific VDEV specification through `kos_req_vdev`, at which point it will load the VDRIVER and ask for the VDEVs it exposes.
- If the GrapeVine daemon is running (gvd), it will read the VDEVs available on the GrapeVine network that the daemon has discovered.

If a VDEV made aware to the KOS matches one of the specifications requested through `kos_req_vdev`, it will send a `KOS_NOTIF_ATTACH_VDEV` notification to the application containing the VDEV's descriptor.

## Why is it called a KOS?

"KOS" is a historical term which originally meant "Kernel/OS" back in AQUA 2.X.
It used to be either an actual kernel or an OS application which was abstracted away by a higher level, the CW.
For example, the actual AQUA 2.X kernel was a KOS and so was the SDL2 emulator, which allowed for easier development of the DE on a Linux machine.

By AQUA 3.X the KOS really just signified "whatever is providing the device interface to the ZVM (the VM AQUA 3.X apps ran on)".
There was a KOS for Android and one for Unix-like platforms, allowing an app to be run as an Android app or on a Linux machine.

This eventually morphed into what it is today, which doesn't have much bearing to the original KOS name.
