#include "array_tetra_mesh_4d.h"

#include "../../math/vector_4d.h"

void ArrayTetraMesh4D::_clear_cache() {
	_cell_positions_cache.clear();
	tetra_mesh_clear_cache();
}

bool ArrayTetraMesh4D::validate_mesh_data() {
	const int64_t cell_indices_count = _cell_indices.size();
	if (cell_indices_count % 4 != 0) {
		return false; // Must be a multiple of 4.
	}
	const int64_t cell_uvw_map_count = _cell_uvw_map.size();
	if (cell_uvw_map_count > 0 && cell_uvw_map_count != cell_indices_count) {
		return false; // Must be the same size as the cell indices.
	}
	const int64_t cell_normals_count = _cell_normals.size();
	if (cell_normals_count > 0 && cell_normals_count * 4 != cell_indices_count) {
		return false; // Must be have one normal per cell (4 indices).
	}
	const int64_t vertex_count = _vertices.size();
	for (int32_t cell_index : _cell_indices) {
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
	_cell_indices.append(p_index_a);
	_cell_indices.append(p_index_b);
	_cell_indices.append(p_index_c);
	_cell_indices.append(p_index_d);
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

void ArrayTetraMesh4D::calculate_normals(const bool p_keep_existing) {
	const int cell_count = _cell_indices.size() / 4;
	_cell_normals.resize(cell_count);
	for (int i = 0; i < cell_count; i++) {
		if (p_keep_existing && _cell_normals[i] != Vector4()) {
			continue;
		}
		const Vector4 pivot = _vertices[_cell_indices[i * 4]];
		const Vector4 a = _vertices[_cell_indices[1 + i * 4]];
		const Vector4 b = _vertices[_cell_indices[2 + i * 4]];
		const Vector4 c = _vertices[_cell_indices[3 + i * 4]];
		Vector4 perp = Vector4D::perpendicular(a - pivot, b - pivot, c - pivot);
		_cell_normals.set(i, perp);
	}
}

void ArrayTetraMesh4D::merge_with(const Ref<ArrayTetraMesh4D> &p_other, const Transform4D &p_transform) {
	const int start_cell_index_count = _cell_indices.size();
	const int start_cell_normal_count = _cell_normals.size();
	const int start_cell_uvw_map_count = _cell_uvw_map.size();
	const int start_vertex_count = _vertices.size();
	const int other_cell_index_count = p_other->_cell_indices.size();
	const int other_cell_normal_count = p_other->_cell_normals.size();
	const int other_cell_uvw_map_count = p_other->_cell_uvw_map.size();
	const int other_vertex_count = p_other->_vertices.size();
	const int end_cell_index_count = start_cell_index_count + other_cell_index_count;
	const int end_vertex_count = start_vertex_count + other_vertex_count;
	_cell_indices.resize(end_cell_index_count);
	_vertices.resize(end_vertex_count);
	for (int i = 0; i < other_cell_index_count; i++) {
		_cell_indices.set(start_cell_index_count + i, p_other->_cell_indices[i] + start_vertex_count);
	}
	for (int i = 0; i < other_vertex_count; i++) {
		_vertices.set(start_vertex_count + i, p_transform * p_other->_vertices[i]);
	}
	// Can't simply add these together in case the first mesh has no normals or UVW maps.
	if (start_cell_normal_count > 0 || other_cell_normal_count > 0) {
		const int end_cell_normal_count = end_cell_index_count / 4;
		_cell_normals.resize(end_cell_normal_count);
		if (other_cell_normal_count > 0) {
			const int cell_normal_write_offset = end_cell_normal_count - other_cell_normal_count;
			for (int i = 0; i < other_cell_normal_count; i++) {
				_cell_normals.set(cell_normal_write_offset + i, p_transform.basis * p_other->_cell_normals[i]);
			}
		}
	}
	if (start_cell_uvw_map_count > 0 || other_cell_uvw_map_count > 0) {
		const int end_cell_uvw_map_count = end_cell_index_count / 4;
		_cell_uvw_map.resize(end_cell_uvw_map_count);
		if (other_cell_uvw_map_count > 0) {
			const int cell_uvw_map_write_offset = end_cell_uvw_map_count - other_cell_uvw_map_count;
			for (int i = 0; i < other_cell_uvw_map_count; i++) {
				_cell_uvw_map.set(cell_uvw_map_write_offset + i, p_other->_cell_uvw_map[i]);
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

PackedInt32Array ArrayTetraMesh4D::get_cell_indices() {
	return _cell_indices;
}

void ArrayTetraMesh4D::set_cell_indices(const PackedInt32Array &p_cell_indices) {
	_cell_indices = p_cell_indices;
	_clear_cache();
	reset_mesh_data_validation();
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
	if (_cell_normals.is_empty()) {
		calculate_normals();
	}
	return _cell_normals;
}

void ArrayTetraMesh4D::set_cell_normals(const PackedVector4Array &p_cell_normals) {
	_cell_normals = p_cell_normals;
	reset_mesh_data_validation();
}

PackedVector3Array ArrayTetraMesh4D::get_cell_uvw_map() {
	return _cell_uvw_map;
}

void ArrayTetraMesh4D::set_cell_uvw_map(const PackedVector3Array &p_cell_uvw_map) {
	_cell_uvw_map = p_cell_uvw_map;
	reset_mesh_data_validation();
}

PackedVector4Array ArrayTetraMesh4D::get_vertices() {
	return _vertices;
}

void ArrayTetraMesh4D::set_vertices(const PackedVector4Array &p_vertices) {
	_vertices = p_vertices;
	_clear_cache();
	reset_mesh_data_validation();
}

void ArrayTetraMesh4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("append_tetra_cell_points", "point_a", "point_b", "point_c", "point_d", "deduplicate_vertices"), &ArrayTetraMesh4D::append_tetra_cell_points, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("append_tetra_cell_indices", "index_a", "index_b", "index_c", "index_d"), &ArrayTetraMesh4D::append_tetra_cell_indices);
	ClassDB::bind_method(D_METHOD("append_vertex", "vertex", "deduplicate_vertices"), &ArrayTetraMesh4D::append_vertex, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("append_vertices", "vertices", "deduplicate_vertices"), &ArrayTetraMesh4D::append_vertices, DEFVAL(true));

	ClassDB::bind_method(D_METHOD("calculate_normals", "keep_existing"), &ArrayTetraMesh4D::calculate_normals, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("merge_with", "other", "offset", "basis"), &ArrayTetraMesh4D::merge_with_bind, DEFVAL(Vector4()), DEFVAL(Projection()));

	// Only bind the setters here because the getters are already bound in TetraMesh4D.
	ClassDB::bind_method(D_METHOD("set_cell_indices", "cell_indices"), &ArrayTetraMesh4D::set_cell_indices);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT32_ARRAY, "cell_indices"), "set_cell_indices", "get_cell_indices");

	ClassDB::bind_method(D_METHOD("set_cell_normals", "cell_normals"), &ArrayTetraMesh4D::set_cell_normals);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR4_ARRAY, "cell_normals"), "set_cell_normals", "get_cell_normals");

	ClassDB::bind_method(D_METHOD("set_cell_uvw_map", "cell_uvw_map"), &ArrayTetraMesh4D::set_cell_uvw_map);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR3_ARRAY, "cell_uvw_map"), "set_cell_uvw_map", "get_cell_uvw_map");

	ClassDB::bind_method(D_METHOD("set_vertices", "vertices"), &ArrayTetraMesh4D::set_vertices);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR4_ARRAY, "vertices"), "set_vertices", "get_vertices");
}
