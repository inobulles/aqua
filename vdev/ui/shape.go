// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package main

import (
	"fmt"
	"math"

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
			Usage:    wgpu.BufferUsageVertex,
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
			Usage:    wgpu.BufferUsageIndex,
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

func (m *Model) gen_pane(b *WgpuBackend, w, h, r float32) error {
	// This function has basically been transliterated from my Mist VR project (see src/win.c).

	const CORNER_RES = 16
	const TAU = 2 * math.Pi

	if CORNER_RES < 2 {
		panic("bad CORNER_RES")
	}

	r = min(r, w/2, h/2)

	// Vertex buffer: 4 for the centre quad, then $CORNER_RES vertices for each of the 4 corners.

	vert_count := 4 + 4*CORNER_RES
	verts := make([]Vertex, vert_count)

	if vert_count > (1 << 16) {
		panic("too many vertices for 16-bit indices")
	}

	// Index buffer: 2 tris for the centre quad, 2 tris for each of the 4 edges, and 1 tri for $CORNER_RES - 1 for each of the 4 corners.

	tri_count := 2 + 4*2 + 4*(CORNER_RES-1)
	tris := make([][3]uint16, tri_count)

	centre_w := w - 2*r
	centre_h := h - 2*r

	// Add the centre quad.
	// Its normals will be facing us.

	verts[0].X, verts[0].Y = centre_w/2, centre_h/2
	verts[1].X, verts[1].Y = -centre_w/2, centre_h/2
	verts[2].X, verts[2].Y = -centre_w/2, -centre_h/2
	verts[3].X, verts[3].Y = centre_w/2, -centre_h/2

	for i := range 4 {
		verts[i].NX, verts[i].NY, verts[i].NZ = 0, 1, 0
	}

	tris[0] = [3]uint16{0, 1, 2}
	tris[1] = [3]uint16{0, 2, 3}

	// Add corners to edge triangles already.
	// We'll add the ends of the triangle fans for the corners later.

	tris[2][0], tris[2][1], tris[3][0] = 0, 1, 1 // Top edge.
	tris[4][0], tris[4][1], tris[5][0] = 1, 2, 2 // Left edge.
	tris[6][0], tris[6][1], tris[7][0] = 2, 3, 2 // Bottom edge.
	tris[8][0], tris[8][1], tris[9][0] = 3, 0, 3 // Right edge.

	// Go through all angles and add vertex data for each vertex.

	vert_off := uint16(4)
	tri_off := 2 + 4*2 // Reserve space for edge triangles.

	for i := range CORNER_RES {
		theta := float64(i) / (CORNER_RES - 1) * TAU / 4
		nx, ny := float32(math.Cos(theta)), float32(math.Sin(theta))

		dist_x, dist_y := centre_w/2+r*nx, centre_h/2+r*ny

		// Top right.

		verts[vert_off].X, verts[vert_off].Y = dist_x, dist_y
		verts[vert_off].NX, verts[vert_off].NY, verts[vert_off].NZ = nx, -ny, 0

		if i != 0 { // Corner triangle fan.
			tris[tri_off] = [3]uint16{0, vert_off - 4, vert_off}
			tri_off++
		}

		if i == 0 { // Edge tris.
			tris[8][2], tris[9][1] = vert_off, vert_off // Right edge.
		}

		if i == CORNER_RES-1 { // Edge tris.
			tris[2][2], tris[3][1] = vert_off, vert_off // Top edge.
		}

		vert_off++

		// Top left.

		verts[vert_off].X, verts[vert_off].Y = -dist_x, dist_y
		verts[vert_off].NX, verts[vert_off].NY, verts[vert_off].NZ = -nx, -ny, 0

		if i != 0 { // Corner triangle fan.
			tris[tri_off] = [3]uint16{1, vert_off - 4, vert_off}
			tri_off++
		}

		if i == 0 { // Edge tris.
			tris[4][2], tris[5][1] = vert_off, vert_off // Left edge.
		}

		if i == CORNER_RES-1 { // Edge tris.
			tris[3][2] = vert_off // Top edge.
		}

		vert_off++

		// Bottom left.

		verts[vert_off].X, verts[vert_off].Y = -dist_x, -dist_y
		verts[vert_off].NX, verts[vert_off].NY, verts[vert_off].NZ = -nx, ny, 0

		if i != 0 { // Corner triangle fan.
			tris[tri_off] = [3]uint16{2, vert_off - 4, vert_off}
			tri_off++
		}

		if i == 0 { // Edge tris.
			tris[5][2] = vert_off // Left edge.
		}

		if i == CORNER_RES-1 { // Edge tris.
			tris[7][1] = vert_off // Bottom edge.
		}

		vert_off++

		// Bottom right.

		verts[vert_off].X, verts[vert_off].Y = dist_x, -dist_y
		verts[vert_off].NX, verts[vert_off].NY, verts[vert_off].NZ = nx, ny, 0

		if i != 0 { // Corner triangle fan.
			tris[tri_off] = [3]uint16{3, vert_off - 4, vert_off}
			tri_off++
		}

		if i == 0 { // Edge tris.
			tris[9][2] = vert_off // Right edge.
		}

		if i == CORNER_RES-1 { // Edge tris.
			tris[7][2], tris[6][2] = vert_off, vert_off // Bottom edge.
		}

		vert_off++
	}

	// Generate texture coordinates from vertex positions.

	for i := range vert_count {
		x := verts[i].X
		y := verts[i].Y

		/*
			verts[i].U = x/(w-r*2) + 0.5
			verts[i].V = 1 - (y/(h-r*2) + 0.5)
		*/

		verts[i].U = x/w + 0.5
		verts[i].V = 1 - (y/h + 0.5)
	}

	// Generate or update model buffers.

	return m.gen_model(b, verts, tris, fmt.Sprintf("%gx%g pane, %g radius", w, h, r))
}
