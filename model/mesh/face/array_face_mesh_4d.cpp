#include "array_face_mesh_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/surface_tool.hpp>
#elif GODOT_MODULE
#include "scene/resources/surface_tool.h"
#endif

void ArrayFaceMesh4D::_save_face_vertex_indices() {
	//printf("ArrayFaceMesh4D::_save_face_vertex_indices\n");
	_face_vertex_indices.clear();
	Vector<Vector<PackedInt32Array>> poly_cell_indices = this->get_poly_cell_indices();
	if (poly_cell_indices.size() < 1)
		return;
	Vector<PackedInt32Array> face_edge_indices = poly_cell_indices[0];
	_face_vertex_indices.resize(face_edge_indices.size());
	PackedInt32Array all_edge_indices = this->get_edge_indices();
	for (int i = 0; i < face_edge_indices.size(); i++) {
		_face_vertex_indices.set(i, PolyMesh4D::_get_vertex_indices_of_face(all_edge_indices, face_edge_indices[i]));
	}
}

void ArrayFaceMesh4D::_generate_triangles() {
	//printf("ArrayFaceMesh4D::_generate_triangles\n");
	_triangle_vertex_indices.clear();
	_triangle_vertex_indices.resize(_face_vertex_indices.size());
	for (int i = 0; i < _face_vertex_indices.size(); i++) {
		PackedInt32Array triangulated = 
			PolyMesh4D::_triangulate_face_vertex_indices(_face_vertex_indices[i], 0);
		_triangle_vertex_indices.set(i, triangulated);
	}
}

void ArrayFaceMesh4D::_set_smooth_shading_face_vertex_normals(const ComputeNormalsMode p_mode, const bool p_recalculate_boundary_normals) {
	//printf("ArrayFaceMesh4D::_set_smooth_shading_face_vertex_normals\n");
	this->_save_face_vertex_indices();
	Vector<Vector<PackedInt32Array>> poly_cell_indices = this->get_poly_cell_indices();
	if (poly_cell_indices.size() < 2)
		return;
	Vector<PackedInt32Array> cell_face_indices = poly_cell_indices[1];
	PackedVector4Array face_normals;
	face_normals.resize(_face_vertex_indices.size());
	for (int i = 0; i < _face_vertex_indices.size(); i++)
		face_normals.set(i, Vector4(0,0,0,0));
	if (p_recalculate_boundary_normals)
		this->calculate_boundary_normals(
			ArrayPolyMesh4D::ComputeNormalsMode::COMPUTE_NORMALS_MODE_FORCE_OUTWARD_FIX_CELL_ORIENTATION, false);
	PackedVector4Array boundary_normals = this->get_poly_cell_boundary_normals();

	// Sum bordering cell normals for all faces
	for (int i = 0; i < cell_face_indices.size(); i++)
		for (int j = 0; j < cell_face_indices[i].size(); j++)
			face_normals.set(cell_face_indices[i][j], face_normals[cell_face_indices[i][j]] + boundary_normals[i]);

	// Normalize face normals
	for (int i = 0; i < face_normals.size(); i++)
		face_normals.set(i, face_normals[i].normalized());

	int num_vertices = this->get_poly_cell_vertices().size();
	_vertex_normals.resize(num_vertices);
	for (int i = 0; i < num_vertices; i++)
		_vertex_normals.set(i, Vector4(0,0,0,0));

	// Sum bordering face normals for all vertices
	for (int i = 0; i < _face_vertex_indices.size(); i++)
		for (int j = 0; j < _face_vertex_indices[i].size(); j++)
			_vertex_normals.set(_face_vertex_indices[i][j], _vertex_normals[_face_vertex_indices[i][j]] + face_normals[i]);

	// Normalize vertex normals
	for (int i = 0; i < num_vertices; i++)
		_vertex_normals.set(i, _vertex_normals[i].normalized());

}

void ArrayFaceMesh4D::face_mesh_clear_cache(bool changed_geometry)  {
	//printf("ArrayFaceMesh4D::face_mesh_clear_cache\n");
	mark_cross_section_mesh_dirty();
	if (changed_geometry) {
		_face_vertex_indices.clear();
		_triangle_vertex_indices.clear();
	}
}

void ArrayFaceMesh4D::poly_mesh_clear_cache(const bool p_normals_only) {
	//printf("ArrayFaceMesh4D::poly_mesh_clear_cache\n");
	this->PolyMesh4D::poly_mesh_clear_cache(p_normals_only);
	this->face_mesh_clear_cache(!p_normals_only);
}

