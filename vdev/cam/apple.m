// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2024 Aymeric Wibo

#if defined(__APPLE__)
#if !defined(__OBJC__)
#error "This file must be compiled as Objective-C"
#endif

#import <AVFoundation/AVFoundation.h>
#include "cam.h"

void backend_probe(void) {
	AVCaptureDeviceDiscoverySession* const session = [AVCaptureDeviceDiscoverySession discoverySessionWithDeviceTypes:@[AVCaptureDeviceTypeBuiltInWideAngleCamera] mediaType:AVMediaTypeVideo position:AVCaptureDevicePositionUnspecified];

	for (AVCaptureDevice* dev in session.devices) {
		// TODO Should a lot of this be in the KOS? Like setting the kind, the host and VDEV ID, etc.

		kos_notif_t notif = {
			.kind = KOS_NOTIF_ATTACH,
			.attach = {
				.vdev = {
					.kind = KOS_VDEV_KIND_LOCAL,
					.spec = SPEC,
					.vers = VERS,
				},
			},
		};

		kos_vdev_descr_t* const vdev = &notif.attach.vdev;
		snprintf((char*) vdev->human, sizeof vdev->human - 1, "%s (%s)", [dev.localizedName UTF8String], [dev.uniqueID UTF8String]);

		strncpy((char*) vdev->vdriver_human, VDRIVER.human, sizeof vdev->vdriver_human - 1);

		VDRIVER.vdev_notif_cb(&notif, VDRIVER.vdev_notif_data);
	}
}
#endif
