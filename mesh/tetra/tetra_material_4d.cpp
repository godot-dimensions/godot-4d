#include "tetra_material_4d.h"

Material4D::ColorSourceFlags _tetra_source_to_flags(const TetraMaterial4D::TetraColorSource p_tetra_source) {
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
