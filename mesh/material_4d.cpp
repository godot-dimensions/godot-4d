#include "material_4d.h"

void Material4D::merge_with(const Ref<Material4D> &p_material, const int p_first_item_count, const int p_second_item_count) {
	const Color start_albedo_color = _albedo_color;
	const ColorSourceFlags start_albedo_source_flags = _albedo_source_flags;
	_albedo_source_flags = ColorSourceFlags(_albedo_source_flags | p_material->_albedo_source_flags);
	// Check if a single color is used, and if it's the same and we can keep it, or if we need to get rid of it.
	if (_albedo_source_flags & COLOR_SOURCE_FLAG_SINGLE_COLOR) {
		if (_albedo_color != p_material->_albedo_color) {
			// They don't use the same color, so we need to switch to using a color array.
			_albedo_source_flags = ColorSourceFlags(_albedo_source_flags & ~COLOR_SOURCE_FLAG_SINGLE_COLOR);
			if (!(_albedo_source_flags & COLOR_SOURCE_FLAG_USES_COLOR_ARRAY)) {
				_albedo_source_flags = ColorSourceFlags(_albedo_source_flags | COLOR_SOURCE_FLAG_USES_COLOR_ARRAY);
			}
		}
	}
	if (_albedo_source_flags & COLOR_SOURCE_FLAG_USES_COLOR_ARRAY) {
		const int albedo_color_array_end_size = p_first_item_count + p_second_item_count;
		_albedo_color_array.resize(albedo_color_array_end_size);
		// Convert existing color array data from this material if needed.
		if (!(start_albedo_source_flags & COLOR_SOURCE_FLAG_USES_COLOR_ARRAY)) {
			// Start does not use a color array, so we need to insert white. This may get multiplied by the start single color next.
			for (int i = 0; i < p_first_item_count; i++) {
				_albedo_color_array.write[i] = Color(1, 1, 1, 1);
			}
		}
		if (!(_albedo_source_flags & COLOR_SOURCE_FLAG_SINGLE_COLOR) && (start_albedo_source_flags & COLOR_SOURCE_FLAG_SINGLE_COLOR)) {
			// Start uses a single color, but destination does not, so we need to "bake" the start color into the array.
			for (int i = 0; i < p_first_item_count; i++) {
				_albedo_color_array.write[i] *= start_albedo_color;
			}
		}
		// Merge the other material's color array data.
		if (p_material->_albedo_source_flags & COLOR_SOURCE_FLAG_USES_COLOR_ARRAY) {
			// Other has a color array, so copy it over.
			for (int i = 0; i < p_second_item_count; i++) {
				_albedo_color_array.write[p_first_item_count + i] = p_material->_albedo_color_array[i];
			}
		} else {
			// Other does not use a color array, so we need to insert white. This may get multiplied by the other single color next.
			for (int i = p_first_item_count; i < albedo_color_array_end_size; i++) {
				_albedo_color_array.write[i] = Color(1, 1, 1, 1);
			}
		}
		if (!(_albedo_source_flags & COLOR_SOURCE_FLAG_SINGLE_COLOR) && (p_material->_albedo_source_flags & COLOR_SOURCE_FLAG_SINGLE_COLOR)) {
			// Other uses a single color, but destination does not, so we need to "bake" the other color into the array.
			for (int i = p_first_item_count; i < albedo_color_array_end_size; i++) {
				_albedo_color_array.write[i] *= p_material->_albedo_color;
			}
		}
	}
}

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
	ClassDB::bind_method(D_METHOD("merge_with", "material", "first_item_count", "second_item_count"), &Material4D::merge_with);

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
