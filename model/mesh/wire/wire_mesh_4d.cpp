#include "wire_mesh_4d.h"

#include "../../../math/vector_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/surface_tool.hpp>
#elif GODOT_MODULE
#include "scene/resources/surface_tool.h"
#endif

void WireMesh4D::wire_mesh_clear_cache(const bool p_reset_validation) {
	_edge_positions_cache.clear();
	// The proxy mesh and rect bounds are also caches, so they are always marked dirty here.
	if (p_reset_validation) {
		reset_mesh_data_validation(); // This also marks the mesh bounds and proxy mesh as dirty.
	} else {
		mark_mesh_bounds_and_proxy_mesh_3d_dirty();
	}
}

PackedVector4Array WireMesh4D::get_edge_positions() {
	if (_edge_positions_cache.is_empty()) {
		const PackedInt32Array edge_indices = get_edge_indices();
		const PackedVector4Array vertices = get_vertex_positions();
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

void WireMesh4D::append_proxy_mesh_surfaces_3d(const Ref<ArrayMesh> &p_proxy_mesh_3d) {
	ERR_FAIL_COND(p_proxy_mesh_3d.is_null());
	// Refuse to build a surface that would overflow Godot's rendering server and crash. See the constants in the header.
	const PackedVector4Array edges = get_edge_positions();
	const int64_t edge_count = edges.size() / 2;
	if (edge_count > PROXY_MAX_EDGES_PER_SURFACE) {
		const int64_t vert_count = edge_count * PROXY_VERTS_PER_EDGE;
		ERR_FAIL_MSG("WireMesh4D: Mesh '" + get_name() + "' has " + itos(edge_count) + " edges, which would make a proxy surface of " + itos(vert_count) + " vertices and " + itos(vert_count * PROXY_BYTES_PER_VERT) + " bytes, but Godot's rendering server can only handle a surface of at most " + itos(PROXY_MAX_EDGES_PER_SURFACE) + " edges (" + itos(PROXY_MAX_VERTS_PER_SURFACE) + " vertices, " + itos(PROXY_MAX_VERTS_PER_SURFACE * PROXY_BYTES_PER_VERT) + " bytes). This surface will not be rendered. Split the mesh into multiple surfaces with different names or materials, or reduce its detail.");
	}
	// Set up SurfaceTool.
	Ref<SurfaceTool> surface_tool_3d;
	surface_tool_3d.instantiate();
	surface_tool_3d->begin(Mesh::PRIMITIVE_LINES);
	// Set up the custom format flags for the SurfaceTool.
	surface_tool_3d->set_custom_format(0, SurfaceTool::CUSTOM_RGBA_FLOAT);
	// Set the material, which SurfaceTool applies to the committed surface.
	const Ref<Material4D> material_4d = get_material();
	if (material_4d.is_valid()) {
		surface_tool_3d->set_material(material_4d->get_cross_section_material_3d());
	}
	// Iterate over the mesh data and append it to the SurfaceTool.
	for (Vector4 edge_vert : edges) {
		surface_tool_3d->set_custom(0, Vector4D::to_color(edge_vert));
		// Not using these positions because it doesn't fit the full vec4, but might as well set it to something sane.
		surface_tool_3d->add_vertex(Vector3(edge_vert.x, edge_vert.y, edge_vert.z));
	}
	// Commit to the proxy mesh. Note that SurfaceTool adds no surface when there are no vertices,
	// so an empty mesh results in a proxy mesh with zero surfaces rather than one empty surface.
	// This is why renderers must use `Mesh4D::get_proxy_surface_index_3d` to find 3D surfaces.
	surface_tool_3d->commit(p_proxy_mesh_3d);
}

void WireMesh4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("wire_mesh_clear_cache", "reset_validation"), &WireMesh4D::wire_mesh_clear_cache, DEFVAL(true));
}
