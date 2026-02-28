#include "array_tetra_mesh_4d.h"

#include "../../../math/vector_4d.h"

void ArrayTetraMesh4D::_clear_cache() {
	_simplex_positions_cache.clear();
	tetra_mesh_clear_cache();
}

bool ArrayTetraMesh4D::validate_mesh_data() {
	const int64_t cell_indices_count = _simplex_cell_indices.size();
	if (cell_indices_count % 4 != 0) {
		return false; // Must be a multiple of 4.
	}
	const int64_t cell_texture_map_count = _simplex_cell_texture_map.size();
	if (cell_texture_map_count > 0 && cell_texture_map_count != cell_indices_count) {
		return false; // Must be the same size as the cell indices.
	}
	const int64_t cell_boundary_normals_count = _simplex_cell_boundary_normals.size();
	if (cell_boundary_normals_count > 0 && cell_boundary_normals_count * 4 != cell_indices_count) {
		return false; // Must have one normal per cell (4 indices).
	}
	const int64_t cell_vertex_normals_count = _simplex_cell_vertex_normals.size();
	if (cell_vertex_normals_count > 0 && cell_vertex_normals_count != cell_indices_count) {
		return false; // Must have one normal per cell vertex instance (1 per index).
	}
	const int64_t vertex_count = _vertices.size();
	for (int32_t cell_index : _simplex_cell_indices) {
		if (cell_index < 0 || cell_index >= vertex_count) {
			return false; // Cells must reference valid vertices.
		}
	}
	return true;
}

void ArrayTetraMesh4D::append_tetra_cell_points(const Vector4 &p_a, const Vector4 &p_b, const Vector4 &p_c, const Vector4 &p_d, const bool p_deduplicate_vertices) {
	const int index_a = append_vertex(p_a, p_deduplicate_vertices);
	const int index_b = append_vertex(p_b, p_deduplicate_vertices);
	const int index_c = append_vertex(p_c, p_deduplicate_vertices);
	const int index_d = append_vertex(p_d, p_deduplicate_vertices);
	append_tetra_cell_indices(index_a, index_b, index_c, index_d);
	reset_mesh_data_validation();
}

void ArrayTetraMesh4D::append_tetra_cell_indices(const int p_index_a, const int p_index_b, const int p_index_c, const int p_index_d) {
	_simplex_cell_indices.append(p_index_a);
	_simplex_cell_indices.append(p_index_b);
	_simplex_cell_indices.append(p_index_c);
	_simplex_cell_indices.append(p_index_d);
	_clear_cache();
	reset_mesh_data_validation();
}

int ArrayTetraMesh4D::append_vertex(const Vector4 &p_vertex, const bool p_deduplicate_vertices) {
	const int vertex_count = _vertices.size();
	if (p_deduplicate_vertices) {
		for (int i = 0; i < vertex_count; i++) {
			if (_vertices[i] == p_vertex) {
				return i;
			}
		}
	}
	ERR_FAIL_COND_V(_vertices.size() > MAX_VERTICES, 2147483647);
	_vertices.push_back(p_vertex);
	reset_mesh_data_validation();
	return vertex_count;
}

PackedInt32Array ArrayTetraMesh4D::append_vertices(const PackedVector4Array &p_vertices, const bool p_deduplicate_vertices) {
	PackedInt32Array indices;
	for (int i = 0; i < p_vertices.size(); i++) {
		indices.append(append_vertex(p_vertices[i], p_deduplicate_vertices));
	}
	reset_mesh_data_validation();
	return indices;
}

void ArrayTetraMesh4D::calculate_boundary_normals(const bool p_keep_existing) {
	const int cell_count = _simplex_cell_indices.size() / 4;
	_simplex_cell_boundary_normals.resize(cell_count);
	for (int i = 0; i < cell_count; i++) {
		if (p_keep_existing && _simplex_cell_boundary_normals[i] != Vector4()) {
			continue;
		}
		const Vector4 pivot = _vertices[_simplex_cell_indices[i * 4]];
		const Vector4 a = _vertices[_simplex_cell_indices[1 + i * 4]];
		const Vector4 b = _vertices[_simplex_cell_indices[2 + i * 4]];
		const Vector4 c = _vertices[_simplex_cell_indices[3 + i * 4]];
		const Vector4 perp = Vector4D::perpendicular(a - pivot, b - pivot, c - pivot);
		_simplex_cell_boundary_normals.set(i, perp.normalized());
	}
}

