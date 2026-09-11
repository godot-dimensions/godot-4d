#include "poly_material_4d.h"

// This function also supports TetraMesh4D converted from PolyMesh4D
// or otherwise have tets grouped by their starting vertex.
void PolyMaterial4D::populate_albedo_color_array_for_poly_mesh(const Ref<TetraMesh4D> &p_poly_mesh) {
	ERR_FAIL_COND(p_poly_mesh.is_null());
	const int64_t poly_color_array_size = _poly_albedo_color_array.size();
	if (poly_color_array_size == 0) {
		return; // Nothing to do.
	}
	const PackedInt32Array tets = p_poly_mesh->get_simplex_cell_vertex_indices();
	ERR_FAIL_COND(tets.size() < 4 || tets.size() % 4 != 0);
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

// For a PolyMaterial4D, the merged items are the polyhedral cells colored by `poly_albedo_color_array`,
// so callers must pass the boundary cell counts of the meshes rather than their vertex counts.
void PolyMaterial4D::merge_with(const Ref<Material4D> &p_material, const int p_first_item_count, const int p_second_item_count) {
	ERR_FAIL_COND_MSG(p_material.is_null(), "PolyMaterial4D.merge_with: Cannot merge with a null material.");
	// Material4D::merge_with merges `_albedo_color_array`, but for PolyMaterial4D that array is only a
	// per-tetrahedron cache derived from `_poly_albedo_color_array`, which holds the real per-cell colors.
	// Build a view of the other material whose color array is per-cell, and merge our per-cell array with it.
	ColorSourceFlags other_flags = p_material->get_albedo_source_flags();
	Color other_color = p_material->get_albedo_color();
	PackedColorArray other_cell_colors;
	const Ref<PolyMaterial4D> other_poly_material = p_material;
	if (other_poly_material.is_valid()) {
		other_cell_colors = other_poly_material->get_poly_albedo_color_array();
	} else if (other_flags & COLOR_SOURCE_FLAG_USES_COLOR_ARRAY) {
		// The other material's colors are per-vertex, per-tetrahedron, or per-edge. Without the mesh there
		// is no way to map those onto polyhedral cells, so the best we can do is keep its single color.
		WARN_PRINT("PolyMaterial4D.merge_with: The other material's color array cannot be converted to per-cell colors, so it will be ignored. Merge with a PolyMaterial4D to preserve per-cell colors.");
		other_flags = ColorSourceFlags(other_flags & ~COLOR_SOURCE_FLAG_USES_COLOR_ARRAY);
		if (!(other_flags & COLOR_SOURCE_FLAG_SINGLE_COLOR)) {
			other_flags = ColorSourceFlags(other_flags | COLOR_SOURCE_FLAG_SINGLE_COLOR);
			other_color = Color(1, 1, 1, 1);
		}
	}
	Ref<Material4D> other_per_cell_material;
	other_per_cell_material.instantiate();
	other_per_cell_material->set_albedo_source_flags(other_flags);
	other_per_cell_material->set_albedo_color(other_color);
	other_per_cell_material->set_albedo_color_array(other_cell_colors);
	// Let the base classes merge the per-cell arrays and update the albedo source, then move the result back.
	_albedo_color_array = _poly_albedo_color_array;
	TetraMaterial4D::merge_with(other_per_cell_material, p_first_item_count, p_second_item_count);
	_poly_albedo_color_array = _albedo_color_array;
	_albedo_color_array.clear();
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

	ClassDB::bind_method(D_METHOD("populate_albedo_color_array_for_poly_mesh", "poly_mesh"), &PolyMaterial4D::populate_albedo_color_array_for_poly_mesh);

	ADD_PROPERTY(PropertyInfo(Variant::PACKED_COLOR_ARRAY, "poly_albedo_color_array"), "set_poly_albedo_color_array", "get_poly_albedo_color_array");
}
