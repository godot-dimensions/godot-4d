#include "array_tetra_mesh_4d.h"

#include "../../../math/vector_4d.h"

void ArrayTetraMesh4D::_clear_cache() {
	_simplex_positions_cache.clear();
	_simplex_cell_boundary_normals.clear();
	tetra_mesh_clear_cache();
}

bool ArrayTetraMesh4D::validate_mesh_data() {
	const int64_t cell_vertex_indices_count = _simplex_cell_vertex_indices.size();
	ERR_FAIL_COND_V_MSG(cell_vertex_indices_count % 4 != 0, false, "ArrayTetraMesh4D: Simplex cell vertex indices size must be a multiple of 4.");
	const int64_t cell_texture_map_indices_count = _simplex_cell_texture_map_indices.size();
	ERR_FAIL_COND_V_MSG(cell_texture_map_indices_count > 0 && cell_texture_map_indices_count != cell_vertex_indices_count, false, "ArrayTetraMesh4D: Simplex cell texture map size must be the same as simplex cell vertex indices size (or empty).");
	const int64_t cell_boundary_normals_count = _simplex_cell_boundary_normals.size();
	ERR_FAIL_COND_V_MSG(cell_boundary_normals_count > 0 && cell_boundary_normals_count * 4 != cell_vertex_indices_count, false, "ArrayTetraMesh4D: Simplex cell boundary normals size must be one fourth of simplex cell vertex indices size (or empty).");
	const int64_t cell_normal_indices_count = _simplex_cell_normal_indices.size();
	ERR_FAIL_COND_V_MSG(cell_normal_indices_count > 0 && cell_normal_indices_count != cell_vertex_indices_count, false, "ArrayTetraMesh4D: Simplex cell normal indices size must be the same as simplex cell vertex indices size (or empty).");
	const int64_t vertex_pos_count = _vertex_positions.size();
	for (int32_t cell_vertex_index : _simplex_cell_vertex_indices) {
		ERR_FAIL_COND_V_MSG(cell_vertex_index < 0 || cell_vertex_index >= vertex_pos_count, false, "ArrayTetraMesh4D: Simplex cell vertex indices must reference valid vertices.");
	}
	const int64_t texture_map_value_count = _texture_map_values.size();
	for (int32_t texture_map_index : _simplex_cell_texture_map_indices) {
		ERR_FAIL_COND_V_MSG(texture_map_index < 0 || texture_map_index >= texture_map_value_count, false, "ArrayTetraMesh4D: Simplex cell texture map indices must reference valid texture map values.");
	}
	const int64_t normal_value_count = _normal_values.size();
	for (int32_t normal_index : _simplex_cell_normal_indices) {
		ERR_FAIL_COND_V_MSG(normal_index < 0 || normal_index >= normal_value_count, false, "ArrayTetraMesh4D: Simplex cell normal indices must reference valid normal values.");
	}
	return true;
}

void ArrayTetraMesh4D::append_tetra_cell_points(const Vector4 &p_a, const Vector4 &p_b, const Vector4 &p_c, const Vector4 &p_d, const bool p_deduplicate_vertices) {
	const int32_t index_a = append_vertex(p_a, p_deduplicate_vertices);
	const int32_t index_b = append_vertex(p_b, p_deduplicate_vertices);
	const int32_t index_c = append_vertex(p_c, p_deduplicate_vertices);
	const int32_t index_d = append_vertex(p_d, p_deduplicate_vertices);
	append_tetra_cell_indices(index_a, index_b, index_c, index_d);
	reset_mesh_data_validation();
}

void ArrayTetraMesh4D::append_tetra_cell_indices(const int32_t p_index_a, const int32_t p_index_b, const int32_t p_index_c, const int32_t p_index_d) {
	_simplex_cell_vertex_indices.append(p_index_a);
	_simplex_cell_vertex_indices.append(p_index_b);
	_simplex_cell_vertex_indices.append(p_index_c);
	_simplex_cell_vertex_indices.append(p_index_d);
	_clear_cache();
	reset_mesh_data_validation();
}

