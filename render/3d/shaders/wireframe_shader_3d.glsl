shader_type spatial;
render_mode skip_vertex_transform, unshaded;

// Wireframe meshes are drawn as a plain projection to 3D, ignoring W aside from the skew below.
// This shader is shared by the cross-section and projected passes: the only difference
// between them is the skewness instance uniform, which only the projected pass sets.

// World space.
// Not allowed to pass matrices through instance uniforms, so have to unpack into vectors.
instance uniform vec4 modelview_origin;
instance uniform vec4 modelview_basis_x;
instance uniform vec4 modelview_basis_y;
instance uniform vec4 modelview_basis_z;
instance uniform vec4 modelview_basis_w;

instance uniform float skewness = 0.0; // -1 to 1. Offsets the perspective projection's forward direction in W.

uniform vec4 albedo : source_color;

void vertex() {
	mat4 modelview_basis_4d = mat4(modelview_basis_x, modelview_basis_y, modelview_basis_z, modelview_basis_w);
	vec4 view_vert_4d = (modelview_basis_4d * CUSTOM0) + modelview_origin;
	view_vert_4d.z += view_vert_4d.w * skewness;

	VERTEX = view_vert_4d.xyz;
	POSITION = PROJECTION_MATRIX * vec4(view_vert_4d.xyz, 1.0);
}

void fragment() {
	ALBEDO = albedo.rgb;
}