void ArrayTetraMesh4D::set_flat_shading_normals(const bool p_recalculate_boundary_normals) {
	_simplex_cell_vertex_normals.clear();
	if (p_recalculate_boundary_normals) {
		calculate_boundary_normals();
	}
	const PackedVector4Array cell_boundary_normals = get_simplex_cell_boundary_normals();
	const int64_t cell_boundary_normal_count = cell_boundary_normals.size();
	CRASH_COND(cell_boundary_normal_count * 4 != _simplex_cell_indices.size());
	_simplex_cell_vertex_normals.resize(cell_boundary_normal_count * 4);
	for (int64_t cell_index = 0; cell_index < cell_boundary_normal_count; cell_index++) {
		const Vector4 &boundary_normal = cell_boundary_normals[cell_index];
		_simplex_cell_vertex_normals.set(cell_index * 4 + 0, boundary_normal);
		_simplex_cell_vertex_normals.set(cell_index * 4 + 1, boundary_normal);
		_simplex_cell_vertex_normals.set(cell_index * 4 + 2, boundary_normal);
		_simplex_cell_vertex_normals.set(cell_index * 4 + 3, boundary_normal);
	}
}

void ArrayTetraMesh4D::merge_with(const Ref<ArrayTetraMesh4D> &p_other, const Transform4D &p_transform) {
	const int64_t start_cell_index_count = _simplex_cell_indices.size();
	const int64_t start_cell_boundary_normal_count = _simplex_cell_boundary_normals.size();
	const int64_t start_cell_vertex_normal_count = _simplex_cell_vertex_normals.size();
	const int64_t start_cell_texture_map_count = _simplex_cell_texture_map.size();
	const int64_t start_vertex_count = _vertices.size();
	const int64_t other_cell_index_count = p_other->_simplex_cell_indices.size();
	const int64_t other_cell_boundary_normal_count = p_other->_simplex_cell_boundary_normals.size();
	const int64_t other_cell_vertex_normal_count = p_other->_simplex_cell_vertex_normals.size();
	const int64_t other_cell_texture_map_count = p_other->_simplex_cell_texture_map.size();
	const int64_t other_vertex_count = p_other->_vertices.size();
	const int64_t end_cell_index_count = start_cell_index_count + other_cell_index_count;
	const int64_t end_vertex_count = start_vertex_count + other_vertex_count;
	_simplex_cell_indices.resize(end_cell_index_count);
	_vertices.resize(end_vertex_count);
	for (int64_t i = 0; i < other_cell_index_count; i++) {
		_simplex_cell_indices.set(start_cell_index_count + i, p_other->_simplex_cell_indices[i] + start_vertex_count);
	}
	for (int64_t i = 0; i < other_vertex_count; i++) {
		_vertices.set(start_vertex_count + i, p_transform * p_other->_vertices[i]);
	}
	// Can't simply add these together in case the first mesh has no normals or UVW maps.
	if (start_cell_boundary_normal_count > 0 || other_cell_boundary_normal_count > 0) {
		const int64_t end_cell_normal_count = end_cell_index_count / 4;
		_simplex_cell_boundary_normals.resize(end_cell_normal_count);
		if (other_cell_boundary_normal_count > 0) {
			const int64_t cell_normal_write_offset = end_cell_normal_count - other_cell_boundary_normal_count;
			for (int64_t i = 0; i < other_cell_boundary_normal_count; i++) {
				_simplex_cell_boundary_normals.set(cell_normal_write_offset + i, p_transform.basis * p_other->_simplex_cell_boundary_normals[i]);
			}
		}
	}
	if (start_cell_vertex_normal_count > 0 || other_cell_vertex_normal_count > 0) {
		const int64_t end_cell_vertex_normal_count = end_cell_index_count;
		_simplex_cell_vertex_normals.resize(end_cell_vertex_normal_count);
		if (other_cell_vertex_normal_count > 0) {
			const int64_t cell_vertex_normal_write_offset = end_cell_vertex_normal_count - other_cell_vertex_normal_count;
			for (int64_t i = 0; i < other_cell_vertex_normal_count; i++) {
				_simplex_cell_vertex_normals.set(cell_vertex_normal_write_offset + i, p_transform.basis * p_other->_simplex_cell_vertex_normals[i]);
			}
		}
	}
	if (start_cell_texture_map_count > 0 || other_cell_texture_map_count > 0) {
		const int64_t end_cell_texture_map_count = end_cell_index_count / 4;
		_simplex_cell_texture_map.resize(end_cell_texture_map_count);
		if (other_cell_texture_map_count > 0) {
			const int64_t cell_texture_map_write_offset = end_cell_texture_map_count - other_cell_texture_map_count;
			for (int64_t i = 0; i < other_cell_texture_map_count; i++) {
				_simplex_cell_texture_map.set(cell_texture_map_write_offset + i, p_other->_simplex_cell_texture_map[i]);
			}
		}
	}
	Ref<Material4D> other_material = p_other->get_material();
	if (other_material.is_valid()) {
		Ref<Material4D> self_material = get_material();
		if (self_material.is_valid()) {
			self_material->merge_with(other_material, start_vertex_count, other_vertex_count);
		} else if (other_material->get_albedo_color_array().size() > 0) {
			self_material.instantiate();
			self_material->merge_with(other_material, start_vertex_count, other_vertex_count);
			set_material(self_material);
		} else {
			set_material(other_material);
		}
	}
	reset_mesh_data_validation();
}

