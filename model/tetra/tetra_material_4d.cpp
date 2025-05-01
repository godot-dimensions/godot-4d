#include "tetra_material_4d.h"

#include "tetra_mesh_4d.h"

Material4D::ColorSourceFlags TetraMaterial4D::_tetra_source_to_flags(const TetraColorSource p_tetra_source) {
	switch (p_tetra_source) {
		case TetraMaterial4D::TETRA_COLOR_SOURCE_SINGLE_COLOR:
			return Material4D::COLOR_SOURCE_FLAG_SINGLE_COLOR;
		case TetraMaterial4D::TETRA_COLOR_SOURCE_PER_VERT_ONLY:
			return Material4D::COLOR_SOURCE_FLAG_PER_VERT;
		case TetraMaterial4D::TETRA_COLOR_SOURCE_PER_CELL_ONLY:
			return Material4D::COLOR_SOURCE_FLAG_PER_CELL;
		case TetraMaterial4D::TETRA_COLOR_SOURCE_CELL_UVW_ONLY:
			return Material4D::COLOR_SOURCE_FLAG_CELL_UVW;
		case TetraMaterial4D::TETRA_COLOR_SOURCE_TEXTURE4D_ONLY:
			return Material4D::COLOR_SOURCE_FLAG_TEXTURE4D;
		case TetraMaterial4D::TETRA_COLOR_SOURCE_PER_VERT_AND_SINGLE:
			return Material4D::ColorSourceFlags(Material4D::COLOR_SOURCE_FLAG_PER_VERT | Material4D::COLOR_SOURCE_FLAG_SINGLE_COLOR);
		case TetraMaterial4D::TETRA_COLOR_SOURCE_PER_CELL_AND_SINGLE:
			return Material4D::ColorSourceFlags(Material4D::COLOR_SOURCE_FLAG_PER_CELL | Material4D::COLOR_SOURCE_FLAG_SINGLE_COLOR);
		case TetraMaterial4D::TETRA_COLOR_SOURCE_CELL_UVW_AND_SINGLE:
			return Material4D::ColorSourceFlags(Material4D::COLOR_SOURCE_FLAG_CELL_UVW | Material4D::COLOR_SOURCE_FLAG_SINGLE_COLOR);
		case TetraMaterial4D::TETRA_COLOR_SOURCE_TEXTURE4D_AND_SINGLE:
			return Material4D::ColorSourceFlags(Material4D::COLOR_SOURCE_FLAG_TEXTURE4D | Material4D::COLOR_SOURCE_FLAG_SINGLE_COLOR);
	}
	return Material4D::COLOR_SOURCE_FLAG_NONE;
}

Color TetraMaterial4D::get_albedo_color_of_edge(const int64_t p_edge_index, const Ref<Mesh4D> &p_for_mesh) {
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
	ERR_FAIL_COND_V_MSG(p_for_mesh.is_null(), _albedo_color, "TetraMaterial4D: Mesh is null.");
	const PackedInt32Array edge_indices = p_for_mesh->get_edge_indices();
	const int64_t edge_count = edge_indices.size() / 2;
	ERR_FAIL_COND_V_MSG(p_edge_index >= edge_count, _albedo_color, "TetraMaterial4D: Edge index out of bounds for mesh.");
	if (_albedo_source_flags & COLOR_SOURCE_FLAG_PER_CELL) {
		Ref<TetraMesh4D> cell_mesh = p_for_mesh;
		ERR_FAIL_COND_V_MSG(cell_mesh.is_null(), _albedo_color, "TetraMaterial4D: Mesh with per-cell colors is not a tetra cell mesh.");
		_edge_albedo_color_cache.resize(edge_count);
		const PackedInt32Array cell_indices = cell_mesh->get_cell_indices();
		constexpr int64_t vertices_per_cell = 4;
		for (int64_t i = 0; i < edge_count; i++) {
			const int32_t first_vertex_index = edge_indices[i * 2];
			const int32_t second_vertex_index = edge_indices[i * 2 + 1];
			Color sum_color = Color(0, 0, 0, 0);
			int color_amount = 0;
			int64_t find_from = 0;
			while (true) {
				find_from = cell_indices.find(first_vertex_index, find_from);
				if (find_from < 0) {
					break;
				}
				// Truncated integer division to get the cell index.
				const int64_t cell_index = find_from / vertices_per_cell;
				ERR_FAIL_INDEX_V_MSG(cell_index, _albedo_color_array.size(), _albedo_color, "TetraMaterial4D: Cell index out of bounds for material's color array.");
				find_from = cell_indices.find(second_vertex_index, cell_index * vertices_per_cell);
				if (find_from < 0) {
					break;
				}
				const int64_t next_cell_start = (cell_index + 1) * vertices_per_cell;
				if (find_from < next_cell_start) {
					// The second vertex is in the same cell as the first vertex, therefore this cell has this edge.
					sum_color += _albedo_color_array[cell_index];
					color_amount++;
				}
				find_from = next_cell_start;
			}
			if (color_amount == 0) {
				// No color found, use the single color as a fallback even if the single color flag is not set.
				sum_color = _albedo_color;
			} else {
				sum_color /= color_amount;
				if (_albedo_source_flags & COLOR_SOURCE_FLAG_SINGLE_COLOR) {
					sum_color *= _albedo_color;
				}
			}
			_edge_albedo_color_cache.set(i, sum_color);
		}
	}
	return Material4D::get_albedo_color_of_edge(p_edge_index, p_for_mesh);
}

