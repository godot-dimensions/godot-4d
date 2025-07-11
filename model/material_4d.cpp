#include "material_4d.h"

#include "mesh_4d.h"

Color Material4D::get_albedo_color_of_edge(const int64_t p_edge_index, const Ref<Mesh4D> &p_for_mesh) {
	if (!(_albedo_source_flags & COLOR_SOURCE_FLAG_USES_COLOR_ARRAY)) {
		// No need to allocate any memory for _edge_albedo_color_cache if the color array is not used.
		if (_albedo_source_flags & COLOR_SOURCE_FLAG_SINGLE_COLOR) {
			return _albedo_color;
		}
		return Color(1, 1, 1, 1);
	}
	if (p_edge_index < _edge_albedo_color_cache.size()) {
		return _edge_albedo_color_cache[p_edge_index];
	}
	ERR_FAIL_COND_V_MSG(p_for_mesh.is_null(), _albedo_color, "Material4D: Mesh is null.");
	const PackedInt32Array edge_indices = p_for_mesh->get_edge_indices();
	const int64_t edge_count = edge_indices.size() / 2;
	ERR_FAIL_COND_V_MSG(p_edge_index >= edge_count, _albedo_color, "Material4D: Edge index out of bounds for mesh.");
	if (_albedo_source_flags & COLOR_SOURCE_FLAG_PER_EDGE) {
		ERR_FAIL_COND_V_MSG(edge_count > _albedo_color_array.size(), _albedo_color, "Material4D: Mesh has more edges than the material's color array.");
		if (_albedo_source_flags & COLOR_SOURCE_FLAG_SINGLE_COLOR) {
			_edge_albedo_color_cache.resize(edge_count);
			for (int64_t i = 0; i < edge_count; i++) {
				_edge_albedo_color_cache.set(i, _albedo_color_array[i] * _albedo_color);
			}
		} else {
			_edge_albedo_color_cache = _albedo_color_array.duplicate();
		}
	} else if (_albedo_source_flags & COLOR_SOURCE_FLAG_PER_VERT) {
		_edge_albedo_color_cache.resize(edge_count);
		for (int64_t i = 0; i < edge_count; i++) {
			const int32_t first_vertex = edge_indices[i * 2];
			const int32_t second_vertex = edge_indices[i * 2 + 1];
			ERR_FAIL_INDEX_V_MSG(first_vertex, _albedo_color_array.size(), _albedo_color, "Material4D: Cannot get vertex color of mesh because the first vertex index is out of bounds of the material's color array.");
			ERR_FAIL_INDEX_V_MSG(second_vertex, _albedo_color_array.size(), _albedo_color, "Material4D: Cannot get vertex color of mesh because the second vertex index is out of bounds of the material's color array.");
			const Color per_edge_color = (_albedo_color_array[first_vertex] + _albedo_color_array[second_vertex]) * 0.5f;
			if (_albedo_source_flags & COLOR_SOURCE_FLAG_SINGLE_COLOR) {
				_edge_albedo_color_cache.set(i, per_edge_color * _albedo_color);
			} else {
				_edge_albedo_color_cache.set(i, per_edge_color);
			}
		}
	} else if (_albedo_source_flags & COLOR_SOURCE_FLAG_SINGLE_COLOR) {
		_edge_albedo_color_cache.resize(edge_count);
		_edge_albedo_color_cache.fill(_albedo_color);
	} else {
		_edge_albedo_color_cache.resize(edge_count);
		_edge_albedo_color_cache.fill(Color(1, 1, 1, 1));
	}
	return _albedo_color;
}

bool Material4D::is_default_material() const {
	return _albedo_source_flags == COLOR_SOURCE_FLAG_SINGLE_COLOR && _albedo_color == Color(1, 1, 1, 1);
}

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
				_albedo_color_array.set(i, Color(1, 1, 1, 1));
			}
		}
		if (!(_albedo_source_flags & COLOR_SOURCE_FLAG_SINGLE_COLOR) && (start_albedo_source_flags & COLOR_SOURCE_FLAG_SINGLE_COLOR)) {
			// Start uses a single color, but destination does not, so we need to "bake" the start color into the array.
			for (int i = 0; i < p_first_item_count; i++) {
				_albedo_color_array.set(i, _albedo_color_array[i] * start_albedo_color);
			}
		}
		// Merge the other material's color array data.
		if (p_material->_albedo_source_flags & COLOR_SOURCE_FLAG_USES_COLOR_ARRAY) {
			// Other has a color array, so copy it over.
			for (int i = 0; i < p_second_item_count; i++) {
				_albedo_color_array.set(p_first_item_count + i, p_material->_albedo_color_array[i]);
			}
		} else {
			// Other does not use a color array, so we need to insert white. This may get multiplied by the other single color next.
			for (int i = p_first_item_count; i < albedo_color_array_end_size; i++) {
				_albedo_color_array.set(i, Color(1, 1, 1, 1));
			}
		}
		if (!(_albedo_source_flags & COLOR_SOURCE_FLAG_SINGLE_COLOR) && (p_material->_albedo_source_flags & COLOR_SOURCE_FLAG_SINGLE_COLOR)) {
			// Other uses a single color, but destination does not, so we need to "bake" the other color into the array.
			for (int i = p_first_item_count; i < albedo_color_array_end_size; i++) {
				_albedo_color_array.set(i, _albedo_color_array[i] * p_material->_albedo_color);
			}
		}
	}
}

