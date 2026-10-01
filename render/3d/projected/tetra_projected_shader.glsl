shader_type spatial;
render_mode skip_vertex_transform, cull_disabled, depth_test_disabled, blend_add;

#include "../../shaders/perpendicular_4d.glsl"

// World space.
// Not allowed to pass matrices through instance uniforms, so have to unpack into vectors.
instance uniform vec4 modelview_origin;
instance uniform vec4 modelview_basis_x;
instance uniform vec4 modelview_basis_y;
instance uniform vec4 modelview_basis_z;
instance uniform vec4 modelview_basis_w;

instance uniform float camera_slope = 1.0; // The tan of the angle of the view frustum in the W direction.
instance uniform float camera_fade = 0.0; // The orthographic-style width of the view frustum in W.
instance uniform float edge_falloff = 2.0; // How quickly the opacity fades at the frustum's W edges (1: not at all, 2: linear, up to infinity).
instance uniform float plane_softness = 0.7; // Emphasis on the region around the slice plane: 1 is none, approaching 0 approaches a cross-section view (0 itself is invalid).
instance uniform float skewness = 0.0; // -1 to 1. Offsets the perspective projection's forward direction in W.

uniform vec4 albedo : source_color;
uniform sampler3D albedo_texture : hint_default_white, source_color;
uniform vec3 albedo_texture_map_offset;
uniform vec3 albedo_texture_map_scale;
// Clip-space depth from the cross-section pass.
// Defaults to 0 (the far plane, under Forward+'s reverse-Z convention) when there's no
// cross-section pass to read from.
uniform sampler2D cross_section_depth_texture : hint_default_black, filter_nearest;
const float DEPTH_BIAS_CLIPSPACE = 1e-6; // To prevent Z-fighting.
const float DEPTH_BIAS_VIEWSPACE = 1e-5;

// The built-in perspective correction can't handle a fragment that corresponds to a whole line of the tet with varying Z, so part of the
// interpolation is redone manually between the center vertex, the other_center vertex (the other end of that line), and a point on the
// triangle's edge opposite the center vertex, which the built-in interpolation between the two non-center vertices gets right.
varying vec3 uvw;
varying flat vec3 center_uvw;
varying flat vec3 other_center_uvw;
varying flat vec4 center_position_4d;
varying flat vec4 other_center_position_4d;
varying float centerness;
varying float position_w; // The other components are stored in VERTEX, but that's a vec3.
// Read by the shared light() function appended to this shader, which normalizes it itself.
varying flat vec4 normal_4d;

// Looks up, from a bit mask of which faces of the tet face +W or -W, the order of the vertices in the projection.
// First number: 0 = 3 triangles (vertex-on), 1 = 4 triangles (edge-on), 2 = 3 triangles (face-on), -1 = impossible.
//   3-triangle case: triangles 123, 134, 142.
//   4-triangle case: edges 13 and 24 cross at extra vertex 5, triangles 512, 523, 534, 541.
const int PROJECTION_LOOKUP[] = {
	-1, 0, 0, 0, 0, // 0000
	0, 3, 0, 2, 1, // 0001
	0, 2, 0, 1, 3, // 0010
	1, 2, 1, 3, 0, // 0011
	0, 1, 0, 3, 2, // 0100
	1, 3, 2, 1, 0, // 0101
	1, 1, 3, 2, 0, // 0110
	2, 0, 3, 2, 1, // 0111
	0, 0, 1, 2, 3, // 1000
	1, 0, 2, 3, 1, // 1001
	1, 2, 3, 0, 1, // 1010
	2, 1, 2, 3, 0, // 1011
	1, 0, 3, 1, 2, // 1100
	2, 2, 3, 1, 0, // 1101
	2, 3, 1, 2, 0, // 1110
	-1, 0, 0, 0, 0 // 1111
};

// Indices to expand the above into a triangle fan.
const int TRIANGLE_FAN_LOOKUP[] = {
	-1, 1, 2, -1, 2, 3, -1, 3, 1, -1, -1, -1,
	-1, 0, 1, -1, 1, 2, -1, 2, 3, -1, 3, 0
};

bool get_projection_case_one(vec4 vert0_4d, vec4 vert1_4d, vec4 vert2_4d) {
	// It is not sufficient to use vert.xy/vert.z. The sign of Z must also be taken into account.
	return determinant(mat3(vert0_4d.xyz, vert1_4d.xyz, vert2_4d.xyz)) > 0.0;
}

