// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package main

/*
#cgo LDFLAGS: -laqua

#include "../../kos/vdev.h"

extern vdriver_t VDRIVER;

static inline void call_notif_cb(kos_notif_t* notif) {
	VDRIVER.notif_cb(notif, VDRIVER.notif_data);
}

extern void GoProbe(void);
*/
import "C"
import "unsafe"

const SPEC = "aquabsd.black.ui"
const VERS = 0
const HUMAN = "UI driver"

var only_vid C.vid_t

//export GoProbe
func GoProbe() {
	if C.VDRIVER.notif_cb == nil {
		panic("VDRIVER.notif_cb is nil")
	}

	// It doesn't really make sense to have more than one UI device.

	only_vid = C.VDRIVER.vdev_id_lo

	// Send attach notification.

	notif := C.kos_notif_t{
		kind: C.KOS_NOTIF_ATTACH,
	}

	// XXX Go doesn't support unions, and vdev happens to be the first field in .attach, so this happens to be fine.
	vdev := (*C.kos_vdev_descr_t) (unsafe.Pointer(&notif.anon0[0]))

	vdev.kind = C.KOS_VDEV_KIND_LOCAL
	vdev.vers = VERS
	// TODO For some reason, cgo has trouble translating this field.
	// vdev.pref = 0
	vdev.host_id = 0
	vdev.vdev_id = only_vid

	copy((*[64]byte)(unsafe.Pointer(&vdev.spec[0]))[:], SPEC)
	copy((*[256]byte)(unsafe.Pointer(&vdev.human[0]))[:], "UI device")
	copy((*[256]byte)(unsafe.Pointer(&vdev.vdriver_human[0]))[:], HUMAN)

	C.call_notif_cb(&notif)
}

//export GoInit
func GoInit() {
	C.VDRIVER.vers = VERS
	C.VDRIVER.probe = (*[0]byte)(C.GoProbe)

	copy((*[64]byte)(unsafe.Pointer(&C.VDRIVER.spec[0]))[:], SPEC)
	copy((*[256]byte)(unsafe.Pointer(&C.VDRIVER.human[0]))[:], HUMAN)
}

func main() {}