int32_t ArrayTetraMesh4D::append_vertex(const Vector4 &p_vertex, const bool p_deduplicate_vertices) {
	const int64_t vertex_pos_count = _vertex_positions.size();
	ERR_FAIL_COND_V_MSG(vertex_pos_count > Mesh4D::MAX_VERTICES, -1, "ArrayTetraMesh4D: Cannot add more vertices to the mesh. Maximum vertex count exceeded.");
	if (p_deduplicate_vertices) {
		for (int64_t i = 0; i < vertex_pos_count; i++) {
			if (_vertex_positions[i] == p_vertex) {
				return i;
			}
		}
	}
	_vertex_positions.push_back(p_vertex);
	tetra_mesh_clear_cache();
	reset_mesh_data_validation();
	return (int32_t)vertex_pos_count;
}

PackedInt32Array ArrayTetraMesh4D::append_vertices(const PackedVector4Array &p_vertices, const bool p_deduplicate_vertices) {
	PackedInt32Array indices;
	for (int i = 0; i < p_vertices.size(); i++) {
		indices.append(append_vertex(p_vertices[i], p_deduplicate_vertices));
	}
	reset_mesh_data_validation();
	return indices;
}

// Explicit compaction functions. Editing operations always leave the mesh in a consistent
// valid state, but may leave unreferenced values in the pools, which wastes space when kept.
// Compaction is not run automatically because it is O(n^2) in the pool size, so it is faster
// to run a sequence of editing operations first and only compact once at the end, if desired.

void ArrayTetraMesh4D::compact_normal_values() {
	ERR_FAIL_COND_MSG(!is_mesh_data_valid(), "ArrayTetraMesh4D: Cannot compact normal values of an invalid mesh.");
	// Mark which values are referenced by the normal indices.
	const int64_t old_value_count = _normal_values.size();
	Vector<bool> referenced;
	referenced.resize(old_value_count);
	for (int64_t i = 0; i < old_value_count; i++) {
		referenced.set(i, false);
	}
	for (const int32_t value_index : _simplex_cell_normal_indices) {
		referenced.set(value_index, true);
	}
	// Build the compacted pool, dropping unreferenced values and deduplicating
	// identical values, while preserving the relative order of the kept values.
	PackedVector4Array compacted_values;
	PackedInt32Array old_to_new;
	old_to_new.resize(old_value_count);
	for (int64_t i = 0; i < old_value_count; i++) {
		if (!referenced[i]) {
			old_to_new.set(i, -1);
			continue;
		}
		old_to_new.set(i, (int32_t)Vector4D::vector4_array_append_deduplicate(compacted_values, _normal_values[i]));
	}
	// Remap the normal indices into the compacted pool.
	for (int64_t i = 0; i < _simplex_cell_normal_indices.size(); i++) {
		_simplex_cell_normal_indices.set(i, old_to_new[_simplex_cell_normal_indices[i]]);
	}
	_normal_values = compacted_values;
	mark_proxy_mesh_3d_dirty();
	reset_mesh_data_validation();
}

void ArrayTetraMesh4D::compact_texture_map_values() {
	ERR_FAIL_COND_MSG(!is_mesh_data_valid(), "ArrayTetraMesh4D: Cannot compact texture map values of an invalid mesh.");
	// Mark which values are referenced by the texture map indices.
	const int64_t old_value_count = _texture_map_values.size();
	Vector<bool> referenced;
	referenced.resize(old_value_count);
	for (int64_t i = 0; i < old_value_count; i++) {
		referenced.set(i, false);
	}
	for (const int32_t value_index : _simplex_cell_texture_map_indices) {
		referenced.set(value_index, true);
	}
	// Build the compacted pool, dropping unreferenced values and deduplicating
	// identical values, while preserving the relative order of the kept values.
	PackedVector3Array compacted_values;
	PackedInt32Array old_to_new;
	old_to_new.resize(old_value_count);
	for (int64_t i = 0; i < old_value_count; i++) {
		if (!referenced[i]) {
			old_to_new.set(i, -1);
			continue;
		}
		old_to_new.set(i, (int32_t)Vector4D::vector3_array_append_deduplicate(compacted_values, _texture_map_values[i]));
	}
	// Remap the texture map indices into the compacted pool.
	for (int64_t i = 0; i < _simplex_cell_texture_map_indices.size(); i++) {
		_simplex_cell_texture_map_indices.set(i, old_to_new[_simplex_cell_texture_map_indices[i]]);
	}
	_texture_map_values = compacted_values;
	mark_proxy_mesh_3d_dirty();
	reset_mesh_data_validation();
}

