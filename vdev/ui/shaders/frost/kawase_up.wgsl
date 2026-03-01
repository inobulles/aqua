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

struct BgCrop {
	// XXX pos and size are entirely unused in the upsampling direction.
	// They are just kept so we can have the same bind group layout as the downsampling shader.
	pos: vec2f,
	size: vec2f,
	res: vec2f,
};

@group(0) @binding(0)
var t: texture_2d<f32>;
@group(0) @binding(1)
var s: sampler;
@group(0) @binding(2)
var<uniform> bg_crop: BgCrop; 

@fragment
fn frag_main(vert: VertOut) -> FragOut {
	var centre: vec2f = vert.pos.xy * bg_crop.size + bg_crop.pos;
	var inv_res: vec2f = vec2f(1. / bg_crop.res.x, 1. / bg_crop.res.y);
	var o: vec2f = inv_res / 2 * 10; // 10 is the strength (sample offset).

	var out: FragOut;
	out.colour = vec4f(0.);

	out.colour += textureSample(t, s, centre + vec2f(-2 * o.x, 0.));
	out.colour += textureSample(t, s, centre + vec2f( 2 * o.x, 0.));
	out.colour += textureSample(t, s, centre + vec2f(0., -2 * o.y));
	out.colour += textureSample(t, s, centre + vec2f(0.,  2 * o.y));

	out.colour += 2 * textureSample(t, s, centre + vec2f( o.x,  o.y));
	out.colour += 2 * textureSample(t, s, centre + vec2f(-o.x,  o.y));
	out.colour += 2 * textureSample(t, s, centre + vec2f( o.x, -o.y));
	out.colour += 2 * textureSample(t, s, centre + vec2f(-o.x, -o.y));

	out.colour /= 12;
	return out;
}
