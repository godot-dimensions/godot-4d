#include "g4mf_material_channel_4d.h"

#include "../g4mf_state_4d.h"

bool G4MFMaterialChannel4D::is_equal_exact(const Ref<G4MFMaterialChannel4D> &p_other) const {
	if (_single_color != p_other->get_single_color()) {
		return false;
	}
	if (_cell_colors_accessor_index != p_other->get_cell_colors_accessor_index()) {
		return false;
	}
	if (_edge_colors_accessor_index != p_other->get_edge_colors_accessor_index()) {
		return false;
	}
	if (_vertex_colors_accessor_index != p_other->get_vertex_colors_accessor_index()) {
		return false;
	}
	if (_cell_texture_map_accessor_index != p_other->get_cell_texture_map_accessor_index()) {
		return false;
	}
	if (_cell_texture_index != p_other->get_cell_texture_index()) {
		return false;
	}
	return true;
}

PackedColorArray G4MFMaterialChannel4D::load_cell_colors(const Ref<G4MFState4D> &p_g4mf_state) const {
	TypedArray<G4MFAccessor4D> state_accessors = p_g4mf_state->get_accessors();
	ERR_FAIL_INDEX_V(_cell_colors_accessor_index, state_accessors.size(), PackedColorArray());
	const Ref<G4MFAccessor4D> accessor = state_accessors[_cell_colors_accessor_index];
	Array color_variants = accessor->decode_accessor_as_variants(p_g4mf_state, Variant::COLOR);
	const int color_variants_size = color_variants.size();
	PackedColorArray packed_colors;
	packed_colors.resize(color_variants_size);
	for (int i = 0; i < color_variants_size; ++i) {
		packed_colors.set(i, color_variants[i]);
	}
	return packed_colors;
}

PackedColorArray G4MFMaterialChannel4D::load_edge_colors(const Ref<G4MFState4D> &p_g4mf_state) const {
	TypedArray<G4MFAccessor4D> state_accessors = p_g4mf_state->get_accessors();
	ERR_FAIL_INDEX_V(_edge_colors_accessor_index, state_accessors.size(), PackedColorArray());
	const Ref<G4MFAccessor4D> accessor = state_accessors[_edge_colors_accessor_index];
	Array color_variants = accessor->decode_accessor_as_variants(p_g4mf_state, Variant::COLOR);
	const int color_variants_size = color_variants.size();
	PackedColorArray packed_colors;
	packed_colors.resize(color_variants_size);
	for (int i = 0; i < color_variants_size; ++i) {
		packed_colors.set(i, color_variants[i]);
	}
	return packed_colors;
}

PackedColorArray G4MFMaterialChannel4D::load_vertex_colors(const Ref<G4MFState4D> &p_g4mf_state) const {
	TypedArray<G4MFAccessor4D> state_accessors = p_g4mf_state->get_accessors();
	ERR_FAIL_INDEX_V(_vertex_colors_accessor_index, state_accessors.size(), PackedColorArray());
	const Ref<G4MFAccessor4D> accessor = state_accessors[_vertex_colors_accessor_index];
	Array color_variants = accessor->decode_accessor_as_variants(p_g4mf_state, Variant::COLOR);
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
	if (p_dict.has("color")) {
		Array color_array = p_dict["color"];
		Color color;
		switch (color_array.size()) {
			case 0: {
				color = Color(-1, -1, -1, -1);
			} break;
			case 1: {
				color = Color(color_array[0], color_array[0], color_array[0]);
			} break;
			case 2: {
				color = Color(color_array[0], color_array[1], 0.0, 1.0);
			} break;
			case 3: {
				color = Color(color_array[0], color_array[1], color_array[2], 1.0);
			} break;
			default: {
				color = Color(color_array[0], color_array[1], color_array[2], color_array[3]);
			} break;
		}
		material->set_single_color(color);
	}
	if (p_dict.has("cellColors")) {
		material->set_cell_colors_accessor_index(p_dict["cellColors"]);
	}
	if (p_dict.has("edgeColors")) {
		material->set_edge_colors_accessor_index(p_dict["edgeColors"]);
	}
	if (p_dict.has("vertexColors")) {
		material->set_vertex_colors_accessor_index(p_dict["vertexColors"]);
	}
	if (p_dict.has("cellTextureMap")) {
		material->set_cell_texture_map_accessor_index(p_dict["cellTextureMap"]);
	}
	if (p_dict.has("cellTexture")) {
		material->set_cell_texture_index(p_dict["cellTexture"]);
	}
	return material;
}

