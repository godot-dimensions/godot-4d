shader_type spatial;
render_mode skip_vertex_transform, unshaded, cull_disabled;

// World space
// Not allowed to pass matrices through instance uniforms, so have to unpack into vectors.
instance uniform vec4 modelview_origin;
instance uniform vec4 modelview_basis_x;
instance uniform vec4 modelview_basis_y;
instance uniform vec4 modelview_basis_z;
instance uniform vec4 modelview_basis_w;

uniform vec4 w_negative_fade_rgbu;
uniform vec4 w_positive_fade_rgbu;
uniform float w_fade_distance;
uniform vec4 base_color_rgba;
uniform float base_color_u;
uniform float tetrachromacy_amount;

varying vec4 normal;
varying float ugaf;
varying float w_distance;

void vertex() {
	vec4 coords = vec4(VERTEX, CUSTOM0.x);
	normal = vec4(NORMAL, CUSTOM0.y);
	mat4 modelview_basis = mat4(modelview_basis_x, modelview_basis_y, modelview_basis_z, modelview_basis_w);

	coords = (modelview_basis * coords) + modelview_origin;
	normal = transpose(inverse(modelview_basis)) * normal;

	ugaf = CUSTOM1.x;

	VERTEX = coords.xyz;
	w_distance = coords.w;

	NORMAL = normal.xyz;
}

// A small 4x4 Bayer matrix can be hardcoded or passed as a texture
const float dither_matrix[16] = float[](
    0.5 /* get rid of the one stray pixel */, 8.0, 2.0, 10.0,
    12.0, 4.0, 14.0, 6.0,
    3.0, 11.0, 1.0, 9.0,
    15.0, 7.0, 13.0, 5.0
);

// Function to get the threshold value for the current fragment
float get_dither_threshold(vec2 screen_uv) {
    // Scale screen coordinates to matrix size (4x4)
    vec2 pos = mod(screen_uv, 4.0);
    int index = int(pos.y) * 4 + int(pos.x);
    // Normalize matrix value from 0-16 range to 0.0-1.0 range
    return dither_matrix[index] / 16.0;
}

vec3 combine_ugaf(vec4 fragcoord, vec4 rgbu) {
	vec2 fragcoord_shifted = vec2(fragcoord.x + TIME * 4.0, fragcoord.y + TIME * 5.0);
	return mod(fragcoord_shifted.x, 10) > 5. && mod(fragcoord_shifted.y, 10) > 5.
		? mix(rgbu.rgb, mix(vec3(0.,0.,0.), vec3(0.478, 0.373, 0.89), rgbu.w), tetrachromacy_amount)
		: rgbu.rgb;
}

float inverseLerp(float a, float b, float value) {
    return (value - a) / (b - a);
}

void fragment() {
	NORMAL = normalize(NORMAL);
	vec4 vertex_base_color = vec4(COLOR.rgb, ugaf) * vec4(base_color_rgba.rgb, base_color_u);
	vec4 color = mix(
		vertex_base_color, 
		w_distance >= 0.0 ? w_positive_fade_rgbu : w_negative_fade_rgbu,
		clamp(w_distance >= 0.0 ? w_distance : -w_distance, 0.0, 1.0));
	float z_depth_fade = 1.0 - clamp(VERTEX.z / 12.0, 0.0, 1.0);
	float w_dist_fade = 1.0 - clamp(abs(w_distance / w_fade_distance), 0.0, 1.0);
	float light = ((dot((vec4(NORMAL, 0.) * INV_VIEW_MATRIX).xyz, vec3(0., 1., 0.)) / 2.) + 0.5);
	ALBEDO = combine_ugaf(FRAGCOORD, color) * light;
	ALPHA = z_depth_fade * w_dist_fade * 0.25 * COLOR.a * base_color_rgba.a;

	/*
	float fade_denom = w_fade_distance + abs(VERTEX.z);
	float fade_factor = w_distance * (0.5 / fade_denom);

	NORMAL = normalize(NORMAL);
	vec4 vertex_base_color = vec4(COLOR.rgb, ugaf) * vec4(base_color_rgba.rgb, base_color_u);
	vec4 signed_fade = w_distance >= 0. ? w_positive_fade_rgbu : w_negative_fade_rgbu;
	vec4 faded = mix(
		vertex_base_color,
		vec4(vertex_base_color.rgb * signed_fade.rgb, signed_fade.w), // Make ugaf able to fade lighter
		clamp((w_distance >= 0. ? -fade_factor : fade_factor) * 1.5, 0., 1.)
	);
	vec4 normaled_rgbu = faded *
		((dot((vec4(NORMAL, 0.) * INV_VIEW_MATRIX).xyz, vec3(0., 1., 0.)) / 2.) + 0.5);
	ALBEDO = combine_ugaf(FRAGCOORD, normaled_rgbu);

	float alpha = clamp(COLOR.a * base_color_rgba.a * (1. - min(1.0, abs(fade_factor))), 0., 1.);
	ALPHA = alpha * 0.0625;
	*/
}