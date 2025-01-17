// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2025 Aymeric Wibo

#include "win.h"
#include <aqua/kos.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define SPEC "aquabsd.black.win"

struct win_softc_t {
	uint64_t hid;
	uint64_t vid;

	kos_cookie_t last_cookie;

	bool is_conn;
	uint64_t conn_id;

	struct {
		uint32_t create;
		uint32_t destroy;
		uint32_t register_interrupt;
		uint32_t loop;
	} fns;

	kos_val_t last_ret;
	kos_ino_t ino;
};

struct win_t {
	win_softc_t sc;
	void* opaque_ptr;
	kos_ino_t ino;
};

void win_init(void) {
	kos_req_vdev("aquabsd.black.win");
}

bool win_probe(kos_vdev_descr_t const* vdev) {
	return strcmp((char*) vdev->spec, SPEC) == 0;
}

win_softc_t win_conn(kos_vdev_descr_t const* vdev) {
	win_softc_t const sc = calloc(1, sizeof *sc);

	if (sc == NULL) {
		return NULL;
	}

	sc->hid = vdev->host_id;
	sc->vid = vdev->vdev_id;

	sc->is_conn = false;
	sc->last_cookie = kos_vdev_conn(sc->hid, sc->vid);

	return sc;
}

void win_disconn(win_softc_t sc) {
	if (sc == NULL) {
		return;
	}

	if (!sc->is_conn) {
		free(sc);
		return;
	}

	kos_vdev_disconn(sc->conn_id);
	free(sc);
}

void win_notif_conn(win_softc_t sc, kos_notif_t const* notif) {
	if (sc == NULL || notif->cookie != sc->last_cookie) {
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

void win_notif_call_ret(win_softc_t sc, kos_notif_t const* notif) {
	if (sc == NULL || !sc->is_conn || notif->cookie != sc->last_cookie) {
		return;
	}

	sc->last_ret = notif->call_ret.ret;
}

// Actual function implementations.

win_t win_create(win_softc_t sc) {
	if (sc == NULL || !sc->is_conn) {
		return NULL;
	}

	win_t const win = calloc(1, sizeof *win);

	if (win == NULL) {
		return NULL;
	}

	sc->last_cookie = kos_vdev_call(sc->conn_id, sc->fns.create, NULL);
	kos_flush(true);

	win->sc = sc;
	win->opaque_ptr = sc->last_ret.opaque_ptr;

	return win;
}

void win_destroy(win_t win) {
	win_softc_t const sc = win->sc;

	if (!sc->is_conn) {
		free(win);
		return;
	}

	kos_val_t const args[] = {
		{
			.opaque_ptr = win->opaque_ptr,
		},
	};

	kos_vdev_call(sc->conn_id, sc->fns.destroy, args);
	kos_flush(true);

	free(win);
}

void win_loop(win_t win) {
	win_softc_t const sc = win->sc;

	if (!sc->is_conn) {
		return;
	}

	kos_val_t const args[] = {
		{
			.opaque_ptr = win->opaque_ptr,
		},
	};

	kos_vdev_call(sc->conn_id, sc->fns.loop, args);
	kos_flush(true);
}
