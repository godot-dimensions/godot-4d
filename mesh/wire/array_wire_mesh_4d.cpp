#include "array_wire_mesh_4d.h"

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
}

void ArrayWireMesh4D::merge_with_bind(const Ref<ArrayWireMesh4D> &p_other, const Vector4 &p_offset, const Projection &p_basis) {
	merge_with(p_other, Transform4D(p_basis, p_offset));
}

PackedInt32Array ArrayWireMesh4D::get_edge_indices() {
	return _edge_indices;
}

void ArrayWireMesh4D::set_edge_indices(const PackedInt32Array &p_edge_indices) {
	_edge_indices = p_edge_indices;
	wire_mesh_clear_cache();
}

PackedVector4Array ArrayWireMesh4D::get_vertices() {
	return _vertices;
}

void ArrayWireMesh4D::set_vertices(const PackedVector4Array &p_vertices) {
	_vertices = p_vertices;
	wire_mesh_clear_cache();
}

void ArrayWireMesh4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("merge_with", "other", "offset", "basis"), &ArrayWireMesh4D::merge_with_bind, DEFVAL(Vector4()), DEFVAL(Projection()));

	// Only bind the setters here because the getters are already bound in WireMesh4D.
	ClassDB::bind_method(D_METHOD("set_edge_indices", "edge_indices"), &ArrayWireMesh4D::set_edge_indices);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT32_ARRAY, "edge_indices"), "set_edge_indices", "get_edge_indices");

	ClassDB::bind_method(D_METHOD("set_vertices", "vertices"), &ArrayWireMesh4D::set_vertices);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR4_ARRAY, "vertices"), "set_vertices", "get_vertices");
}
