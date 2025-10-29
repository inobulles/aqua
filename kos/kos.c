// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2024-2025 Aymeric Wibo

#include "action.h"
#include "conn.h"
#include "gv.h"

#include "lib/vdriver.h"
#include "lib/vdriver_loader.h"

#include <aqua/gv_proto.h>
#include <aqua/kos.h>

#include <umber.h>

#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <ifaddrs.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static umber_class_t const* init_cls = NULL;
static umber_class_t const* notif_cls = NULL;
static umber_class_t const* action_cls = NULL;
static umber_class_t const* conn_cls = NULL;
static umber_class_t const* call_cls = NULL;

static bool has_init = false;
static uint64_t local_host_id;
static kos_notif_cb_t client_notif_cb = NULL;
static void* client_notif_data = NULL;

static kos_cookie_t cookies = 0;
static kos_ino_t inos = 0;

void __attribute__((constructor)) kos_init(void) {
	has_init = true;

	init_cls = umber_class_new("aqua.kos.init", UMBER_LVL_VERBOSE, "KOS initialization.");
	notif_cls = umber_class_new("aqua.kos.notif", UMBER_LVL_WARN, "KOS notifications.");
	action_cls = umber_class_new("aqua.kos.action", UMBER_LVL_WARN, "KOS action queue.");
	conn_cls = umber_class_new("aqua.kos.conn", UMBER_LVL_WARN, "KOS connections.");
	call_cls = umber_class_new("aqua.kos.call", UMBER_LVL_WARN, "KOS calls.");

	vdriver_loader_init();

	LOG_V(init_cls, "KOS initialized.");
}

kos_api_vers_t kos_hello(kos_api_vers_t min, kos_api_vers_t max, kos_descr_v4_t* descr) {
	assert(min <= max);
	assert(has_init);

	LOG_V(init_cls, "KOS hello.");

	if (min > KOS_API_V4 || max < KOS_API_V4) {
		LOG_E(init_cls, "KOS API version mismatch: requested [%" PRIu64 ", %" PRIu64 "], but only KOS_API_V4 is supported.", min, max);
		return KOS_API_VERS_NONE;
	}

	if (get_gv_host_id(&local_host_id) < 0) {
		LOG_I(init_cls, "KOS running locally.");
		local_host_id = 0;
	}

	descr->api_vers = KOS_API_V4;
	descr->best_api_vers = KOS_API_V4;
	strcpy((char*) descr->name, "Generic Unix-like system's KOS");

	LOG_I(init_cls, "KOS hello successful: API version %" PRIu64 ", best API version %" PRIu64 ", name \"%s\".", descr->api_vers, descr->best_api_vers, descr->name);

	return descr->api_vers;
}

static void notif_cb(kos_notif_t const* notif, void* data) {
	if (notif->kind >= KOS_NOTIF_KIND_COUNT) {
		LOG_E(notif_cls, "Received notification of unknown kind %d.", notif->kind);
		return;
	}

	LOG_V(notif_cls, "Received notification of kind %s.", kos_notif_kind_str[notif->kind]);

	switch (notif->kind) {
	case KOS_NOTIF_CONN:
		// TODO Describe what this is doing exactly, cuz even I'm not sure anymore.

		LOG_V(notif_cls, "Received connection notification for connection ID %" PRIu64 ".", notif->conn_id);

		assert(notif->conn_id < conn_count);
		conn_t* const conn = &conns[notif->conn_id];

		conn->alive = true;
		conn->fn_count = notif->conn.fn_count;
		conn->fns = notif->conn.fns;

		break;
	default:
		break;
	}

	LOG_V(notif_cls, "Forwarding notification to client.");
	client_notif_cb(notif, data);
}

void kos_sub_to_notif(kos_notif_cb_t cb, void* data) {
	assert(has_init);

	LOG_V(init_cls, "Subscribing to KOS notifications (cb=%p, data=%p).", cb, data);

	client_notif_cb = cb;
	client_notif_data = data;
}

