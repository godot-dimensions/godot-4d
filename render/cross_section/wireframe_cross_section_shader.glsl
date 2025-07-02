shader_type spatial;
render_mode skip_vertex_transform, unshaded;

// World space
// Not allowed to pass matrices through instance uniforms, so have to unpack into vectors.
instance uniform vec4 modelview_origin;
instance uniform vec4 modelview_basis_x;
instance uniform vec4 modelview_basis_y;
instance uniform vec4 modelview_basis_z;
instance uniform vec4 modelview_basis_w;

uniform vec4 albedo : source_color;

void vertex() {
	mat4 modelview_basis = mat4(modelview_basis_x, modelview_basis_y, modelview_basis_z, modelview_basis_w);
	vec4 view_vert = (modelview_basis * CUSTOM0) + modelview_origin;

	VERTEX = view_vert.xyz;
	POSITION = PROJECTION_MATRIX * vec4(view_vert.xyz, 1.0);
}

void fragment() {
	ALBEDO = albedo.rgb;
}
