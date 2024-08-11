#include "array_wire_mesh_4d.h"

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
	// Only bind the setters here because the getters are already bound in WireMesh4D.
	ClassDB::bind_method(D_METHOD("set_edge_indices"), &ArrayWireMesh4D::set_edge_indices);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT32_ARRAY, "edge_indices"), "set_edge_indices", "get_edge_indices");

	ClassDB::bind_method(D_METHOD("set_vertices"), &ArrayWireMesh4D::set_vertices);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR4_ARRAY, "vertices"), "set_vertices", "get_vertices");
}
