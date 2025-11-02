// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#include "agent.h"

#include <aqua/gv_proto.h>
#include <aqua/kos.h>

#include <umber.h>

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <string.h>
#include <sys/socket.h>

struct gv_agent_t {
	umber_class_t const* cls;

	int sock;
	uint64_t hid;
	uint64_t vid;
	bool vdev_found;

	kos_cookie_t conn_cookie;
	uint64_t conn_id;

	uint32_t last_fn_id;
	uint32_t fn_count;
	kos_fn_t const* fns;
};

static void notif_cb(kos_notif_t const* notif, void* data) {
	gv_agent_t* const a = data;

	// TODO In here, notifications should be serialized and sent over our socket.

	void* packet_data = malloc(sizeof(gv_packet_t));
	assert(packet_data != NULL);

	gv_packet_t* packet = packet_data;
	size_t size = 0; // If size == 0, there is no packet to send.

	switch (notif->kind) {
	case KOS_NOTIF_ATTACH:
		// TODO I'm not sure these VIDs can just be sent like this. I think this depends a lot on the order of VDRIVERs loaded by gvd. Not sure what an elegant solution is here. Perhaps we mask out the slice?

		if (notif->attach.vdev.vdev_id != a->vid) {
			break;
		}

		LOG_I(a->cls, "Found our VDEV: %s.", notif->attach.vdev.human);

		a->hid = notif->attach.vdev.host_id;
		a->vdev_found = true;

		break;
	case KOS_NOTIF_DETACH:
	case KOS_NOTIF_CONN_FAIL:
		packet->header.type = GV_PACKET_TYPE_CONN_VDEV_FAIL;
		size = sizeof packet->header;

		break;
	case KOS_NOTIF_CONN:
		if (notif->cookie != a->conn_cookie) {
			break;
		}

		a->conn_id = notif->conn_id;

		// Keep track of functions for future calls.

		a->fn_count = notif->conn.fn_count;
		a->fns = notif->conn.fns;

		// Prepare packet.

		packet->header.type = GV_PACKET_TYPE_CONN_VDEV_RES;
		gv_conn_vdev_res_t* conn_vdev_res = &packet->conn_vdev_res;
		size = sizeof packet->header + sizeof *conn_vdev_res;

		conn_vdev_res->conn_id = notif->conn_id;
		conn_vdev_res->const_count = notif->conn.const_count;
		conn_vdev_res->fn_count = notif->conn.fn_count;

		// Serialize consts.

		size_t consts_size = 0;

		for (size_t i = 0; i < notif->conn.const_count; i++) {
			consts_size += gv_serialize_const_size(&notif->conn.consts[i]);
		}

		packet = realloc(packet, size + consts_size);
		assert(packet != NULL);

		size_t const prev_size = size;

		for (size_t i = 0; i < notif->conn.const_count; i++) {
			// Don't forget to cast to void*!!!! Thank you Théo <3
			size += gv_serialize_const((void*) packet + size, &notif->conn.consts[i]);
		}

		assert(size == prev_size + consts_size);

		// Serialize functions.

		size_t fns_size = 0;

		for (size_t i = 0; i < notif->conn.fn_count; i++) {
			fns_size += gv_serialize_fn_size(&notif->conn.fns[i]);
		}

		packet = realloc(packet, size + fns_size);
		assert(packet != NULL);

		for (size_t i = 0; i < notif->conn.fn_count; i++) {
			size += gv_serialize_fn((void*) packet + size, &notif->conn.fns[i]);
		}

		conn_vdev_res = &packet->conn_vdev_res;
		conn_vdev_res->size = size - sizeof packet->header;

		break;
	case KOS_NOTIF_CALL_FAIL:
		// TODO
		break;
	case KOS_NOTIF_CALL_RET:
		LOG_V(a->cls, "Got call return notification from KOS.");
		packet->header.type = GV_PACKET_TYPE_KOS_CALL_RET;

		kos_type_t const ret_type = a->fns[a->last_fn_id].ret_type;
		kos_val_t const* const ret = &notif->call_ret.ret;

		size_t const val_size = gv_serialize_val_size(ret_type, ret);
		packet->kos_call_ret.size = val_size;

		size_t const proto_header_size = sizeof packet->header + sizeof packet->kos_call_ret;
		size = proto_header_size + val_size;

		packet = realloc(packet, size);
		assert(packet != NULL);

		assert(gv_serialize_val((void*) packet + proto_header_size, ret_type, ret) == val_size);

		break;
	case KOS_NOTIF_INTERRUPT:
		break;
	}

	if (size != 0 && send(a->sock, packet, size, 0) != (ssize_t) size) {
		LOG_E(a->cls, "Failed to send %s packet.", gv_packet_type_strs[packet->header.type]);
	}

	free(packet);
}