PackedInt32Array ArrayFaceMesh4D::get_vert_bones(int vert) {
	//printf("ArrayFaceMesh4D::get_vert_bones\n");
	PackedInt32Array result;
	if (_vertex_bones.size() > vert)
		result = _vertex_bones[vert];
	if (result.size() != 4) {
		result.resize(4);
		result.set(0, 0);
		result.set(1, 0);
		result.set(2, 0);
		result.set(3, 0);
	}
	return result;
}

PackedFloat32Array ArrayFaceMesh4D::get_vert_weights(int vert) {
	//printf("ArrayFaceMesh4D::get_vert_weights\n");
	PackedFloat32Array result;
	if (_vertex_bone_weights.size() > vert)
		result = _vertex_bone_weights[vert];
	if (result.size() != 4) {
		result.resize(4);
		result.set(0, 0.0);
		result.set(1, 0.0);
		result.set(2, 0.0);
		result.set(3, 0.0);
	}
	return result;
}

Color ArrayFaceMesh4D::get_vert_color(int vert) {
	//printf("ArrayFaceMesh4D::get_vert_color\n");
	Color result;
	if (_vertex_colors.size() > vert)
		result = _vertex_colors[vert];
	else
		result = Color(1.0, 1.0, 1.0, 1.0);
	return result;
}

Vector4 ArrayFaceMesh4D::get_vert_custom1(int vert) {
	//printf("ArrayFaceMesh4D::get_vert_custom1\n");
	Vector4 result;
	if (_vertex_custom1.size() > vert)
		result = _vertex_custom1[vert];
	else
		result = Vector4(1.0, 1.0, 0.0, 0.0);
	return result;
}

Vector4 ArrayFaceMesh4D::get_vert_custom2(int vert) {
	//printf("ArrayFaceMesh4D::get_vert_custom2\n");
	Vector4 result;
	if (_vertex_custom2.size() > vert)
		result = _vertex_custom2[vert];
	else
		result = Vector4(1.0, 1.0, 0.0, 0.0);
	return result;
}

Vector4 ArrayFaceMesh4D::get_vert_custom3(int vert) {
	//printf("ArrayFaceMesh4D::get_vert_custom3\n");
	Vector4 result;
	if (_vertex_custom3.size() > vert)
		result = _vertex_custom3[vert];
	else
		result = Vector4(1.0, 1.0, 0.0, 0.0);
	return result;
}

Vector4 ArrayFaceMesh4D::get_vert_normal(int vert) {
	//printf("ArrayFaceMesh4D::get_vert_normal\n");
	Vector4 result;
	if (_vertex_normals.size() > vert)
		result = _vertex_normals[vert];
	else
		result = Vector4(1.0, 0.0, 0.0, 0.0);
	return result;
}

Vector2 ArrayFaceMesh4D::get_vert_uv(int vert) {
	//printf("ArrayFaceMesh4D::get_vert_uv\n");
    Vector2 result;
	if (_vertex_uv.size() > vert)
		result = _vertex_uv[vert];
	else
		result = Vector2(0.0, 0.0);
	return result;
}

Vector2 ArrayFaceMesh4D::get_vert_uv2(int vert) {
	//printf("ArrayFaceMesh4D::get_vert_uv2\n");
	Vector2 result;
	if (_vertex_uv2.size() > vert)
		result = _vertex_uv2[vert];
	else
		result = Vector2(0.0, 0.0);
	return result;
}

Vector<PackedInt32Array> ArrayFaceMesh4D::get_all_vertex_bones() { 
	//printf("ArrayFaceMesh4D::get_all_vertex_bones\n");
	return _vertex_bones;
}

void ArrayFaceMesh4D::set_all_vertex_bones(Vector<PackedInt32Array> value) {
	//printf("ArrayFaceMesh4D::set_all_vertex_bones\n");
	_vertex_bones = value;
	face_mesh_clear_cache(false);
}

TypedArray<PackedInt32Array> ArrayFaceMesh4D::get_all_vertex_bones_bind() {
	//printf("ArrayFaceMesh4D::get_all_vertex_bones_bind\n");
	TypedArray<PackedInt32Array> tarr;
	Vector<PackedInt32Array> vertex_bones = get_all_vertex_bones();
	tarr.resize(vertex_bones.size());
	for (int i = 0; i < vertex_bones.size(); i++)
		tarr[i] = vertex_bones[i];
	return tarr;
}