static int write_ptr(kos_ptr_t ptr, void const* data, uint32_t size) {
	// XXX For now, only support writing pointers to local host.

	if (ptr.host_id != local_host_id) {
		return -1;
	}

	memcpy((void*) (uintptr_t) ptr.ptr, data, size);
	return 0;
}

void kos_req_vdev(char const* spec) {
	assert(has_init);

	// TODO Not sure I like how init_cls is used here.

	LOG_V(init_cls, "Trying to find local VDEV for spec \"%s\".", spec);
	vdriver_loader_req_local_vdev(spec, local_host_id, notif_cb, client_notif_data, write_ptr);

	LOG_V(init_cls, "Trying to find VDEV on the GrapeVine for spec '%s'.", spec);

	kos_vdev_descr_t* gv_vdevs;
	ssize_t const gv_vdev_count = query_gv_vdevs(&gv_vdevs);

	if (gv_vdev_count < 0) {
		return;
	}

	for (size_t i = 0; i < (size_t) gv_vdev_count; i++) {
		kos_vdev_descr_t* const gv_vdev = &gv_vdevs[i];

		if (strcmp((char*) gv_vdev->spec, spec) != 0) {
			continue;
		}

		LOG_V(init_cls, "Found GrapeVine VDEV for '%s' spec ('%s').", spec, gv_vdev->human);

		kos_notif_t notif = {
			.kind = KOS_NOTIF_ATTACH,
			.attach.vdev = *gv_vdev,
		};

		notif_cb(&notif, client_notif_data);
	}

	free(gv_vdevs);

	LOG_V(init_cls, "Done looking for VDEVs.");
}

static void conn_local(kos_cookie_t cookie, action_t* action, bool sync) {
	(void) sync; // This is always done synchronously.

	LOG_V(conn_cls, "Try to connect to VDEV locally (cookie=0x%" PRIx64 ").", cookie);

	vdriver_t* const vdriver = vdriver_loader_find_loaded_by_vid(action->conn.vdev_id);

	if (vdriver == NULL) {
		LOG_E(conn_cls, "Could not find a VDRIVER associated with VDEV ID %" PRIu64, action->conn.vdev_id);

		kos_notif_t notif = {
			.kind = KOS_NOTIF_CONN_FAIL,
			.cookie = cookie,
		};

		client_notif_cb(&notif, client_notif_data);
		return;
	}

	LOG_V(conn_cls, "Found VDRIVER, connecting.");
	vdriver->conn(cookie, action->conn.vdev_id, conn_new_local(vdriver));
}

