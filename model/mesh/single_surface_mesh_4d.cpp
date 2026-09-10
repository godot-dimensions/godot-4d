#include "single_surface_mesh_4d.h"

#include "wire/array_wire_mesh_4d.h"

bool SingleSurfaceMesh4D::has_edge_indices(int p_first, int p_second) {
	if (p_first > p_second) {
		SWAP(p_first, p_second);
	}
	PackedInt32Array edge_indices = get_edge_indices();
	for (int i = 0; i < edge_indices.size() - 1; i += 2) {
		if (edge_indices[i] == p_first && edge_indices[i + 1] == p_second) {
			return true;
		}
	}
	return false;
}

Ref<ArrayWireMesh4D> SingleSurfaceMesh4D::to_array_wire_mesh() {
	Ref<ArrayWireMesh4D> wire_mesh;
	wire_mesh.instantiate();
	wire_mesh->set_vertex_positions(get_vertex_positions());
	wire_mesh->set_edge_indices(get_edge_indices());
	wire_mesh->set_material(get_material());
	return wire_mesh;
}

Ref<WireMesh4D> SingleSurfaceMesh4D::to_wire_mesh() {
	return to_array_wire_mesh();
}

const Rect4 &SingleSurfaceMesh4D::get_rect_bounds() {
	if (likely(!_is_rect_bounds_dirty)) {
		return _rect_bounds;
	}
	_rect_bounds = Rect4(); // Start by including the mesh's local origin always, even if the mesh does not cover that point.
	const PackedVector4Array vertices = get_vertex_positions();
	for (int vertex_index = 0; vertex_index < vertices.size(); vertex_index++) {
		_rect_bounds.expand_self_to_point(vertices[vertex_index]);
	}
	_is_rect_bounds_dirty = false;
	return _rect_bounds;
}

Ref<Material4D> SingleSurfaceMesh4D::get_material() const {
	return _material;
}

void SingleSurfaceMesh4D::set_material(const Ref<Material4D> &p_material) {
	_material = p_material;
}

Ref<Material4D> SingleSurfaceMesh4D::get_fallback_material() {
	Ref<Material4D> material;
	GDVIRTUAL_CALL(_get_fallback_material, material);
	return material;
}

void SingleSurfaceMesh4D::validate_material_for_mesh(const Ref<Material4D> &p_material) {
	// Always call the virtual method to allow derived classes to provide more material validation.
	GDVIRTUAL_CALL(_validate_material_for_mesh, p_material);
	// For all SingleSurfaceMesh4D-derived meshes: Validate the material's color arrays against the mesh's vertex and edge counts.
	const Material4D::ColorSourceFlags albedo_source_flags = p_material->get_albedo_source_flags();
	if (albedo_source_flags & Material4D::COLOR_SOURCE_FLAG_USES_COLOR_ARRAY) {
		if (albedo_source_flags & Material4D::COLOR_SOURCE_FLAG_PER_VERT) {
			const PackedVector4Array vertices = get_vertex_positions();
			PackedColorArray color_array = p_material->get_albedo_color_array();
			if (color_array.size() < vertices.size()) {
				p_material->resize_albedo_color_array(vertices.size());
			}
		}
		if (albedo_source_flags & Material4D::COLOR_SOURCE_FLAG_PER_EDGE) {
			const PackedInt32Array edge_indices = get_edge_indices();
			PackedColorArray color_array = p_material->get_albedo_color_array();
			const int edge_count = edge_indices.size() / 2;
			if (color_array.size() < edge_count) {
				p_material->resize_albedo_color_array(edge_count);
			}
		}
	}
}

PackedInt32Array SingleSurfaceMesh4D::get_edge_indices() {
	PackedInt32Array edge_indices;
	GDVIRTUAL_CALL(_get_edge_indices, edge_indices);
	return edge_indices;
}

PackedVector4Array SingleSurfaceMesh4D::get_edge_positions() {
	PackedVector4Array edge_positions;
	GDVIRTUAL_CALL(_get_edge_positions, edge_positions);
	return edge_positions;
}

PackedVector4Array SingleSurfaceMesh4D::get_vertex_positions() {
	PackedVector4Array vertex_positions;
	GDVIRTUAL_CALL(_get_vertex_positions, vertex_positions);
	return vertex_positions;
}

PackedVector4Array SingleSurfaceMesh4D::get_normal_values() {
	PackedVector4Array vertex_normals;
	GDVIRTUAL_CALL(_get_normal_values, vertex_normals);
	return vertex_normals;
}

PackedVector3Array SingleSurfaceMesh4D::get_texture_map_values() {
	PackedVector3Array texture_map;
	GDVIRTUAL_CALL(_get_texture_map_values, texture_map);
	return texture_map;
}

void SingleSurfaceMesh4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("has_edge_indices", "first", "second"), &SingleSurfaceMesh4D::has_edge_indices);

	ClassDB::bind_method(D_METHOD("mark_mesh_bounds_and_proxy_mesh_3d_dirty"), &SingleSurfaceMesh4D::mark_mesh_bounds_and_proxy_mesh_3d_dirty);

	ClassDB::bind_method(D_METHOD("to_array_wire_mesh"), &SingleSurfaceMesh4D::to_array_wire_mesh);
	ClassDB::bind_method(D_METHOD("to_wire_mesh"), &SingleSurfaceMesh4D::to_wire_mesh);

	ClassDB::bind_method(D_METHOD("get_material"), &SingleSurfaceMesh4D::get_material);
	ClassDB::bind_method(D_METHOD("set_material", "material"), &SingleSurfaceMesh4D::set_material);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "material", PROPERTY_HINT_RESOURCE_TYPE, "Material4D"), "set_material", "get_material");

	ClassDB::bind_method(D_METHOD("get_edge_indices"), &SingleSurfaceMesh4D::get_edge_indices);
	ClassDB::bind_method(D_METHOD("get_edge_positions"), &SingleSurfaceMesh4D::get_edge_positions);
	ClassDB::bind_method(D_METHOD("get_vertex_positions"), &SingleSurfaceMesh4D::get_vertex_positions);
	ClassDB::bind_method(D_METHOD("get_normal_values"), &SingleSurfaceMesh4D::get_normal_values);
	ClassDB::bind_method(D_METHOD("get_texture_map_values"), &SingleSurfaceMesh4D::get_texture_map_values);

	GDVIRTUAL_BIND(_get_edge_indices);
	GDVIRTUAL_BIND(_get_edge_positions);
	GDVIRTUAL_BIND(_get_vertex_positions);
	GDVIRTUAL_BIND(_get_normal_values);
	GDVIRTUAL_BIND(_get_texture_map_values);

	GDVIRTUAL_BIND(_get_fallback_material);
}
