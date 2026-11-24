// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025-2026 Aymeric Wibo

package main

import (
	"fmt"

	"obiw.ac/aqua/wgpu"
)

type Vertex struct {
	X, Y       float32
	U, V       float32
	NX, NY, NZ float32
}

type Model struct {
	vbo         *wgpu.Buffer
	ibo         *wgpu.Buffer
	index_count uint32
}

func (m *Model) release() {
	if m.vbo != nil {
		m.vbo.Release()
	}
	if m.ibo != nil {
		m.ibo.Release()
	}
}

func (m *Model) gen_model(b *WgpuBackend, verts []Vertex, tris [][3]uint16, label string) error {
	var err error

	vert_bytes := wgpu.ToBytes(verts)
	index_bytes := wgpu.ToBytes(tris)

	if m.vbo == nil {
		if m.vbo, err = b.dev.CreateBufferInit(&wgpu.BufferInitDescriptor{
			Label:    fmt.Sprintf("VBO (%s)", label),
			Contents: vert_bytes,
			Usage:    wgpu.BufferUsageVertex | wgpu.BufferUsageCopyDst,
		}); err != nil {
			m.release()
			return err
		}
	} else {
		b.queue.WriteBuffer(m.vbo, 0, vert_bytes)
	}

	if m.ibo == nil {
		if m.ibo, err = b.dev.CreateBufferInit(&wgpu.BufferInitDescriptor{
			Label:    fmt.Sprintf("IBO (%s)", label),
			Contents: index_bytes,
			Usage:    wgpu.BufferUsageIndex | wgpu.BufferUsageCopyDst,
		}); err != nil {
			m.release()
			return err
		}
	} else {
		b.queue.WriteBuffer(m.ibo, 0, index_bytes)
	}

	m.index_count = uint32(len(tris) * 3)
	return nil
}

func (m *Model) draw(r *wgpu.RenderPassEncoder) {
	r.SetVertexBuffer(0, m.vbo, 0, wgpu.WholeSize)
	r.SetIndexBuffer(m.ibo, wgpu.IndexFormatUint16, 0, wgpu.WholeSize)
	r.DrawIndexed(m.index_count, 1, 0, 0, 0)
}

func (m *Model) gen_quad(b *WgpuBackend, w, h float32) error {
	verts := []Vertex{
		{-w / 2, -h / 2, 0, 1, 0, 1, 0},
		{w / 2, -h / 2, 1, 1, 0, 1, 0},
		{w / 2, h / 2, 1, 0, 0, 1, 0},
		{-w / 2, h / 2, 0, 0, 0, 1, 0},
	}

	indices := [][3]uint16{
		{0, 1, 2},
		{2, 3, 0},
	}

	return m.gen_model(b, verts, indices, fmt.Sprintf("%gx%g quad", w, h))
}
