// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2026 Aymeric Wibo

struct VertOut {
	@builtin(position) pos: vec4f,
};

struct FragOut {
	@location(0) colour: vec4f,
};

@vertex
fn vert_main(@builtin(vertex_index) index: u32) -> VertOut {
	let quad = array<vec2f, 6>(
		vec2f(-1., -1.),
		vec2f( 1., -1.),
		vec2f(-1.,  1.),
		vec2f(-1.,  1.),
		vec2f( 1., -1.),
		vec2f( 1.,  1.),
	);

	var out: VertOut;
	out.pos = vec4(quad[index], 0., 1.);
	return out;
}

@group(0) @binding(0)
var t: texture_2d<f32>;
@group(0) @binding(1)
var s: sampler;

@fragment
fn frag_main(vert: VertOut) -> FragOut {
	let dims = vec2f(textureDimensions(t));
	var out: FragOut;
	out.colour = textureSample(t, s, vert.pos.xy / dims);
	return out;
}
