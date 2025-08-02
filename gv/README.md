# GrapeVine

Systems wanting to participate in the GrapeVine network must run the GrapeVine daemon (gvd).
GrapeVine does not concern itself with routing between hosts, so hosts must all be accessible to eachother on the same network.
GrapeVine also does not concern itself with encrypting connections, so it is advised to set up a VPN between GV hosts instead of just running it as-is on your LAN.

Hosts discover eachother through echolocation packets (ELPs) occasionally broadcast by gvd.
Each host maintains a list of other hosts it knows about.
If it doesn't receive an ELP from a host in a certain amount of time (`NODE_TTL * ELP_DELAY`) it will consider that host has died.

If a new host is found or the `unique` value in the ELP of an existing host has changed (indicating an update), we send out a QUERY packet to that host which should reply with a QUERY_RES packet containing all the VDEVs it exposes.
