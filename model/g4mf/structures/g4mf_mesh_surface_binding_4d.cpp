#include "g4mf_mesh_surface_binding_4d.h"

#include "../g4mf_state_4d.h"

bool G4MFMeshSurfaceBindingGeometry4D::is_equal_exact(const Ref<G4MFMeshSurfaceBindingGeometry4D> &p_other) const {
	return (_indices_accessor_index == p_other->get_indices_accessor_index() &&
			_decomposition_dimension == p_other->get_decomposition_dimension() &&
			_geometry_index == p_other->get_geometry_index());
}

PackedInt32Array G4MFMeshSurfaceBindingGeometry4D::load_indices(const Ref<G4MFState4D> &p_g4mf_state) const {
	TypedArray<G4MFAccessor4D> state_accessors = p_g4mf_state->get_g4mf_accessors();
	ERR_FAIL_INDEX_V(_indices_accessor_index, state_accessors.size(), PackedInt32Array());
	const Ref<G4MFAccessor4D> accessor = state_accessors[_indices_accessor_index];
	return accessor->decode_int32s_from_bytes(p_g4mf_state);
}

bool G4MFMeshSurfaceBinding4D::is_equal_exact(const Ref<G4MFMeshSurfaceBinding4D> &p_other) const {
	const TypedArray<G4MFMeshSurfaceBindingGeometry4D> other_geometry_decompositions = p_other->get_geometry_decompositions();
	if (_geometry_decompositions.size() != other_geometry_decompositions.size()) {
		return false;
	}
	for (int i = 0; i < _geometry_decompositions.size(); i++) {
		const Ref<G4MFMeshSurfaceBindingGeometry4D> geom = _geometry_decompositions[i];
		const Ref<G4MFMeshSurfaceBindingGeometry4D> other = other_geometry_decompositions[i];
		if (!geom->is_equal_exact(other)) {
			return false;
		}
	}
	return (_edges_accessor_index == p_other->get_edges_accessor_index() &&
			_per_edge_accessor_index == p_other->get_per_edge_accessor_index() &&
			_per_simplex_accessor_index == p_other->get_per_simplex_accessor_index() &&
			_simplexes_accessor_index == p_other->get_simplexes_accessor_index() &&
			_values_accessor_index == p_other->get_values_accessor_index() &&
			_vertices_accessor_index == p_other->get_vertices_accessor_index());
}

PackedInt32Array G4MFMeshSurfaceBinding4D::load_edge_indices(const Ref<G4MFState4D> &p_g4mf_state) const {
	TypedArray<G4MFAccessor4D> state_accessors = p_g4mf_state->get_g4mf_accessors();
	ERR_FAIL_INDEX_V(_edges_accessor_index, state_accessors.size(), PackedInt32Array());
	const Ref<G4MFAccessor4D> accessor = state_accessors[_edges_accessor_index];
	return accessor->decode_int32s_from_bytes(p_g4mf_state);
}

PackedInt32Array G4MFMeshSurfaceBinding4D::load_per_edge_indices(const Ref<G4MFState4D> &p_g4mf_state) const {
	TypedArray<G4MFAccessor4D> state_accessors = p_g4mf_state->get_g4mf_accessors();
	ERR_FAIL_INDEX_V(_per_edge_accessor_index, state_accessors.size(), PackedInt32Array());
	const Ref<G4MFAccessor4D> accessor = state_accessors[_per_edge_accessor_index];
	return accessor->decode_int32s_from_bytes(p_g4mf_state);
}

PackedInt32Array G4MFMeshSurfaceBinding4D::load_per_simplex_indices(const Ref<G4MFState4D> &p_g4mf_state) const {
	TypedArray<G4MFAccessor4D> state_accessors = p_g4mf_state->get_g4mf_accessors();
	ERR_FAIL_INDEX_V(_per_simplex_accessor_index, state_accessors.size(), PackedInt32Array());
	const Ref<G4MFAccessor4D> accessor = state_accessors[_per_simplex_accessor_index];
	return accessor->decode_int32s_from_bytes(p_g4mf_state);
}

