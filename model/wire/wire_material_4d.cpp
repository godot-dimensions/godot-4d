#include "wire_material_4d.h"

Material4D::ColorSourceFlags WireMaterial4D::_wire_source_to_flags(const WireColorSource p_wire_source) {
	switch (p_wire_source) {
		case WireMaterial4D::WIRE_COLOR_SOURCE_SINGLE_COLOR:
			return Material4D::COLOR_SOURCE_FLAG_SINGLE_COLOR;
		case WireMaterial4D::WIRE_COLOR_SOURCE_PER_EDGE_ONLY:
			return Material4D::COLOR_SOURCE_FLAG_PER_EDGE;
		case WireMaterial4D::WIRE_COLOR_SOURCE_PER_EDGE_AND_SINGLE:
			return Material4D::ColorSourceFlags(Material4D::COLOR_SOURCE_FLAG_PER_EDGE | Material4D::COLOR_SOURCE_FLAG_SINGLE_COLOR);
	}
	return Material4D::COLOR_SOURCE_FLAG_NONE;
}

void WireMaterial4D::merge_with(const Ref<Material4D> &p_material, const int p_first_item_count, const int p_second_item_count) {
	Material4D::merge_with(p_material, p_first_item_count, p_second_item_count);
	// Read _albedo_source_flags and set the wire material's albedo source enum.
	if (_albedo_source_flags & Material4D::COLOR_SOURCE_FLAG_SINGLE_COLOR) {
		if (_albedo_source_flags & Material4D::COLOR_SOURCE_FLAG_PER_EDGE) {
			set_albedo_source(WIRE_COLOR_SOURCE_PER_EDGE_AND_SINGLE);
		} else {
			set_albedo_source(WIRE_COLOR_SOURCE_SINGLE_COLOR);
		}
	} else if (_albedo_source_flags & Material4D::COLOR_SOURCE_FLAG_PER_EDGE) {
		set_albedo_source(WIRE_COLOR_SOURCE_PER_EDGE_ONLY);
	}
}

WireMaterial4D::WireColorSource WireMaterial4D::get_albedo_source() const {
	return _albedo_source;
}

void WireMaterial4D::set_albedo_source(const WireColorSource p_albedo_source) {
	_albedo_source = p_albedo_source;
	_albedo_source_flags = _wire_source_to_flags(_albedo_source);
	notify_property_list_changed();
}

real_t WireMaterial4D::get_line_thickness() const {
	return _line_thickness;
}

void WireMaterial4D::set_line_thickness(const real_t p_line_thickness) {
	_line_thickness = p_line_thickness;
}

void WireMaterial4D::_get_property_list(List<PropertyInfo> *p_list) const {
	for (List<PropertyInfo>::Element *E = p_list->front(); E; E = E->next()) {
		PropertyInfo &prop = E->get();
		if (prop.name == StringName("albedo_color")) {
			prop.usage = (_albedo_source_flags & COLOR_SOURCE_FLAG_SINGLE_COLOR) ? PROPERTY_USAGE_DEFAULT : PROPERTY_USAGE_NONE;
		} else if (prop.name == StringName("albedo_color_array")) {
			prop.usage = (_albedo_source_flags & COLOR_SOURCE_FLAG_USES_COLOR_ARRAY) ? PROPERTY_USAGE_DEFAULT : PROPERTY_USAGE_NONE;
		}
	}
	Material4D::_get_property_list(p_list);
}

WireMaterial4D::WireMaterial4D() {
	set_albedo_source(WIRE_COLOR_SOURCE_SINGLE_COLOR);
}

void WireMaterial4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_line_thickness"), &WireMaterial4D::get_line_thickness);
	ClassDB::bind_method(D_METHOD("set_line_thickness", "line_thickness"), &WireMaterial4D::set_line_thickness);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "line_thickness"), "set_line_thickness", "get_line_thickness");

	ClassDB::bind_method(D_METHOD("get_albedo_source"), &WireMaterial4D::get_albedo_source);
	ClassDB::bind_method(D_METHOD("set_albedo_source", "albedo_source"), &WireMaterial4D::set_albedo_source);

	//ADD_GROUP("Albedo", "albedo_");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "albedo_source", PROPERTY_HINT_ENUM, "Single Color,Per Edge Only,Per Edge and Single Color"), "set_albedo_source", "get_albedo_source");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "albedo_color"), "set_albedo_color", "get_albedo_color");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_COLOR_ARRAY, "albedo_color_array"), "set_albedo_color_array", "get_albedo_color_array");

	BIND_ENUM_CONSTANT(WIRE_COLOR_SOURCE_SINGLE_COLOR);
	BIND_ENUM_CONSTANT(WIRE_COLOR_SOURCE_PER_EDGE_ONLY);
	BIND_ENUM_CONSTANT(WIRE_COLOR_SOURCE_PER_EDGE_AND_SINGLE);
}
