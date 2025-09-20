#include "wire_mesh_4d.h"
#include "../../../math/vector_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/surface_tool.hpp>
#elif GODOT_MODULE
#include "scene/resources/surface_tool.h"
#endif

void WireMesh4D::wire_mesh_clear_cache() {
	_edge_positions_cache.clear();
	mark_cross_section_mesh_dirty();
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

Ref<WireMaterial4D> WireMesh4D::_fallback_material;

Ref<Material4D> WireMesh4D::get_fallback_material() {
	return _fallback_material;
}

void WireMesh4D::init_fallback_material() {
	_fallback_material.instantiate();
}

void WireMesh4D::cleanup_fallback_material() {
	_fallback_material.unref();
}

void WireMesh4D::update_cross_section_mesh() {
	ERR_FAIL_COND(_cross_section_mesh.is_null());
	_cross_section_mesh->clear_surfaces();

	Ref<SurfaceTool> surface_tool;
	surface_tool.instantiate();
	surface_tool->begin(Mesh::PRIMITIVE_LINES);
	surface_tool->set_custom_format(0, SurfaceTool::CUSTOM_RGBA_FLOAT);
	Ref<Material4D> material = get_material();
	if (material.is_valid()) {
		surface_tool->set_material(material->get_cross_section_material());
	}

	PackedVector4Array edges = get_edge_positions();
	for (Vector4 edge_vert : edges) {
		surface_tool->set_custom(0, Vector4D::to_color(edge_vert));
		// Not using these positions because it doesn't fit the full vec4, but might as well set it to something sane.
		surface_tool->add_vertex(Vector3(edge_vert.x, edge_vert.y, edge_vert.z));
	}
	surface_tool->commit(_cross_section_mesh);
}

void WireMesh4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("wire_mesh_clear_cache"), &WireMesh4D::wire_mesh_clear_cache);
}
