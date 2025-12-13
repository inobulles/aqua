// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

struct VertOut {
	@builtin(position) pos: vec4f,
	@location(0) uv: vec2f,
};

@vertex
fn vert_main(
	@location(0) pos: vec2f,
	@location(1) uv: vec2f,
) -> VertOut {
	var out: VertOut;

	out.pos = vec4(pos, 1., 1.);
	out.uv = uv;

	return out;
}

struct FragOut {
	@location(0) colour: vec4f,
};

@group(0) @binding(0)
var t: texture_2d<f32>;
@group(0) @binding(1)
var s: sampler;

@fragment
fn frag_main(vert: VertOut) -> FragOut {
	var out: FragOut;

	out.colour = textureSample(t, s, vert.uv.yx);

	return out;
}