void TetraMaterial4D::merge_with(const Ref<Material4D> &p_material, const int p_first_item_count, const int p_second_item_count) {
	Material4D::merge_with(p_material, p_first_item_count, p_second_item_count);
	// Read _albedo_source_flags and set the tetra material's albedo source enum.
	if (_albedo_source_flags & Material4D::COLOR_SOURCE_FLAG_SINGLE_COLOR) {
		if (_albedo_source_flags & Material4D::COLOR_SOURCE_FLAG_PER_CELL) {
			set_albedo_source(TETRA_COLOR_SOURCE_PER_CELL_AND_SINGLE);
		} else if (_albedo_source_flags & Material4D::COLOR_SOURCE_FLAG_PER_VERT) {
			set_albedo_source(TETRA_COLOR_SOURCE_PER_VERT_AND_SINGLE);
		} else if (_albedo_source_flags & Material4D::COLOR_SOURCE_FLAG_CELL_UVW) {
			set_albedo_source(TETRA_COLOR_SOURCE_CELL_UVW_AND_SINGLE);
		} else if (_albedo_source_flags & Material4D::COLOR_SOURCE_FLAG_TEXTURE4D) {
			set_albedo_source(TETRA_COLOR_SOURCE_TEXTURE4D_AND_SINGLE);
		} else {
			set_albedo_source(TETRA_COLOR_SOURCE_SINGLE_COLOR);
		}
	} else if (_albedo_source_flags & Material4D::COLOR_SOURCE_FLAG_PER_CELL) {
		set_albedo_source(TETRA_COLOR_SOURCE_PER_CELL_ONLY);
	} else if (_albedo_source_flags & Material4D::COLOR_SOURCE_FLAG_PER_VERT) {
		set_albedo_source(TETRA_COLOR_SOURCE_PER_VERT_ONLY);
	} else if (_albedo_source_flags & Material4D::COLOR_SOURCE_FLAG_CELL_UVW) {
		set_albedo_source(TETRA_COLOR_SOURCE_CELL_UVW_ONLY);
	} else if (_albedo_source_flags & Material4D::COLOR_SOURCE_FLAG_TEXTURE4D) {
		set_albedo_source(TETRA_COLOR_SOURCE_TEXTURE4D_ONLY);
	}
}

TetraMaterial4D::TetraColorSource TetraMaterial4D::get_albedo_source() const {
	return _albedo_source;
}

void TetraMaterial4D::set_albedo_source(const TetraColorSource p_albedo_source) {
	_albedo_source = p_albedo_source;
	_albedo_source_flags = _tetra_source_to_flags(_albedo_source);
	notify_property_list_changed();
}

void TetraMaterial4D::_get_property_list(List<PropertyInfo> *p_list) const {
	for (List<PropertyInfo>::Element *E = p_list->front(); E; E = E->next()) {
		PropertyInfo &prop = E->get();
		if (prop.name == StringName("albedo_color")) {
			prop.usage = (_albedo_source_flags & COLOR_SOURCE_FLAG_SINGLE_COLOR) ? PROPERTY_USAGE_DEFAULT : PROPERTY_USAGE_NONE;
		} else if (prop.name == StringName("albedo_color_array")) {
			prop.usage = (_albedo_source_flags & COLOR_SOURCE_FLAG_USES_COLOR_ARRAY) ? PROPERTY_USAGE_DEFAULT : PROPERTY_USAGE_NONE;
		}
	}
}

TetraMaterial4D::TetraMaterial4D() {
	set_albedo_source(TETRA_COLOR_SOURCE_SINGLE_COLOR);
}

void TetraMaterial4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_albedo_source"), &TetraMaterial4D::get_albedo_source);
	ClassDB::bind_method(D_METHOD("set_albedo_source", "albedo_source"), &TetraMaterial4D::set_albedo_source);

	//ADD_GROUP("Albedo", "albedo_");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "albedo_source", PROPERTY_HINT_ENUM, "Single Color,Per Vertex Only,Per Cell Only,Cell UVW Only,Texture4D Only,Per Vertex and Single Color,Per Cell and Single Color,Cell UVW and Single Color,Texture4D and Single Color"), "set_albedo_source", "get_albedo_source");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "albedo_color"), "set_albedo_color", "get_albedo_color");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_COLOR_ARRAY, "albedo_color_array"), "set_albedo_color_array", "get_albedo_color_array");

	BIND_ENUM_CONSTANT(TETRA_COLOR_SOURCE_SINGLE_COLOR);
	BIND_ENUM_CONSTANT(TETRA_COLOR_SOURCE_PER_VERT_ONLY);
	BIND_ENUM_CONSTANT(TETRA_COLOR_SOURCE_PER_CELL_ONLY);
	BIND_ENUM_CONSTANT(TETRA_COLOR_SOURCE_CELL_UVW_ONLY);
	BIND_ENUM_CONSTANT(TETRA_COLOR_SOURCE_TEXTURE4D_ONLY);
	BIND_ENUM_CONSTANT(TETRA_COLOR_SOURCE_PER_VERT_AND_SINGLE);
	BIND_ENUM_CONSTANT(TETRA_COLOR_SOURCE_PER_CELL_AND_SINGLE);
	BIND_ENUM_CONSTANT(TETRA_COLOR_SOURCE_CELL_UVW_AND_SINGLE);
	BIND_ENUM_CONSTANT(TETRA_COLOR_SOURCE_TEXTURE4D_AND_SINGLE);
}
