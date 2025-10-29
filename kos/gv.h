// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

/**
 * Interface used by the KOS to read IPC information provided by the GrapeVine daemon.
 */

#pragma once

#include <aqua/kos.h>

#include <netinet/in.h>
#include <unistd.h>

/**
 * Get all VDEVs available on the GrapeVine.
 *
 * @param vdevs_out Pointer to an array of VDEV descriptors that will be filled with the available VDEVs.
 * @return The number of VDEVs found, or a negative value on error.
 */
ssize_t query_gv_vdevs(kos_vdev_descr_t** vdevs_out);

/**
 * Get the host ID of the current machine.
 *
 * @param host_id_out Pointer to a variable that will be filled with the host ID.
 * @return 0 on success, or a negative value on error.
 */
int get_gv_host_id(uint64_t* host_id_out);

/**
 * Get the IP address of a node by its host ID.
 *
 * TODO Don't really know how we should handle IPv6 here nor do I really care at the moment.
 *
 * @param host_id The host ID of the node we wanna get the IP of.
 * @param ipv4 Reference to the IPv4 address the node IP should be written to.
 * @return 0 on success, or a negative value if node couldn't be found.
 */
int gv_get_ip_by_host_id(uint64_t host_id, in_addr_t* ipv4);
