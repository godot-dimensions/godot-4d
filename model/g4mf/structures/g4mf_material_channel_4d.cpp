#include "g4mf_material_channel_4d.h"

#include "../g4mf_state_4d.h"

bool G4MFMaterialChannel4D::is_equal_exact(const Ref<G4MFMaterialChannel4D> &p_other) const {
	if (_factor != p_other->get_factor()) {
		return false;
	}
	if (_per_simplex_accessor_index != p_other->get_per_simplex_accessor_index()) {
		return false;
	}
	if (_per_edge_accessor_index != p_other->get_per_edge_accessor_index()) {
		return false;
	}
	if (_per_vertex_accessor_index != p_other->get_per_vertex_accessor_index()) {
		return false;
	}
	if (_texture_map_accessor_index != p_other->get_texture_map_accessor_index()) {
		return false;
	}
	if (_texture_index != p_other->get_texture_index()) {
		return false;
	}
	return true;
}

PackedColorArray G4MFMaterialChannel4D::load_simplex_colors(const Ref<G4MFState4D> &p_g4mf_state) const {
	TypedArray<G4MFAccessor4D> state_accessors = p_g4mf_state->get_g4mf_accessors();
	ERR_FAIL_INDEX_V(_per_simplex_accessor_index, state_accessors.size(), PackedColorArray());
	const Ref<G4MFAccessor4D> accessor = state_accessors[_per_simplex_accessor_index];
	Array color_variants = accessor->decode_variants_from_bytes(p_g4mf_state, Variant::COLOR);
	const int color_variants_size = color_variants.size();
	PackedColorArray packed_colors;
	packed_colors.resize(color_variants_size);
	for (int i = 0; i < color_variants_size; ++i) {
		packed_colors.set(i, color_variants[i]);
	}
	return packed_colors;
}

PackedColorArray G4MFMaterialChannel4D::load_edge_colors(const Ref<G4MFState4D> &p_g4mf_state) const {
	TypedArray<G4MFAccessor4D> state_accessors = p_g4mf_state->get_g4mf_accessors();
	ERR_FAIL_INDEX_V(_per_edge_accessor_index, state_accessors.size(), PackedColorArray());
	const Ref<G4MFAccessor4D> accessor = state_accessors[_per_edge_accessor_index];
	Array color_variants = accessor->decode_variants_from_bytes(p_g4mf_state, Variant::COLOR);
	const int color_variants_size = color_variants.size();
	PackedColorArray packed_colors;
	packed_colors.resize(color_variants_size);
	for (int i = 0; i < color_variants_size; ++i) {
		packed_colors.set(i, color_variants[i]);
	}
	return packed_colors;
}

PackedColorArray G4MFMaterialChannel4D::load_vertex_colors(const Ref<G4MFState4D> &p_g4mf_state) const {
	TypedArray<G4MFAccessor4D> state_accessors = p_g4mf_state->get_g4mf_accessors();
	ERR_FAIL_INDEX_V(_per_vertex_accessor_index, state_accessors.size(), PackedColorArray());
	const Ref<G4MFAccessor4D> accessor = state_accessors[_per_vertex_accessor_index];
	Array color_variants = accessor->decode_variants_from_bytes(p_g4mf_state, Variant::COLOR);
	const int color_variants_size = color_variants.size();
	PackedColorArray packed_colors;
	packed_colors.resize(color_variants_size);
	for (int i = 0; i < color_variants_size; ++i) {
		packed_colors.set(i, color_variants[i]);
	}
	return packed_colors;
}

Ref<G4MFMaterialChannel4D> G4MFMaterialChannel4D::from_dictionary(const Dictionary &p_dict) {
	Ref<G4MFMaterialChannel4D> material;
	material.instantiate();
	material->read_item_entries_from_dictionary(p_dict);
	if (p_dict.has("factor")) {
		Array factor_array = p_dict["factor"];
		Color factor_color;
		switch (factor_array.size()) {
			case 0: {
				factor_color = Color(-1, -1, -1, -1);
			} break;
			case 1: {
				factor_color = Color(factor_array[0], factor_array[0], factor_array[0]);
			} break;
			case 2: {
				factor_color = Color(factor_array[0], factor_array[1], 0.0, 1.0);
			} break;
			case 3: {
				factor_color = Color(factor_array[0], factor_array[1], factor_array[2], 1.0);
			} break;
			default: {
				factor_color = Color(factor_array[0], factor_array[1], factor_array[2], factor_array[3]);
			} break;
		}
		material->set_factor(factor_color);
	}
	if (p_dict.has("perSimplex")) {
		material->set_per_simplex_accessor_index(p_dict["perSimplex"]);
	}
	if (p_dict.has("perEdge")) {
		material->set_edge_colors_accessor_index(p_dict["perEdge"]);
	}
	if (p_dict.has("perVertex")) {
		material->set_per_vertex_accessor_index(p_dict["perVertex"]);
	}
	if (p_dict.has("texture")) {
		material->set_texture_index(p_dict["texture"]);
	}
	if (p_dict.has("textureMap")) {
		material->set_texture_map_accessor_index(p_dict["textureMap"]);
	}
	if (p_dict.has("topologyTextureMap")) {
		material->set_topology_texture_map_accessor_index(p_dict["topologyTextureMap"]);
	}
	return material;
}

