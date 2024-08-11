#include "array_tetra_mesh_4d.h"

void ArrayTetraMesh4D::_clear_cache() {
	_cell_positions_cache.clear();
	tetra_mesh_clear_cache();
}

PackedInt32Array ArrayTetraMesh4D::get_cell_indices() {
	return _cell_indices;
}

void ArrayTetraMesh4D::set_cell_indices(const PackedInt32Array &p_cell_indices) {
	_cell_indices = p_cell_indices;
	_clear_cache();
}

PackedVector4Array ArrayTetraMesh4D::get_cell_positions() {
	if (_cell_positions_cache.is_empty()) {
		for (const int i : _cell_indices) {
			_cell_positions_cache.append(_vertices[i]);
		}
	}
	return _cell_positions_cache;
}

PackedVector4Array ArrayTetraMesh4D::get_cell_normals() {
	return _cell_normals;
}

void ArrayTetraMesh4D::set_cell_normals(const PackedVector4Array &p_cell_normals) {
	_cell_normals = p_cell_normals;
}

PackedVector3Array ArrayTetraMesh4D::get_cell_uvw_map() {
	return _cell_uvw_map;
}

void ArrayTetraMesh4D::set_cell_uvw_map(const PackedVector3Array &p_cell_uvw_map) {
	_cell_uvw_map = p_cell_uvw_map;
}

PackedVector4Array ArrayTetraMesh4D::get_vertices() {
	return _vertices;
}

void ArrayTetraMesh4D::set_vertices(const PackedVector4Array &p_vertices) {
	_vertices = p_vertices;
	_clear_cache();
}

void ArrayTetraMesh4D::_bind_methods() {
	// Only bind the setters here because the getters are already bound in TetraMesh4D.
	ClassDB::bind_method(D_METHOD("set_cell_indices"), &ArrayTetraMesh4D::set_cell_indices);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT32_ARRAY, "cell_indices"), "set_cell_indices", "get_cell_indices");

	ClassDB::bind_method(D_METHOD("set_cell_normals"), &ArrayTetraMesh4D::set_cell_normals);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR4_ARRAY, "cell_normals"), "set_cell_normals", "get_cell_normals");

	ClassDB::bind_method(D_METHOD("set_cell_uvw_map"), &ArrayTetraMesh4D::set_cell_uvw_map);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR3_ARRAY, "cell_uvw_map"), "set_cell_uvw_map", "get_cell_uvw_map");

	ClassDB::bind_method(D_METHOD("set_vertices"), &ArrayTetraMesh4D::set_vertices);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR4_ARRAY, "vertices"), "set_vertices", "get_vertices");
}