Material4D::ColorSourceFlags Material4D::get_albedo_source_flags() const {
	return _albedo_source_flags;
}

void Material4D::set_albedo_source_flags(const ColorSourceFlags p_albedo_source_flags) {
	_albedo_source_flags = p_albedo_source_flags;
	_edge_albedo_color_cache.clear();
	update_cross_section_material();
}

Color Material4D::get_albedo_color() const {
	return _albedo_color;
}

void Material4D::set_albedo_color(const Color &p_albedo_color) {
	_albedo_color = p_albedo_color;
	_edge_albedo_color_cache.clear();
	update_cross_section_material();
}

PackedColorArray Material4D::get_albedo_color_array() const {
	return _albedo_color_array;
}

void Material4D::set_albedo_color_array(const PackedColorArray &p_albedo_color_array) {
	_albedo_color_array = p_albedo_color_array;
	_edge_albedo_color_cache.clear();
	update_cross_section_material();
}

void Material4D::append_albedo_color(const Color &p_albedo_color) {
	_albedo_color_array.push_back(p_albedo_color);
	_edge_albedo_color_cache.clear();
	update_cross_section_material();
}

void Material4D::resize_albedo_color_array(const int64_t p_size, const Color &p_fill_color) {
	const int64_t existing_size = _albedo_color_array.size();
	_albedo_color_array.resize(p_size);
	for (int64_t i = existing_size; i < p_size; i++) {
		_albedo_color_array.set(i, p_fill_color);
	}
	_edge_albedo_color_cache.clear();
	update_cross_section_material();
}

Ref<ShaderMaterial> Material4D::get_cross_section_material() {
	if (_cross_section_material.is_null()) {
		_cross_section_material.instantiate();
		update_cross_section_material();
	}
	return _cross_section_material;
}

void Material4D::update_cross_section_material() {
}

void Material4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_albedo_color_of_edge", "edge_index", "for_mesh"), &Material4D::get_albedo_color_of_edge);
	ClassDB::bind_method(D_METHOD("is_default_material"), &Material4D::is_default_material);
	ClassDB::bind_method(D_METHOD("merge_with", "material", "first_item_count", "second_item_count"), &Material4D::merge_with);

	ClassDB::bind_method(D_METHOD("get_albedo_source_flags"), &Material4D::get_albedo_source_flags);
	ClassDB::bind_method(D_METHOD("set_albedo_source_flags", "albedo_source_flags"), &Material4D::set_albedo_source_flags);

	ClassDB::bind_method(D_METHOD("get_albedo_color"), &Material4D::get_albedo_color);
	ClassDB::bind_method(D_METHOD("set_albedo_color", "albedo_color"), &Material4D::set_albedo_color);

	ClassDB::bind_method(D_METHOD("get_albedo_color_array"), &Material4D::get_albedo_color_array);
	ClassDB::bind_method(D_METHOD("set_albedo_color_array", "albedo_color_array"), &Material4D::set_albedo_color_array);
	ClassDB::bind_method(D_METHOD("append_albedo_color", "albedo_color"), &Material4D::append_albedo_color);
	ClassDB::bind_method(D_METHOD("resize_albedo_color_array", "size", "fill_color"), &Material4D::resize_albedo_color_array, DEFVAL(Color(1, 1, 1, 1)));

	ClassDB::bind_method(D_METHOD("get_cross_section_material"), &Material4D::get_cross_section_material);

	BIND_ENUM_CONSTANT(COLOR_SOURCE_FLAG_SINGLE_COLOR);
	BIND_ENUM_CONSTANT(COLOR_SOURCE_FLAG_PER_VERT);
	BIND_ENUM_CONSTANT(COLOR_SOURCE_FLAG_PER_EDGE);
	BIND_ENUM_CONSTANT(COLOR_SOURCE_FLAG_PER_FACE);
	BIND_ENUM_CONSTANT(COLOR_SOURCE_FLAG_PER_CELL);
	BIND_ENUM_CONSTANT(COLOR_SOURCE_FLAG_TEXTURE3D_VERT_UVW);
	BIND_ENUM_CONSTANT(COLOR_SOURCE_FLAG_TEXTURE3D_EDGE_UVW);
	BIND_ENUM_CONSTANT(COLOR_SOURCE_FLAG_TEXTURE3D_FACE_UVW);
	BIND_ENUM_CONSTANT(COLOR_SOURCE_FLAG_TEXTURE3D_CELL_UVW);
	BIND_ENUM_CONSTANT(COLOR_SOURCE_FLAG_TEXTURE4D_DIRECT);
}