void ArrayFaceMesh4D::set_all_vertex_bones_bind(const TypedArray<PackedInt32Array> &vertex_bones) {
	//printf("ArrayFaceMesh4D::set_all_vertex_bones_bind\n");
	Vector<PackedInt32Array> vec;
	vec.resize(vertex_bones.size());
	for (int i = 0; i < vertex_bones.size(); i++)
		vec.set(i, vertex_bones[i]);
	set_all_vertex_bones(vec);
}

Vector<PackedFloat32Array> ArrayFaceMesh4D::get_all_vertex_weights() {
	//printf("ArrayFaceMesh4D::get_all_vertex_weights\n");
	return _vertex_bone_weights;
}

void ArrayFaceMesh4D::set_all_vertex_weights(Vector<PackedFloat32Array> value) {
	//printf("ArrayFaceMesh4D::set_all_vertex_weights\n");
	_vertex_bone_weights = value;
	face_mesh_clear_cache(false);
}

TypedArray<PackedFloat32Array> ArrayFaceMesh4D::get_all_vertex_weights_bind() {
	//printf("ArrayFaceMesh4D::get_all_vertex_weights_bind\n");
	TypedArray<PackedFloat32Array> tarr;
	Vector<PackedFloat32Array> vertex_weights = get_all_vertex_weights();
	tarr.resize(vertex_weights.size());
	for (int i = 0; i < vertex_weights.size(); i++)
		tarr[i] = vertex_weights[i];
	return tarr;
}

void ArrayFaceMesh4D::set_all_vertex_weights_bind(const TypedArray<PackedFloat32Array> &vertex_weights) {
	//printf("ArrayFaceMesh4D::set_all_vertex_weights_bind\n");
	Vector<PackedFloat32Array> vec;
	vec.resize(vertex_weights.size());
	for (int i = 0; i < vertex_weights.size(); i++)
		vec.set(i, vertex_weights[i]);
	set_all_vertex_weights(vec);
}

PackedColorArray ArrayFaceMesh4D::get_all_vertex_colors() {
	//printf("ArrayFaceMesh4D::get_all_vertex_colors\n");
	return _vertex_colors;
}
void ArrayFaceMesh4D::set_all_vertex_colors(PackedColorArray value) {
	//printf("ArrayFaceMesh4D::set_all_vertex_colors\n");
	_vertex_colors = value;
	face_mesh_clear_cache(false); 
}

PackedVector4Array ArrayFaceMesh4D::get_all_vertex_custom1() {
	//printf("ArrayFaceMesh4D::get_all_vertex_custom1\n");
	return _vertex_custom1;
}

void ArrayFaceMesh4D::set_all_vertex_custom1(PackedVector4Array value) {
	//printf("ArrayFaceMesh4D::set_all_vertex_custom1\n");
	_vertex_custom1 = value;
	face_mesh_clear_cache(false);
}

PackedVector4Array ArrayFaceMesh4D::get_all_vertex_custom2() {
	//printf("ArrayFaceMesh4D::get_all_vertex_custom2\n");
	return _vertex_custom2;
}

void ArrayFaceMesh4D::set_all_vertex_custom2(PackedVector4Array value) {
	//printf("ArrayFaceMesh4D::set_all_vertex_custom2\n");
	_vertex_custom2 = value;
	face_mesh_clear_cache(false);
}

PackedVector4Array ArrayFaceMesh4D::get_all_vertex_custom3() {
	//printf("ArrayFaceMesh4D::get_all_vertex_custom3\n");
	return _vertex_custom3;
}

void ArrayFaceMesh4D::set_all_vertex_custom3(PackedVector4Array value) {
	//printf("ArrayFaceMesh4D::set_all_vertex_custom3\n");
	_vertex_custom3 = value;
	face_mesh_clear_cache(false);
}

PackedVector4Array ArrayFaceMesh4D::get_all_vertex_normals() {
	//printf("ArrayFaceMesh4D::get_all_vertex_normals\n");
	return _vertex_normals;
}

void ArrayFaceMesh4D::set_all_vertex_normals(PackedVector4Array value) {
	//printf("ArrayFaceMesh4D::set_all_vertex_normals\n");
	_vertex_normals = value;
	face_mesh_clear_cache(false);
}

