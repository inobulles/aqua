// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

struct VertOut {
	@builtin(position) pos: vec4f,
};

@group(0) @binding(0)
var<uniform> mvp: mat4x4<f32>;

@vertex
fn vert_main(
	@location(0) pos: vec2f,
	@location(1) uv: vec2f,
	@location(2) normal: vec3f,
) -> VertOut {
	var out: VertOut;

	out.pos = mvp * vec4(pos, 0., 1.);

	return out;
}

struct FragOut {
	@location(0) colour: vec4f,
};

@group(0) @binding(1)
var<uniform> colour: vec4f;

@fragment
fn frag_main(vert: VertOut) -> FragOut {
	var out: FragOut;

	out.colour = colour;

	return out;
}