// Generate a big-endian 4-bit mask of, for each face of the perspective-projected tetrahedron, whether its normal is in the +W or -W direction.
// This number determines which triangles are needed to send to the frag shader.
int get_projection_case(vec4[4] verts_4d) {
	// Bitwise ops limit compatibility so summing instead.
	int negative1 = get_projection_case_one(verts_4d[1], verts_4d[2], verts_4d[3]) ? 8 : 0;
	int negative2 = get_projection_case_one(verts_4d[0], verts_4d[3], verts_4d[2]) ? 4 : 0;
	int negative3 = get_projection_case_one(verts_4d[0], verts_4d[1], verts_4d[3]) ? 2 : 0;
	int negative4 = get_projection_case_one(verts_4d[0], verts_4d[2], verts_4d[1]) ? 1 : 0;
	return negative1 + negative2 + negative3 + negative4;
}

void vertex() {
	mat4 modelview_basis_4d = mat4(modelview_basis_x, modelview_basis_y, modelview_basis_z, modelview_basis_w);

	vec4 verts_4d[] = { CUSTOM0, CUSTOM1, CUSTOM2, CUSTOM3 };
	for (int i = 0; i < 4; i++) {
		verts_4d[i] = (modelview_basis_4d * verts_4d[i]) + modelview_origin;
		verts_4d[i].z += verts_4d[i].w * skewness;
	}

	// Vertex 2's UVW is exact; vertex 1's W and all of vertex 4 are 16-bit offsets from it in the bone weights, scaled by 2^(bone index 0 - 64).
	vec3 uvw2 = vec3(UV2, VERTEX.y);
	vec4 uvw_offsets = (BONE_WEIGHTS * 2.0 - 1.0) * exp2(float(BONE_INDICES.x) - 64.0);
	vec3 uvws[] = { vec3(UV, uvw2.z + uvw_offsets.x), uvw2, vec3(NORMAL.xy / NORMAL.z, VERTEX.z), uvw2 + uvw_offsets.yzw };

	vec3[] verts_proj = {
		verts_4d[0].xyw / verts_4d[0].z,
		verts_4d[1].xyw / verts_4d[1].z,
		verts_4d[2].xyw / verts_4d[2].z,
		verts_4d[3].xyw / verts_4d[3].z
	};
	// Compute flat normals.
	normal_4d = perpendicular_4d(verts_4d[1] - verts_4d[0], verts_4d[2] - verts_4d[0], verts_4d[3] - verts_4d[0]);
	// Not from verts_proj alone, which can be wrong when a vertex is behind the camera.
	bool back_face = dot(verts_4d[0], normal_4d) >= 0.0;
	// The skewed normal is needed for the backface calculation because verts_4d[0] is also skewed,
	// but lighting needs the original normal. Undo z' = z + skewness * w using the transpose.
	normal_4d.w += normal_4d.z * skewness;

	int vertex_id = int(VERTEX.x);
	int projection_case = get_projection_case(verts_4d);
	int[] vert_indices = {
		PROJECTION_LOOKUP[projection_case * 5 + 1],
		PROJECTION_LOOKUP[projection_case * 5 + 2],
		PROJECTION_LOOKUP[projection_case * 5 + 3],
		PROJECTION_LOOKUP[projection_case * 5 + 4]
	};
	if (
			vertex_id % 12 >= 9 && PROJECTION_LOOKUP[projection_case * 5] != 1 || // The fourth triangle of the 3-triangle case.
			PROJECTION_LOOKUP[projection_case * 5] < 0 || // Impossible case (i.e. degenerate tet or floating point error).
			back_face) {
		// This vertex is unused, cull it.
		POSITION = vec4(0.0, 0.0, CLIP_SPACE_FAR, 1.0);
	} else {
		vec4 position_4d;
		if (vertex_id % 3 == 0) { // Center vertex.
			centerness = 1.0;
			if (PROJECTION_LOOKUP[projection_case * 5] == 1) { // 4-triangle case, this is the extra center vertex.
				// Need to find the intersection of the two edges in 2D space (basically screen-space).
				vec3 base1 = verts_proj[vert_indices[0]];
				vec3 point1 = verts_proj[vert_indices[2]] - base1;
				vec3 base2 = verts_proj[vert_indices[1]];
				vec3 point2 = verts_proj[vert_indices[3]] - base2;

				// Solve: (base1.xy + t1 * point1.xy) = (base2.xy + t2 * point2.xy).
				// (base1 - base2).xy = [-point1.xy, point2.xy] [t1, t2]
				vec2 solution = inverse(mat2(-point1.xy, point2.xy)) * (base1 - base2).xy;
				float t1 = solution.x;
				t1 = t1 / verts_4d[vert_indices[2]].z / (t1 / verts_4d[vert_indices[2]].z + (1.0 - t1) / verts_4d[vert_indices[0]].z); // Perspective correction.
				t1 = clamp(t1, 0.0, 1.0); // Not ideal: this algorithm isn't numerically stable, so clamping limits the worst errors.
				float t2 = solution.y;
				t2 = t2 / verts_4d[vert_indices[3]].z / (t2 / verts_4d[vert_indices[3]].z + (1.0 - t2) / verts_4d[vert_indices[1]].z);
				t2 = clamp(t2, 0.0, 1.0);
				center_position_4d = mix(verts_4d[vert_indices[0]], verts_4d[vert_indices[2]], t1);
				other_center_position_4d = mix(verts_4d[vert_indices[1]], verts_4d[vert_indices[3]], t2);
				center_uvw = mix(uvws[vert_indices[0]], uvws[vert_indices[2]], t1);
				other_center_uvw = mix(uvws[vert_indices[1]], uvws[vert_indices[3]], t2);
			} else { // 3-triangle case, this is the center vertex.
				vec3 target = verts_proj[vert_indices[0]];
				vec3 base = verts_proj[vert_indices[1]];
				vec3 point1 = verts_proj[vert_indices[2]] - base;
				vec3 point2 = verts_proj[vert_indices[3]] - base;

				// Solve: (base + t1 * point1 + t2 * point2).xy = target.xy
				vec2 solution = inverse(mat2(point1.xy, point2.xy)) * (target - base).xy;
				float t1 = solution.x; // Clamping to a triangle rather than a square isn't worth the complexity; it should usually do nothing anyway.
				float t2 = solution.y;
				float normalization = t1 / verts_4d[vert_indices[2]].z + t2 / verts_4d[vert_indices[3]].z + (1.0 - t1 - t2) / verts_4d[vert_indices[1]].z;
				t1 = clamp(t1 / verts_4d[vert_indices[2]].z / normalization, 0.0, 1.0); // Perspective correction.
				t2 = clamp(t2 / verts_4d[vert_indices[3]].z / normalization, 0.0, 1.0);
				center_position_4d = verts_4d[vert_indices[0]];
				other_center_position_4d = verts_4d[vert_indices[1]] + t1 * (verts_4d[vert_indices[2]] - verts_4d[vert_indices[1]]) + t2 * (verts_4d[vert_indices[3]] - verts_4d[vert_indices[1]]);
				center_uvw = uvws[vert_indices[0]];
				other_center_uvw = uvws[vert_indices[1]] + t1 * (uvws[vert_indices[2]] - uvws[vert_indices[1]]) + t2 * (uvws[vert_indices[3]] - uvws[vert_indices[1]]);
				if (PROJECTION_LOOKUP[projection_case * 5] == 2) {
					vec4 temp_position_4d = other_center_position_4d;
					other_center_position_4d = center_position_4d;
					center_position_4d = temp_position_4d;
					vec3 temp_uvw = other_center_uvw;
					other_center_uvw = center_uvw;
					center_uvw = temp_uvw;
				}
			}
			position_4d = center_position_4d;
			uvw = center_uvw;
		} else {
			int index = vert_indices[TRIANGLE_FAN_LOOKUP[vertex_id % 12 + (PROJECTION_LOOKUP[projection_case * 5] % 2) * 12]];
			position_4d = verts_4d[index];
			uvw = uvws[index];
			centerness = 0.0;
		}
		// Vertex is view space and used for lighting, position is clip space and used for rasterizing.
		VERTEX = position_4d.xyz;
		position_w = position_4d.w;
		POSITION = PROJECTION_MATRIX * vec4(position_4d.xyz, 1.0);

		vec3 normal = normalize(normal_4d.xyz);
		vec3 tangent = normalize((verts_4d[1] - verts_4d[0]).xyz); // May not be perpendicular to the normal after projecting to 3D; unclear if that matters.
		vec3 binormal = normalize(cross(normal, tangent));
		NORMAL = normal;
		TANGENT = tangent;
		BINORMAL = binormal;
	}
}

