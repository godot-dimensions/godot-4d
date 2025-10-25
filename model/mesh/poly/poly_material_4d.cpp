#include "poly_material_4d.h"

// This function also supports TetraMesh4D converted from PolyMesh4D
// or otherwise have tets grouped by their starting vertex.
void PolyMaterial4D::populate_albedo_color_array_for_poly_mesh(const Ref<TetraMesh4D> &p_poly_mesh) {
	ERR_FAIL_COND(p_poly_mesh.is_null());
	const PackedInt32Array tets = p_poly_mesh->get_cell_indices();
	ERR_FAIL_COND(tets.size() < 4 || tets.size() % 4 != 0);
	const int64_t poly_color_array_size = _poly_albedo_color_array.size();
	ERR_FAIL_COND(poly_color_array_size == 0);
	_albedo_color_array.clear();
	_albedo_color_array.append(_poly_albedo_color_array[0]);
	int64_t color_index = 0;
	int32_t last_tet_start = tets[0];
	for (int64_t tet_start_index = 4; tet_start_index < tets.size(); tet_start_index += 4) {
		const int32_t tet_start = tets[tet_start_index];
		if (tet_start != last_tet_start) {
			last_tet_start = tet_start;
			color_index++;
			ERR_FAIL_INDEX(color_index, poly_color_array_size);
		}
		_albedo_color_array.append(_poly_albedo_color_array[color_index]);
	}
}

void PolyMaterial4D::set_poly_albedo_color_array(const PackedColorArray &p_colors) {
	_poly_albedo_color_array = p_colors;
	_albedo_color_array.clear();
}

void PolyMaterial4D::_validate_property(PropertyInfo &p_property) const {
	TetraMaterial4D::_validate_property(p_property);
	if (p_property.name == StringName("poly_albedo_color_array")) {
		p_property.usage = (_albedo_source_flags & COLOR_SOURCE_FLAG_USES_COLOR_ARRAY) ? PROPERTY_USAGE_DEFAULT : PROPERTY_USAGE_NONE;
	} else if (p_property.name == StringName("albedo_color_array")) {
		p_property.usage = PROPERTY_USAGE_NONE;
	}
}

PolyMaterial4D::PolyMaterial4D() {
	set_albedo_source(TETRA_COLOR_SOURCE_SINGLE_COLOR);
}

void PolyMaterial4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_poly_albedo_color_array"), &PolyMaterial4D::get_poly_albedo_color_array);
	ClassDB::bind_method(D_METHOD("set_poly_albedo_color_array", "colors"), &PolyMaterial4D::set_poly_albedo_color_array);

	ADD_PROPERTY(PropertyInfo(Variant::PACKED_COLOR_ARRAY, "poly_albedo_color_array"), "set_poly_albedo_color_array", "get_poly_albedo_color_array");
}