Dictionary G4MFMaterialChannel4D::to_dictionary() const {
	Dictionary dict = write_item_entries_to_dictionary();
	if (_factor.r > -1 && !Color(1, 1, 1, 1).is_equal_approx(_factor)) {
		Array factor_array;
		if (_factor.a == 1.0) {
			factor_array.resize(3);
			factor_array[0] = _factor.r;
			factor_array[1] = _factor.g;
			factor_array[2] = _factor.b;
		} else {
			factor_array.resize(4);
			factor_array[0] = _factor.r;
			factor_array[1] = _factor.g;
			factor_array[2] = _factor.b;
			factor_array[3] = _factor.a;
		}
		dict["factor"] = factor_array;
	}
	if (_per_simplex_accessor_index >= 0) {
		dict["perSimplex"] = _per_simplex_accessor_index;
	}
	if (_per_edge_accessor_index >= 0) {
		dict["perEdge"] = _per_edge_accessor_index;
	}
	if (_per_vertex_accessor_index >= 0) {
		dict["perVertex"] = _per_vertex_accessor_index;
	}
	if (_texture_index >= 0) {
		dict["texture"] = _texture_index;
	}
	if (_texture_map_accessor_index >= 0) {
		dict["textureMap"] = _texture_map_accessor_index;
	}
	if (_topology_texture_map_accessor_index >= 0) {
		dict["topologyTextureMap"] = _topology_texture_map_accessor_index;
	}
	return dict;
}

void G4MFMaterialChannel4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_factor"), &G4MFMaterialChannel4D::get_factor);
	ClassDB::bind_method(D_METHOD("set_factor", "single_color"), &G4MFMaterialChannel4D::set_factor);
	ClassDB::bind_method(D_METHOD("get_per_simplex_accessor_index"), &G4MFMaterialChannel4D::get_per_simplex_accessor_index);
	ClassDB::bind_method(D_METHOD("set_per_simplex_accessor_index", "per_simplex_colors_accessor_index"), &G4MFMaterialChannel4D::set_per_simplex_accessor_index);
	ClassDB::bind_method(D_METHOD("get_per_edge_accessor_index"), &G4MFMaterialChannel4D::get_per_edge_accessor_index);
	ClassDB::bind_method(D_METHOD("set_edge_colors_accessor_index", "per_edge_colors_accessor_index"), &G4MFMaterialChannel4D::set_edge_colors_accessor_index);
	ClassDB::bind_method(D_METHOD("get_per_vertex_accessor_index"), &G4MFMaterialChannel4D::get_per_vertex_accessor_index);
	ClassDB::bind_method(D_METHOD("set_per_vertex_accessor_index", "vertex_colors_accessor_index"), &G4MFMaterialChannel4D::set_per_vertex_accessor_index);
	ClassDB::bind_method(D_METHOD("get_texture_index"), &G4MFMaterialChannel4D::get_texture_index);
	ClassDB::bind_method(D_METHOD("set_texture_index", "texture_index"), &G4MFMaterialChannel4D::set_texture_index);
	ClassDB::bind_method(D_METHOD("get_texture_map_accessor_index"), &G4MFMaterialChannel4D::get_texture_map_accessor_index);
	ClassDB::bind_method(D_METHOD("set_texture_map_accessor_index", "texture_map_accessor_index"), &G4MFMaterialChannel4D::set_texture_map_accessor_index);
	ClassDB::bind_method(D_METHOD("get_topology_texture_map_accessor_index"), &G4MFMaterialChannel4D::get_topology_texture_map_accessor_index);
	ClassDB::bind_method(D_METHOD("set_topology_texture_map_accessor_index", "topology_texture_map_accessor_index"), &G4MFMaterialChannel4D::set_topology_texture_map_accessor_index);

	ClassDB::bind_method(D_METHOD("is_equal_exact", "other"), &G4MFMaterialChannel4D::is_equal_exact);
	ClassDB::bind_method(D_METHOD("load_simplex_colors", "g4mf_state"), &G4MFMaterialChannel4D::load_simplex_colors);
	ClassDB::bind_method(D_METHOD("load_edge_colors", "g4mf_state"), &G4MFMaterialChannel4D::load_edge_colors);
	ClassDB::bind_method(D_METHOD("load_vertex_colors", "g4mf_state"), &G4MFMaterialChannel4D::load_vertex_colors);

	ClassDB::bind_static_method("G4MFMaterialChannel4D", D_METHOD("from_dictionary", "dict"), &G4MFMaterialChannel4D::from_dictionary);
	ClassDB::bind_method(D_METHOD("to_dictionary"), &G4MFMaterialChannel4D::to_dictionary);

	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "factor", PROPERTY_HINT_COLOR_NO_ALPHA), "set_factor", "get_factor");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "per_simplex_accessor_index"), "set_per_simplex_accessor_index", "get_per_simplex_accessor_index");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "per_edge_accessor_index"), "set_edge_colors_accessor_index", "get_per_edge_accessor_index");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "per_vertex_accessor_index"), "set_per_vertex_accessor_index", "get_per_vertex_accessor_index");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "texture_index"), "set_texture_index", "get_texture_index");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "texture_map_accessor_index"), "set_texture_map_accessor_index", "get_texture_map_accessor_index");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "topology_texture_map_accessor_index"), "set_topology_texture_map_accessor_index", "get_topology_texture_map_accessor_index");
}