// The opacity per thickness varies with W position in clip space. This is the integral of that.
float density_integral(float w_clip) {
	return pow(1.0 - pow(1.0 - min(1.0, abs(w_clip)), edge_falloff), plane_softness) * sign(w_clip);
}

// The (right-)inverse function to the above: given the density integral, return the clip space coordinate.
float inv_density_integral(float density_int) {
	return (1.0 - pow(1.0 - pow(abs(density_int), 1.0 / plane_softness), 1.0 / edge_falloff)) * sign(density_int);
}

vec3 from_homogeneous(vec4 vec) {
	return vec.xyz / vec.w;
}

// A fragment of the 2D view corresponds to a line of fragments of the conceptual 3D view, so the correct result is the weighted integral
// of the color along it. It is approximated by the line's length and the color at its weighted centroid. The line runs from position,
// at coordinate 0, to other_position, at coordinate 1.
void fragment() {
	vec4 position_4d = vec4(VERTEX, position_w);
	NORMAL = normalize(NORMAL);
	// This is not very numerically stable for centerness near 1, but this doesn't appear to cause visible issues.
	vec4 peripheral_position_4d = center_position_4d + (position_4d - center_position_4d) / (1.0 - centerness);
	float other_centerness = (centerness * center_position_4d.z / other_center_position_4d.z) / (centerness * center_position_4d.z / other_center_position_4d.z + (1.0 - centerness));
	vec4 other_position_4d = mix(peripheral_position_4d, other_center_position_4d, other_centerness);
	// Undo the skewness transformation so it doesn't affect the frustum shape, opacity, and depth clipping.
	position_4d.z -= position_4d.w * skewness;
	other_position_4d.z -= other_position_4d.w * skewness;

	float z_near_limit = from_homogeneous(INV_PROJECTION_MATRIX * vec4(0.0, 0.0, 1.0, 1.0)).z; // 1 is near Z in clip space, so this is near Z in view space.
	float cross_section_depth = texture(cross_section_depth_texture, SCREEN_UV).r + DEPTH_BIAS_CLIPSPACE;
	float z_far_limit = from_homogeneous(INV_PROJECTION_MATRIX * vec4(0.0, 0.0, cross_section_depth, 1.0)).z * (1.0 - DEPTH_BIAS_VIEWSPACE);
	// The coordinates, within the line, of each end of the section of the line that may be visible.
	float line_end_1;
	if (position_4d.z > z_near_limit) {
		line_end_1 = (position_4d.z - z_near_limit) / (position_4d.z - other_position_4d.z); // A division by 0 makes thickness NaN, which renders as 0, the correct answer then.
	} else if (position_4d.z < z_far_limit) {
		line_end_1 = (position_4d.z - z_far_limit) / (position_4d.z - other_position_4d.z);
	} else {
		line_end_1 = 0.0;
	}
	float line_end_2;
	if (other_position_4d.z > z_near_limit) {
		line_end_2 = (position_4d.z - z_near_limit) / (position_4d.z - other_position_4d.z);
	} else if (other_position_4d.z < z_far_limit) {
		line_end_2 = (position_4d.z - z_far_limit) / (position_4d.z - other_position_4d.z);
	} else {
		line_end_2 = 1.0;
	}
	vec4 end_position_1_4d = mix(position_4d, other_position_4d, line_end_1);
	vec4 end_position_2_4d = mix(position_4d, other_position_4d, line_end_2);
	// Godot's camera looks down -Z. Keep this denominator negative to preserve the endpoint
	// orientation used by the signed density integral below, while its magnitude grows with depth.
	float perspective_1 = end_position_1_4d.z * camera_slope - camera_fade;
	float perspective_2 = end_position_2_4d.z * camera_slope - camera_fade;
	float w_clip_1 = end_position_1_4d.w / perspective_1;
	float w_clip_2 = end_position_2_4d.w / perspective_2;
	float thickness = density_integral(w_clip_1) - density_integral(w_clip_2);
	float w_clip_middle = inv_density_integral((density_integral(w_clip_1) + density_integral(w_clip_2)) / 2.0);
	// The position, along the line, of its opacity-weighted centroid.
	float middle_weight = mix(line_end_1, line_end_2, (w_clip_middle - w_clip_1) / perspective_2 / ((w_clip_middle - w_clip_1) / perspective_2 + (w_clip_2 - w_clip_middle) / perspective_1));
	middle_weight = isnan(middle_weight) ? 0.5 : clamp(middle_weight, 0.0, 1.0);

	vec4 middle_position_4d = mix(position_4d, other_position_4d, middle_weight);
	LIGHT_VERTEX = middle_position_4d.xyz;
	/* LIGHT_VERTEX_W_ASSIGNMENT_THIS_IS_REPLACED_IN_TETRA_MATERIAL_CPP_CODE */
	vec3 other_uvw = mix(center_uvw + (uvw - center_uvw) / (1.0 - centerness), other_center_uvw, other_centerness);
	vec3 middle_uvw = mix(uvw, other_uvw, middle_weight);
	ALBEDO = albedo.rgb * texture(albedo_texture, middle_uvw * albedo_texture_map_scale + albedo_texture_map_offset).rgb;
	ALPHA = sqrt(max(thickness, 0.0) / 2.0); // The sqrt is to compensate for a bug in the definition of Godot's add blend mode.
	ALBEDO *= ALPHA; // Also compensating for the bug.
}
