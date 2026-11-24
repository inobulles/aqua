// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025-2026 Aymeric Wibo

// Mega shader with multiple fragment shader entry points.
// Pipelines choose their entry point and only bind what that entry point uses.
// All entry points must output premultiplied alpha.

struct VertOut {
	@builtin(position) pos: vec4f,
	@location(0) uv: vec2f,
};

@group(0) @binding(0)
var<uniform> mvp: mat4x4<f32>;

@vertex
fn vert_main(@builtin(vertex_index) index: u32) -> VertOut {
	// Unit quad centred on the origin, which the MVP matrix scales to the element's size.

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

// Colour (not premultiplied).
// This is drawn over everything else, so (0, 0, 0, 0) means no overlay.

@group(0) @binding(1)
var<uniform> colour: vec4f;

struct Rounding {
	size: vec2f,
	radius: f32,
};

@group(0) @binding(2)
var<uniform> rounding: Rounding;

@group(0) @binding(3)
var t: texture_2d<f32>;
@group(0) @binding(4)
var s: sampler;

struct FrostParams {
	// XXX pos and size are entirely unused in the upsampling direction.
	// They are just kept so we can have the same bind group layout as the downsampling shader.
	pos: vec2f,
	size: vec2f,
	res: vec2f,
	off: f32,
};

@group(0) @binding(5)
var frost_t: texture_2d<f32>;
@group(0) @binding(6)
var frost_s: sampler;
@group(0) @binding(7)
var<uniform> frost_params: FrostParams;

// Rounded rectangle SDF.

fn rounded_rect_sdf(p: vec2f, half_size: vec2f, r: f32) -> f32 {
	let q = abs(p) - half_size + r;
	return length(max(q, vec2f(0.))) + min(max(q.x, q.y), 0.) - r;
}

// How much of the current pixel is covered by the rounded rectangle, for anti-aliasing.

fn rounding_coverage(uv: vec2f) -> f32 {
	let half_size = rounding.size / 2;
	let r = min(rounding.radius, min(half_size.x, half_size.y));

	let d = rounded_rect_sdf((uv - 0.5) * rounding.size, half_size, r);
	let aa = fwidth(d);

	return 1. - smoothstep(-aa / 2, aa / 2, d);
}

// Draw colour over what's below it (premultiplied).

fn over_colour(below: vec4f) -> vec4f {
	let premul = vec4f(colour.rgb * colour.a, colour.a);
	return premul + below * (1. - colour.a);
}

@fragment
fn frag_solid_main(vert: VertOut) -> FragOut {
	let coverage = rounding_coverage(vert.uv);

	var out: FragOut;
	out.colour = over_colour(vec4f(0.)) * coverage;

	return out;
}

@fragment
fn frag_texture_main(vert: VertOut) -> FragOut {
	let coverage = rounding_coverage(vert.uv);

	var out: FragOut;
	out.colour = over_colour(textureSample(t, s, vert.uv)) * coverage;

	return out;
}

// For textures whose alpha channel is undefined, e.g. the padding in XRGB8888.

@fragment
fn frag_texture_opaque_main(vert: VertOut) -> FragOut {
	let coverage = rounding_coverage(vert.uv);

	var out: FragOut;
	out.colour = over_colour(vec4(textureSample(t, s, vert.uv).rgb, 1.)) * coverage;

	return out;
}

@fragment
fn frag_text_main(vert: VertOut) -> FragOut {
	var out: FragOut;
	out.colour = textureSample(t, s, vert.uv);

	return out;
}

fn sample_frost(centre: vec2f, off: vec2f) -> vec3f {
	let colour = textureSample(frost_t, frost_s, centre + off);
	return colour.rgb * colour.rgb; // Gamma correction (approximating with ^2).
}

@fragment
fn frag_frost_main(vert: VertOut) -> FragOut {
	let coverage = rounding_coverage(vert.uv);

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

	// Now, we just add our texture and colour on top of that.

	let tex_colour = textureSample(t, s, vert.uv);
	let below = tex_colour + vec4f(frost_colour, 1.) * (1. - tex_colour.a);

	var out: FragOut;
	out.colour = over_colour(below) * coverage;

	return out;
}
