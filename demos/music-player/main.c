// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#include <aqua/audio.h>

#include <umber.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <mpg123.h>

#define SAMPLE_RATE 48000
#define MP3_PATH "misery-business.mp3"

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

	LOG_I(cls, "Found %zu configs. Looking for F32 (%d).", config_count, AUDIO_SAMPLE_FORMAT_F32);

	audio_config_t const* config = NULL;

	for (size_t i = 0; i < config_count; i++) {
		if (
			configs[i].sample_format == AUDIO_SAMPLE_FORMAT_U32 &&
			SAMPLE_RATE >= configs[i].min_sample_rate &&
			SAMPLE_RATE <= configs[i].max_sample_rate
		) {
			config = &configs[i];
		}
	}

	if (config == NULL) {
		LOG_F(cls, "Couldn't find satisfactory config.");
		goto err_conn;
	}

	LOG_I(cls, "Config sample format: %d", config->sample_format);
	LOG_I(cls, "Config channels: %d", config->channels);
	LOG_I(cls, "Config sample rate range: [%u; %u]", config->min_sample_rate, config->max_sample_rate);
	LOG_I(cls, "Config buffer size range: [%u; %u]", config->min_buf_size, config->max_buf_size);

	size_t const RINGBUF_SEC = 20;
	audio_stream_t const stream = audio_open_stream(audio_ctx, config->sample_format, 1, SAMPLE_RATE, 1000, SAMPLE_RATE * RINGBUF_SEC);

	// MP3 decoding.

	mpg123_handle* mh = NULL;
	int err = MPG123_OK;

	if (mpg123_init() != MPG123_OK) {
		LOG_F(cls, "mpg123_init() failed.");
		return EXIT_FAILURE;
	}

	mh = mpg123_new(NULL, &err);
	if (!mh) {
		LOG_F(cls, "mpg123_new failed: %s", mpg123_plain_strerror(err));
		return EXIT_FAILURE;
	}

	mpg123_param(mh, MPG123_VERBOSE, 2, 0.0);

	mpg123_format_none(mh);
	mpg123_format(mh, SAMPLE_RATE, 1, MPG123_ENC_FLOAT_32);

	if (mpg123_open(mh, MP3_PATH) != MPG123_OK) {
		LOG_F(cls, "Failed to open MP3: %s", MP3_PATH);
		return EXIT_FAILURE;
	}

	size_t temp_buf_bytes = 4096 * config->channels * sizeof(float);
	float* decode_buf = malloc(temp_buf_bytes);

	while (1) {
		size_t done = 0;
		int ret = mpg123_read(mh, (unsigned char*) decode_buf, temp_buf_bytes, &done);

		if (ret == MPG123_DONE) {
			LOG_I(cls, "Finished decoding.");
			break;
		}
		if (ret != MPG123_OK && ret != MPG123_NEW_FORMAT) {
			LOG_F(cls, "mpg123_read error: %s", mpg123_strerror(mh));
			break;
		}

		if (done == 0) {
			continue;
		}

		// Push decoded PCM to audio stream.

		audio_stream_write(stream, decode_buf, done);
	}

	free(decode_buf);
	mpg123_close(mh);
	mpg123_delete(mh);
	mpg123_exit();

	LOG_I(cls, "Waiting for playback...");
	sleep(20);

	rv = EXIT_SUCCESS;
	audio_disconn(audio_ctx);

err_conn:
err_no_vdev:

	return rv;
}
