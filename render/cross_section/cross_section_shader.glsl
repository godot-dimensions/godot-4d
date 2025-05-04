shader_type spatial;
render_mode skip_vertex_transform;
render_mode unshaded;

// World space
// Not allowed to pass matrices through instance uniforms, so have to unpack into vectors.
instance uniform vec4 modelview_origin;
instance uniform vec4 modelview_basis_x;
instance uniform vec4 modelview_basis_y;
instance uniform vec4 modelview_basis_z;
instance uniform vec4 modelview_basis_w;

uniform vec4 albedo: source_color; 

void vertex() {
    mat4 modelview_basis = mat4(modelview_basis_x, modelview_basis_y, modelview_basis_z, modelview_basis_w);
    // Make an arbitrary triangle from the cell so there's something on the screen.
    vec4 position = (VERTEX.x == 1.0 ? CUSTOM0 : (VERTEX.y == 1.0 ? CUSTOM1 : CUSTOM2));
    position = (modelview_basis * position) + modelview_origin;
    POSITION = PROJECTION_MATRIX * vec4(position.xyz, 1.0);
    // TODO actual cross-sectioning
}

void fragment() {
    ALBEDO = albedo.rgb;
}