PackedInt32Array G4MFMeshSurfaceBinding4D::load_simplex_indices(const Ref<G4MFState4D> &p_g4mf_state) const {
	TypedArray<G4MFAccessor4D> state_accessors = p_g4mf_state->get_g4mf_accessors();
	ERR_FAIL_INDEX_V(_simplexes_accessor_index, state_accessors.size(), PackedInt32Array());
	const Ref<G4MFAccessor4D> accessor = state_accessors[_simplexes_accessor_index];
	return accessor->decode_int32s_from_bytes(p_g4mf_state);
}

PackedInt32Array G4MFMeshSurfaceBinding4D::load_vertex_indices(const Ref<G4MFState4D> &p_g4mf_state) const {
	TypedArray<G4MFAccessor4D> state_accessors = p_g4mf_state->get_g4mf_accessors();
	ERR_FAIL_INDEX_V(_vertices_accessor_index, state_accessors.size(), PackedInt32Array());
	const Ref<G4MFAccessor4D> accessor = state_accessors[_vertices_accessor_index];
	return accessor->decode_int32s_from_bytes(p_g4mf_state);
}

Array G4MFMeshSurfaceBinding4D::load_values_as_variants(const Ref<G4MFState4D> &p_g4mf_state, const Variant::Type p_variant_type) const {
	TypedArray<G4MFAccessor4D> state_accessors = p_g4mf_state->get_g4mf_accessors();
	ERR_FAIL_INDEX_V(_values_accessor_index, state_accessors.size(), Array());
	const Ref<G4MFAccessor4D> accessor = state_accessors[_values_accessor_index];
	return accessor->decode_variants_from_bytes(p_g4mf_state, p_variant_type);
}

PackedColorArray G4MFMeshSurfaceBinding4D::load_values_as_colors(const Ref<G4MFState4D> &p_g4mf_state) const {
	TypedArray<G4MFAccessor4D> state_accessors = p_g4mf_state->get_g4mf_accessors();
	ERR_FAIL_INDEX_V(_values_accessor_index, state_accessors.size(), PackedColorArray());
	const Ref<G4MFAccessor4D> accessor = state_accessors[_values_accessor_index];
	return accessor->decode_colors_from_bytes(p_g4mf_state);
}

Ref<G4MFMeshSurfaceBindingGeometry4D> G4MFMeshSurfaceBindingGeometry4D::from_dictionary(const Dictionary &p_dict) {
	Ref<G4MFMeshSurfaceBindingGeometry4D> geometry_decomposition;
	geometry_decomposition.instantiate();
	geometry_decomposition->read_item_entries_from_dictionary(p_dict);
	if (p_dict.has("accessor")) {
		geometry_decomposition->set_indices_accessor_index(int(p_dict["accessor"]));
	}
	if (p_dict.has("dimension")) {
		geometry_decomposition->set_decomposition_dimension(int(p_dict["dimension"]));
	}
	if (p_dict.has("index")) {
		geometry_decomposition->set_geometry_index(int(p_dict["index"]));
	}
	return geometry_decomposition;
}

Dictionary G4MFMeshSurfaceBindingGeometry4D::to_dictionary() const {
	Dictionary dict = write_item_entries_to_dictionary();
	dict["accessor"] = _indices_accessor_index; // Required.
	if (_decomposition_dimension != 0) {
		dict["dimension"] = _decomposition_dimension;
	}
	dict["index"] = _geometry_index; // Required.
	return dict;
}

