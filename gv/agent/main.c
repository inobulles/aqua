// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#include <aqua/gv_proto.h>
#include <aqua/kos.h>

#include <umber.h>

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct {
	int sock;
	uint64_t hid;
	uint64_t vid;
	bool vdev_found;
	kos_cookie_t conn_cookie;

	uint32_t last_fn_id;
	uint32_t fn_count;
	kos_fn_t const* fns;
} state_t;

static umber_class_t const* cls;

static void notif_cb(kos_notif_t const* notif, void* data) {
	state_t* const s = data;

	// TODO In here, notifications should be serialized and sent over our socket.

	void* packet_data = malloc(sizeof(gv_packet_t));
	assert(packet_data != NULL);

	gv_packet_t* packet = packet_data;
	size_t size = 0; // If size == 0, there is no packet to send.

	switch (notif->kind) {
	case KOS_NOTIF_ATTACH:
		// TODO I'm not sure these VIDs can just be sent like this. I think this depends a lot on the order of VDRIVERs loaded by gvd. Not sure what an elegant solution is here. Perhaps we mask out the slice?

		if (notif->attach.vdev.vdev_id != s->vid) {
			break;
		}

		LOG_I(cls, "Found our VDEV: %s.", notif->attach.vdev.human);

		s->hid = notif->attach.vdev.host_id;
		s->vdev_found = true;

		break;
	case KOS_NOTIF_DETACH:
	case KOS_NOTIF_CONN_FAIL:
		packet->header.type = GV_PACKET_TYPE_CONN_VDEV_FAIL;
		size = sizeof packet->header;

		break;
	case KOS_NOTIF_CONN:
		if (notif->cookie != s->conn_cookie) {
			break;
		}

		// Keep track of functions for future calls.

		s->fn_count = notif->conn.fn_count;
		s->fns = notif->conn.fns;

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
		LOG_V(cls, "Got call return notification from KOS.");
		packet->header.type = GV_PACKET_TYPE_KOS_CALL_RET;

		kos_type_t const ret_type = s->fns[s->last_fn_id].ret_type;
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

	if (size != 0 && send(s->sock, packet, size, 0) != (ssize_t) size) {
		LOG_E(cls, "Failed to send %s packet.", gv_packet_type_strs[packet->header.type]);
	}

	free(packet);
}

static void call(state_t* s, gv_kos_call_t* call) {
	LOG_V(cls, "Calling KOS function (fn_id=%u).", call->fn_id);

	void* const arg_buf = malloc(call->size);
	assert(arg_buf != NULL);

	if (recv(s->sock, arg_buf, call->size, 0) != (ssize_t) call->size) {
		LOG_E(cls, "recv: %s", strerror(errno));
		free(arg_buf);
		goto fail;
	}

	if (call->fn_id >= s->fn_count) { // Do this after so we do clear the buffer.
		LOG_E(cls, "Function ID %zu doesn't exist (%zu functions total).", call->fn_id, s->fn_count);
		free(arg_buf);
		goto fail;
	}

	size_t const arg_count = s->fns[call->fn_id].param_count;
	kos_val_t* const args = malloc(arg_count * sizeof *args);
	assert(args != NULL);

	size_t size = 0;

	for (size_t i = 0; i < arg_count; i++) {
		size += gv_deserialize_val(arg_buf + size, s->fns[call->fn_id].params[i].type, &args[i]);
	}

	free(arg_buf);

	if (size != call->size) {
		LOG_E(cls, "Deserialized size (%zu) is not the same as reported size (%zu).", size, call->size);
		free(args);
		goto fail;
	}

	s->last_fn_id = call->fn_id;
	kos_vdev_call(call->conn_id, call->fn_id, args);
	kos_flush(true);

	return;

fail:

	LOG_F(cls, "TODO");
}

int main(int argc, char* argv[]) {
	cls = umber_class_new("gv.agent", UMBER_LVL_WARN, "GrapeVine KOS agent.");

	state_t s = {
		.sock = 3, // Set by gvd when spawning us.
		.vid = -1ull,
		.vdev_found = false,
	};

	char const* spec = NULL;

	int c;

	while ((c = getopt(argc, argv, "s:v:")) != -1) {
		switch (c) {
		case 's':
			spec = optarg;
			break;
		case 'v':
			s.vid = atoi(optarg);
			break;
		default:
			LOG_F(cls, "Unknown option: %c", c);
			return EXIT_FAILURE;
		}
	}

	argc -= optind;
	argv += optind;

	if (argc != 0) {
		LOG_F(cls, "More arguments than expected.");
		return EXIT_FAILURE;
	}

	if (spec == NULL) {
		LOG_F(cls, "Expected spec argument (-s).");
		return EXIT_FAILURE;
	}

	if (s.vid == -1ull) {
		LOG_F(cls, "Expected VID argument (-v).");
		return EXIT_FAILURE;
	}

	LOG_V(cls, "Initiate connection with KOS.");
	kos_descr_v4_t descr;

	if (kos_hello(KOS_API_V4, KOS_API_V4, &descr) != KOS_API_V4) {
		LOG_F(cls, "Could not initiate connection with KOS.");
		return EXIT_FAILURE;
	}

	LOG_V(cls, "Subscribe to KOS notifications.");
	kos_sub_to_notif(notif_cb, &s);

	LOG_V(cls, "Request %s.", spec);
	kos_req_vdev(spec); // TODO Maybe there should be a way to just request local?

	// TODO We don't need a flush call with the current implementation, but should we require one? I'm guessing yes, but this should be defined in the spec.

	if (!s.vdev_found) {
		LOG_E(cls, "Couldn't find VDEV with ID %" PRIu64 ".", s.vid);

		// TODO Here, if we didn't find the VDEV we were looking for, we should send a CONN_FAIL.

		return EXIT_FAILURE;
	}

	LOG_V(cls, "Establish connection to VDEV.");

	s.conn_cookie = kos_vdev_conn(s.hid, s.vid);
	kos_flush(true);

	LOG_V(cls, "Listening for packets.");

	gv_packet_t buf;
	int len;

	while ((len = recv(s.sock, &buf.header, sizeof buf.header, 0)) > 0) {
		LOG_V(cls, "Got %s packet.", gv_packet_type_strs[buf.header.type]);

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
			LOG_E(cls, "Unexpected packet. This should not happen!");
			break;
		case GV_PACKET_TYPE_KOS_CALL:
			if (recv(s.sock, &buf.kos_call, sizeof buf.kos_call, 0) != sizeof buf.kos_call) {
				LOG_E(cls, "recv: %s", strerror(errno));
				break;
			}

			call(&s, &buf.kos_call);
			break;
		}
	}

	return EXIT_SUCCESS;
}
