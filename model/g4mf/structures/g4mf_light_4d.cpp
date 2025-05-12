#include "g4mf_light_4d.h"

Ref<G4MFLight4D> G4MFLight4D::from_dictionary(const Dictionary &p_dict) {
	Ref<G4MFLight4D> light;
	light.instantiate();
	light->read_item_entries_from_dictionary(p_dict);
	if (p_dict.has("type")) {
		light->set_light_type(p_dict["type"]);
	}
	if (p_dict.has("color")) {
		light->set_color(json_array_to_color(p_dict["color"]));
	}
	if (p_dict.has("coneInnerAngle")) {
		light->set_cone_inner_angle(p_dict["coneInnerAngle"]);
	}
	if (p_dict.has("coneOuterAngle")) {
		light->set_cone_outer_angle(p_dict["coneOuterAngle"]);
	}
	if (p_dict.has("intensity")) {
		light->set_intensity(p_dict["intensity"]);
	}
	if (p_dict.has("range")) {
		light->set_range(p_dict["range"]);
	}
	return light;
}

Dictionary G4MFLight4D::to_dictionary() const {
	Dictionary dict = write_item_entries_to_dictionary();
	if (_color != Color(1.0f, 1.0f, 1.0f)) {
		dict["color"] = color_to_json_array(_color, false);
	}
	if (_cone_inner_angle != 0.0) {
		dict["coneInnerAngle"] = _cone_inner_angle;
	}
	if (_cone_outer_angle != 0.7853981633974483) {
		dict["coneOuterAngle"] = _cone_outer_angle;
	}
	dict["intensity"] = _intensity;
	if (_range != INFINITY) {
		dict["range"] = _range;
	}
	dict["type"] = _light_type;
	return dict;
}

void G4MFLight4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_light_type"), &G4MFLight4D::get_light_type);
	ClassDB::bind_method(D_METHOD("set_light_type", "type"), &G4MFLight4D::set_light_type);
	ClassDB::bind_method(D_METHOD("get_color"), &G4MFLight4D::get_color);
	ClassDB::bind_method(D_METHOD("set_color", "color"), &G4MFLight4D::set_color);
	ClassDB::bind_method(D_METHOD("get_cone_inner_angle"), &G4MFLight4D::get_cone_inner_angle);
	ClassDB::bind_method(D_METHOD("set_cone_inner_angle", "angle"), &G4MFLight4D::set_cone_inner_angle);
	ClassDB::bind_method(D_METHOD("get_cone_outer_angle"), &G4MFLight4D::get_cone_outer_angle);
	ClassDB::bind_method(D_METHOD("set_cone_outer_angle", "angle"), &G4MFLight4D::set_cone_outer_angle);
	ClassDB::bind_method(D_METHOD("get_intensity"), &G4MFLight4D::get_intensity);
	ClassDB::bind_method(D_METHOD("set_intensity", "intensity"), &G4MFLight4D::set_intensity);
	ClassDB::bind_method(D_METHOD("get_range"), &G4MFLight4D::get_range);
	ClassDB::bind_method(D_METHOD("set_range", "range"), &G4MFLight4D::set_range);

	ClassDB::bind_static_method("G4MFLight4D", D_METHOD("from_dictionary", "dict"), &G4MFLight4D::from_dictionary);
	ClassDB::bind_method(D_METHOD("to_dictionary"), &G4MFLight4D::to_dictionary);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "light_type"), "set_light_type", "get_light_type");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color"), "set_color", "get_color");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "cone_inner_angle"), "set_cone_inner_angle", "get_cone_inner_angle");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "cone_outer_angle"), "set_cone_outer_angle", "get_cone_outer_angle");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "intensity"), "set_intensity", "get_intensity");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "range"), "set_range", "get_range");
}
