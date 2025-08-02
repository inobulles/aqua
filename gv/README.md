# GrapeVine

Systems wanting to participate in the GrapeVine network must run the GrapeVine daemon (gvd).
GrapeVine does not concern itself with routing between hosts, so hosts must all be accessible to eachother on the same network.
GrapeVine also does not concern itself with encrypting connections, so it is advised to set up a VPN between GV hosts instead of just running it as-is on your LAN.

## VDEV discovery

When gvd starts up, it takes inventory of all the VDEVs available locally on the system.
It does this by loading all the VDRIVERs in `VDRIVER_PATH` and querying them for the VDEVs they expose.

It will also randomly generate a `unique` value, which it includes in its echolocation packet (ELP) as essentially a hash of the VDEVs it exposes.
If another host sees this `unique` value change, it must invalidate the list of VDEVs it has for that host and query the host for the new list of VDEVs it exposes.

### Limitations

Currently, this `unique` value is randomly generated only once when gvd starts up, so if you want to refresh the VDEVs the host exposes, you must restart gvd.

See issue #10 for more information.

## Host discovery (echolocation)

Hosts automatically discover eachother through echolocation packets (ELPs) occasionally broadcast by gvd.
Each host maintains a list of other hosts it knows about.
If it doesn't receive an ELP from a host in a certain amount of time (`NODE_TTL * ELP_DELAY`) it will consider that host as dead.

If a new host is found or the `unique` value in the ELP of an existing host has changed (indicating an update), gvd will send out a QUERY packet to that host which should reply with a QUERY_RES packet containing all the VDEVs it exposes.

The list of known hosts and their VDEVs is sorted in the `GV_NODES_PATH` file, which any number of KOSs can read to report to the application what VDEVs are available on the GrapeVine network.

## KOS agent

When gvd on host B receives a CONN packet from a KOS on host A, it spins up a KOS agent for that KOS.
In practice this is done by forking a new process in order to keep the connection that sent the CONN packet alive and then linking a new KOS at runtime.

A KOS agent is responsible for one and only one KOS; if multiple KOSs initiate connections (as would be the case if host A were running multiple applications), a KOS agent will be created for each of them on host B, and an associated KOS will be loaded to each one.

All further communication (VDEV connections, function calls, etc) happen between host A's KOS and the KOS agent on host B over the connection previously established.
If this connection is broken, the KOS agent is killed.

It doesn't matter if a VDRIVER or VDEV is added/removed on host B while the KOS agent is running and the connection is live; the KOS on host A should still always be reading available VDEVs from its gvd and, if it interested in a new VDEV, it should reestablish a new connection.

**TODO** What happens if we want to maintain the existing connection though?
