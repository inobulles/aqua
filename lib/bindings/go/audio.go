// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package aqua

/*
#cgo LDFLAGS: -laqua_audio

#include <aqua/audio.h>
*/
import "C"
import (
	"errors"
	"unsafe"
)

type AudioComponent struct {
	Component
}

type AudioCtx struct {
	ctx C.audio_ctx_t
}

type AudioConfig struct {
	SampleFormat  uint8
	MinSampleRate uint32
	MaxSampleRate uint32
	MinBufSize    uint32
	MaxBufSize    uint32
	Channels      uint16
}

type AudioStream struct {
	stream C.audio_stream_t
}

type AudioSample interface {
	~int8 | ~uint8 |
		~int16 | ~uint16 |
		~int32 | ~uint32 |
		~int64 | ~uint64 |
		~float32 | ~float64
}

type AudioBuffer struct {
	data unsafe.Pointer
	size C.size_t
}

func (c *Context) AudioInit() *AudioComponent {
	comp := C.audio_init(c.internal)

	if comp == nil {
		return nil
	}

	return &AudioComponent{
		Component{internal: comp},
	}
}

func (c *AudioComponent) Conn(vdev *VdevDescr) *AudioCtx {
	if vdev == nil {
		return nil
	}

	ctx := C.audio_conn(&vdev.internal)

	if ctx == nil {
		return nil
	}

	return &AudioCtx{ctx}
}

func (c *AudioCtx) Disconn() {
	C.audio_disconn(c.ctx)
}

func (c *AudioCtx) GetConfigs() []AudioConfig {
	var count C.size_t
	raw_configs := C.audio_get_configs(c.ctx, &count)
	defer C.free(unsafe.Pointer(raw_configs))

	configs := make([]AudioConfig, count)

	for i := range count {
		raw_config := (*C.audio_config_t)(unsafe.Pointer(uintptr(unsafe.Pointer(raw_configs)) + uintptr(i)*unsafe.Sizeof(*raw_configs)))

		configs[i] = AudioConfig{
			SampleFormat:  uint8(raw_config.sample_format),
			MinSampleRate: uint32(raw_config.min_sample_rate),
			MaxSampleRate: uint32(raw_config.max_sample_rate),
			MinBufSize:    uint32(raw_config.min_buf_size),
			MaxBufSize:    uint32(raw_config.max_buf_size),
			Channels:      uint16(raw_config.channels),
		}
	}

	return configs
}

func (c *AudioCtx) OpenStream(
	config_sample_format uint8,
	config_channels uint16,
	config_sample_rate uint32,
	config_buf_size uint32,
	ringbuf_size uint32,
) (*AudioStream, error) {
	stream := C.audio_open_stream(
		c.ctx,
		C.uint8_t(config_sample_format),
		C.uint16_t(config_channels),
		C.uint32_t(config_sample_rate),
		C.uint32_t(config_buf_size),
		C.uint32_t(ringbuf_size),
	)

	if stream == nil {
		return nil, errors.New("failed to create stream")
	}

	return &AudioStream{stream}, nil
}

func AudioBufferNew[T AudioSample](s []T) AudioBuffer {
	if len(s) == 0 {
		return AudioBuffer{}
	}

	return AudioBuffer{
		data: unsafe.Pointer(&s[0]),
		size: C.size_t(unsafe.Sizeof(s[0]) * uintptr(len(s))),
	}
}

func (s *AudioStream) Write(buf *AudioBuffer) {
	C.audio_stream_write(s.stream, buf.data, buf.size)
}
