#include "mesh_4d.h"

#include "wire/array_wire_mesh_4d.h"

#if GDEXTENSION
#include <godot_cpp/templates/hash_set.hpp>
#elif GODOT_MODULE
#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR < 6
#include "servers/rendering_server.h"
#else
#include "servers/rendering/rendering_server.h"
#endif
#endif

PackedInt32Array Mesh4D::deduplicate_edge_indices(const PackedInt32Array &p_items) {
	HashSet<Vector2i> unique_items;
	PackedInt32Array deduplicated_items;
	for (int i = 0; i < p_items.size() - 1; i += 2) {
		Vector2i edge_indices = Vector2i(p_items[i], p_items[i + 1]);
		if (edge_indices.x > edge_indices.y) {
			SWAP(edge_indices.x, edge_indices.y);
		}
		if (unique_items.has(edge_indices)) {
			continue;
		}
		unique_items.insert(edge_indices);
		deduplicated_items.push_back(edge_indices.x);
		deduplicated_items.push_back(edge_indices.y);
	}
	return deduplicated_items;
}

bool Mesh4D::has_edge_indices(int p_first, int p_second) {
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

bool Mesh4D::is_mesh_data_valid() {
	if (likely(_is_mesh_data_valid)) {
		return true;
	}
	_is_mesh_data_valid = validate_mesh_data();
	if (!_is_mesh_data_valid) {
		ERR_PRINT("Mesh4D: Mesh data is invalid on mesh '" + get_name() + "'.");
	}
	return _is_mesh_data_valid;
}

void Mesh4D::reset_mesh_data_validation() {
	_is_mesh_data_valid = false;
}

bool Mesh4D::validate_mesh_data() {
	bool ret = false;
	GDVIRTUAL_CALL(_validate_mesh_data, ret);
	return ret;
}

void Mesh4D::update_proxy_mesh_3d() {
	GDVIRTUAL_CALL(_update_proxy_mesh_3d);
}

void Mesh4D::validate_material_for_mesh(const Ref<Material4D> &p_material) {
	GDVIRTUAL_CALL(_validate_material_for_mesh, p_material);
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

Ref<ArrayWireMesh4D> Mesh4D::to_array_wire_mesh() {
	Ref<ArrayWireMesh4D> wire_mesh;
	wire_mesh.instantiate();
	wire_mesh->set_vertex_positions(get_vertex_positions());
	wire_mesh->set_edge_indices(get_edge_indices());
	wire_mesh->set_material(get_material());
	return wire_mesh;
}

Ref<WireMesh4D> Mesh4D::to_wire_mesh() {
	return to_array_wire_mesh();
}

Rect4 Mesh4D::get_rect_bounds() {
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

Ref<ArrayMesh> Mesh4D::get_proxy_mesh_3d() {
	if (_proxy_mesh_3d.is_null()) {
		_proxy_mesh_3d.instantiate();
	}
	if (_is_proxy_mesh_3d_dirty) {
		const String mesh_path_or_name = get_path().is_empty() ? get_name() : get_path();
		const String proxy_mesh_hint = mesh_path_or_name + String(" Proxy Mesh 3D");
		_proxy_mesh_3d->set_name(proxy_mesh_hint);
		update_proxy_mesh_3d();
		_is_proxy_mesh_3d_dirty = false;
#if GODOT_MODULE
		if (RenderingServer::get_singleton() != nullptr && _proxy_mesh_3d->get_rid().is_valid()) {
			RenderingServer::get_singleton()->mesh_set_path(_proxy_mesh_3d->get_rid(), proxy_mesh_hint);
		}
#endif
	}
	return _proxy_mesh_3d;
}

Ref<Material4D> Mesh4D::get_material() const {
	return _material;
}

void Mesh4D::set_material(const Ref<Material4D> &p_material) {
	_material = p_material;
}

Ref<Material4D> Mesh4D::get_fallback_material() {
	Ref<Material4D> material;
	GDVIRTUAL_CALL(_get_fallback_material, material);
	return material;
}

PackedInt32Array Mesh4D::get_edge_indices() {
	PackedInt32Array edge_indices;
	GDVIRTUAL_CALL(_get_edge_indices, edge_indices);
	return edge_indices;
}

PackedVector4Array Mesh4D::get_edge_positions() {
	PackedVector4Array edge_positions;
	GDVIRTUAL_CALL(_get_edge_positions, edge_positions);
	return edge_positions;
}

PackedVector4Array Mesh4D::get_vertex_positions() {
	PackedVector4Array vertex_positions;
	GDVIRTUAL_CALL(_get_vertex_positions, vertex_positions);
	return vertex_positions;
}

PackedVector4Array Mesh4D::get_normal_values() {
	PackedVector4Array vertex_normals;
	GDVIRTUAL_CALL(_get_normal_values, vertex_normals);
	return vertex_normals;
}

PackedVector3Array Mesh4D::get_texture_map_values() {
	PackedVector3Array texture_map;
	GDVIRTUAL_CALL(_get_texture_map_values, texture_map);
	return texture_map;
}

void Mesh4D::_bind_methods() {
	ClassDB::bind_static_method("Mesh4D", D_METHOD("deduplicate_edge_indices", "items"), &Mesh4D::deduplicate_edge_indices);
	ClassDB::bind_method(D_METHOD("has_edge_indices", "first", "second"), &Mesh4D::has_edge_indices);

	ClassDB::bind_method(D_METHOD("is_mesh_data_valid"), &Mesh4D::is_mesh_data_valid);
	ClassDB::bind_method(D_METHOD("reset_mesh_data_validation"), &Mesh4D::reset_mesh_data_validation);
	ClassDB::bind_method(D_METHOD("validate_material_for_mesh", "material"), &Mesh4D::validate_material_for_mesh);
	ClassDB::bind_method(D_METHOD("mark_proxy_mesh_3d_dirty"), &Mesh4D::mark_proxy_mesh_3d_dirty);
	ClassDB::bind_method(D_METHOD("mark_mesh_bounds_and_proxy_mesh_3d_dirty"), &Mesh4D::mark_mesh_bounds_and_proxy_mesh_3d_dirty);
	ClassDB::bind_method(D_METHOD("update_proxy_mesh_3d"), &Mesh4D::update_proxy_mesh_3d);

	ClassDB::bind_method(D_METHOD("to_array_wire_mesh"), &Mesh4D::to_array_wire_mesh);
	ClassDB::bind_method(D_METHOD("to_wire_mesh"), &Mesh4D::to_wire_mesh);
	ClassDB::bind_method(D_METHOD("get_proxy_mesh_3d"), &Mesh4D::get_proxy_mesh_3d);

	ClassDB::bind_method(D_METHOD("get_material"), &Mesh4D::get_material);
	ClassDB::bind_method(D_METHOD("set_material", "material"), &Mesh4D::set_material);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "material", PROPERTY_HINT_RESOURCE_TYPE, "Material4D"), "set_material", "get_material");

	ClassDB::bind_method(D_METHOD("get_edge_indices"), &Mesh4D::get_edge_indices);
	ClassDB::bind_method(D_METHOD("get_edge_positions"), &Mesh4D::get_edge_positions);
	ClassDB::bind_method(D_METHOD("get_vertex_positions"), &Mesh4D::get_vertex_positions);
	ClassDB::bind_method(D_METHOD("get_normal_values"), &Mesh4D::get_normal_values);
	ClassDB::bind_method(D_METHOD("get_texture_map_values"), &Mesh4D::get_texture_map_values);

	GDVIRTUAL_BIND(_get_edge_indices);
	GDVIRTUAL_BIND(_get_edge_positions);
	GDVIRTUAL_BIND(_get_vertex_positions);
	GDVIRTUAL_BIND(_get_normal_values);
	GDVIRTUAL_BIND(_get_texture_map_values);

	GDVIRTUAL_BIND(_get_fallback_material);
	GDVIRTUAL_BIND(_validate_material_for_mesh, "material");
	GDVIRTUAL_BIND(_validate_mesh_data);
	GDVIRTUAL_BIND(_update_proxy_mesh_3d);
}