static void conn_gv(kos_cookie_t cookie, action_t* action, bool sync) {
	LOG_V(
		conn_cls,
		"Trying to connect to VDEV (%" PRIu64 ":%" PRIu64 ") on the GrapeVine (cookie=0x%" PRIx64 ").",
		action->conn.host_id,
		action->conn.vdev_id,
		cookie
	);

	in_addr_t ipv4;

	if (gv_get_ip_by_host_id(action->conn.host_id, &ipv4) < 0) {
		LOG_E(conn_cls, "Failed to find IP address of host ID %" PRIu64 ".", action->conn.host_id);
		goto fail;
	}

	int const sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (sock < 0) {
		LOG_E(conn_cls, "Failed to create socket: %s", strerror(errno));
		goto fail;
	}

	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_port = htons(GV_PORT),
		.sin_addr.s_addr = ipv4,
	};

	if (connect(sock, (struct sockaddr*) &addr, sizeof addr) < 0) {
		LOG_E(conn_cls, "Failed to connect: %s", strerror(errno));
		close(sock);
		goto fail;
	}

	gv_packet_t const conn_packet = {
		.header.type = GV_PACKET_TYPE_CONN_VDEV,
		.conn_vdev.vdev_id = action->conn.vdev_id,
	};

	size_t const conn_size = sizeof conn_packet.header + sizeof conn_packet.conn_vdev;

	if (send(sock, &conn_packet, conn_size, 0) != (ssize_t) conn_size) {
		LOG_E(conn_cls, "Failed to send VDEV connection packet: %s", strerror(errno));
		close(sock);
		goto fail;
	}

	if (!sync) { // TODO
		LOG_W(conn_cls, "Currently, all GrapeVine VDEV connection requests are considered to be sync.");
	}

	LOG_V(conn_cls, "Wait for VDEV connection response.");

	gv_packet_t conn_res_packet;

	if (recv(sock, &conn_res_packet, sizeof conn_res_packet.header, 0) != sizeof conn_res_packet.header) {
		LOG_E(conn_cls, "Failed to get response header: %s", strerror(errno));
		close(sock);
		goto fail;
	}

	if (conn_res_packet.header.type == GV_PACKET_TYPE_CONN_VDEV_FAIL) {
		LOG_E(conn_cls, "Got a VDEV connection failure response.");
		close(sock);
		goto fail;
	}

	if (conn_res_packet.header.type != GV_PACKET_TYPE_CONN_VDEV_RES) {
		LOG_E(conn_cls, "Got a unexpected response to VDEV connection request: %s.", gv_packet_type_strs[conn_res_packet.header.type]);
		close(sock);
		goto fail;
	}

	if (recv(sock, &conn_res_packet.conn_vdev_res, sizeof conn_res_packet.conn_vdev_res, 0) != sizeof conn_res_packet.conn_vdev_res) {
		LOG_E(conn_cls, "Failed to get response payload: %s", strerror(errno));
		close(sock);
		goto fail;
	}

	gv_conn_vdev_res_t* const conn_vdev_res = malloc(conn_res_packet.conn_vdev_res.size);
	assert(conn_vdev_res != NULL);
	memcpy(conn_vdev_res, &conn_res_packet.conn_vdev_res, sizeof conn_res_packet.conn_vdev_res);
	size_t const remaining = (ssize_t) conn_vdev_res->size - sizeof *conn_vdev_res;

	if (recv(sock, (void*) conn_vdev_res + sizeof conn_res_packet.conn_vdev_res, remaining, 0) != (ssize_t) remaining) {
		LOG_E(conn_cls, "Failed to get response payload: %s", strerror(errno));
		free(conn_vdev_res);
		close(sock);
		goto fail;
	}

	LOG_V(conn_cls, "Managed to connect to VDEV (const_count=%zu, fn_count=%zu)!", conn_vdev_res->const_count, conn_vdev_res->fn_count);

	kos_notif_t notif = {
		.kind = KOS_NOTIF_CONN,
		.cookie = cookie,
		.conn = {
			.const_count = conn_vdev_res->const_count,
			.fn_count = conn_vdev_res->fn_count,
		},
	};

	// TODO Where the hell do we free all of this?

	// Deserialize the rest of the CONN_VDEV_RES packet.

	notif.conn.consts = malloc(conn_vdev_res->const_count * sizeof *notif.conn.consts);
	assert(notif.conn.consts != NULL);

	notif.conn.fns = malloc(conn_vdev_res->fn_count * sizeof *notif.conn.fns);
	assert(notif.conn.fns != NULL);

	void* buf = (void*) conn_vdev_res + sizeof *conn_vdev_res;

	for (size_t i = 0; i < conn_vdev_res->const_count; i++) {
		buf += gv_deserialize_const(buf, (kos_const_t*) &notif.conn.consts[i]);
	}

	for (size_t i = 0; i < conn_vdev_res->fn_count; i++) {
		buf += gv_deserialize_fn(buf, (kos_fn_t*) &notif.conn.fns[i]);
	}

	// Create connection.

	notif.conn_id = conn_new_gv(sock, conn_vdev_res->conn_id);
	conn_t* const conn = &conns[notif.conn_id];

	conn->alive = true;
	conn->fn_count = notif.conn.fn_count;
	conn->fns = notif.conn.fns;

	LOG_V(conn_cls, "Created connection (cid=%" PRIu64 ", remote_cid=%" PRIu64 ").", notif.conn_id, conn_vdev_res->conn_id);

	// Finally, send notification.

	free(conn_vdev_res);
	client_notif_cb(&notif, client_notif_data);

	return;

fail:;

	kos_notif_t fail_notif = {
		.kind = KOS_NOTIF_CONN_FAIL,
		.cookie = cookie,
	};

	client_notif_cb(&fail_notif, client_notif_data);
}

