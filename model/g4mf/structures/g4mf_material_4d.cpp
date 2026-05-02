#include "g4mf_material_4d.h"

#include "../../mesh/poly/poly_material_4d.h"
#include "../g4mf_state_4d.h"

bool G4MFMaterial4D::is_equal_exact(const Ref<G4MFMaterial4D> &p_other) const {
	if ((_base_color_channel.is_valid() != p_other->get_base_color_channel().is_valid()) ||
			(_emissive_channel.is_valid() != p_other->get_emissive_channel().is_valid()) ||
			(_normal_channel.is_valid() != p_other->get_normal_channel().is_valid()) ||
			(_orm_channel.is_valid() != p_other->get_orm_channel().is_valid())) {
		return false;
	}
	if (_base_color_channel.is_valid() && !_base_color_channel->is_equal_exact(p_other->get_base_color_channel())) {
		return false;
	}
	if (_emissive_channel.is_valid() && !_emissive_channel->is_equal_exact(p_other->get_emissive_channel())) {
		return false;
	}
	if (_normal_channel.is_valid() && !_normal_channel->is_equal_exact(p_other->get_normal_channel())) {
		return false;
	}
	if (_orm_channel.is_valid() && !_orm_channel->is_equal_exact(p_other->get_orm_channel())) {
		return false;
	}
	return true;
}

Ref<TetraMaterial4D> G4MFMaterial4D::generate_tetra_material(const Ref<G4MFState4D> &p_g4mf_state) const {
	Ref<TetraMaterial4D> tetra_material;
	tetra_material.instantiate();
	tetra_material->set_name(get_name());
	if (_base_color_channel.is_valid()) {
		const Color single_base_color = _base_color_channel->get_factor();
		const bool has_single_base_color = single_base_color.r > -1 && !Color(1, 1, 1, 1).is_equal_approx(single_base_color);
		if (has_single_base_color) {
			tetra_material->set_albedo_color(single_base_color);
		}
		const Ref<G4MFMeshSurfaceBinding4D> base_color_binding = _base_color_channel->get_element_map_binding();
		if (base_color_binding.is_valid()) {
			const PackedColorArray original_colors = base_color_binding->load_values_as_colors(p_g4mf_state);
			if (base_color_binding->get_per_simplex_accessor_index() != 0) {
				PackedInt32Array simplex_indices = base_color_binding->load_per_simplex_indices(p_g4mf_state);
				PackedColorArray simplex_colors;
				simplex_colors.resize(simplex_indices.size());
				for (int i = 0; i < simplex_indices.size(); i++) {
					const int color_index = simplex_indices[i];
					ERR_FAIL_INDEX_V(color_index, original_colors.size(), tetra_material);
					simplex_colors.set(i, original_colors[color_index]);
				}
				tetra_material->set_albedo_color_array(simplex_colors);
				if (has_single_base_color) {
					tetra_material->set_albedo_source(TetraMaterial4D::TETRA_COLOR_SOURCE_PER_CELL_AND_SINGLE);
				} else {
					tetra_material->set_albedo_source(TetraMaterial4D::TETRA_COLOR_SOURCE_PER_CELL_ONLY);
				}
			}
		}
	}
	if (_emissive_channel.is_valid()) {
		WARN_PRINT("G4MFMaterial4D.generate_tetra_material: Emissive maps are not supported yet.");
	}
	if (_normal_channel.is_valid()) {
		WARN_PRINT("G4MFMaterial4D.generate_tetra_material: Normal maps are not supported yet.");
	}
	if (_orm_channel.is_valid()) {
		WARN_PRINT("G4MFMaterial4D.generate_tetra_material: ORM maps are not supported yet.");
	}
	return tetra_material;
}

