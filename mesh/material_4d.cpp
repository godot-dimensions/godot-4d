#include "material_4d.h"

Material4D::ColorSourceFlags Material4D::get_albedo_source_flags() const {
	return _albedo_source_flags;
}

void Material4D::set_albedo_source_flags(const ColorSourceFlags p_albedo_source_flags) {
	_albedo_source_flags = p_albedo_source_flags;
}

Color Material4D::get_albedo_color() const {
	return _albedo_color;
}

void Material4D::set_albedo_color(const Color &p_albedo_color) {
	_albedo_color = p_albedo_color;
}

PackedColorArray Material4D::get_albedo_color_array() const {
	return _albedo_color_array;
}

void Material4D::set_albedo_color_array(const PackedColorArray &p_albedo_color_array) {
	_albedo_color_array = p_albedo_color_array;
}

void Material4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_albedo_source_flags"), &Material4D::get_albedo_source_flags);
	ClassDB::bind_method(D_METHOD("set_albedo_source_flags", "albedo_source_flags"), &Material4D::set_albedo_source_flags);

	ClassDB::bind_method(D_METHOD("get_albedo_color"), &Material4D::get_albedo_color);
	ClassDB::bind_method(D_METHOD("set_albedo_color", "albedo_color"), &Material4D::set_albedo_color);

	ClassDB::bind_method(D_METHOD("get_albedo_color_array"), &Material4D::get_albedo_color_array);
	ClassDB::bind_method(D_METHOD("set_albedo_color_array", "albedo_color_array"), &Material4D::set_albedo_color_array);

	BIND_ENUM_CONSTANT(COLOR_SOURCE_FLAG_SINGLE_COLOR);
	BIND_ENUM_CONSTANT(COLOR_SOURCE_FLAG_PER_VERT);
	BIND_ENUM_CONSTANT(COLOR_SOURCE_FLAG_PER_EDGE);
	BIND_ENUM_CONSTANT(COLOR_SOURCE_FLAG_PER_FACE);
	BIND_ENUM_CONSTANT(COLOR_SOURCE_FLAG_PER_CELL);
	BIND_ENUM_CONSTANT(COLOR_SOURCE_FLAG_VERT_UVW);
	BIND_ENUM_CONSTANT(COLOR_SOURCE_FLAG_EDGE_UVW);
	BIND_ENUM_CONSTANT(COLOR_SOURCE_FLAG_FACE_UVW);
	BIND_ENUM_CONSTANT(COLOR_SOURCE_FLAG_CELL_UVW);
	BIND_ENUM_CONSTANT(COLOR_SOURCE_FLAG_TEXTURE4D);
}