Ref<G4MFMeshSurfaceBinding4D> G4MFMeshSurfaceBinding4D::from_dictionary(const Dictionary &p_dict) {
	Ref<G4MFMeshSurfaceBinding4D> binding;
	binding.instantiate();
	binding->read_item_entries_from_dictionary(p_dict);
	if (p_dict.has("geometry")) {
		Array geometry_array = p_dict["geometry"];
		TypedArray<G4MFMeshSurfaceBindingGeometry4D> geometry_decompositions;
		for (int i = 0; i < geometry_array.size(); i++) {
			Dictionary geom_dict = geometry_array[i];
			Ref<G4MFMeshSurfaceBindingGeometry4D> geom = G4MFMeshSurfaceBindingGeometry4D::from_dictionary(geom_dict);
			geometry_decompositions.append(geom);
		}
		binding->set_geometry_decompositions(geometry_decompositions);
	}
	if (p_dict.has("edges")) {
		binding->set_edges_accessor_index(int(p_dict["edges"]));
	}
	if (p_dict.has("perEdge")) {
		binding->set_per_edge_accessor_index(int(p_dict["perEdge"]));
	}
	if (p_dict.has("perSimplex")) {
		binding->set_per_simplex_accessor_index(int(p_dict["perSimplex"]));
	}
	if (p_dict.has("simplexes")) {
		binding->set_simplexes_accessor_index(int(p_dict["simplexes"]));
	}
	if (p_dict.has("values")) {
		binding->set_values_accessor_index(int(p_dict["values"]));
	}
	if (p_dict.has("vertices")) {
		binding->set_vertices_accessor_index(int(p_dict["vertices"]));
	}
	return binding;
}

Dictionary G4MFMeshSurfaceBinding4D::to_dictionary() const {
	Dictionary dict = write_item_entries_to_dictionary();
	if (_geometry_decompositions.size() > 0) {
		Array geometry_array;
		for (int i = 0; i < _geometry_decompositions.size(); i++) {
			Ref<G4MFMeshSurfaceBindingGeometry4D> geom = _geometry_decompositions[i];
			geometry_array.append(geom->to_dictionary());
		}
		dict["geometry"] = geometry_array;
	}
	if (_edges_accessor_index >= 0) {
		dict["edges"] = _edges_accessor_index;
	}
	if (_per_edge_accessor_index >= 0) {
		dict["perEdge"] = _per_edge_accessor_index;
	}
	if (_per_simplex_accessor_index >= 0) {
		dict["perSimplex"] = _per_simplex_accessor_index;
	}
	if (_simplexes_accessor_index >= 0) {
		dict["simplexes"] = _simplexes_accessor_index;
	}
	dict["values"] = _values_accessor_index; // Required.
	if (_vertices_accessor_index >= 0) {
		dict["vertices"] = _vertices_accessor_index;
	}
	return dict;
}

void G4MFMeshSurfaceBindingGeometry4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_indices_accessor_index"), &G4MFMeshSurfaceBindingGeometry4D::get_indices_accessor_index);
	ClassDB::bind_method(D_METHOD("set_indices_accessor_index", "indices_accessor_index"), &G4MFMeshSurfaceBindingGeometry4D::set_indices_accessor_index);
	ClassDB::bind_method(D_METHOD("get_decomposition_dimension"), &G4MFMeshSurfaceBindingGeometry4D::get_decomposition_dimension);
	ClassDB::bind_method(D_METHOD("set_decomposition_dimension", "decomposition_dimension"), &G4MFMeshSurfaceBindingGeometry4D::set_decomposition_dimension);
	ClassDB::bind_method(D_METHOD("get_geometry_index"), &G4MFMeshSurfaceBindingGeometry4D::get_geometry_index);
	ClassDB::bind_method(D_METHOD("set_geometry_index", "geometry_index"), &G4MFMeshSurfaceBindingGeometry4D::set_geometry_index);

	ClassDB::bind_method(D_METHOD("is_equal_exact", "other"), &G4MFMeshSurfaceBindingGeometry4D::is_equal_exact);
	ClassDB::bind_static_method("G4MFMeshSurfaceBindingGeometry4D", D_METHOD("from_dictionary", "dict"), &G4MFMeshSurfaceBindingGeometry4D::from_dictionary);
	ClassDB::bind_method(D_METHOD("to_dictionary"), &G4MFMeshSurfaceBindingGeometry4D::to_dictionary);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "indices_accessor_index"), "set_indices_accessor_index", "get_indices_accessor_index");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "decomposition_dimension"), "set_decomposition_dimension", "get_decomposition_dimension");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "geometry_index"), "set_geometry_index", "get_geometry_index");
}

