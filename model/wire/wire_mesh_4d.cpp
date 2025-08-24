#include "wire_mesh_4d.h"

void WireMesh4D::wire_mesh_clear_cache() {
	_edge_positions_cache.clear();
}

PackedVector4Array WireMesh4D::get_edge_positions() {
	if (_edge_positions_cache.is_empty()) {
		const PackedInt32Array edge_indices = get_edge_indices();
		const PackedVector4Array vertices = get_vertices();
		const int32_t vertices_count = vertices.size();
		for (const int edge_index : edge_indices) {
			ERR_FAIL_COND_V(edge_index >= vertices_count, _edge_positions_cache);
			_edge_positions_cache.append(vertices[edge_index]);
		}
	}
	return _edge_positions_cache;
}

void WireMesh4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("wire_mesh_clear_cache"), &WireMesh4D::wire_mesh_clear_cache);
}