Ref<WireMaterial4D> G4MFMaterial4D::generate_wire_material(const Ref<G4MFState4D> &p_g4mf_state) const {
	Ref<WireMaterial4D> wire_material;
	wire_material.instantiate();
	wire_material->set_name(get_name());
	if (_base_color_channel.is_valid()) {
		const Color single_base_color = _base_color_channel->get_factor();
		const bool has_single_base_color = single_base_color.r > -1 && !Color(1, 1, 1, 1).is_equal_approx(single_base_color);
		if (has_single_base_color) {
			wire_material->set_albedo_color(single_base_color);
		}
		const Ref<G4MFMeshSurfaceBinding4D> base_color_binding = _base_color_channel->get_element_map_binding();
		if (base_color_binding.is_valid()) {
			const PackedInt32Array per_edge_indices = base_color_binding->load_geometry_binding_indices(p_g4mf_state, 1, 1);
			if (per_edge_indices.size() > 0) {
				const PackedColorArray original_colors = base_color_binding->load_values_as_colors(p_g4mf_state);
				PackedColorArray per_edge_colors;
				per_edge_colors.resize(per_edge_indices.size());
				for (int i = 0; i < per_edge_indices.size(); i++) {
					const int color_index = per_edge_indices[i];
					ERR_FAIL_INDEX_V(color_index, original_colors.size(), wire_material);
					per_edge_colors.set(i, original_colors[color_index]);
				}
				wire_material->set_albedo_color_array(per_edge_colors);
				if (has_single_base_color) {
					wire_material->set_albedo_source(WireMaterial4D::WIRE_COLOR_SOURCE_PER_EDGE_AND_SINGLE);
				} else {
					wire_material->set_albedo_source(WireMaterial4D::WIRE_COLOR_SOURCE_PER_EDGE_ONLY);
				}
			}
		}
	}
	if (_emissive_channel.is_valid()) {
		WARN_PRINT("G4MFMaterial4D.generate_wire_material: Emissive maps are not supported yet.");
	}
	if (_normal_channel.is_valid()) {
		WARN_PRINT("G4MFMaterial4D.generate_wire_material: Normal maps are not supported yet.");
	}
	if (_orm_channel.is_valid()) {
		WARN_PRINT("G4MFMaterial4D.generate_wire_material: ORM maps are not supported yet.");
	}
	return wire_material;
}

void G4MFMaterial4D::_append_geometry_binding(TypedArray<G4MFMeshSurfaceBindingGeometry4D> &p_geometry_bindings, const int p_geometry_dimension, const int p_decompose_dimension, const int p_indices_accessor_index) {
	Ref<G4MFMeshSurfaceBindingGeometry4D> geom;
	geom.instantiate();
	geom->set_geometry_dimension(p_geometry_dimension);
	geom->set_decompose_dimension(p_decompose_dimension);
	geom->set_indices_accessor_index(p_indices_accessor_index);
	p_geometry_bindings.append(geom);
}

