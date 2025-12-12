// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#include <aqua/audio.h>

#include <umber.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
	umber_class_t const* const cls = umber_class_new("demos.music-player", UMBER_LVL_VERBOSE, "Super simple music player demo");
	int rv = EXIT_FAILURE;

	aqua_ctx_t const ctx = aqua_init();

	if (ctx == NULL) {
		LOG_F(cls, "Failed to initialize AQUA library.");
		return EXIT_FAILURE;
	}

	// Get list of audio VDEVs.

	aqua_component_t const comp = audio_init(ctx);

	aqua_vdev_it_t it = aqua_vdev_it(comp);
	kos_vdev_descr_t* audio_vdev = NULL;

	for (; it.vdev != NULL; aqua_vdev_it_next(&it)) {
		kos_vdev_descr_t* const vdev = it.vdev;

		LOG_I(cls, "Found VDEV: %s (spec=%s, pref=%d).", vdev->human, vdev->spec, vdev->pref);

		if (audio_vdev == NULL || strncmp((char const*) vdev->human, "oss", 3)) {
			audio_vdev = vdev;
		}
	}

	if (audio_vdev == NULL) {
		LOG_F(cls, "No VDEV found.");
		goto err_no_vdev;
	}

	else {
		LOG_I(cls, "Best VDEV found: %s.", audio_vdev->human);
	}

	audio_ctx_t const audio_ctx = audio_conn(audio_vdev);

	if (audio_ctx == NULL) {
		LOG_F(cls, "Failed to connect to audio VDEV.");
		goto err_conn;
	}

	size_t config_count;
	audio_config_t const* const configs = audio_get_configs(audio_ctx, &config_count);

	(void) configs;
	LOG_I(cls, "Found %zu configs.", config_count);

	// TODO Generate sound over here so we can actually play it. Read from MP3 file immediately because cool.

	rv = EXIT_SUCCESS;
	audio_disconn(audio_ctx);

err_conn:
err_no_vdev:

	return rv;
}
