shader_type spatial;
render_mode skip_vertex_transform, cull_disabled, depth_test_disabled, blend_add;

#include "../shaders/perpendicular_4d.glsl"

// World space
// Not allowed to pass matrices through instance uniforms, so have to unpack into vectors.
instance uniform vec4 modelview_origin;
instance uniform vec4 modelview_basis_x;
instance uniform vec4 modelview_basis_y;
instance uniform vec4 modelview_basis_z;
instance uniform vec4 modelview_basis_w;

instance uniform float camera_slope = 1.0; // the tan of the angle of the view frustum in the W direction
instance uniform float camera_fade = 0.0; // the orthographic-style width of the view frustum in W
instance uniform float edge_falloff = 2.0; // how quickly to fade the opacity at the edges of the view frustum (1: not at all, 2: linear, up to infinity)
instance uniform float plane_softness = 0.7; // how much the region around the slice plane is emphasized: 1 is no extra emphasis, the limit as it approaches 0 is like a cross-section view (but 0 isn't actually a valid value).
instance uniform float skewness = 0.0; // -1 to 1. Not yet used.

uniform vec4 albedo : source_color;
uniform sampler3D albedo_texture : hint_default_white, source_color;
// Clip-space depth from the cross-section pass.
// Defaults to 0 (the far plane, under Forward+'s reverse-Z convention) when there's no
// cross-section pass to read from.
uniform sampler2D cross_section_depth_texture : hint_default_black, filter_nearest;
const float DEPTH_BIAS_CLIPSPACE = 1e-6; // to prevent z-fighting
const float DEPTH_BIAS_VIEWSPACE = 1e-5;

// The built-in perspective correction isn't able to deal with the fact that each fragment corresponds to a whole line on the input tet, with multiple different z values. It's necessary to do some of the interpolation manually.
// This re-interpolation is done between the center vertex, the other_center vertex (i.e. the other end of the line in the tet corresponding to the centre vertex in the triangle), and some point on the edge of the triangle not containing the centre vertex (which is interpolated correctly by the built-in interpolation between the two non-center vertices).
varying vec3 uvw;
varying flat vec3 center_uvw; // TODO: actually set the extra UVWs.
varying flat vec3 other_center_uvw;
varying flat vec4 center_position;
varying flat vec4 other_center_position;
varying float centerness;
varying float position_w; // the other components are stored in VERTEX, but that's a vec3.
// Read by the shared light() function appended to this shader, which normalizes it itself.
varying flat vec4 normal_4d;

// Look up, using a bit-mask of whether each face of the tet faces +W or -W, the order the vertices take to form the projection.
// Format: first number 0 for 3-triangle case (vertex-on), 1 for 4-triangle case (edge-on), 2 for 3-triangle case (face-on), -1 for impossible case.
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
	-1, 1, 2, -1, 2, 3, -1, 3, 1, -1,-1,-1,
	-1, 0, 1, -1, 1, 2, -1, 2, 3, -1, 3, 0
};

bool get_projection_case_one(vec4 vert0, vec4 vert1, vec4 vert2) {
  // It is not sufficient to use vert.xy/vert.z. The sign of z must also be taken into accout.
	return determinant(mat3(vert0.xyz, vert1.xyz, vert2.xyz)) > 0.;
}

// Generate a big-endian 4-bit mask of, for each face of the perspective-projected tetrahedron, whether its normal is in the +W or -W direction.
// This number determines which triangles are needed to send to the frag shader.
int get_projection_case(vec4[4] verts) {
	// Bitwise ops limit compatibility so summing instead.
	int negative1 = get_projection_case_one(verts[1], verts[2], verts[3]) ? 8 : 0;
	int negative2 = get_projection_case_one(verts[0], verts[3], verts[2]) ? 4 : 0;
	int negative3 = get_projection_case_one(verts[0], verts[1], verts[3]) ? 2 : 0;
	int negative4 = get_projection_case_one(verts[0], verts[2], verts[1]) ? 1 : 0;
	return negative1 + negative2 + negative3 + negative4;
}

