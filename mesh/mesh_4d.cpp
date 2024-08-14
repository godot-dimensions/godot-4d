#include "mesh_4d.h"

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

Ref<Material4D> Mesh4D::get_material() const {
	return _material;
}

void Mesh4D::set_material(const Ref<Material4D> &p_material) {
	_material = p_material;
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

	ClassDB::bind_method(D_METHOD("get_material"), &Mesh4D::get_material);
	ClassDB::bind_method(D_METHOD("set_material", "material"), &Mesh4D::set_material);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "material", PROPERTY_HINT_RESOURCE_TYPE, "Material4D"), "set_material", "get_material");

	ClassDB::bind_method(D_METHOD("get_edge_indices"), &Mesh4D::get_edge_indices);
	ClassDB::bind_method(D_METHOD("get_edge_positions"), &Mesh4D::get_edge_positions);
	ClassDB::bind_method(D_METHOD("get_vertices"), &Mesh4D::get_vertices);

	GDVIRTUAL_BIND(_get_edge_indices);
	GDVIRTUAL_BIND(_get_edge_positions);
	GDVIRTUAL_BIND(_get_vertices);
}
