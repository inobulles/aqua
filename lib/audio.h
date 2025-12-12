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
 * Get all streaming configs supported by VDEV.
 *
 * @param ctx The audio library component context.
 * @param config_count_ref Reference to config count.
 * @return A flat list of config structs. The caller is responsible for freeing this memory. Returns `NULL` on error.
 */
audio_config_t const* audio_get_configs(audio_ctx_t ctx, size_t* config_count_ref);