PackedVector2Array ArrayFaceMesh4D::get_all_vertex_uv() {
	//printf("ArrayFaceMesh4D::get_all_vertex_uv\n");
	return _vertex_uv;
}

void ArrayFaceMesh4D::set_all_vertex_uv(PackedVector2Array value) {
	//printf("ArrayFaceMesh4D::set_all_vertex_uv\n");
	_vertex_uv = value;
	face_mesh_clear_cache(false);
}

PackedVector2Array ArrayFaceMesh4D::get_all_vertex_uv2() {
	//printf("ArrayFaceMesh4D::get_all_vertex_uv2\n");
	return _vertex_uv2;
}

void ArrayFaceMesh4D::set_all_vertex_uv2(PackedVector2Array value) {
	//printf("ArrayFaceMesh4D::set_all_vertex_uv2\n");
	_vertex_uv2 = value;
	face_mesh_clear_cache(false);
}

void ArrayFaceMesh4D::update_cross_section_mesh() {
	//printf("ArrayFaceMesh4D::update_cross_section_mesh\n");
	ERR_FAIL_COND(_cross_section_mesh.is_null());
	_cross_section_mesh->clear_surfaces();

	// Initialize surface tool
	Ref<SurfaceTool> surface_tool;
	surface_tool.instantiate();
	surface_tool->begin(Mesh::PRIMITIVE_TRIANGLES);
	surface_tool->set_smooth_group(-1);

	this->_set_smooth_shading_face_vertex_normals(ArrayPolyMesh4D::ComputeNormalsMode::COMPUTE_NORMALS_MODE_FORCE_OUTWARD_FIX_CELL_ORIENTATION, true);
	//this->set_flat_shading_normals(ArrayPolyMesh4D::ComputeNormalsMode::COMPUTE_NORMALS_MODE_FORCE_OUTWARD_FIX_CELL_ORIENTATION, false);

	Ref<Material4D> material = get_material();
	if (material.is_valid()) {
		surface_tool->set_material(material->get_cross_section_material());
	}
	surface_tool->set_custom_format(0, SurfaceTool::CUSTOM_RGBA_FLOAT);
	surface_tool->set_custom_format(1, SurfaceTool::CUSTOM_RGBA_FLOAT);
	surface_tool->set_custom_format(2, SurfaceTool::CUSTOM_RGBA_FLOAT);
	surface_tool->set_custom_format(3, SurfaceTool::CUSTOM_RGBA_FLOAT);

	PackedVector4Array vertex_positions = this->get_poly_cell_vertices();
	this->_generate_triangles();
	
	for (int i = 0; i < _triangle_vertex_indices.size(); i++) {
		for (int j = 0; j < _triangle_vertex_indices[i].size(); j++) {
			int vertex_index = _triangle_vertex_indices[i][j];
			Vector4 v_normal = get_vert_normal(vertex_index);

			surface_tool->set_custom(0,
				Vector4D::to_color(Vector4(
					vertex_positions[vertex_index].w,
					v_normal.w,
					0,
					0
				))
			);
			surface_tool->set_custom(1, Vector4D::to_color(get_vert_custom1(vertex_index)));
			surface_tool->set_custom(2, Vector4D::to_color(get_vert_custom2(vertex_index)));
			surface_tool->set_custom(3, Vector4D::to_color(get_vert_custom3(vertex_index)));
			surface_tool->set_normal(Vector3(
				v_normal.x,
				v_normal.y,
				v_normal.z
			));
			surface_tool->set_color(get_vert_color(vertex_index));
			surface_tool->set_bones(get_vert_bones(vertex_index));
			surface_tool->set_weights(get_vert_weights(vertex_index));
			surface_tool->set_uv(get_vert_uv(vertex_index));
			surface_tool->set_uv2(get_vert_uv2(vertex_index));

			surface_tool->add_vertex(Vector3(
				vertex_positions[vertex_index].x,
				vertex_positions[vertex_index].y,
				vertex_positions[vertex_index].z
			));
		}
	}

	surface_tool->commit(_cross_section_mesh);
}

