# VDEV

A VDEV is a virtual device exposed by a VDRIVER.

VDEVs are conceptual; they can be mapped to a physical device, such as a camera, but could also be a "fake" device such as a video feed acting as a camera or even things you wouldn't traditionally call a device such as an OS window.

They are what abstract away the details of the real world and provide a standardized interface.

This directory contains reference VDRIVER implementations, which already support multiple different platforms (mostly FreeBSD, Linux, and macOS to some extent).
One could substitute in a completely different VDEV however to support any other platform or hardware, or really to do any number of crazy and unholy things.
