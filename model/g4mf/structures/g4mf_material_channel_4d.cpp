#include "g4mf_material_channel_4d.h"

#include "../g4mf_state_4d.h"

bool G4MFMaterialChannel4D::is_equal_exact(const Ref<G4MFMaterialChannel4D> &p_other) const {
	if (_factor != p_other->get_factor()) {
		return false;
	}
	if (_texture_index != p_other->get_texture_index()) {
		return false;
	}
	if (_element_map_binding != p_other->get_element_map_binding()) {
		return false;
	}
	if (_texture_map_binding != p_other->get_texture_map_binding()) {
		return false;
	}
	return true;
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
	if (p_dict.has("elementMap")) {
		Ref<G4MFMeshSurfaceBinding4D> binding = G4MFMeshSurfaceBinding4D::from_dictionary(p_dict["elementMap"]);
		material->set_element_map_binding(binding);
	}
	if (p_dict.has("texture")) {
		material->set_texture_index(p_dict["texture"]);
	}
	if (p_dict.has("textureMap")) {
		Ref<G4MFMeshSurfaceBinding4D> texture_map_binding = G4MFMeshSurfaceBinding4D::from_dictionary(p_dict["textureMap"]);
		material->set_texture_map_binding(texture_map_binding);
	}
	return material;
}

Dictionary G4MFMaterialChannel4D::to_dictionary() const {
	Dictionary dict = write_item_entries_to_dictionary();
	if (_element_map_binding.is_valid()) {
		dict["elementMap"] = _element_map_binding->to_dictionary();
	}
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
	if (_texture_index >= 0) {
		dict["texture"] = _texture_index;
	}
	if (_texture_map_binding.is_valid()) {
		dict["textureMap"] = _texture_map_binding->to_dictionary();
	}
	return dict;
}

void G4MFMaterialChannel4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_factor"), &G4MFMaterialChannel4D::get_factor);
	ClassDB::bind_method(D_METHOD("set_factor", "single_color"), &G4MFMaterialChannel4D::set_factor);
	ClassDB::bind_method(D_METHOD("get_element_map_binding"), &G4MFMaterialChannel4D::get_element_map_binding);
	ClassDB::bind_method(D_METHOD("set_element_map_binding", "element_map_binding"), &G4MFMaterialChannel4D::set_element_map_binding);
	ClassDB::bind_method(D_METHOD("get_texture_index"), &G4MFMaterialChannel4D::get_texture_index);
	ClassDB::bind_method(D_METHOD("set_texture_index", "texture_index"), &G4MFMaterialChannel4D::set_texture_index);
	ClassDB::bind_method(D_METHOD("get_texture_map_binding"), &G4MFMaterialChannel4D::get_texture_map_binding);
	ClassDB::bind_method(D_METHOD("set_texture_map_binding", "texture_map_binding"), &G4MFMaterialChannel4D::set_texture_map_binding);

	ClassDB::bind_method(D_METHOD("is_equal_exact", "other"), &G4MFMaterialChannel4D::is_equal_exact);

	ClassDB::bind_static_method("G4MFMaterialChannel4D", D_METHOD("from_dictionary", "dict"), &G4MFMaterialChannel4D::from_dictionary);
	ClassDB::bind_method(D_METHOD("to_dictionary"), &G4MFMaterialChannel4D::to_dictionary);

	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "factor", PROPERTY_HINT_COLOR_NO_ALPHA), "set_factor", "get_factor");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "element_map_binding", PROPERTY_HINT_RESOURCE_TYPE, "G4MFMeshSurfaceBinding4D"), "set_element_map_binding", "get_element_map_binding");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "texture_index"), "set_texture_index", "get_texture_index");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture_map_binding", PROPERTY_HINT_RESOURCE_TYPE, "G4MFMeshSurfaceBinding4D"), "set_texture_map_binding", "get_texture_map_binding");
}