void ArrayTetraMesh4D::calculate_boundary_normals(const bool p_keep_existing) {
	const int cell_count = _simplex_cell_vertex_indices.size() / 4;
	_simplex_cell_boundary_normals.resize(cell_count);
	for (int i = 0; i < cell_count; i++) {
		if (p_keep_existing && _simplex_cell_boundary_normals[i] != Vector4()) {
			continue;
		}
		const Vector4 pivot = _vertex_positions[_simplex_cell_vertex_indices[i * 4]];
		const Vector4 a = _vertex_positions[_simplex_cell_vertex_indices[1 + i * 4]];
		const Vector4 b = _vertex_positions[_simplex_cell_vertex_indices[2 + i * 4]];
		const Vector4 c = _vertex_positions[_simplex_cell_vertex_indices[3 + i * 4]];
		const Vector4 perp = Vector4D::perpendicular(a - pivot, b - pivot, c - pivot);
		_simplex_cell_boundary_normals.set(i, perp.normalized());
	}
}

void ArrayTetraMesh4D::set_flat_shading_normals(const bool p_force_recalculate_boundary_normals) {
	_simplex_cell_normal_indices.clear();
	_normal_values.clear();
	if (p_force_recalculate_boundary_normals || _simplex_cell_boundary_normals.is_empty()) {
		calculate_boundary_normals();
	}
	const PackedVector4Array cell_boundary_normals = get_simplex_cell_boundary_normals();
	const int64_t cell_boundary_normal_count = cell_boundary_normals.size();
	CRASH_COND(cell_boundary_normal_count * 4 != _simplex_cell_vertex_indices.size());
	_normal_values = cell_boundary_normals;
	_simplex_cell_normal_indices.resize(cell_boundary_normal_count * 4);
	for (int64_t cell_index = 0; cell_index < cell_boundary_normal_count; cell_index++) {
		_simplex_cell_normal_indices.set(cell_index * 4 + 0, cell_index);
		_simplex_cell_normal_indices.set(cell_index * 4 + 1, cell_index);
		_simplex_cell_normal_indices.set(cell_index * 4 + 2, cell_index);
		_simplex_cell_normal_indices.set(cell_index * 4 + 3, cell_index);
	}
	mark_proxy_mesh_3d_dirty();
	reset_mesh_data_validation();
}