void vertex() {
	mat4 modelview_basis = mat4(modelview_basis_x, modelview_basis_y, modelview_basis_z, modelview_basis_w);

	vec4 verts[] = { CUSTOM0, CUSTOM1, CUSTOM2, CUSTOM3 };
	for (int i = 0; i < 4; i++) {
		verts[i] = (modelview_basis * verts[i]) + modelview_origin;
		verts[i].z += verts[i].w * skewness;
	}

	vec3 uvws[] = { vec3(UV, COLOR.a), vec3(UV2, VERTEX.y), vec3(NORMAL.xy / NORMAL.z, VERTEX.z), COLOR.rgb };
	
	vec3[] verts_proj = {
		verts[0].xyw / verts[0].z,
		verts[1].xyw / verts[1].z,
		verts[2].xyw / verts[2].z,
		verts[3].xyw / verts[3].z
	};
	// Compute flat normals.
	normal_4d = perpendicular_4d(verts[1] - verts[0], verts[2] - verts[0], verts[3] - verts[0]);
	// This mustn't be computed from verts_proj alone, as that would give the wrong answer sometimes if a vertex is behind the camera.
	bool back_face = dot(verts[0], normal_4d) >= 0.;
	normal_4d.w -= normal_4d.z * skewness; // The skewed normal is needed for the backface calculation, (given that verts[0] is also skewed at this point) but the normal must be unskewed for lighting.

	int vertex_id = int(VERTEX.x);
	int projection_case = get_projection_case(verts);
	int[] vert_indices = {
		PROJECTION_LOOKUP[projection_case * 5 + 1],
		PROJECTION_LOOKUP[projection_case * 5 + 2],
		PROJECTION_LOOKUP[projection_case * 5 + 3],
		PROJECTION_LOOKUP[projection_case * 5 + 4]
	};
	if (
		vertex_id % 12 >= 9 && PROJECTION_LOOKUP[projection_case * 5] != 1 || // the fourth triangle of the 3-triangle case
		PROJECTION_LOOKUP[projection_case * 5] < 0 || // impossible case (i.e. degenerate tet or floating point error)
		back_face
	) {
		// This vertex is unused, cull
		POSITION = vec4(0., 0., CLIP_SPACE_FAR, 1.);
	} else {
		vec4 position;
		if (vertex_id % 3 == 0) { // centre vertex
		  centerness = 1.;
			if (PROJECTION_LOOKUP[projection_case * 5] == 1) { // 4-triangle case, this is the extra centre vertex.
				// need to find the intersection of the two edges in 2D space (basically screen-space).
				vec3 base1 = verts_proj[vert_indices[0]];
				vec3 point1 = verts_proj[vert_indices[2]] - base1;
				vec3 base2 = verts_proj[vert_indices[1]];
				vec3 point2 = verts_proj[vert_indices[3]] - base2;
				
				// solve: (base1.xy + t1 * point1.xy) = (base2.xy + t2 * point2.xy)
				// (base1 - base2).xy = [-point1.xy, point2.xy] [t1, t2]
				vec2 solution = inverse(mat2(-point1.xy, point2.xy)) * (base1 - base2).xy;
				float t1 = solution.x;
				t1 = t1 / verts[vert_indices[2]].z / (t1 / verts[vert_indices[2]].z + (1. - t1) / verts[vert_indices[0]].z); // perspective correction
				t1 = clamp(t1, 0., 1.); // The use of clamping here is not ideal. This algorithm is not numerically stable. Hopefully clamping will prevent the worst errors.
				float t2 = solution.y;
				t2 = t2 / verts[vert_indices[3]].z / (t2 / verts[vert_indices[3]].z + (1. - t2) / verts[vert_indices[1]].z);
				t2 = clamp(t2, 0., 1.);
				center_position = mix(verts[vert_indices[0]], verts[vert_indices[2]], t1);
				other_center_position = mix(verts[vert_indices[1]], verts[vert_indices[3]], t2);
				center_uvw = mix(uvws[vert_indices[0]], uvws[vert_indices[2]], t1);
				other_center_uvw = mix(uvws[vert_indices[1]], uvws[vert_indices[3]], t2);
			} else { // 3-triangle case, this is the centre vertex.
				vec3 target = verts_proj[vert_indices[0]];
				vec3 base = verts_proj[vert_indices[1]];
				vec3 point1 = verts_proj[vert_indices[2]] - base;
				vec3 point2 = verts_proj[vert_indices[3]] - base;
				
				// solve: (base + t1 * point1 + t2 * point2).xy = target.xy
				vec2 solution = inverse(mat2(point1.xy, point2.xy)) * (target - base).xy;
				float t1 = solution.x; // Ideally these would be clamped to a triangle not a square, but it isn't really worth the extra complexity given the clamping should usually do nothing anyway.
				float t2 = solution.y;
				float normalization = t1 / verts[vert_indices[2]].z + t2 / verts[vert_indices[3]].z + (1. - t1 - t2) / verts[vert_indices[1]].z;
				t1 = clamp(t1 / verts[vert_indices[2]].z / normalization, 0., 1.); // perspective correction
				t2 = clamp(t2 / verts[vert_indices[3]].z / normalization, 0., 1.);
				center_position = verts[vert_indices[0]];
				other_center_position = verts[vert_indices[1]] + t1 * (verts[vert_indices[2]] - verts[vert_indices[1]]) + t2 * (verts[vert_indices[3]] - verts[vert_indices[1]]);
				center_uvw = uvws[vert_indices[0]];
				other_center_uvw = uvws[vert_indices[1]] + t1 * (uvws[vert_indices[2]] - uvws[vert_indices[1]]) + t2 * (uvws[vert_indices[3]] - uvws[vert_indices[1]]);
				if (PROJECTION_LOOKUP[projection_case * 5] == 2) {
				  vec4 temp_pos = other_center_position;
				  other_center_position = center_position;
				  center_position = temp_pos;
				  vec3 temp_uvw = other_center_uvw;
				  other_center_uvw = center_uvw;
				  center_uvw = temp_uvw;
			  }
			}
			position = center_position;
			uvw = center_uvw;
		} else {
		  int index = vert_indices[TRIANGLE_FAN_LOOKUP[vertex_id % 12 + (PROJECTION_LOOKUP[projection_case * 5] % 2) * 12]];
			position = verts[index];
			uvw = uvws[index];
			centerness = 0.;
		}
		// Vertex is view space and used for lighting, position is clip space and used for rasterizing.
		VERTEX = position.xyz;
		position_w = position.w;
		POSITION = PROJECTION_MATRIX * vec4(position.xyz, 1.);

		vec3 normal = normalize(normal_4d.xyz);
		vec3 tangent = normalize((verts[1] - verts[0]).xyz); // not necessarily perpendicular to the normal after projecting to 3D. I'm not sure if this matters.
		vec3 binormal = normalize(cross(normal, tangent));
		NORMAL = normal;
		TANGENT = tangent;
		BINORMAL = binormal;
	}
}

