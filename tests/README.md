# E2E tests
This is the E2E (end-to-end) testing framework for AQUA.

The testing framework sets up a small network with 3 hosts with the following structure.
Since GV does not concern itself with routing, each host has its own IP address and the network topology looks like a fully connected graph to GV, where every host can directly access every other host.

It makes use of (aquariums)[https://github.com/inobulles/aquarium] to build this structure, and thus only work on FreeBSD.

In the future it might be cool for these aquariums to use special GVOS images to keep them small & quick.

## Test ideas

In a network with hosts A, B, and C, A should be able to initiate a connection to both B and C's test VDEVs.
A should request some pointer from B, and try to pass it to C.
When C gets this pointer, it should be able to vitrify it through the C-B link.