void ArrayTetraMesh4D::merge_with(const Ref<ArrayTetraMesh4D> &p_other, const Transform4D &p_transform) {
	ERR_FAIL_COND_MSG(p_other.is_null(), "ArrayTetraMesh4D: Cannot merge a null mesh.");
	ERR_FAIL_COND_MSG(!is_mesh_data_valid(), "ArrayTetraMesh4D: Cannot merge into an invalid mesh.");
	ERR_FAIL_COND_MSG(!p_other->is_mesh_data_valid(), "ArrayTetraMesh4D: Cannot merge an invalid mesh.");
	const int64_t start_cell_vertex_index_count = _simplex_cell_vertex_indices.size();
	const int64_t start_cell_normal_index_count = _simplex_cell_normal_indices.size();
	const int64_t start_cell_texture_map_index_count = _simplex_cell_texture_map_indices.size();
	const int64_t start_cell_boundary_normal_count = _simplex_cell_boundary_normals.size();
	const int64_t start_normal_value_count = _normal_values.size();
	const int64_t start_texture_map_value_count = _texture_map_values.size();
	const int64_t start_vertex_pos_count = _vertex_positions.size();
	const int64_t other_cell_vertex_index_count = p_other->_simplex_cell_vertex_indices.size();
	const int64_t other_cell_normal_index_count = p_other->_simplex_cell_normal_indices.size();
	const int64_t other_cell_texture_map_index_count = p_other->_simplex_cell_texture_map_indices.size();
	const int64_t other_cell_boundary_normal_count = p_other->_simplex_cell_boundary_normals.size();
	const int64_t other_normal_value_count = p_other->_normal_values.size();
	const int64_t other_texture_map_value_count = p_other->_texture_map_values.size();
	const int64_t other_vertex_pos_count = p_other->_vertex_positions.size();
	const int64_t end_cell_vertex_index_count = start_cell_vertex_index_count + other_cell_vertex_index_count;
	const int64_t end_vertex_pos_count = start_vertex_pos_count + other_vertex_pos_count;
	_simplex_cell_vertex_indices.resize(end_cell_vertex_index_count);
	_vertex_positions.resize(end_vertex_pos_count);
	for (int64_t i = 0; i < other_cell_vertex_index_count; i++) {
		_simplex_cell_vertex_indices.set(start_cell_vertex_index_count + i, p_other->_simplex_cell_vertex_indices[i] + start_vertex_pos_count);
	}
	for (int64_t i = 0; i < other_vertex_pos_count; i++) {
		_vertex_positions.set(start_vertex_pos_count + i, p_transform * p_other->_vertex_positions[i]);
	}
	// Merge the value pools. The other mesh's normal values need to be transformed.
	_normal_values.resize(start_normal_value_count + other_normal_value_count);
	for (int64_t i = 0; i < other_normal_value_count; i++) {
		_normal_values.set(start_normal_value_count + i, p_transform.basis * p_other->_normal_values[i]);
	}
	_texture_map_values.resize(start_texture_map_value_count + other_texture_map_value_count);
	for (int64_t i = 0; i < other_texture_map_value_count; i++) {
		_texture_map_values.set(start_texture_map_value_count + i, p_other->_texture_map_values[i]);
	}
	// Can't simply add these together in case either mesh has no normals or texture map.
	if (start_cell_boundary_normal_count > 0 || other_cell_boundary_normal_count > 0) {
		const int64_t end_cell_boundary_normal_count = end_cell_vertex_index_count / 4;
		_simplex_cell_boundary_normals.resize(end_cell_boundary_normal_count);
		if (other_cell_boundary_normal_count > 0) {
			const int64_t cell_normal_write_offset = end_cell_boundary_normal_count - other_cell_boundary_normal_count;
			for (int64_t i = 0; i < other_cell_boundary_normal_count; i++) {
				_simplex_cell_boundary_normals.set(cell_normal_write_offset + i, p_transform.basis * p_other->_simplex_cell_boundary_normals[i]);
			}
		}
	}
	if (start_cell_normal_index_count > 0 || other_cell_normal_index_count > 0) {
		_simplex_cell_normal_indices.resize(end_cell_vertex_index_count);
		if (start_cell_normal_index_count < start_cell_vertex_index_count || other_cell_normal_index_count < other_cell_vertex_index_count) {
			// At least one of the meshes is missing normal indices, so point the missing entries at a zero normal value.
			const int32_t zero_normal_value_index = (int32_t)Vector4D::vector4_array_append_deduplicate(_normal_values, Vector4());
			for (int64_t i = start_cell_normal_index_count; i < start_cell_vertex_index_count; i++) {
				_simplex_cell_normal_indices.set(i, zero_normal_value_index);
			}
			if (other_cell_normal_index_count == 0) {
				for (int64_t i = start_cell_vertex_index_count; i < end_cell_vertex_index_count; i++) {
					_simplex_cell_normal_indices.set(i, zero_normal_value_index);
				}
			}
		}
		for (int64_t i = 0; i < other_cell_normal_index_count; i++) {
			_simplex_cell_normal_indices.set(start_cell_vertex_index_count + i, p_other->_simplex_cell_normal_indices[i] + int32_t(start_normal_value_count));
		}
	}
	if (start_cell_texture_map_index_count > 0 || other_cell_texture_map_index_count > 0) {
		_simplex_cell_texture_map_indices.resize(end_cell_vertex_index_count);
		if (start_cell_texture_map_index_count < start_cell_vertex_index_count || other_cell_texture_map_index_count < other_cell_vertex_index_count) {
			// At least one of the meshes is missing texture map indices, so point the missing entries at a zero texture map value.
			const int32_t zero_texture_map_value_index = (int32_t)Vector4D::vector3_array_append_deduplicate(_texture_map_values, Vector3());
			for (int64_t i = start_cell_texture_map_index_count; i < start_cell_vertex_index_count; i++) {
				_simplex_cell_texture_map_indices.set(i, zero_texture_map_value_index);
			}
			if (other_cell_texture_map_index_count == 0) {
				for (int64_t i = start_cell_vertex_index_count; i < end_cell_vertex_index_count; i++) {
					_simplex_cell_texture_map_indices.set(i, zero_texture_map_value_index);
				}
			}
		}
		for (int64_t i = 0; i < other_cell_texture_map_index_count; i++) {
			_simplex_cell_texture_map_indices.set(start_cell_vertex_index_count + i, p_other->_simplex_cell_texture_map_indices[i] + int32_t(start_texture_map_value_count));
		}
	}
	Ref<Material4D> other_material = p_other->get_material();
	if (other_material.is_valid()) {
		Ref<Material4D> self_material = get_material();
		if (self_material.is_valid()) {
			self_material->merge_with(other_material, start_vertex_pos_count, other_vertex_pos_count);
		} else if (other_material->get_albedo_color_array().size() > 0) {
			self_material.instantiate();
			self_material->merge_with(other_material, start_vertex_pos_count, other_vertex_pos_count);
			set_material(self_material);
		} else {
			set_material(other_material);
		}
	}
	tetra_mesh_clear_cache();
	reset_mesh_data_validation();
}