Dictionary G4MFMaterialChannel4D::to_dictionary() const {
	Dictionary dict = write_item_entries_to_dictionary();
	if (_single_color.r > -1 && !Color(1, 1, 1, 1).is_equal_approx(_single_color)) {
		Array color_array;
		if (_single_color.a == 1.0) {
			color_array.resize(3);
			color_array[0] = _single_color.r;
			color_array[1] = _single_color.g;
			color_array[2] = _single_color.b;
		} else {
			color_array.resize(4);
			color_array[0] = _single_color.r;
			color_array[1] = _single_color.g;
			color_array[2] = _single_color.b;
			color_array[3] = _single_color.a;
		}
		dict["color"] = color_array;
	}
	if (_cell_colors_accessor_index >= 0) {
		dict["cellColors"] = _cell_colors_accessor_index;
	}
	if (_edge_colors_accessor_index >= 0) {
		dict["edgeColors"] = _edge_colors_accessor_index;
	}
	if (_vertex_colors_accessor_index >= 0) {
		dict["vertexColors"] = _vertex_colors_accessor_index;
	}
	if (_cell_texture_map_accessor_index >= 0) {
		dict["cellTextureMap"] = _cell_texture_map_accessor_index;
	}
	if (_cell_texture_index >= 0) {
		dict["cellTexture"] = _cell_texture_index;
	}
	return dict;
}

void G4MFMaterialChannel4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_single_color"), &G4MFMaterialChannel4D::get_single_color);
	ClassDB::bind_method(D_METHOD("set_single_color", "single_color"), &G4MFMaterialChannel4D::set_single_color);
	ClassDB::bind_method(D_METHOD("get_cell_colors_accessor_index"), &G4MFMaterialChannel4D::get_cell_colors_accessor_index);
	ClassDB::bind_method(D_METHOD("set_cell_colors_accessor_index", "cell_colors_accessor_index"), &G4MFMaterialChannel4D::set_cell_colors_accessor_index);
	ClassDB::bind_method(D_METHOD("get_edge_colors_accessor_index"), &G4MFMaterialChannel4D::get_edge_colors_accessor_index);
	ClassDB::bind_method(D_METHOD("set_edge_colors_accessor_index", "edge_colors_accessor_index"), &G4MFMaterialChannel4D::set_edge_colors_accessor_index);
	ClassDB::bind_method(D_METHOD("get_vertex_colors_accessor_index"), &G4MFMaterialChannel4D::get_vertex_colors_accessor_index);
	ClassDB::bind_method(D_METHOD("set_vertex_colors_accessor_index", "vertex_colors_accessor_index"), &G4MFMaterialChannel4D::set_vertex_colors_accessor_index);
	ClassDB::bind_method(D_METHOD("get_cell_texture_map_accessor_index"), &G4MFMaterialChannel4D::get_cell_texture_map_accessor_index);
	ClassDB::bind_method(D_METHOD("set_cell_texture_map_accessor_index", "cell_texture_map_accessor_index"), &G4MFMaterialChannel4D::set_cell_texture_map_accessor_index);
	ClassDB::bind_method(D_METHOD("get_cell_texture_index"), &G4MFMaterialChannel4D::get_cell_texture_index);
	ClassDB::bind_method(D_METHOD("set_cell_texture_index", "cell_texture_index"), &G4MFMaterialChannel4D::set_cell_texture_index);

	ClassDB::bind_method(D_METHOD("is_equal_exact", "other"), &G4MFMaterialChannel4D::is_equal_exact);
	ClassDB::bind_method(D_METHOD("load_cell_colors", "g4mf_state"), &G4MFMaterialChannel4D::load_cell_colors);
	ClassDB::bind_method(D_METHOD("load_edge_colors", "g4mf_state"), &G4MFMaterialChannel4D::load_edge_colors);
	ClassDB::bind_method(D_METHOD("load_vertex_colors", "g4mf_state"), &G4MFMaterialChannel4D::load_vertex_colors);

	ClassDB::bind_static_method("G4MFMaterialChannel4D", D_METHOD("from_dictionary", "dict"), &G4MFMaterialChannel4D::from_dictionary);
	ClassDB::bind_method(D_METHOD("to_dictionary"), &G4MFMaterialChannel4D::to_dictionary);

	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "single_color", PROPERTY_HINT_COLOR_NO_ALPHA), "set_single_color", "get_single_color");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "cell_colors_accessor_index"), "set_cell_colors_accessor_index", "get_cell_colors_accessor_index");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "edge_colors_accessor_index"), "set_edge_colors_accessor_index", "get_edge_colors_accessor_index");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "vertex_colors_accessor_index"), "set_vertex_colors_accessor_index", "get_vertex_colors_accessor_index");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "cell_texture_map_accessor_index"), "set_cell_texture_map_accessor_index", "get_cell_texture_map_accessor_index");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "cell_texture_index"), "set_cell_texture_index", "get_cell_texture_index");
}