void ArrayTetraMesh4D::merge_with_bind(const Ref<ArrayTetraMesh4D> &p_other, const Vector4 &p_offset, const Projection &p_basis) {
	merge_with(p_other, Transform4D(p_basis, p_offset));
}

PackedInt32Array ArrayTetraMesh4D::get_simplex_cell_indices() {
	return _simplex_cell_indices;
}

void ArrayTetraMesh4D::set_simplex_cell_indices(const PackedInt32Array &p_simplex_cell_indices) {
	_simplex_cell_indices = p_simplex_cell_indices;
	_clear_cache();
	reset_mesh_data_validation();
}

PackedVector4Array ArrayTetraMesh4D::get_simplex_cell_boundary_normals() {
	if (_simplex_cell_boundary_normals.is_empty()) {
		calculate_boundary_normals();
	}
	return _simplex_cell_boundary_normals;
}

void ArrayTetraMesh4D::set_simplex_cell_boundary_normals(const PackedVector4Array &p_simplex_cell_boundary_normals) {
	_simplex_cell_boundary_normals = p_simplex_cell_boundary_normals;
	reset_mesh_data_validation();
}

PackedVector4Array ArrayTetraMesh4D::get_simplex_cell_vertex_normals() {
	if (_simplex_cell_vertex_normals.is_empty()) {
		calculate_boundary_normals();
	}
	return _simplex_cell_vertex_normals;
}

void ArrayTetraMesh4D::set_simplex_cell_vertex_normals(const PackedVector4Array &p_simplex_cell_vertex_normals) {
	_simplex_cell_vertex_normals = p_simplex_cell_vertex_normals;
	reset_mesh_data_validation();
}

PackedVector3Array ArrayTetraMesh4D::get_simplex_cell_texture_map() {
	return _simplex_cell_texture_map;
}

void ArrayTetraMesh4D::set_simplex_cell_texture_map(const PackedVector3Array &p_simplex_cell_texture_map) {
	_simplex_cell_texture_map = p_simplex_cell_texture_map;
	reset_mesh_data_validation();
}

PackedVector4Array ArrayTetraMesh4D::get_vertices() {
	return _vertices;
}

void ArrayTetraMesh4D::set_vertices(const PackedVector4Array &p_vertices) {
	ERR_FAIL_COND(p_vertices.size() > MAX_VERTICES); // Prevent overflow.
	_vertices = p_vertices;
	_clear_cache();
	reset_mesh_data_validation();
}

