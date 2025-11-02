// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2024-2025 Aymeric Wibo

#include "conn.h"
#include "query.h"

#include <aqua/gv_proto.h>
#include <aqua/vdriver_loader.h>

#include <umber.h>

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <spawn.h>
#include <stdio.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>

#include <arpa/inet.h>

static umber_class_t const* cls = NULL;

static __attribute__((constructor)) void init(void) {
	cls = umber_class_new("aqua.gv.conn", UMBER_LVL_INFO, "GrapeVine daemon connection handling.");
}

static void send_sock_to_proc(vdriver_t* vdriver, conn_t* conn, uint64_t vdev_id) {
	LOG_V(cls, "Passing connection to another process through a UDS (spec=%s).", vdriver->spec);

	// Create and connect to UDS.

	int const uds = socket(AF_UNIX, SOCK_STREAM, 0);

	if (uds < 0) {
		LOG_E(cls, "socket(AF_UNIX): %s", strerror(errno));
		return;
	}

	struct sockaddr_un addr = {0};
	addr.sun_family = AF_UNIX;
	addr.sun_path[0] = '\0'; // Abstract UDS.
	strncpy(addr.sun_path + 1, vdriver->spec, sizeof addr.sun_path - 1);

	if (connect(uds, (struct sockaddr*) &addr, sizeof addr) < 0) {
		LOG_E(cls, "connect: %s", strerror(errno));
		goto err_connect;
	}

	// Send over VDEV connection's socket.

	char control[CMSG_SPACE(sizeof(int))] = {0};

	struct iovec iov = {
		.iov_base = &vdev_id,
		.iov_len = sizeof vdev_id
	};

	struct msghdr msg = {
		.msg_control = control,
		.msg_controllen = sizeof control,
		.msg_iov = &iov,
		.msg_iovlen = 1,
	};

	struct cmsghdr* const cmsg = CMSG_FIRSTHDR(&msg);
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	cmsg->cmsg_len = CMSG_LEN(sizeof(int));

	memcpy(CMSG_DATA(cmsg), &conn->sock, sizeof conn->sock);

	if (sendmsg(uds, &msg, 0) < 0) {
		LOG_E(cls, "sendmsg: %s", strerror(errno));
		goto err_sendmsg;
	}

err_sendmsg:
err_connect:

	close(uds);
}

static void spawn_kos_agent(vdriver_t* vdriver, conn_t* conn, uint64_t vdev_id) {
	// Spawn KOS agent process.
	// TODO Note that if you're stuck on an issue here, it might be that gv-agent failed to start; it will fail silently if so!

	LOG_V(cls, "Spawning KOS agent process (spec=%s).", vdriver->spec);

	char vid_str[16];
	snprintf(vid_str, sizeof vid_str, "%" PRIu64, vdev_id);

	char* const path = "gv-agent";
	char* const argv[] = {path, "-s", vdriver->spec, "-v", vid_str, NULL};

	posix_spawn_file_actions_t actions;
	posix_spawn_file_actions_init(&actions);

	posix_spawn_file_actions_adddup2(&actions, conn->sock, 3); // Make socket available as fd 3 in child.

	extern char** environ;
	pid_t pid;

	if (posix_spawnp(&pid, path, &actions, NULL, argv, environ) < 0) {
		LOG_E(cls, "posix_spawnp(\"%s\"): %s", path, strerror(errno));
	}

	posix_spawn_file_actions_destroy(&actions);

	// TODO We should keep track of all our KOS agents so that we can ask them to terminate all connections when we go down.
}

static void conn_vdev(conn_t* conn, uint64_t vdev_id) {
	// TODO If this fails, we are responsible for sending a CONN_FAIL (or whatever).

	LOG_V(cls, "Looking for VDRIVER associated to VID %" PRIu64 ".", vdev_id);

	vdriver_t* const vdriver = vdriver_loader_find_loaded_by_vid(vdev_id);

	if (vdriver == NULL) {
		LOG_E(cls, "Could not find associated VDRIVER.");
		goto done;
	}

	if (strcmp(vdriver->spec, "aquabsd.black.vr") == 0) { // TODO Hardcoding this for the time being.
		send_sock_to_proc(vdriver, conn, vdev_id);
	}

	else {
		spawn_kos_agent(vdriver, conn, vdev_id);
	}

done:

	close(conn->sock);
}

