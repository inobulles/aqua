# GrapeVine Storage

## Goals

I want to be able to pool together the storage of a bunch of GrapeVine nodes.
Then, I want to be able to divide this storage into "replication zones", which are groups of nodes that automatically mirror a fixed quota of data.
I want to be able to access and write to data from any replication zone from any node in the GV, and for this to be done completely opaquely (and selecting the closest node with that data of course, perhaps even pooling together multiple nodes for faster reads, though that will require a rethink of the architecture).
I want writes to propagate as quickly as possible (i.e. not just run an rsync cron job a couple times a day).
Ideally this would be compatible with an existing filesystem (NFS) so that existing systems can connect to individual nodes without the whole GV functionality.

## Replication zones

The idea with replication zones is that you can group together GV nodes and say "this zone can grow to 2 TB in size, and has to be replicated to nodes A, B, and C".
A replication zone can ideally only be as big as the node with the smallest storage capacity left in the zone.
I guess there's no reason we should have a hard limit for this (kinda like ZFS dataset quotas can sum to larger than the pool), but we should at least warn a user adding a new node that's smaller than the zone's quota.

Here is an example of how this would be used.
Take the following nodes:

- Chimay Bleue: NAS & build server, 8 TB of RAID-1 HDD storage, 512 GB of NVMe storage.
- Hertog Jan: NAS, 4 TB of RAID-1 HDD storage.
- Cara Pils: Regular laptop, 512 GB of NVMe storage.

For scratch data such as build caches, we could commission 512 GB of only Chimay Bleue to be its own replication zone called "Build cache (fast)" and 4 TB to be a replication zone called "Build cache (slow)".
For regular photo backups and archives, we could commission 4 TB of Chimay Bleue and 4 TB of Hertog Jan to be a replication zone called "Tier 2".
For extremely sensitive information such as personal documents and perhaps recent photos we'd like quick access to, we could commission 512 GB of Chimay Bleue, Hertog Jan, and Cara Pils to be a replication zone called "Tier 1".

With this set up, "Tier 2" files are spread across two systems, which can be in physically different locations.
Our "Tier 1" files are spread across three systems, which can also be in physically different locations.
Since it is replicated to the laptop too, these files are accessible even offline or when the GV is down.
Build cache files are kept in only one place, but they should still be mountable from any other node in the GV.

In practice, if using ZFS, a dataset should be created per replication zone.
Chimay Bleue would thus have something like this:

- `tank/gv-zones/build-cache-fast` on the NVMe storage, quota 512 GB.
- `tank/gv-zones/build-cache-slow` mirrored dataset, on the two 8 TB HDDs, quota 4 TB.
- `tank/gv-zones/tier-2` mirrored dataset, on the two 8 TB HDDs, quota 4 TB.
- `tank/gv-zones/tier-1` mirrored dataset, on the two 8 TB HDDs, quota 512 GB.

Hertog Jan would have:

- `tank/gv-zones/tier-2` mirrored dataset, on the two 4 TB HDDs, quota 4 TB.
- `tank/gv-zones/tier-1` mirrored dataset, on the two 4 TB HDDs, quota 512 GB.

And Cara Pils would have: 

- `tank/gv-zones/tier-1` on the 512 GB NVMe, quota 512 GB.

## Problems to solve

The two big problems to solve are the following:

- How do we make data opaquely accessible to a user when it could be stored on any node in the GV?
- How do we make sure data is replicated as quickly as possible on all nodes in the replication zone when modified?

### Opaque data access

We should use a custom NFS server for this.
XetData has written [nfsserve](https://github.com/xetdata/nfsserve) which seems to fit our needs quite well.
This way, we can easily mount a replication zone dataset (e.g. `tank/gv-zones/build-cache-fast`) and all reads & writes will go through our special NFS server implementation.
When a node wants to access its own data, it should also connect to its own NFS server locally instead of accessing the replication zone datasets directly.
That way, its reads and writes are treated in the exact same way as when accessed by any other node.
I'm not too concerned about the performance of such a solution here, because local networking on UNIX systems is very quick and IO is more likely to be the bottleneck.

In the end, I think it would be wonderful to have your home directory on aquaBSD be a replication zone and for almost all operations you do on your filesystem to go through the NFS/ZFS stack (except for system files, of course).
Actually, we should probably find a different name for such a filesystem? GVFS? Could be confused with GNOME's GVFS though. Maybe we should just always brand it as GrapeVineFS in full.

An alternative solution to having a custom NFS server is to check for filesystem events in the replication zone datasets, but I don't know that might be more error prone.

### Quick replication

This is actually gonna be super tricky.

Things to consider:

- A file could simultaneously be modified on two nodes in the GV. How to handle merging? Probably just keep both versions and notify user.
- What happens if we're offline for a while? We need to make sure everything is synced up when we come back online.

All GV nodes broadcast ELP (echolocation) packets on the GV.
These packets could also contain information about which files have been modified on the system.

Specifically, for each replication zone a given node A is in, it could expose a version number in its ELP.
If writes have happened to a zone since the last ELP, the version number is incremented.
If a node B sees a higher version number in the ELP of node A than its version number, it should pull the new data from that node.
Once it has done that (and only once it has done that! probably should block writing until then), it should update its own version.
That way, a node C in the same replication zone as B but without direct access to A will similarly pull data from B.

A lot of these issues have been solved by distributed VCSs like git, but obviously I don't want to reimplement git nor do I expect a user to handle merge conflicts.
