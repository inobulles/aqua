// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025-2026 Aymeric Wibo

struct VertOut {
	@builtin(position) pos: vec4f,
	@location(0) uv: vec2f,
};

@group(0) @binding(0)
var<uniform> mvp: mat4x4<f32>;

@vertex
fn vert_main(@builtin(vertex_index) index: u32) -> VertOut {
	let quad = array<vec2f, 6>(
		vec2f(-0.5, -0.5),
		vec2f( 0.5, -0.5),
		vec2f(-0.5,  0.5),
		vec2f(-0.5,  0.5),
		vec2f( 0.5, -0.5),
		vec2f( 0.5,  0.5),
	);

	let pos = quad[index];

	var out: VertOut;

	out.pos = mvp * vec4(pos, 0., 1.);
	out.uv = vec2f(pos.x + 0.5, 0.5 - pos.y);

	return out;
}

struct FragOut {
	@location(0) colour: vec4f,
};

@group(0) @binding(1)
var t: texture_2d<f32>;
@group(0) @binding(2)
var s: sampler;

@fragment
fn frag_main(vert: VertOut) -> FragOut {
	var out: FragOut;

	out.colour = textureSample(t, s, vert.uv);

	return out;
}

// For textures whose alpha channel is undefined, e.g. the padding in XRGB8888.

@fragment
fn frag_opaque_main(vert: VertOut) -> FragOut {
	var out: FragOut;

	out.colour = vec4(textureSample(t, s, vert.uv).rgb, 1.);

	return out;
}
