// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2026 Aymeric Wibo

struct VertOut {
	@builtin(position) pos: vec4f,
	@location(0) uv: vec2f,
	@location(1) normal: vec3f,
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
	out.uv = uv;
	out.normal = normal;

	return out;
}

struct FragOut {
	@location(0) colour: vec4f,
};

struct FrostParams {
	// XXX pos and size are entirely unused in the upsampling direction.
	// They are just kept so we can have the same bind group layout as the downsampling shader.
	pos: vec2f,
	size: vec2f,
	res: vec2f,
	off: f32,
};

@group(0) @binding(1)
var t: texture_2d<f32>;
@group(0) @binding(2)
var s: sampler;
@group(0) @binding(3)
var frost_t: texture_2d<f32>;
@group(0) @binding(4)
var frost_s: sampler;
@group(0) @binding(5)
var<uniform> frost_params: FrostParams;

fn sample_frost(centre: vec2f, off: vec2f) -> vec3f {
	let colour = textureSample(frost_t, frost_s, centre + off);
	return colour.rgb * colour.rgb; // Gamma correction (approximating with ^2).
}

@fragment
fn frag_main(vert: VertOut) -> FragOut {
	// Here, we're actually doing the last stage of the dual Kawase blur algorithm.

	let inv_res = 1 / frost_params.res;
	let centre = vert.uv;
	let o = inv_res / 2 * frost_params.off;

	var frost_colour = vec3f(0.);

	frost_colour += sample_frost(centre, vec2f(-2 * o.x, 0.));
	frost_colour += sample_frost(centre, vec2f( 2 * o.x, 0.));
	frost_colour += sample_frost(centre, vec2f(0., -2 * o.y));
	frost_colour += sample_frost(centre, vec2f(0.,  2 * o.y));

	frost_colour += 2 * sample_frost(centre, vec2f( o.x,  o.y));
	frost_colour += 2 * sample_frost(centre, vec2f(-o.x,  o.y));
	frost_colour += 2 * sample_frost(centre, vec2f( o.x, -o.y));
	frost_colour += 2 * sample_frost(centre, vec2f(-o.x, -o.y));

	frost_colour = sqrt(frost_colour / 12);

	// Now, we just add our texture on top of that.

	let tex_colour = textureSample(t, s, vert.uv);

	var out: FragOut;
	out.colour = tex_colour + vec4f(frost_colour, 1.) * (1. - tex_colour.a);
	return out;
}
