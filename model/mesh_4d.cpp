#include "mesh_4d.h"

#include "wire/array_wire_mesh_4d.h"

#if GDEXTENSION
#include <godot_cpp/templates/hash_set.hpp>
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

void Mesh4D::update_cross_section_mesh() {
	GDVIRTUAL_CALL(_update_cross_section_mesh);
}

void Mesh4D::validate_material_for_mesh(const Ref<Material4D> &p_material) {
	GDVIRTUAL_CALL(_validate_material_for_mesh, p_material);
	const Material4D::ColorSourceFlags albedo_source_flags = p_material->get_albedo_source_flags();
	if (albedo_source_flags & Material4D::COLOR_SOURCE_FLAG_USES_COLOR_ARRAY) {
		if (albedo_source_flags & Material4D::COLOR_SOURCE_FLAG_PER_VERT) {
			const PackedVector4Array vertices = get_vertices();
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
	wire_mesh->set_vertices(get_vertices());
	wire_mesh->set_edge_indices(get_edge_indices());
	wire_mesh->set_material(get_material());
	return wire_mesh;
}

Ref<WireMesh4D> Mesh4D::to_wire_mesh() {
	return to_array_wire_mesh();
}

Ref<ArrayMesh> Mesh4D::get_cross_section_mesh() {
	if (_is_cross_section_mesh_dirty || _cross_section_mesh.is_null()) {
		_cross_section_mesh.instantiate();
		update_cross_section_mesh();
		_is_cross_section_mesh_dirty = false;
	}
	return _cross_section_mesh;
}

Ref<Material4D> Mesh4D::get_material() const {
	return _material;
}

void Mesh4D::set_material(const Ref<Material4D> &p_material) {
	_material = p_material;
}

Ref<Material4D> Mesh4D::get_default_material() {
	Ref<Material4D> material;
	GDVIRTUAL_CALL(_get_default_material, material);
	return material;
}

PackedInt32Array Mesh4D::get_edge_indices() {
	PackedInt32Array indices;
	GDVIRTUAL_CALL(_get_edge_indices, indices);
	return indices;
}

PackedVector4Array Mesh4D::get_edge_positions() {
	PackedVector4Array edges;
	GDVIRTUAL_CALL(_get_edge_positions, edges);
	return edges;
}

PackedVector4Array Mesh4D::get_vertices() {
	PackedVector4Array vertices;
	GDVIRTUAL_CALL(_get_vertices, vertices);
	return vertices;
}

void Mesh4D::_bind_methods() {
	ClassDB::bind_static_method("Mesh4D", D_METHOD("deduplicate_edge_indices", "items"), &Mesh4D::deduplicate_edge_indices);
	ClassDB::bind_method(D_METHOD("has_edge_indices", "first", "second"), &Mesh4D::has_edge_indices);

	ClassDB::bind_method(D_METHOD("is_mesh_data_valid"), &Mesh4D::is_mesh_data_valid);
	ClassDB::bind_method(D_METHOD("reset_mesh_data_validation"), &Mesh4D::reset_mesh_data_validation);
	ClassDB::bind_method(D_METHOD("validate_material_for_mesh", "material"), &Mesh4D::validate_material_for_mesh);
	ClassDB::bind_method(D_METHOD("mark_cross_section_mesh_dirty"), &Mesh4D::mark_cross_section_mesh_dirty);
	ClassDB::bind_method(D_METHOD("update_cross_section_mesh"), &Mesh4D::update_cross_section_mesh);

	ClassDB::bind_method(D_METHOD("to_array_wire_mesh"), &Mesh4D::to_array_wire_mesh);
	ClassDB::bind_method(D_METHOD("to_wire_mesh"), &Mesh4D::to_wire_mesh);
	ClassDB::bind_method(D_METHOD("get_cross_section_mesh"), &Mesh4D::get_cross_section_mesh);

	ClassDB::bind_method(D_METHOD("get_material"), &Mesh4D::get_material);
	ClassDB::bind_method(D_METHOD("set_material", "material"), &Mesh4D::set_material);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "material", PROPERTY_HINT_RESOURCE_TYPE, "Material4D"), "set_material", "get_material");

	ClassDB::bind_method(D_METHOD("get_edge_indices"), &Mesh4D::get_edge_indices);
	ClassDB::bind_method(D_METHOD("get_edge_positions"), &Mesh4D::get_edge_positions);
	ClassDB::bind_method(D_METHOD("get_vertices"), &Mesh4D::get_vertices);

	GDVIRTUAL_BIND(_get_edge_indices);
	GDVIRTUAL_BIND(_get_edge_positions);
	GDVIRTUAL_BIND(_get_vertices);
	GDVIRTUAL_BIND(_get_default_material);
	GDVIRTUAL_BIND(_validate_material_for_mesh, "material");
	GDVIRTUAL_BIND(_validate_mesh_data);
	GDVIRTUAL_BIND(_update_cross_section_mesh);
}
