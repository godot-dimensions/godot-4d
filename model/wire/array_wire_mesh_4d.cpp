#include "array_wire_mesh_4d.h"

bool ArrayWireMesh4D::validate_mesh_data() {
	const int64_t edge_indices_count = _edge_indices.size();
	if (edge_indices_count % 2 != 0) {
		return false; // Must be a multiple of 2.
	}
	const int64_t vertex_count = _vertices.size();
	for (int32_t edge_index : _edge_indices) {
		if (edge_index < 0 || edge_index >= vertex_count) {
			return false; // Edges must reference valid vertices.
		}
	}
	return true;
}

void ArrayWireMesh4D::append_edge_points(const Vector4 &p_point_a, const Vector4 &p_point_b, const bool p_deduplicate_vertices) {
	int index_a = append_vertex(p_point_a, p_deduplicate_vertices);
	int index_b = append_vertex(p_point_b, p_deduplicate_vertices);
	append_edge_indices(index_a, index_b);
	reset_mesh_data_validation();
}

void ArrayWireMesh4D::append_edge_indices(int p_index_a, int p_index_b) {
	if (p_index_a > p_index_b) {
		SWAP(p_index_a, p_index_b);
	}
	_edge_indices.append(p_index_a);
	_edge_indices.append(p_index_b);
	wire_mesh_clear_cache();
	reset_mesh_data_validation();
}