void ArrayTetraMesh4D::merge_with_bind(const Ref<ArrayTetraMesh4D> &p_other, const Vector4 &p_offset, const Projection &p_basis) {
	merge_with(p_other, Transform4D(p_basis, p_offset));
}

PackedInt32Array ArrayTetraMesh4D::get_simplex_cell_vertex_indices() {
	return _simplex_cell_vertex_indices;
}

PackedInt32Array ArrayTetraMesh4D::get_simplex_cell_normal_indices() {
	return _simplex_cell_normal_indices;
}

PackedInt32Array ArrayTetraMesh4D::get_simplex_cell_texture_map_indices() {
	return _simplex_cell_texture_map_indices;
}

void ArrayTetraMesh4D::set_simplex_cell_vertex_indices(const PackedInt32Array &p_simplex_cell_vertex_indices) {
	_simplex_cell_vertex_indices = p_simplex_cell_vertex_indices;
	_clear_cache();
	reset_mesh_data_validation();
}

void ArrayTetraMesh4D::set_simplex_cell_normal_indices(const PackedInt32Array &p_simplex_cell_normal_indices) {
	_simplex_cell_normal_indices = p_simplex_cell_normal_indices;
	mark_proxy_mesh_3d_dirty();
	reset_mesh_data_validation();
}

void ArrayTetraMesh4D::set_simplex_cell_texture_map_indices(const PackedInt32Array &p_simplex_cell_texture_map_indices) {
	_simplex_cell_texture_map_indices = p_simplex_cell_texture_map_indices;
	mark_proxy_mesh_3d_dirty();
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
	tetra_mesh_clear_cache();
	reset_mesh_data_validation();
}

PackedVector4Array ArrayTetraMesh4D::get_normal_values() {
	return _normal_values;
}

void ArrayTetraMesh4D::set_normal_values(const PackedVector4Array &p_normal_values) {
	_normal_values = p_normal_values;
	mark_proxy_mesh_3d_dirty();
	reset_mesh_data_validation();
}

PackedVector3Array ArrayTetraMesh4D::get_texture_map_values() {
	return _texture_map_values;
}

void ArrayTetraMesh4D::set_texture_map_values(const PackedVector3Array &p_texture_map_values) {
	_texture_map_values = p_texture_map_values;
	mark_proxy_mesh_3d_dirty();
	reset_mesh_data_validation();
}

PackedVector4Array ArrayTetraMesh4D::get_vertex_positions() {
	return _vertex_positions;
}

