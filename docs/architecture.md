# AQUA Architecture

This document describes the various components of AQUA and how they interact with eachother through examples.

The current version of AQUA is 4.0.

## Overview

The 3 main components of AQUA are:

- *The KOS*: This is the actual runtime all AQUA applications are in some way linked to, and is all that the application actually interacts with. More information can be found [here](../kos/README.md).
- *VDRIVERs*: These are the platform-specific implementations of VDEV interfaces. One VDRIVER may expose more than one VDEV. For example, the `.cam` VDRIVER would expose one VDEV for each physical camera on the system. More information can be found [here](../vdev/README.md).
- *The GrapeVine daemon (gvd)*: This is a daemon that runs on each network-enabled AQUA host and is responsible for discovering other hosts and their exposed VDEVs, as well as initiating connections to them. More information can be found [here](../gv/README.md).

## Examples

Here are a few examples of application setups.

Some of these examples make use of multiple hosts, named A, B, and C.

Thick lines indicate components linked together, thin lines indicate connections such as through the network or IPC, and dotted lines indicate one-off communication, usually during initial setup.

Each example roughly builds upon the previous ones, so labeled arrows are not repeated after the first time they are used.

### No devices used

This is a pretty boring situation in which the application isn't able to do all that much.
It is, however, the simplest setup which can be called "AQUA".

```mermaid
graph TD
	subgraph Host A
		app["Application"]
		kos["KOS"]
	end

	app <== "App binary is linked to KOS." ==> kos
```

### Rendering a triangle with WebGPU to a window (locally)

In this example, the application asks the KOS for both .wgpu (WebGPU) and .win (window) VDEVs.
The KOS then finds VDRIVERs supporting these interfaces, loads them and links them, and reports back to the application the two VDEVs found (each VDRIVER reports only one VDEV in this situation).

The application, through the KOS, initiates connections to the .wgpu and .win VDEVs, and creates a window with .win.
It received an opaque pointer to this window, which is an unreadable handle containing the host ID of the machine this pointer is on.

It then creates a WebGPU context with its .wgpu connection, and passes its window's opaque pointer to it.
The .wgpu VDRIVER can read the host ID of the window's opaque pointer and can see it is on the same machine, so it can directly and natively render to this window.

```mermaid
graph TD
	subgraph Host A
		app["Application"]
		kos["KOS"]
		wgpu_vdriver[".wgpu VDRIVER"]
		win_vdriver[".win VDRIVER"]
		wgpu_vdev[".wgpu VDEV"]
		win_vdev[".win VDEV"]
	end

	app <==> kos

	kos <== "KOS loads .wgpu VDRIVER on request of app. The app subsequently requests a connection to the one VDEV available." ==> wgpu_vdriver
	kos <== "KOS loads .win VDRIVER on request of app. The app subsequently requests a connection to the one VDEV available." ==> win_vdriver

	wgpu_vdriver <== ".wgpu VDRIVER reports only one VDEV." ==> wgpu_vdev
	win_vdriver <== ".win VDRIVER reports only one VDEV." ==> win_vdev

	wgpu_vdev == ".wgpu can render directly to .win." ==> win_vdev
```

### Rendering a triangle with WebGPU to a remote window

This situation is a little trickier.

Before the app is run, the GrapeVine daemons on both hosts A and B are running and discover eachother.
They are aware of the VDEVs available on eachother at any given time.

When the application is started up and asks for the `.wgpu` and `.win` VDEVs, its KOS will let it know of the VDEVs locally available, but it will also ask gvd of all the VDEVs it knows about on remote hosts and expose them to the application.

In this situation the application can only choose the local `.wgpu` and remote `.win` VDEVs, but if there also was a `.win` VDEV locally, it could have chosen that one instead, and same thing if there was a `.wgpu` VDEV on host B.

Since the application chose host B's `.win` VDEV, the KOS will ask host B's gvd to set up a remote KOS agent for it and to establish a connection to this KOS agent.
This agent will handle communication with all VDEVs on that host, and all requests to those VDEVs made by the app will be forwarded through it by host A's KOS.

For all intents and purposes, the remote KOS agent acts like a mini AQUA app, only serving to maintain the connection to the host which requested its creation and to forward requests to its KOS.

The opaque pointer to the window created is still passed to `.wgpu` during context creation, but this time the `.wgpu` VDRIVER will see that the host ID of the opaque pointer is not the same as its own, so it might error out saying it needs a local native window to render to, or it might be able to render it to an offscreen buffer, ask the window for a pointer to the window's framebuffer, and vitrify it so it can flip the contents of its offscreen buffer to it, thereby rendering to it over the network.