static void call(gv_agent_t* a, gv_kos_call_t* call) {
	LOG_V(a->cls, "Calling KOS function (fn_id=%u).", call->fn_id);

	void* const arg_buf = malloc(call->size);
	assert(arg_buf != NULL);

	if (recv(a->sock, arg_buf, call->size, 0) != (ssize_t) call->size) {
		LOG_E(a->cls, "recv: %s", strerror(errno));
		free(arg_buf);
		goto fail;
	}

	if (call->fn_id >= a->fn_count) { // Do this after so we do clear the buffer.
		LOG_E(a->cls, "Function ID %zu doesn't exist (%zu functions total).", call->fn_id, a->fn_count);
		free(arg_buf);
		goto fail;
	}

	size_t const arg_count = a->fns[call->fn_id].param_count;
	kos_val_t* const args = malloc(arg_count * sizeof *args);
	assert(args != NULL);

	size_t size = 0;

	for (size_t i = 0; i < arg_count; i++) {
		size += gv_deserialize_val(arg_buf + size, a->fns[call->fn_id].params[i].type, &args[i]);
	}

	free(arg_buf);

	if (size != call->size) {
		LOG_E(a->cls, "Deserialized size (%zu) is not the same as reported size (%zu).", size, call->size);
		free(args);
		goto fail;
	}

	a->last_fn_id = call->fn_id;
	kos_vdev_call(call->conn_id, call->fn_id, args);
	kos_flush(true);

	return;

fail:

	LOG_F(a->cls, "TODO");
}

gv_agent_t* gv_agent_create(int sock, char const* spec, uint64_t vdev_id) {
	gv_agent_t* const a = calloc(1, sizeof *a);
	assert(a != NULL);

	a->cls = umber_class_new("gv.agent", UMBER_LVL_WARN, "GrapeVine KOS agent (library).");
	a->sock = sock;
	a->vid = vdev_id;
	a->vdev_found = false;

	LOG_V(a->cls, "Initiate connection with KOS.");
	kos_descr_v4_t descr;

	if (kos_hello(KOS_API_V4, KOS_API_V4, &descr) != KOS_API_V4) {
		LOG_F(a->cls, "Could not initiate connection with KOS.");
		gv_agent_destroy(a);
		return NULL;
	}

	LOG_V(a->cls, "Subscribe to KOS notifications.");
	kos_sub_to_notif(notif_cb, a);

	LOG_V(a->cls, "Request %s.", spec);
	kos_req_vdev(spec); // TODO Maybe there should be a way to just request local?

	// TODO We don't need a flush call with the current implementation, but should we require one? I'm guessing yes, but this should be defined in the spec.

	if (!a->vdev_found) {
		LOG_E(a->cls, "Couldn't find VDEV with ID %" PRIu64 ".", a->vid);

		// TODO Here, if we didn't find the VDEV we were looking for, we should send a CONN_FAIL.

		gv_agent_destroy(a);
		return NULL;
	}

	LOG_V(a->cls, "Establish connection to VDEV.");

	a->conn_cookie = kos_vdev_conn(a->hid, a->vid);
	kos_flush(true);

	return a;
}

void gv_agent_loop(gv_agent_t* a) {
	LOG_V(a->cls, "Listening for packets.");

	gv_packet_t buf;
	int len;

	while ((len = recv(a->sock, &buf.header, sizeof buf.header, 0)) > 0) {
		LOG_V(a->cls, "Got %s packet.", gv_packet_type_strs[buf.header.type]);

		switch (buf.header.type) {
		case GV_PACKET_TYPE_ELP:
		case GV_PACKET_TYPE_QUERY:
		case GV_PACKET_TYPE_QUERY_RES:
		case GV_PACKET_TYPE_CONN_VDEV:
		case GV_PACKET_TYPE_CONN_VDEV_FAIL:
		case GV_PACKET_TYPE_CONN_VDEV_RES:
		case GV_PACKET_TYPE_KOS_CALL_FAIL:
		case GV_PACKET_TYPE_KOS_CALL_RET:
		case GV_PACKET_TYPE_LEN:
		default:
			LOG_E(a->cls, "Unexpected packet. This should not happen!");
			break;
		case GV_PACKET_TYPE_KOS_CALL:
			if (recv(a->sock, &buf.kos_call, sizeof buf.kos_call, 0) != sizeof buf.kos_call) {
				LOG_E(a->cls, "recv: %s", strerror(errno));
				break;
			}

			call(a, &buf.kos_call);
			break;
		}
	}
}

void gv_agent_destroy(gv_agent_t* a) {
	// TODO Should we also be responsible for closing the socket?

	kos_vdev_disconn(a->conn_id);

	free((void*) a->cls);
	free(a);
}