int ArrayWireMesh4D::append_vertex(const Vector4 &p_vertex, const bool p_deduplicate_vertices) {
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

PackedInt32Array ArrayWireMesh4D::append_vertices(const PackedVector4Array &p_vertices, const bool p_deduplicate_vertices) {
	PackedInt32Array indices;
	for (int i = 0; i < p_vertices.size(); i++) {
		indices.append(append_vertex(p_vertices[i], p_deduplicate_vertices));
	}
	reset_mesh_data_validation();
	return indices;
}

void ArrayWireMesh4D::merge_with(const Ref<ArrayWireMesh4D> &p_other, const Transform4D &p_transform) {
	const int start_edge_count = _edge_indices.size();
	const int start_vertex_count = _vertices.size();
	const int other_edge_count = p_other->_edge_indices.size();
	const int other_vertex_count = p_other->_vertices.size();
	const int end_edge_count = start_edge_count + other_edge_count;
	const int end_vertex_count = start_vertex_count + other_vertex_count;
	_edge_indices.resize(end_edge_count);
	_vertices.resize(end_vertex_count);
	for (int i = 0; i < other_edge_count; i++) {
		_edge_indices.set(start_edge_count + i, p_other->_edge_indices[i] + start_vertex_count);
	}
	for (int i = 0; i < other_vertex_count; i++) {
		_vertices.set(start_vertex_count + i, p_transform * p_other->_vertices[i]);
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

void ArrayWireMesh4D::merge_with_bind(const Ref<ArrayWireMesh4D> &p_other, const Vector4 &p_offset, const Projection &p_basis) {
	merge_with(p_other, Transform4D(p_basis, p_offset));
}

void ArrayWireMesh4D::subdivide_edges(const int64_t p_subdivision_segments) {
	ERR_FAIL_COND_MSG(p_subdivision_segments < 2, "ArrayWireMesh4D: Cannot subdivide each edge into " + itos(p_subdivision_segments) + " segments, must be at least 2.");
	const int64_t old_edge_indices_size = _edge_indices.size();
	const int64_t old_edge_count = old_edge_indices_size / 2;
	int64_t used_edge_index_count = old_edge_indices_size;
	int64_t used_vertex_count = _vertices.size();
	const int64_t final_vertex_count = used_vertex_count + old_edge_count * (p_subdivision_segments - 1);
	ERR_FAIL_COND_MSG(final_vertex_count > 2147483647, "ArrayWireMesh4D: Too many vertices after subdivision.");
	_edge_indices.resize(old_edge_indices_size * p_subdivision_segments);
	_vertices.resize(final_vertex_count);
	for (int64_t edge_index = 0; edge_index < old_edge_indices_size; edge_index += 2) {
		const Vector4 start_point = _vertices[_edge_indices[edge_index]];
		const Vector4 end_point = _vertices[_edge_indices[edge_index + 1]];
		const Vector4 step = (end_point - start_point) / p_subdivision_segments;
		const int32_t end_edge_index = _edge_indices[edge_index + 1];
		_edge_indices.set(edge_index + 1, used_vertex_count);
		for (int64_t i = 1; i < p_subdivision_segments; i++) {
			const Vector4 new_point = start_point + step * i;
			_vertices.set(used_vertex_count, new_point);
			_edge_indices.set(used_edge_index_count, used_vertex_count);
			used_vertex_count++;
			used_edge_index_count++;
			if (i == p_subdivision_segments - 1) {
				// This is the end of the loop, so the endpoint of this
				// edge is the endpoint of the original edge.
				_edge_indices.set(used_edge_index_count, end_edge_index);
			} else {
				// This isn't the end of the loop, so the endpoint of this
				// edge is the vertex added by the next loop iteration.
				_edge_indices.set(used_edge_index_count, used_vertex_count);
			}
			used_edge_index_count++;
		}
	}
	// This should never fail, but if it does, we need to be noisy about it.
	CRASH_COND(used_edge_index_count != _edge_indices.size() || used_vertex_count != _vertices.size());
	wire_mesh_clear_cache();
}

void ArrayWireMesh4D::subdivide_one_edge(const int64_t p_edge_number, const int64_t p_subdivision_segments) {
	ERR_FAIL_COND_MSG(p_subdivision_segments < 2, "ArrayWireMesh4D: Cannot subdivide an edge into " + itos(p_subdivision_segments) + " segments, must be at least 2.");
	const int64_t start_edge_index = p_edge_number * 2;
	const int64_t end_edge_index = start_edge_index + 1;
	ERR_FAIL_INDEX_MSG(end_edge_index, _edge_indices.size(), "ArrayWireMesh4D: Edge number " + itos(p_edge_number) + " does not exist.");
	int64_t used_edge_index_count = _edge_indices.size();
	int64_t used_vertex_count = _vertices.size();
	const int64_t new_vertices = p_subdivision_segments - 1;
	_edge_indices.resize(used_edge_index_count + new_vertices * 2);
	_vertices.resize(used_vertex_count + new_vertices);
	const Vector4 start_point = _vertices[_edge_indices[start_edge_index]];
	const Vector4 end_point = _vertices[_edge_indices[end_edge_index]];
	const Vector4 step = (end_point - start_point) / p_subdivision_segments;
	_edge_indices.set(end_edge_index, used_vertex_count);
	for (int64_t i = 1; i < p_subdivision_segments; i++) {
		const Vector4 new_point = start_point + step * i;
		_vertices.set(used_vertex_count, new_point);
		_edge_indices.set(used_edge_index_count, used_vertex_count);
		used_vertex_count++;
		used_edge_index_count++;
		if (i == p_subdivision_segments - 1) {
			// This is the end of the loop, so the endpoint of this
			// edge is the endpoint of the original edge.
			_edge_indices.set(used_edge_index_count, end_edge_index);
		} else {
			// This isn't the end of the loop, so the endpoint of this
			// edge is the vertex added by the next loop iteration.
			_edge_indices.set(used_edge_index_count, used_vertex_count);
		}
		used_edge_index_count++;
	}
	wire_mesh_clear_cache();
}

PackedInt32Array ArrayWireMesh4D::get_edge_indices() {
	return _edge_indices;
}

void ArrayWireMesh4D::set_edge_indices(const PackedInt32Array &p_edge_indices) {
	_edge_indices = p_edge_indices;
	wire_mesh_clear_cache();
	reset_mesh_data_validation();
}

PackedVector4Array ArrayWireMesh4D::get_vertices() {
	return _vertices;
}

void ArrayWireMesh4D::set_vertices(const PackedVector4Array &p_vertices) {
	_vertices = p_vertices;
	wire_mesh_clear_cache();
	reset_mesh_data_validation();
}

void ArrayWireMesh4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("append_edge_points", "point_a", "point_b", "deduplicate"), &ArrayWireMesh4D::append_edge_points, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("append_edge_indices", "index_a", "index_b"), &ArrayWireMesh4D::append_edge_indices);
	ClassDB::bind_method(D_METHOD("append_vertex", "vertex", "deduplicate"), &ArrayWireMesh4D::append_vertex, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("append_vertices", "vertices", "deduplicate"), &ArrayWireMesh4D::append_vertices, DEFVAL(true));

	ClassDB::bind_method(D_METHOD("merge_with", "other", "offset", "basis"), &ArrayWireMesh4D::merge_with_bind, DEFVAL(Vector4()), DEFVAL(Projection()));

	ClassDB::bind_method(D_METHOD("subdivide_edges", "subdivision_segments"), &ArrayWireMesh4D::subdivide_edges, DEFVAL(2));
	ClassDB::bind_method(D_METHOD("subdivide_one_edge", "edge_number", "subdivision_segments"), &ArrayWireMesh4D::subdivide_one_edge, DEFVAL(2));

	// Only bind the setters here because the getters are already bound in WireMesh4D.
	ClassDB::bind_method(D_METHOD("set_edge_indices", "edge_indices"), &ArrayWireMesh4D::set_edge_indices);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT32_ARRAY, "edge_indices"), "set_edge_indices", "get_edge_indices");

	ClassDB::bind_method(D_METHOD("set_vertices", "vertices"), &ArrayWireMesh4D::set_vertices);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR4_ARRAY, "vertices"), "set_vertices", "get_vertices");
}