```mermaid
graph TD
	subgraph Host A
		app["Application"]
		kos["KOS"]
		wgpu_vdriver[".wgpu VDRIVER"]
		wgpu_vdev[".wgpu VDEV"]

		gvd_a["gvd"]
	end

	subgraph Host B
		kos_agent["KOS agent"]
		kos_remote["KOS (remote)"]
		win_vdriver[".win VDRIVER"]
		win_vdev[".win VDEV"]

		gvd_b["gvd"]
	end

	gvd_a <-- "Each gvd knows about the other host and the VDEVs it exposes through ELP and QUERY packets." --> gvd_b
	gvd_b -. "gvd starts KOS at the behest of host A's KOS." .-> kos_agent

	gvd_a -. "KOS learns about the .win VDEV on host B through gvd." .-> kos
	app <== "App chooses .wgpu VDEV available locally and host B's .win (remote)." ==> kos

	kos <==> wgpu_vdriver
	kos_agent <==> kos_remote
	kos_remote <==> win_vdriver
	kos -. "KOS asks host B's gvd to set up a KOS agent for it and to establish a connection." .-> gvd_b
	kos <-- "Connection established by gvd. Handles communication on behalf of app and other VDEVs that might need it." --> kos_agent

	wgpu_vdriver <==> wgpu_vdev
	win_vdriver <==> win_vdev

	wgpu_vdev == "When .wgpu needs to render to .win, it does this through its KOS." ==> kos
```

### Rendering remote camera feeds to a remote window

This example is even trickier, and is a situation which AQUA currently does not handle ideally.

The application, still on host A, is connecting to 2 `.cam` (camera) VDEVs on host C and wants to write their feeds on a window created on host B.
Currently it must vitrify both the pointers to the camera feeds and the pointer to the window framebuffer, and read the memory from the camera feeds to the framebuffer.

This requires copying the data over the network twice; once from host C to host A's memory, then from host A's memory to host B.
In the future, it would be good to somehow tell host C to write the camera feed directly to host B's framebuffer, though this would be very complex.

```mermaid
graph TD
	subgraph Host B
		kos_agent_b["KOS agent"]
		kos_remote_b["KOS (remote)"]
		win_vdriver[".win VDRIVER"]
		win_vdev[".win VDEV"]
		gvd_b["gvd"]

		gvd_b -..-> kos_agent_b
		kos_agent_b <==> kos_remote_b
		kos_remote_b <==> win_vdriver
		win_vdriver <==> win_vdev
	end

	subgraph Host A
		app["Application"]
		kos["KOS"]
		gvd_a["gvd"]

		app <==> kos
		gvd_a -..-> kos
	end

	kos <--> kos_agent_b
	kos <--> kos_agent_c

	subgraph Host C
		kos_agent_c["KOS agent"]
		kos_remote_c["KOS (remote)"]
		cam_vdriver[".cam VDRIVER"]
		cam_vdev_1[".cam VDEV #1"]
		cam_vdev_2[".cam VDEV #2"]
		gvd_c["gvd"]

		gvd_c -..-> kos_agent_c
		kos_agent_c <==> kos_remote_c
		kos_remote_c <==> cam_vdriver
		cam_vdriver <==> cam_vdev_1
		cam_vdriver <==> cam_vdev_2
	end

	gvd_a <--> gvd_b
	gvd_b <--> gvd_c
	gvd_c <--> gvd_a

	kos -..-> gvd_c
	kos -..-> gvd_b
```

### Rendering a triangle with remote WebGPU to a remote window

This is a similar situation to the one above, except the `.wgpu` VDEV would be the one initiating writes to the window's framebuffer, which actually makes things much easier.

The `.wgpu` VDEV gets the opaque pointer to the window's framebuffer, and it sees that it is neither local nor on host A, so it must first establish a new connection to the KOS agent on host B originally created for host A.
**TODO** How this is done is not currently defined. Probably we could have some kind of "application ID" that is passed around and can be used to group KOSs related to the same application instance.
Once that is done, it can quite simply vitrify that memory and write to it.

```mermaid
graph TD
	subgraph Host B
		kos_agent_b["KOS agent"]
		kos_remote_b["KOS (remote)"]
		win_vdriver[".win VDRIVER"]
		win_vdev[".win VDEV"]
		gvd_b["gvd"]

		gvd_b -..-> kos_agent_b
		kos_agent_b <==> kos_remote_b
		kos_remote_b <==> win_vdriver
		win_vdriver <==> win_vdev
	end

	subgraph Host A
		app["Application"]
		kos["KOS"]
		gvd_a["gvd"]

		app <==> kos
		gvd_a -..-> kos
	end

	kos <--> kos_agent_b
	kos <--> kos_agent_c

	subgraph Host C
		kos_agent_c["KOS agent"]
		kos_remote_c["KOS (remote)"]
		wgpu_vdriver[".wgpu VDRIVER"]
		wgpu_vdev[".wgpu VDEV"]
		gvd_c["gvd"]

		gvd_c -..-> kos_agent_c
		kos_agent_c <==> kos_remote_c
		kos_remote_c <==> wgpu_vdriver
		wgpu_vdriver <==> wgpu_vdev
	end

	gvd_a <--> gvd_b
	gvd_b <--> gvd_c
	gvd_c <--> gvd_a

	kos -..-> gvd_c
	kos -..-> gvd_b

	kos_remote_c -- "Host C's KOS initiates a connection to host B's KOS agent, originally created for host A. It can then flip its offscreen buffer to the vitrified window framebuffer." --> kos_agent_b
```