void ArrayTetraMesh4D::set_vertex_positions(const PackedVector4Array &p_vertex_positions) {
	ERR_FAIL_COND(p_vertex_positions.size() > MAX_VERTICES); // Prevent overflow.
	_vertex_positions = p_vertex_positions;
	_clear_cache();
	reset_mesh_data_validation();
}

bool ArrayTetraMesh4D::_set(const StringName &p_name, const Variant &p_value) {
	// Compatibility with old names for the vertex indices.
	if (p_name == StringName("cell_indices") || p_name == StringName("simplex_cell_indices")) {
		set_simplex_cell_vertex_indices(p_value);
		return true;
	} else if (p_name == StringName("cell_boundary_normals")) {
		set_simplex_cell_boundary_normals(p_value);
		return true;
	} else if (p_name == StringName("cell_vertex_normals") || p_name == StringName("simplex_cell_vertex_normals")) {
		// Compatibility with old per-cell-vertex normals: use them as the
		// value pool, with one index per cell vertex pointing at its value.
		const PackedVector4Array dense_normals = p_value;
		PackedInt32Array normal_indices;
		normal_indices.resize(dense_normals.size());
		for (int64_t i = 0; i < dense_normals.size(); i++) {
			normal_indices.set(i, i);
		}
		set_normal_values(dense_normals);
		set_simplex_cell_normal_indices(normal_indices);
		return true;
	} else if (p_name == StringName("cell_uvw_map") || p_name == StringName("simplex_cell_texture_map")) {
		// Compatibility with old per-cell-vertex texture maps: use them as the
		// value pool, with one index per cell vertex pointing at its value.
		const PackedVector3Array dense_texture_map = p_value;
		PackedInt32Array texture_map_indices;
		texture_map_indices.resize(dense_texture_map.size());
		for (int64_t i = 0; i < dense_texture_map.size(); i++) {
			texture_map_indices.set(i, i);
		}
		set_texture_map_values(dense_texture_map);
		set_simplex_cell_texture_map_indices(texture_map_indices);
		return true;
	}
	return false;
}

bool ArrayTetraMesh4D::_get(const StringName &p_name, Variant &r_ret) const {
	// Compatibility with old names for the vertex indices.
	if (p_name == StringName("cell_indices") || p_name == StringName("simplex_cell_indices")) {
		r_ret = PackedInt32Array(_simplex_cell_vertex_indices);
		return true;
	} else if (p_name == StringName("cell_boundary_normals")) {
		r_ret = PackedVector4Array(_simplex_cell_boundary_normals);
		return true;
	} else if (p_name == StringName("cell_vertex_normals") || p_name == StringName("simplex_cell_vertex_normals")) {
		// Compatibility with old per-cell-vertex normals: sample the indexed values densely.
		PackedVector4Array dense_normals;
		dense_normals.resize(_simplex_cell_normal_indices.size());
		for (int64_t i = 0; i < _simplex_cell_normal_indices.size(); i++) {
			const int32_t normal_index = _simplex_cell_normal_indices[i];
			ERR_CONTINUE(normal_index < 0 || normal_index >= _normal_values.size());
			dense_normals.set(i, _normal_values[normal_index]);
		}
		r_ret = dense_normals;
		return true;
	} else if (p_name == StringName("cell_uvw_map") || p_name == StringName("simplex_cell_texture_map")) {
		// Compatibility with old per-cell-vertex texture maps: sample the indexed values densely.
		PackedVector3Array dense_texture_map;
		dense_texture_map.resize(_simplex_cell_texture_map_indices.size());
		for (int64_t i = 0; i < _simplex_cell_texture_map_indices.size(); i++) {
			const int32_t texture_map_index = _simplex_cell_texture_map_indices[i];
			ERR_CONTINUE(texture_map_index < 0 || texture_map_index >= _texture_map_values.size());
			dense_texture_map.set(i, _texture_map_values[texture_map_index]);
		}
		r_ret = dense_texture_map;
		return true;
	}
	return false;
}