void* conn_thread(void* arg) {
	conn_t* const conn = arg;

	gv_packet_t buf;
	int len;

	// The recvs for packets we don't expect to receive are just a best attempt at clearing the buffer until the next packet.
	// Doesn't really matter if they fail, this is a bad condition to be in anyway and we did our best to recover.

	while ((len = recv(conn->sock, &buf.header, sizeof buf.header, 0)) > 0) {
		// TODO We should check that we can even do gv_packet_type_strs[buf.header.type]... Maybe even just have an inline function to do this for us with the table hidden inside so we don't access this directly.
		// TODO Should we really be goto stop;'ing everywhere?

		LOG_V(
			cls,
			"Got %s packet from %s:0x%x.",
			gv_packet_type_strs[buf.header.type],
			inet_ntoa(conn->addr.sin_addr),
			ntohs(conn->addr.sin_port)
		);

		switch (buf.header.type) {
		case GV_PACKET_TYPE_ELP:
			recv(conn->sock, &buf.elp, sizeof buf.elp, 0);

			LOG_E(
				cls,
				"Received ELP from %s:0x%x (host %" PRIx64 ") on TCP. This should not happen!",
				inet_ntoa(conn->addr.sin_addr),
				ntohs(conn->addr.sin_port),
				buf.elp.host_id
			);
			break;
		case GV_PACKET_TYPE_QUERY:
			if (query_res(conn) < 0) {
				goto stop;
			}

			break;
		case GV_PACKET_TYPE_QUERY_RES:
			recv(conn->sock, &buf.query_res, sizeof buf.query_res, 0);

			LOG_E(
				cls,
				"Received QUERY_RES from %s:0x%x. This should not happen, as this connection isn't ever used to send out QUERY packets!",
				inet_ntoa(conn->addr.sin_addr),
				ntohs(conn->addr.sin_port)
			);
			break;
		case GV_PACKET_TYPE_CONN_VDEV:
			if (recv(conn->sock, &buf.conn_vdev, sizeof buf.conn_vdev, 0) != sizeof buf.conn_vdev) {
				LOG_E(cls, "recv: %s", strerror(errno));
				goto stop;
			}

			conn_vdev(conn, buf.conn_vdev.vdev_id);
			break;
		case GV_PACKET_TYPE_CONN_VDEV_RES:
			recv(conn->sock, &buf.conn_vdev_res, sizeof buf.conn_vdev_res, 0);
			__attribute__((fallthrough));
		case GV_PACKET_TYPE_CONN_VDEV_FAIL:
			LOG_E(
				cls,
				"Received %s from %s:0x%x (host %" PRIx64 "). This should not happen, as this connection isn't ever used to send out CONN_VDEV packets!",
				gv_packet_type_strs[buf.header.type],
				inet_ntoa(conn->addr.sin_addr),
				ntohs(conn->addr.sin_port),
				buf.elp.host_id
			);
			break;
		case GV_PACKET_TYPE_KOS_CALL:
			// TODO Read remaining bytes of packet.
		case GV_PACKET_TYPE_KOS_CALL_FAIL:
			// TODO Read remaining bytes of packet.
		case GV_PACKET_TYPE_KOS_CALL_RET:
			// TODO Read remaining bytes of packet.
			LOG_E(
				cls,
				"Received %s from %s:0x%x (host %" PRIx64 "). This should not happen, as this connection should have already been passed on to a KOS agent!",
				gv_packet_type_strs[buf.header.type],
				inet_ntoa(conn->addr.sin_addr),
				ntohs(conn->addr.sin_port),
				buf.elp.host_id
			);
			break;
		case GV_PACKET_TYPE_LEN:
			assert(false);
		}
	}

stop:

	close(conn->sock);
	conn->slot_used = false;

	return NULL;
}