bool ArrayTetraMesh4D::_set(const StringName &p_name, const Variant &p_value) {
	// Compatibility with old non-simplex "cell" names.
	if (p_name == StringName("cell_indices")) {
		set_simplex_cell_indices(p_value);
		return true;
	} else if (p_name == StringName("cell_boundary_normals")) {
		set_simplex_cell_boundary_normals(p_value);
		return true;
	} else if (p_name == StringName("cell_vertex_normals")) {
		set_simplex_cell_vertex_normals(p_value);
		return true;
	} else if (p_name == StringName("cell_uvw_map")) {
		set_simplex_cell_texture_map(p_value);
		return true;
	}
	return false;
}

bool ArrayTetraMesh4D::_get(const StringName &p_name, Variant &r_ret) const {
	// Compatibility with old non-simplex "cell" names.
	if (p_name == StringName("cell_indices")) {
		r_ret = PackedInt32Array(_simplex_cell_indices);
		return true;
	} else if (p_name == StringName("cell_boundary_normals")) {
		r_ret = PackedVector4Array(_simplex_cell_boundary_normals);
		return true;
	} else if (p_name == StringName("cell_vertex_normals")) {
		r_ret = PackedVector4Array(_simplex_cell_vertex_normals);
		return true;
	} else if (p_name == StringName("cell_uvw_map")) {
		r_ret = PackedVector3Array(_simplex_cell_texture_map);
		return true;
	}
	return false;
}

void ArrayTetraMesh4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("append_tetra_cell_points", "point_a", "point_b", "point_c", "point_d", "deduplicate_vertices"), &ArrayTetraMesh4D::append_tetra_cell_points, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("append_tetra_cell_indices", "index_a", "index_b", "index_c", "index_d"), &ArrayTetraMesh4D::append_tetra_cell_indices);
	ClassDB::bind_method(D_METHOD("append_vertex", "vertex", "deduplicate_vertices"), &ArrayTetraMesh4D::append_vertex, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("append_vertices", "vertices", "deduplicate_vertices"), &ArrayTetraMesh4D::append_vertices, DEFVAL(true));

	ClassDB::bind_method(D_METHOD("calculate_boundary_normals", "keep_existing"), &ArrayTetraMesh4D::calculate_boundary_normals, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("set_flat_shading_normals", "recalculate_boundary_normals"), &ArrayTetraMesh4D::set_flat_shading_normals, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("merge_with", "other", "offset", "basis"), &ArrayTetraMesh4D::merge_with_bind, DEFVAL(Vector4()), DEFVAL(Projection()));

	// Only bind the setters here because the getters are already bound in TetraMesh4D.
	ClassDB::bind_method(D_METHOD("set_simplex_cell_indices", "simplex_cell_indices"), &ArrayTetraMesh4D::set_simplex_cell_indices);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT32_ARRAY, "simplex_cell_indices"), "set_simplex_cell_indices", "get_simplex_cell_indices");

	ClassDB::bind_method(D_METHOD("set_simplex_cell_boundary_normals", "simplex_cell_boundary_normals"), &ArrayTetraMesh4D::set_simplex_cell_boundary_normals);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR4_ARRAY, "simplex_cell_boundary_normals"), "set_simplex_cell_boundary_normals", "get_simplex_cell_boundary_normals");

	ClassDB::bind_method(D_METHOD("set_simplex_cell_vertex_normals", "simplex_cell_vertex_normals"), &ArrayTetraMesh4D::set_simplex_cell_vertex_normals);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR4_ARRAY, "simplex_cell_vertex_normals"), "set_simplex_cell_vertex_normals", "get_simplex_cell_vertex_normals");

	ClassDB::bind_method(D_METHOD("set_simplex_cell_texture_map", "simplex_cell_texture_map"), &ArrayTetraMesh4D::set_simplex_cell_texture_map);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR3_ARRAY, "simplex_cell_texture_map"), "set_simplex_cell_texture_map", "get_simplex_cell_texture_map");

	ClassDB::bind_method(D_METHOD("set_vertices", "vertices"), &ArrayTetraMesh4D::set_vertices);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR4_ARRAY, "vertices"), "set_vertices", "get_vertices");
}