// The opacity per thickness varies with w position in clip space. This is the integral of that.
float density_integral(float w_clip) {
  return pow(1. - pow(1. - min(1., abs(w_clip)), edge_falloff), plane_softness) * sign(w_clip);
}

// the (right-)inverse function to the above: given the density integral, return the clip space coordinate.
float inv_density_integral(float density_int) {
  return (1. - pow(1. - pow(abs(density_int), 1./plane_softness), 1./edge_falloff)) * sign(density_int);
}

vec3 from_homogeneous(vec4 vec) {
	return vec.xyz / vec.w;
}

// A single fragment of the actual 2D view corresponds to a line of fragments of the conceptual 3D view. The correct result is the weighted integral of the fragment color along this line. This is approximated by computing the length of the line, and the color at its weighted centroid.
// The line extends from position to other_position, and coordinates along the line are from 0 at position to 1 at other_position.
void fragment() {
  vec4 position = vec4(VERTEX, position_w);
	NORMAL = normalize(NORMAL);
  // This is not very numerically stable for centerness near 1, but this doesn't appear to cause visible issues.
  vec4 peripheral_position = center_position + (position - center_position) / (1. - centerness);
  float other_centerness = (centerness * center_position.z / other_center_position.z) / (centerness * center_position.z / other_center_position.z + (1. - centerness));
  vec4 other_position = mix(peripheral_position, other_center_position, other_centerness);
  // Undo the skewness transformation so it doesn't affect the frustum shape, opacity, and depth clipping.
  position.z -= position.w * skewness;
  other_position.z -= other_position.w * skewness;
	
	float z_near_limit = from_homogeneous(INV_PROJECTION_MATRIX * vec4(0.,0.,1.,1.)).z; // 1 is near Z in clip space, so this is near Z in view space.
	float cross_section_depth = texture(cross_section_depth_texture, SCREEN_UV).r + DEPTH_BIAS_CLIPSPACE;
	float z_far_limit = from_homogeneous(INV_PROJECTION_MATRIX * vec4(0.,0.,cross_section_depth,1.)).z * (1. - DEPTH_BIAS_VIEWSPACE);
	// the coordinates, within the line, of each end of the section of the line that may be visible.
	float line_end_1 = position.z > z_near_limit ?
			(position.z - z_near_limit) / (position.z - other_position.z) : // If this division is by 0, thickness will end up as NaN, which renders as 0, which is the correct answer in that case.
		position.z < z_far_limit ?
			(position.z - z_far_limit) / (position.z - other_position.z)
			: 0.;
	float line_end_2 = other_position.z > z_near_limit ?
			(position.z - z_near_limit) / (position.z - other_position.z) :
		other_position.z < z_far_limit ?
			(position.z - z_far_limit) / (position.z - other_position.z)
			: 1.;
	vec4 end_position_1 = mix(position, other_position, line_end_1);
	vec4 end_position_2 = mix(position, other_position, line_end_2);
	float perspective_1 = end_position_1.z * camera_slope + camera_fade;
	float perspective_2 = end_position_2.z * camera_slope + camera_fade;
	float w_clip_1 = end_position_1.w / perspective_1;
	float w_clip_2 = end_position_2.w / perspective_2;
	float thickness = density_integral(w_clip_1) - density_integral(w_clip_2);
	float w_clip_middle = inv_density_integral((density_integral(w_clip_1) + density_integral(w_clip_2)) / 2.);
	// the position, along the line, of its opacity-weighted centroid.
	float middle_weight = mix(line_end_1, line_end_2, (w_clip_middle - w_clip_1) / perspective_2 / ((w_clip_middle - w_clip_1) / perspective_2 + (w_clip_2 - w_clip_middle) / perspective_1));
	middle_weight = isnan(middle_weight) ? 0.5 : clamp(middle_weight, 0., 1.);
	
	vec4 middle_position = mix(position, other_position, middle_weight);
	LIGHT_VERTEX = middle_position.xyz;
	LIGHT_VERTEX_W = middle_position.w;
	vec3 other_uvw = mix(center_uvw + (uvw - center_uvw) / (1. - centerness), other_center_uvw, other_centerness);
	vec3 middle_uvw = mix(uvw, other_uvw, middle_weight);
	ALBEDO = albedo.rgb * texture(albedo_texture, middle_uvw).rgb;
	ALPHA = sqrt(thickness / 2.); // The sqrt is to compensate for a bug in the definition of Godot's add blend mode.
	ALBEDO *= ALPHA; // also compensating for the bug.
}