int G4MFMaterial4D::convert_material_into_state(Ref<G4MFState4D> p_g4mf_state, const Ref<Material4D> &p_material, const bool p_deduplicate) {
	Ref<G4MFMaterial4D> g4mf_material;
	g4mf_material.instantiate();
	g4mf_material->set_name(p_material->get_name());
	const Material4D::ColorSourceFlags albedo_source_flags = p_material->get_albedo_source_flags();
	const PackedColorArray albedo_colors = p_material->get_albedo_color_array();
	if (albedo_source_flags & Material4D::COLOR_SOURCE_FLAG_USES_COLOR_ARRAY && albedo_colors.size() > 0) {
		Ref<G4MFMaterialChannel4D> base_color_channel;
		base_color_channel.instantiate();
		Ref<G4MFMeshSurfaceBinding4D> element_map_binding;
		element_map_binding.instantiate();
		TypedArray<G4MFMeshSurfaceBindingGeometry4D> geometry_bindings;
		// Deduplicate colors with indices to store the data in the G4MF style.
		PackedColorArray deduplicated_colors;
		PackedInt64Array deduplicated_simplex_indices;
		deduplicated_simplex_indices.resize(albedo_colors.size());
		HashMap<Color, int> color_to_index_map;
		for (int i = 0; i < albedo_colors.size(); i++) {
			const Color color = albedo_colors[i];
			if (color_to_index_map.has(color)) {
				deduplicated_simplex_indices.set(i, color_to_index_map[color]);
			} else {
				const int new_index = deduplicated_colors.size();
				deduplicated_colors.append(color);
				color_to_index_map[color] = new_index;
				deduplicated_simplex_indices.set(i, new_index);
			}
		}
		// Handle PolyMaterial4D's separate color array.
		Ref<PolyMaterial4D> poly_material = p_material;
		if (poly_material.is_valid()) {
			const PackedColorArray poly_array = poly_material->get_poly_albedo_color_array();
			PackedInt64Array deduplicated_poly_indices;
			deduplicated_poly_indices.resize(poly_array.size());
			for (int i = 0; i < poly_array.size(); i++) {
				const Color color = poly_array[i];
				if (color_to_index_map.has(color)) {
					deduplicated_poly_indices.set(i, color_to_index_map[color]);
				} else {
					const int new_index = deduplicated_colors.size();
					deduplicated_colors.append(color);
					color_to_index_map[color] = new_index;
					deduplicated_poly_indices.set(i, new_index);
				}
			}
			const int poly_indices_accessor_index = G4MFAccessor4D::encode_new_accessor_from_int64s(p_g4mf_state, deduplicated_poly_indices, p_deduplicate);
			if (albedo_source_flags & Material4D::COLOR_SOURCE_FLAG_PER_CELL) {
				_append_geometry_binding(geometry_bindings, 3, 3, poly_indices_accessor_index);
			} else if (albedo_source_flags & Material4D::COLOR_SOURCE_FLAG_PER_FACE) {
				_append_geometry_binding(geometry_bindings, 2, 2, poly_indices_accessor_index);
			} else if (albedo_source_flags & Material4D::COLOR_SOURCE_FLAG_PER_EDGE) {
				_append_geometry_binding(geometry_bindings, 1, 1, poly_indices_accessor_index);
			} else if (albedo_source_flags & Material4D::COLOR_SOURCE_FLAG_PER_VERT) {
				_append_geometry_binding(geometry_bindings, 0, 0, poly_indices_accessor_index);
			} else {
				ERR_PRINT("G4MFMaterial4D.convert_material_into_state: Unhandled albedo source flag for poly material.");
			}
		}
		// Handle the main material's color array.
		if (albedo_source_flags & Material4D::COLOR_SOURCE_FLAG_PER_CELL) {
			int simplex_indices_accessor_index = G4MFAccessor4D::encode_new_accessor_from_int64s(p_g4mf_state, deduplicated_simplex_indices, p_deduplicate);
			element_map_binding->set_per_simplex_accessor_index(simplex_indices_accessor_index);
		} else if (albedo_source_flags & Material4D::COLOR_SOURCE_FLAG_PER_EDGE) {
			// Only encode per-edge indices here if it wasn't already handled above by the poly material's per-vertex/edge/face/cell colors.
			// deduplicated_simplex_indices is just the data from the array, it is not necessarily exactly the same simplex indices as the tetrahedra.
			if (poly_material.is_null()) {
				int edge_indices_accessor_index = G4MFAccessor4D::encode_new_accessor_from_int64s(p_g4mf_state, deduplicated_simplex_indices, p_deduplicate);
				_append_geometry_binding(geometry_bindings, 1, 1, edge_indices_accessor_index);
			}
		} else {
			ERR_PRINT("G4MFMaterial4D.convert_material_into_state: Unhandled albedo source flag.");
		}
		// Set the bindings, values, and factor for the material channel.
		element_map_binding->set_geometry_bindings(geometry_bindings);
		element_map_binding->set_values_accessor_index(G4MFAccessor4D::encode_new_accessor_from_colors(p_g4mf_state, deduplicated_colors, p_deduplicate));
		if (albedo_source_flags & Material4D::COLOR_SOURCE_FLAG_SINGLE_COLOR) {
			base_color_channel->set_factor(p_material->get_albedo_color());
		}
		g4mf_material->set_base_color_channel(base_color_channel);
	} else if (albedo_source_flags & Material4D::COLOR_SOURCE_FLAG_SINGLE_COLOR) {
		const Color base_color = p_material->get_albedo_color();
		if (!Color(1, 1, 1, 1).is_equal_approx(base_color)) {
			Ref<G4MFMaterialChannel4D> base_color_channel;
			base_color_channel.instantiate();
			base_color_channel->set_factor(base_color);
			g4mf_material->set_base_color_channel(base_color_channel);
		}
	}
	if (albedo_source_flags & Material4D::COLOR_SOURCE_FLAG_USES_TEXTURE_MAP) {
		ERR_PRINT("G4MFMaterial4D.convert_material_into_state: Texture maps are not supported yet.");
	}
	// Add the G4MFMaterial4D to the G4MFState4D, but check for duplicates first.
	TypedArray<G4MFMaterial4D> state_materials = p_g4mf_state->get_g4mf_materials();
	const int state_material_count = state_materials.size();
	if (p_deduplicate) {
		for (int i = 0; i < state_material_count; i++) {
			const Ref<G4MFMaterial4D> state_material = state_materials[i];
			if (g4mf_material->is_equal_exact(state_material)) {
				// An identical material already exists in state, we can just use that.
				return i;
			}
		}
	}
	g4mf_material->set_name(p_material->get_name());
	state_materials.append(g4mf_material);
	p_g4mf_state->set_g4mf_materials(state_materials);
	return state_material_count;
}

