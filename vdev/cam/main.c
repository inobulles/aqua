// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2024 Aymeric Wibo

#include "cam.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static void init(void) {
}

static void probe(void) {
	assert(VDRIVER.vdev_notif_cb != NULL);
	backend_probe();
}

/*
static void* conn(vid_t vdev_id) {
	assert(vdev_id < VDRIVER.vdev_id_lo || vdev_id > VDRIVER.vdev_id_hi); // The KOS should ensure the VDEV ID requested is within our VID slice before calling this.

	cam_t* const cam = malloc(sizeof *cam);
	assert(cam != NULL);

	return cam;
}
*/

vdriver_t VDRIVER = {
	.spec = SPEC,
	.human = "Default camera driver for FreeBSD, Linux, and macOS",
	.vers = VERS,

	.init = init,
	.probe = probe,
	// .conn = conn,
};
