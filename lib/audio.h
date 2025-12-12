// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#pragma once

#include "root.h"

#include <aqua/kos.h>

/**
 * Audio library component context.
 */
typedef struct audio_ctx_t* audio_ctx_t;

/**
 * Audio stream object.
 */
typedef struct audio_stream_t* audio_stream_t;

/**
 * Sample formats.
 */
uint8_t AUDIO_SAMPLE_FORMAT_I8;
uint8_t AUDIO_SAMPLE_FORMAT_I16;
uint8_t AUDIO_SAMPLE_FORMAT_I24;
uint8_t AUDIO_SAMPLE_FORMAT_I32;
uint8_t AUDIO_SAMPLE_FORMAT_I64;
uint8_t AUDIO_SAMPLE_FORMAT_U8;
uint8_t AUDIO_SAMPLE_FORMAT_U16;
uint8_t AUDIO_SAMPLE_FORMAT_U32;
uint8_t AUDIO_SAMPLE_FORMAT_U64;
uint8_t AUDIO_SAMPLE_FORMAT_F32;
uint8_t AUDIO_SAMPLE_FORMAT_F64;

/**
 * An audio stream config range.
 *
 * TODO Document these members.
 */
typedef struct {
	uint8_t sample_format;
	uint32_t min_sample_rate;
	uint32_t max_sample_rate;
	uint32_t min_buf_size;
	uint32_t max_buf_size;
	uint16_t channels;
} audio_config_t;

/**
 * Initialize the audio library component.
 *
 * @param ctx The AQUA library context.
 * @return The audio library component handle.
 */
aqua_component_t audio_init(aqua_ctx_t ctx);

/**
 * Connect to an audio VDEV.
 *
 * {@link audio_disconn} must be called to disconnect from the VDEV and to free the context.
 *
 * @param vdev The descriptor of the audio VDEV to connect to.
 * @return The audio library component context or `NULL` if allocation failed.
 */
audio_ctx_t audio_conn(kos_vdev_descr_t const* vdev);

/**
 * Disconnect from an audio VDEV.
 *
 * This function disconnects from the audio VDEV and frees the context.
 *
 * @param ctx The audio library component context.
 */
void audio_disconn(audio_ctx_t ctx);

/**
 * Get all audio streaming configs supported by an audio VDEV.
 *
 * @param ctx The audio library component context.
 * @param config_count_ref Reference to config count.
 * @return A flat list of config structs. The caller is responsible for freeing this memory. Returns `NULL` on error.
 */
audio_config_t const* audio_get_configs(audio_ctx_t ctx, size_t* config_count_ref);

/**
 * Open a new audio stream.
 *
 * The ringbuffer size dictates how much data we can push in advance to the VDEV.
 * This is especially useful in high latency/jittery scenarios, such as when streaming audio over the network, where you'd want to set this higher.
 *
 * @param ctx The audio library component context.
 * @param config_sample_format Sample format of the desired config.
 * @param config_channels Number of channels of the desired config.
 * @param config_sample_rate Sample rate of the desired config.
 * @param config_buf_size Buffer size of the desired config.
 * @param ringbuf_size Size of the ringbuffer, in units of the sample format (i.e. not bytes!).
 * @returns A stream handle.
 */
audio_stream_t audio_open_stream(
	audio_ctx_t ctx,
	uint8_t config_sample_format,
	uint16_t config_channels,
	uint32_t config_sample_rate,
	uint32_t config_buf_size,
	uint32_t ringbuf_size
);

/**
 * Write samples to an audio stream.
 *
 * @param stream The audio stream to write samples to.
 * @param buf The buffer of samples to write.
 * @param len The length of the sample buffer. This should be a multiple of the size of the sampling format selected when opening the stream.
 */
void audio_stream_write(audio_stream_t stream, void* buf, size_t len);
