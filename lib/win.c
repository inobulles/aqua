// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2025 Aymeric Wibo

#include "win.h"
#include <aqua/kos.h>

#include <stdbool.h>
#include <string.h>

#define SPEC "aquabsd.black.win"

void win_init(void) {
	kos_req_vdev("aquabsd.black.win");
}

bool win_probe(kos_vdev_descr_t const* vdev) {
	return strcmp((char*) vdev->spec, SPEC) == 0;
}

void win_conn(win_softc_t* sc, kos_vdev_descr_t const* vdev) {
	sc->hid = vdev->host_id;
	sc->vid = vdev->vdev_id;

	sc->is_conn = false;
	sc->last_cookie = kos_vdev_conn(sc->hid, sc->vid);
	kos_flush(true);
}

void win_notif_conn(win_softc_t* sc, kos_notif_t const* notif) {
	if (notif->cookie != sc->last_cookie) {
		return;
	}

	sc->conn_id = notif->conn.conn_id;
	sc->is_conn = true;

	// Read functions.

	memset(&sc->fns, 0xFF, sizeof sc->fns);

	for (size_t i = 0; i < notif->conn.fn_count; i++) {
		kos_fn_t const* const fn = &notif->conn.fns[i];
		char const* const name = (void*) fn->name;

		if (
			strcmp(name, "create") == 0 &&
			fn->ret_type == KOS_TYPE_OPAQUE_PTR &&
			fn->param_count == 0
		) {
			sc->fns.create = i;
		}

		if (
			strcmp(name, "destroy") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "win") == 0
		) {
			sc->fns.destroy = i;
		}

		if (
			strcmp(name, "loop") == 0 &&
			fn->ret_type == KOS_TYPE_VOID &&
			fn->param_count == 1 &&
			fn->params[0].type == KOS_TYPE_OPAQUE_PTR &&
			strcmp((char*) fn->params[0].name, "win") == 0
		) {
			sc->fns.loop = i;
		}
	}

	for (size_t i = 0; i < sizeof sc->fns / sizeof(uint32_t); i++) {
		if (((uint32_t*) &sc->fns)[i] == -1u) {
			sc->is_conn = false;
			break;
		}
	}
}

void win_notif_call_ret(win_softc_t* sc, kos_notif_t const* notif) {
	if (!sc->is_conn || notif->cookie != sc->last_cookie) {
		return;
	}

	sc->last_ret = notif->call_ret.ret;
}

// Actual function implementations.

win_t win_create(win_softc_t* sc) {
	if (!sc->is_conn) {
		return NULL;
	}

	sc->last_cookie = kos_vdev_call(sc->conn_id, sc->fns.create, NULL);
	kos_flush(true);

	return sc->last_ret.opaque_ptr;
}

void win_destroy(win_softc_t* sc, win_t win) {
	if (!sc->is_conn) {
		return;
	}

	kos_vdev_call(sc->conn_id, sc->fns.destroy, (kos_val_t[]) {
		{ .opaque_ptr = win, },
	});

	kos_flush(true);
}

void win_loop(win_softc_t* sc, win_t win) {
	if (!sc->is_conn) {
		return;
	}

	kos_vdev_call(sc->conn_id, sc->fns.loop, (kos_val_t[]) {
		{ .opaque_ptr = win, },
	});

	kos_flush(true);
}