void ArrayFaceMesh4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_all_vertex_bones"), &ArrayFaceMesh4D::get_all_vertex_bones_bind);
	ClassDB::bind_method(
		D_METHOD("set_all_vertex_bones", "array"), &ArrayFaceMesh4D::set_all_vertex_bones_bind);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "all_vertex_bones"), "set_all_vertex_bones", "get_all_vertex_bones");

	ClassDB::bind_method(D_METHOD("get_all_vertex_weights"), &ArrayFaceMesh4D::get_all_vertex_weights_bind);
	ClassDB::bind_method(
		D_METHOD("set_all_vertex_weights", "array"), &ArrayFaceMesh4D::set_all_vertex_weights_bind);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "all_vertex_weights"), "set_all_vertex_weights", "get_all_vertex_weights");

	ClassDB::bind_method(D_METHOD("get_all_vertex_colors"), &ArrayFaceMesh4D::get_all_vertex_colors);
	ClassDB::bind_method(
		D_METHOD("set_all_vertex_colors", "array"), &ArrayFaceMesh4D::set_all_vertex_colors);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "all_vertex_colors"), "set_all_vertex_colors", "get_all_vertex_colors");

	ClassDB::bind_method(D_METHOD("get_all_vertex_custom1"), &ArrayFaceMesh4D::get_all_vertex_custom1);
	ClassDB::bind_method(
		D_METHOD("set_all_vertex_custom1", "array"), &ArrayFaceMesh4D::set_all_vertex_custom1);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "all_vertex_custom1"), "set_all_vertex_custom1", "get_all_vertex_custom1");

	ClassDB::bind_method(D_METHOD("get_all_vertex_custom2"), &ArrayFaceMesh4D::get_all_vertex_custom2);
	ClassDB::bind_method(
		D_METHOD("set_all_vertex_custom2", "array"), &ArrayFaceMesh4D::set_all_vertex_custom2);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "all_vertex_custom2"), "set_all_vertex_custom2", "get_all_vertex_custom2");

	ClassDB::bind_method(D_METHOD("get_all_vertex_custom3"), &ArrayFaceMesh4D::get_all_vertex_custom3);
	ClassDB::bind_method(
		D_METHOD("set_all_vertex_custom3", "array"), &ArrayFaceMesh4D::set_all_vertex_custom3);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "all_vertex_custom3"), "set_all_vertex_custom3", "get_all_vertex_custom3");

	ClassDB::bind_method(D_METHOD("get_all_vertex_normals"), &ArrayFaceMesh4D::get_all_vertex_normals);
	ClassDB::bind_method(
		D_METHOD("set_all_vertex_normals", "array"), &ArrayFaceMesh4D::set_all_vertex_normals);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "all_vertex_normals"), "set_all_vertex_normals", "get_all_vertex_normals");
	
	ClassDB::bind_method(D_METHOD("get_all_vertex_uv"), &ArrayFaceMesh4D::get_all_vertex_uv);
	ClassDB::bind_method(
		D_METHOD("set_all_vertex_uv", "array"), &ArrayFaceMesh4D::set_all_vertex_uv);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "all_vertex_uv"), "set_all_vertex_uv", "get_all_vertex_uv");

	ClassDB::bind_method(D_METHOD("get_all_vertex_uv2"), &ArrayFaceMesh4D::get_all_vertex_uv2);
	ClassDB::bind_method(
		D_METHOD("set_all_vertex_uv2", "array"), &ArrayFaceMesh4D::set_all_vertex_uv2);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "all_vertex_uv2"), "set_all_vertex_uv2", "get_all_vertex_uv2");

	ClassDB::bind_method(D_METHOD("get_vert_bones", "index"), &ArrayFaceMesh4D::get_vert_bones);
	ClassDB::bind_method(D_METHOD("get_vert_weights", "index"), &ArrayFaceMesh4D::get_vert_weights);
	ClassDB::bind_method(D_METHOD("get_vert_color", "index"), &ArrayFaceMesh4D::get_vert_color);
	ClassDB::bind_method(D_METHOD("get_vert_custom1", "index"), &ArrayFaceMesh4D::get_vert_custom1);
	ClassDB::bind_method(D_METHOD("get_vert_custom2", "index"), &ArrayFaceMesh4D::get_vert_custom2);
	ClassDB::bind_method(D_METHOD("get_vert_custom3", "index"), &ArrayFaceMesh4D::get_vert_custom3);
	ClassDB::bind_method(D_METHOD("get_vert_normal", "index"), &ArrayFaceMesh4D::get_vert_normal);
	ClassDB::bind_method(D_METHOD("get_vert_uv", "index"), &ArrayFaceMesh4D::get_vert_uv);
	ClassDB::bind_method(D_METHOD("get_vert_uv2", "index"), &ArrayFaceMesh4D::get_vert_uv2);
}