kos_cookie_t kos_vdev_conn(uint64_t host_id, uint64_t vdev_id) {
	// Generate cookie and add action to queue.

	kos_cookie_t const cookie = cookies++;

	action_t const action = {
		.cookie = cookie,
		.cb = host_id == local_host_id ? conn_local : conn_gv,
		.conn = {
			.host_id = host_id,
			.vdev_id = vdev_id,
		},
	};

	LOG_V(conn_cls, "Adding to action queue to request connection to %" PRIx64 ":%" PRIu64 " (cookie=0x%" PRIx64 ").", host_id, vdev_id, cookie);

	PUSH_QUEUE(action);

	if (action_queue_tail - action_queue_head > ACTION_QUEUE_SIZE) {
		LOG_E(conn_cls, "Too many actions in the KOS' action queue, dropping the most recent one.");
		action_queue_tail--;
	}

	return cookie;
}

static void call_local(kos_cookie_t cookie, action_t* action, bool sync) {
	(void) sync; // This is always done synchronously.

	LOG_V(call_cls, "Passing call on to VDRIVER (cookie=0x%" PRIx64 ").", cookie);

	conn_t* const conn = &conns[action->call.conn_id];

	vdriver_t* const vdriver = conn->vdriver;
	assert(vdriver != NULL);

	vdriver->call(cookie, action->call.conn_id, action->call.fn_id, action->call.args);
}

static void call_gv(kos_cookie_t cookie, action_t* action, bool sync) {
	LOG_V(call_cls, "Passing call on to GrapeVine connection (cookie=0x%" PRIx64 ").", cookie);
	conn_t* const conn = &conns[action->call.conn_id];

	// Serialize call.

	kos_fn_t const* const fn = &conn->fns[action->call.fn_id];

	gv_packet_t proto_packet = {
		.header.type = GV_PACKET_TYPE_KOS_CALL,
		.kos_call = {
			.conn_id = conn->remote_cid,
			.fn_id = action->call.fn_id,
		},
	};

	size_t size = sizeof proto_packet.header + sizeof proto_packet.kos_call;
	size_t const proto_packet_size = size;

	for (size_t i = 0; i < fn->param_count; i++) {
		size += gv_serialize_val_size(fn->params[i].type, &action->call.args[i]);
	}

	void* const packet = malloc(size);
	assert(packet != NULL);

	// TODO Probably a cleaner way to do this lol.

	proto_packet.kos_call.size = size - proto_packet_size;
	memcpy(packet, &proto_packet, sizeof proto_packet);

	void* buf = packet + proto_packet_size;

	for (size_t i = 0; i < fn->param_count; i++) {
		buf += gv_serialize_val(buf, fn->params[i].type, &action->call.args[i]);
	}

	// Send packet.

	if (send(conn->sock, packet, size, 0) != (ssize_t) size) {
		LOG_E(call_cls, "Failed to send KOS call packet: %s", strerror(errno));
		free(packet);
		goto fail;
	}

	if (!sync) { // TODO
		LOG_W(call_cls, "Currently, all GrapeVine VDEV connection requests are considered to be sync.");
	}

	LOG_V(call_cls, "Wait for KOS call return.");
	gv_packet_t res_packet;

	if (recv(conn->sock, &res_packet, sizeof res_packet.header, 0) != sizeof res_packet.header) {
		LOG_E(call_cls, "Failed to get response header: %s", strerror(errno));
		goto fail;
	}

	if (res_packet.header.type == GV_PACKET_TYPE_KOS_CALL_FAIL) {
		LOG_E(call_cls, "Got a KOS call failure response.");
		goto fail;
	}

	if (res_packet.header.type != GV_PACKET_TYPE_KOS_CALL_RET) {
		LOG_E(call_cls, "Got a unexpected response to KOS call: %s.", gv_packet_type_strs[res_packet.header.type]);
		goto fail;
	}

	gv_kos_call_ret_t ret;

	if (recv(conn->sock, &ret, sizeof ret, 0) != (ssize_t) sizeof ret) {
		LOG_E(call_cls, "Failed to get response payload (part 1): %s", strerror(errno));
		free(packet);
		goto fail;
	}

	void* const ret_buf = malloc(ret.size);
	assert(ret_buf != NULL);

	if (recv(conn->sock, ret_buf, ret.size, 0) != (ssize_t) ret.size) {
		LOG_E(call_cls, "Failed to get response payload (part 2): %s", strerror(errno));
		free(packet);
		goto fail;
	}

	kos_type_t const ret_type = fn->ret_type;
	kos_val_t ret_val;

	size_t const deserialized_size = gv_deserialize_val(ret_buf, ret_type, &ret_val);

	if (deserialized_size != ret.size) {
		LOG_E(call_cls, "Deserialized size (%zu) not expected size (%zu).", deserialized_size, ret.size);
		free(packet);
		goto fail;
	}

	free(packet);

	LOG_V(call_cls, "Got return response, notifying the client.");

	kos_notif_t const notif = {
		.kind = KOS_NOTIF_CALL_RET,
		.cookie = cookie,
		.conn_id = action->call.conn_id,
		.call_ret.ret = ret_val,
	};

	client_notif_cb(&notif, client_notif_data);
	return;

fail:

	; // TODO Send CALL_FAIL notification.
}