void ArrayTetraMesh4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("append_tetra_cell_points", "point_a", "point_b", "point_c", "point_d", "deduplicate_vertices"), &ArrayTetraMesh4D::append_tetra_cell_points, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("append_tetra_cell_indices", "index_a", "index_b", "index_c", "index_d"), &ArrayTetraMesh4D::append_tetra_cell_indices);
	ClassDB::bind_method(D_METHOD("append_vertex", "vertex", "deduplicate_vertices"), &ArrayTetraMesh4D::append_vertex, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("append_vertices", "vertices", "deduplicate_vertices"), &ArrayTetraMesh4D::append_vertices, DEFVAL(true));

	ClassDB::bind_method(D_METHOD("compact_normal_values"), &ArrayTetraMesh4D::compact_normal_values);
	ClassDB::bind_method(D_METHOD("compact_texture_map_values"), &ArrayTetraMesh4D::compact_texture_map_values);

	ClassDB::bind_method(D_METHOD("calculate_boundary_normals", "keep_existing"), &ArrayTetraMesh4D::calculate_boundary_normals, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("set_flat_shading_normals", "force_recalculate_boundary_normals"), &ArrayTetraMesh4D::set_flat_shading_normals, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("merge_with", "other", "offset", "basis"), &ArrayTetraMesh4D::merge_with_bind, DEFVAL(Vector4()), DEFVAL(Projection()));

	// Only bind the setters here because the getters are already bound in TetraMesh4D.
	ClassDB::bind_method(D_METHOD("set_simplex_cell_vertex_indices", "simplex_cell_vertex_indices"), &ArrayTetraMesh4D::set_simplex_cell_vertex_indices);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT32_ARRAY, "simplex_cell_vertex_indices"), "set_simplex_cell_vertex_indices", "get_simplex_cell_vertex_indices");
#ifndef DISABLE_DEPRECATED
	// Compatibility property to handle reading existing serialized data.
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT32_ARRAY, "simplex_cell_indices", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_INTERNAL), "set_simplex_cell_vertex_indices", "get_simplex_cell_vertex_indices");
#endif // DISABLE_DEPRECATED

	ClassDB::bind_method(D_METHOD("set_simplex_cell_normal_indices", "simplex_cell_normal_indices"), &ArrayTetraMesh4D::set_simplex_cell_normal_indices);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT32_ARRAY, "simplex_cell_normal_indices"), "set_simplex_cell_normal_indices", "get_simplex_cell_normal_indices");

	ClassDB::bind_method(D_METHOD("set_simplex_cell_texture_map_indices", "simplex_cell_texture_map_indices"), &ArrayTetraMesh4D::set_simplex_cell_texture_map_indices);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT32_ARRAY, "simplex_cell_texture_map_indices"), "set_simplex_cell_texture_map_indices", "get_simplex_cell_texture_map_indices");

	ClassDB::bind_method(D_METHOD("set_simplex_cell_boundary_normals", "simplex_cell_boundary_normals"), &ArrayTetraMesh4D::set_simplex_cell_boundary_normals);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR4_ARRAY, "simplex_cell_boundary_normals"), "set_simplex_cell_boundary_normals", "get_simplex_cell_boundary_normals");

	ClassDB::bind_method(D_METHOD("set_normal_values", "normal_values"), &ArrayTetraMesh4D::set_normal_values);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR4_ARRAY, "normal_values"), "set_normal_values", "get_normal_values");

	ClassDB::bind_method(D_METHOD("set_texture_map_values", "texture_map_values"), &ArrayTetraMesh4D::set_texture_map_values);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR3_ARRAY, "texture_map_values"), "set_texture_map_values", "get_texture_map_values");

	ClassDB::bind_method(D_METHOD("set_vertex_positions", "vertex_positions"), &ArrayTetraMesh4D::set_vertex_positions);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR4_ARRAY, "vertex_positions"), "set_vertex_positions", "get_vertex_positions");
#ifndef DISABLE_DEPRECATED
	// Compatibility property to handle reading existing serialized data.
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR4_ARRAY, "vertices", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_INTERNAL), "set_vertex_positions", "get_vertex_positions");
#endif // DISABLE_DEPRECATED
}