void G4MFMeshSurfaceBinding4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_geometry_decompositions"), &G4MFMeshSurfaceBinding4D::get_geometry_decompositions);
	ClassDB::bind_method(D_METHOD("set_geometry_decompositions", "geometry_decompositions"), &G4MFMeshSurfaceBinding4D::set_geometry_decompositions);
	ClassDB::bind_method(D_METHOD("get_edges_accessor_index"), &G4MFMeshSurfaceBinding4D::get_edges_accessor_index);
	ClassDB::bind_method(D_METHOD("set_edges_accessor_index", "edges_accessor_index"), &G4MFMeshSurfaceBinding4D::set_edges_accessor_index);
	ClassDB::bind_method(D_METHOD("get_per_edge_accessor_index"), &G4MFMeshSurfaceBinding4D::get_per_edge_accessor_index);
	ClassDB::bind_method(D_METHOD("set_per_edge_accessor_index", "per_edge_accessor_index"), &G4MFMeshSurfaceBinding4D::set_per_edge_accessor_index);
	ClassDB::bind_method(D_METHOD("get_per_simplex_accessor_index"), &G4MFMeshSurfaceBinding4D::get_per_simplex_accessor_index);
	ClassDB::bind_method(D_METHOD("set_per_simplex_accessor_index", "per_simplex_accessor_index"), &G4MFMeshSurfaceBinding4D::set_per_simplex_accessor_index);
	ClassDB::bind_method(D_METHOD("get_simplexes_accessor_index"), &G4MFMeshSurfaceBinding4D::get_simplexes_accessor_index);
	ClassDB::bind_method(D_METHOD("set_simplexes_accessor_index", "simplexes_accessor_index"), &G4MFMeshSurfaceBinding4D::set_simplexes_accessor_index);
	ClassDB::bind_method(D_METHOD("get_values_accessor_index"), &G4MFMeshSurfaceBinding4D::get_values_accessor_index);
	ClassDB::bind_method(D_METHOD("set_values_accessor_index", "values_accessor_index"), &G4MFMeshSurfaceBinding4D::set_values_accessor_index);
	ClassDB::bind_method(D_METHOD("get_vertices_accessor_index"), &G4MFMeshSurfaceBinding4D::get_vertices_accessor_index);
	ClassDB::bind_method(D_METHOD("set_vertices_accessor_index", "vertices_accessor_index"), &G4MFMeshSurfaceBinding4D::set_vertices_accessor_index);

	ClassDB::bind_method(D_METHOD("is_equal_exact", "other"), &G4MFMeshSurfaceBinding4D::is_equal_exact);
	ClassDB::bind_static_method("G4MFMeshSurfaceBinding4D", D_METHOD("from_dictionary", "dict"), &G4MFMeshSurfaceBinding4D::from_dictionary);
	ClassDB::bind_method(D_METHOD("to_dictionary"), &G4MFMeshSurfaceBinding4D::to_dictionary);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "geometry_decompositions", PROPERTY_HINT_ARRAY_TYPE, "G4MFMeshSurfaceBindingGeometry4D"), "set_geometry_decompositions", "get_geometry_decompositions");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "edges_accessor_index"), "set_edges_accessor_index", "get_edges_accessor_index");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "per_edge_accessor_index"), "set_per_edge_accessor_index", "get_per_edge_accessor_index");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "per_simplex_accessor_index"), "set_per_simplex_accessor_index", "get_per_simplex_accessor_index");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "simplexes_accessor_index"), "set_simplexes_accessor_index", "get_simplexes_accessor_index");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "values_accessor_index"), "set_values_accessor_index", "get_values_accessor_index");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "vertices_accessor_index"), "set_vertices_accessor_index", "get_vertices_accessor_index");
}