Ref<G4MFMaterial4D> G4MFMaterial4D::from_dictionary(const Dictionary &p_dict) {
	Ref<G4MFMaterial4D> material;
	material.instantiate();
	material->read_item_entries_from_dictionary(p_dict);
	if (p_dict.has("baseColor")) {
		material->set_base_color_channel(G4MFMaterialChannel4D::from_dictionary(p_dict["baseColor"]));
	}
	if (p_dict.has("emissive")) {
		material->set_emissive_channel(G4MFMaterialChannel4D::from_dictionary(p_dict["emissive"]));
	}
	if (p_dict.has("normal")) {
		material->set_normal_channel(G4MFMaterialChannel4D::from_dictionary(p_dict["normal"]));
	}
	if (p_dict.has("orm")) {
		material->set_orm_channel(G4MFMaterialChannel4D::from_dictionary(p_dict["orm"]));
	}
	return material;
}

Dictionary G4MFMaterial4D::to_dictionary() const {
	Dictionary dict = write_item_entries_to_dictionary();
	if (_base_color_channel.is_valid()) {
		dict["baseColor"] = _base_color_channel->to_dictionary();
	}
	if (_emissive_channel.is_valid()) {
		dict["emissive"] = _emissive_channel->to_dictionary();
	}
	if (_normal_channel.is_valid()) {
		dict["normal"] = _normal_channel->to_dictionary();
	}
	if (_orm_channel.is_valid()) {
		dict["orm"] = _orm_channel->to_dictionary();
	}
	return dict;
}

void G4MFMaterial4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_base_color_channel"), &G4MFMaterial4D::get_base_color_channel);
	ClassDB::bind_method(D_METHOD("set_base_color_channel", "channel"), &G4MFMaterial4D::set_base_color_channel);
	ClassDB::bind_method(D_METHOD("get_emissive_channel"), &G4MFMaterial4D::get_emissive_channel);
	ClassDB::bind_method(D_METHOD("set_emissive_channel", "channel"), &G4MFMaterial4D::set_emissive_channel);
	ClassDB::bind_method(D_METHOD("get_normal_channel"), &G4MFMaterial4D::get_normal_channel);
	ClassDB::bind_method(D_METHOD("set_normal_channel", "channel"), &G4MFMaterial4D::set_normal_channel);
	ClassDB::bind_method(D_METHOD("get_orm_channel"), &G4MFMaterial4D::get_orm_channel);
	ClassDB::bind_method(D_METHOD("set_orm_channel", "channel"), &G4MFMaterial4D::set_orm_channel);

	ClassDB::bind_method(D_METHOD("is_equal_exact", "other"), &G4MFMaterial4D::is_equal_exact);

	ClassDB::bind_method(D_METHOD("generate_tetra_material", "g4mf_state"), &G4MFMaterial4D::generate_tetra_material);
	ClassDB::bind_method(D_METHOD("generate_wire_material", "g4mf_state"), &G4MFMaterial4D::generate_wire_material);
	ClassDB::bind_static_method("G4MFMaterial4D", D_METHOD("convert_material_into_state", "g4mf_state", "material", "deduplicate"), &G4MFMaterial4D::convert_material_into_state, DEFVAL(true));

	ClassDB::bind_static_method("G4MFMaterial4D", D_METHOD("from_dictionary", "dict"), &G4MFMaterial4D::from_dictionary);
	ClassDB::bind_method(D_METHOD("to_dictionary"), &G4MFMaterial4D::to_dictionary);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "base_color_channel", PROPERTY_HINT_RESOURCE_TYPE, "G4MFMaterialChannel4D"), "set_base_color_channel", "get_base_color_channel");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "emissive_channel", PROPERTY_HINT_RESOURCE_TYPE, "G4MFMaterialChannel4D"), "set_emissive_channel", "get_emissive_channel");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "normal_channel", PROPERTY_HINT_RESOURCE_TYPE, "G4MFMaterialChannel4D"), "set_normal_channel", "get_normal_channel");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "orm_channel", PROPERTY_HINT_RESOURCE_TYPE, "G4MFMaterialChannel4D"), "set_orm_channel", "get_orm_channel");
}
