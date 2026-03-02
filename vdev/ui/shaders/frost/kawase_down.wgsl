// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2026 Aymeric Wibo

struct VertOut {
	@builtin(position) pos: vec4f,
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

struct FragOut {
	@location(0) colour: vec4f,
};

struct FrostParams {
	pos: vec2f,
	size: vec2f,
	res: vec2f,
	off: f32,
};

@group(0) @binding(0)
var t: texture_2d<f32>;
@group(0) @binding(1)
var s: sampler;
@group(0) @binding(2)
var<uniform> frost_params: FrostParams;

fn sample(centre: vec2f, off: vec2f) -> vec3f {
	let colour = textureSample(t, s, centre + off);
	return colour.rgb * colour.rgb; // Gamma correction (approximating with ^2).
}

@fragment
fn frag_main(vert: VertOut) -> FragOut {
	let centre = vert.pos.xy * frost_params.size + frost_params.pos;
	let inv_res = 1 / frost_params.res;
	let o = inv_res / 2 * frost_params.off;

	var colour = 4 * sample(centre, vec2f(0., 0.));

	colour += sample(centre, vec2f( o.x,  o.y));
	colour += sample(centre, vec2f(-o.x,  o.y));
	colour += sample(centre, vec2f( o.x, -o.y));
	colour += sample(centre, vec2f(-o.x, -o.y));

	colour = sqrt(colour / 8);

	var out: FragOut;
	out.colour = vec4f(colour, 1.);
	return out;
}
