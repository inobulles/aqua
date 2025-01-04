// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2024 Aymeric Wibo

#include <aqua/kos.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
} state_t;

void vdev_notif_cb(kos_notif_t* notif, void* data) {
	state_t* const state = data;
	(void) state;

	switch (notif->kind) {
	case KOS_NOTIF_ATTACH:;
		kos_vdev_descr_t* const vdev = &notif->attach.vdev;

		if (strcmp((char const*) vdev->spec, "aquabsd.black.cam") != 0) {
			break;
		}

		printf("Found camera VDEV: %s (from vdriver \"%s\")\n", vdev->human, vdev->vdriver_human);

		break;
	case KOS_NOTIF_DETACH:
		break;
	}
}

int main(void) {
	// Initiate communication with the KOS.

	kos_descr_v4_t descr;

	if (kos_hello(KOS_API_V4, KOS_API_V4, &descr) != KOS_API_V4) {
		return EXIT_FAILURE;
	}

	printf("Loaded KOS v4: %s\n", descr.name);

	// Get the camera VDEV.

	state_t state;
	kos_sub_to_vdev_notif(vdev_notif_cb, &state);

	kos_req_vdev("aquabsd.black.cam");

	return EXIT_SUCCESS;
}