static void call_fail(kos_cookie_t cookie, action_t* action, bool sync) {
	(void) sync; // This is always done synchronously.
	(void) action;

	LOG_E(call_cls, "Call failed for cookie 0x%" PRIx64 ".", cookie);

	kos_notif_t const notif = {
		.kind = KOS_NOTIF_CALL_FAIL,
		.cookie = cookie,
	};

	client_notif_cb(&notif, client_notif_data);
}

kos_cookie_t kos_vdev_call(uint64_t conn_id, uint32_t fn_id, void const* args) {
	// Generate cookie.

	kos_cookie_t const cookie = cookies++;

	action_t action = {
		.cookie = cookie,
		.cb = call_fail,
	};

	LOG_V(call_cls, "Adding to action queue to call function %u on connection %" PRIu64 " (cookie=0x%" PRIx64 ").", fn_id, conn_id, cookie);

	// Find connection.

	if (conn_id >= conn_count) {
		LOG_E(call_cls, "Connection ID %" PRIu64 " invalid.", conn_id);
		goto fail;
	}

	conn_t* const conn = &conns[conn_id];

	if (!conn->alive) {
		LOG_E(call_cls, "Connection ID %" PRIu64 " is not alive.", conn_id);
		goto fail;
	}

	if (fn_id >= conn->fn_count) {
		LOG_E(call_cls, "Function ID %u is invalid.", fn_id);
		goto fail;
	}

	// Success!

	action.cb = conn->type == CONN_TYPE_LOCAL ? call_local : call_gv;

	action.call.conn_id = conn_id;
	action.call.fn_id = fn_id;
	action.call.args = args;

fail:;

	// Actually add action to queue.

	PUSH_QUEUE(action);

	if (action_queue_tail - action_queue_head > ACTION_QUEUE_SIZE) {
		LOG_E(call_cls, "Too many actions in the KOS' action queue, dropping the most recent one.");
		action_queue_tail--;
	}

	return cookie;
}

void kos_flush(bool sync) {
	LOG_V(action_cls, "Flushing KOS action queue (sync=%d).", sync);

	while (action_queue_head != action_queue_tail) {
		action_t action;
		POP_QUEUE(action);
		action.cb(action.cookie, &action, sync);
	}
}

void kos_vdev_disconn(uint64_t conn_id) {
	LOG_V(conn_cls, "Disconnecting from VDEV with connection ID %" PRIu64 ".", conn_id);

	if (conn_id >= conn_count) {
		LOG_E(conn_cls, "Connection ID %" PRIu64 " invalid.", conn_id);
		return;
	}

	if (conns[conn_id].type == CONN_TYPE_GV) {
		close(conns[conn_id].sock);
	}

	conns[conn_id].alive = false;
}

kos_ino_t kos_gen_ino(void) {
	return inos++;
}
