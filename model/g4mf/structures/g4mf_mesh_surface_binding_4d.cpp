#include "g4mf_mesh_surface_binding_4d.h"

#include "../g4mf_state_4d.h"

bool G4MFMeshSurfaceBindingGeometry4D::is_equal_exact(const Ref<G4MFMeshSurfaceBindingGeometry4D> &p_other) const {
	return (_indices_accessor_index == p_other->get_indices_accessor_index() &&
			_decompose_dimension == p_other->get_decompose_dimension() &&
			_geometry_dimension == p_other->get_geometry_dimension());
}

PackedInt32Array G4MFMeshSurfaceBindingGeometry4D::load_indices(const Ref<G4MFState4D> &p_g4mf_state) const {
	TypedArray<G4MFAccessor4D> state_accessors = p_g4mf_state->get_g4mf_accessors();
	ERR_FAIL_INDEX_V(_indices_accessor_index, state_accessors.size(), PackedInt32Array());
	const Ref<G4MFAccessor4D> accessor = state_accessors[_indices_accessor_index];
	return accessor->decode_int32s_from_bytes(p_g4mf_state);
}

bool G4MFMeshSurfaceBinding4D::is_equal_exact(const Ref<G4MFMeshSurfaceBinding4D> &p_other) const {
	const TypedArray<G4MFMeshSurfaceBindingGeometry4D> other_geometry_decompositions = p_other->get_geometry_bindings();
	if (_geometry_bindings.size() != other_geometry_decompositions.size()) {
		return false;
	}
	for (int i = 0; i < _geometry_bindings.size(); i++) {
		const Ref<G4MFMeshSurfaceBindingGeometry4D> geom = _geometry_bindings[i];
		const Ref<G4MFMeshSurfaceBindingGeometry4D> other = other_geometry_decompositions[i];
		if (!geom->is_equal_exact(other)) {
			return false;
		}
	}
	return (_per_simplex_accessor_index == p_other->get_per_simplex_accessor_index() &&
			_simplexes_accessor_index == p_other->get_simplexes_accessor_index() &&
			_values_accessor_index == p_other->get_values_accessor_index());
}

PackedInt32Array G4MFMeshSurfaceBinding4D::load_geometry_binding_indices(const Ref<G4MFState4D> &p_g4mf_state, const int p_geometry_dimension, const int p_decompose_dimension) const {
	ERR_FAIL_COND_V(p_g4mf_state.is_null(), PackedInt32Array());
	int accessor_index = -1;
	for (int i = 0; i < _geometry_bindings.size(); i++) {
		const Ref<G4MFMeshSurfaceBindingGeometry4D> geom = _geometry_bindings[i];
		if (geom->get_geometry_dimension() == p_geometry_dimension && geom->get_decompose_dimension() == p_decompose_dimension) {
			accessor_index = geom->get_indices_accessor_index();
			break;
		}
	}
	if (accessor_index == -1) {
		return PackedInt32Array();
	}
	TypedArray<G4MFAccessor4D> state_accessors = p_g4mf_state->get_g4mf_accessors();
	ERR_FAIL_INDEX_V(accessor_index, state_accessors.size(), PackedInt32Array());
	const Ref<G4MFAccessor4D> accessor = state_accessors[accessor_index];
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

PackedVector3Array G4MFMeshSurfaceBinding4D::load_values_as_vector3s(const Ref<G4MFState4D> &p_g4mf_state) const {
	TypedArray<G4MFAccessor4D> state_accessors = p_g4mf_state->get_g4mf_accessors();
	ERR_FAIL_INDEX_V(_values_accessor_index, state_accessors.size(), PackedVector3Array());
	const Ref<G4MFAccessor4D> accessor = state_accessors[_values_accessor_index];
	return accessor->decode_vector3s_from_bytes(p_g4mf_state);
}

PackedVector4Array G4MFMeshSurfaceBinding4D::load_values_as_vector4s(const Ref<G4MFState4D> &p_g4mf_state) const {
	TypedArray<G4MFAccessor4D> state_accessors = p_g4mf_state->get_g4mf_accessors();
	ERR_FAIL_INDEX_V(_values_accessor_index, state_accessors.size(), PackedVector4Array());
	const Ref<G4MFAccessor4D> accessor = state_accessors[_values_accessor_index];
	return accessor->decode_vector4s_from_bytes(p_g4mf_state);
}

Ref<G4MFMeshSurfaceBindingGeometry4D> G4MFMeshSurfaceBindingGeometry4D::from_dictionary(const Dictionary &p_dict) {
	Ref<G4MFMeshSurfaceBindingGeometry4D> geometry_binding;
	geometry_binding.instantiate();
	geometry_binding->read_item_entries_from_dictionary(p_dict);
#ifndef DISABLE_DEPRECATED
	if (p_dict.has("dimension")) {
		geometry_binding->set_decompose_dimension(int(p_dict["dimension"]));
	}
	if (p_dict.has("index")) {
		geometry_binding->set_geometry_dimension(2 + int(p_dict["index"]));
	}
#endif // DISABLE_DEPRECATED
	if (p_dict.has("accessor")) {
		geometry_binding->set_indices_accessor_index(int(p_dict["accessor"]));
	}
	if (p_dict.has("decomposeDimension")) {
		geometry_binding->set_decompose_dimension(int(p_dict["decomposeDimension"]));
	}
	if (p_dict.has("geometryDimension")) {
		geometry_binding->set_geometry_dimension(int(p_dict["geometryDimension"]));
	}
	return geometry_binding;
}

Dictionary G4MFMeshSurfaceBindingGeometry4D::to_dictionary() const {
	Dictionary dict = write_item_entries_to_dictionary();
	dict["accessor"] = _indices_accessor_index; // Required.
	if (_decompose_dimension != 0) {
		dict["decomposeDimension"] = _decompose_dimension;
	}
	dict["geometryDimension"] = _geometry_dimension; // Required.
	return dict;
}

Ref<G4MFMeshSurfaceBinding4D> G4MFMeshSurfaceBinding4D::from_dictionary(const Dictionary &p_dict) {
	Ref<G4MFMeshSurfaceBinding4D> binding;
	binding.instantiate();
	binding->read_item_entries_from_dictionary(p_dict);
#ifndef DISABLE_DEPRECATED
	if (p_dict.has("edges")) {
		Ref<G4MFMeshSurfaceBindingGeometry4D> geom;
		geom.instantiate();
		geom->set_geometry_dimension(1);
		geom->set_decompose_dimension(0);
		geom->set_indices_accessor_index(int(p_dict["edges"]));
		TypedArray<G4MFMeshSurfaceBindingGeometry4D> geometry_bindings;
		geometry_bindings.append(geom);
		binding->set_geometry_bindings(geometry_bindings);
	}
	if (p_dict.has("perEdge")) {
		Ref<G4MFMeshSurfaceBindingGeometry4D> geom;
		geom.instantiate();
		geom->set_geometry_dimension(1);
		geom->set_decompose_dimension(1);
		geom->set_indices_accessor_index(int(p_dict["perEdge"]));
		TypedArray<G4MFMeshSurfaceBindingGeometry4D> geometry_bindings;
		geometry_bindings.append(geom);
		binding->set_geometry_bindings(geometry_bindings);
	}
	if (p_dict.has("vertices")) {
		Ref<G4MFMeshSurfaceBindingGeometry4D> geom;
		geom.instantiate();
		geom->set_geometry_dimension(0);
		geom->set_decompose_dimension(0);
		geom->set_indices_accessor_index(int(p_dict["vertices"]));
		TypedArray<G4MFMeshSurfaceBindingGeometry4D> geometry_bindings;
		geometry_bindings.append(geom);
		binding->set_geometry_bindings(geometry_bindings);
	}
#endif // DISABLE_DEPRECATED
	if (p_dict.has("geometry")) {
		Array geometry_array = p_dict["geometry"];
		TypedArray<G4MFMeshSurfaceBindingGeometry4D> geometry_bindings;
		for (int i = 0; i < geometry_array.size(); i++) {
			Dictionary geom_dict = geometry_array[i];
			Ref<G4MFMeshSurfaceBindingGeometry4D> geom = G4MFMeshSurfaceBindingGeometry4D::from_dictionary(geom_dict);
			geometry_bindings.append(geom);
		}
		binding->set_geometry_bindings(geometry_bindings);
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
	return binding;
}

Dictionary G4MFMeshSurfaceBinding4D::to_dictionary() const {
	Dictionary dict = write_item_entries_to_dictionary();
	if (_geometry_bindings.size() > 0) {
		Array geometry_array;
		for (int i = 0; i < _geometry_bindings.size(); i++) {
			Ref<G4MFMeshSurfaceBindingGeometry4D> geom = _geometry_bindings[i];
			geometry_array.append(geom->to_dictionary());
		}
		dict["geometry"] = geometry_array;
	}
	if (_per_simplex_accessor_index >= 0) {
		dict["perSimplex"] = _per_simplex_accessor_index;
	}
	if (_simplexes_accessor_index >= 0) {
		dict["simplexes"] = _simplexes_accessor_index;
	}
	dict["values"] = _values_accessor_index; // Required.
	return dict;
}

void G4MFMeshSurfaceBindingGeometry4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_indices_accessor_index"), &G4MFMeshSurfaceBindingGeometry4D::get_indices_accessor_index);
	ClassDB::bind_method(D_METHOD("set_indices_accessor_index", "indices_accessor_index"), &G4MFMeshSurfaceBindingGeometry4D::set_indices_accessor_index);
	ClassDB::bind_method(D_METHOD("get_decompose_dimension"), &G4MFMeshSurfaceBindingGeometry4D::get_decompose_dimension);
	ClassDB::bind_method(D_METHOD("set_decompose_dimension", "decompose_dimension"), &G4MFMeshSurfaceBindingGeometry4D::set_decompose_dimension);
	ClassDB::bind_method(D_METHOD("get_geometry_dimension"), &G4MFMeshSurfaceBindingGeometry4D::get_geometry_dimension);
	ClassDB::bind_method(D_METHOD("set_geometry_dimension", "source_dimension"), &G4MFMeshSurfaceBindingGeometry4D::set_geometry_dimension);

	ClassDB::bind_method(D_METHOD("is_equal_exact", "other"), &G4MFMeshSurfaceBindingGeometry4D::is_equal_exact);
	ClassDB::bind_method(D_METHOD("load_indices", "g4mf_state"), &G4MFMeshSurfaceBindingGeometry4D::load_indices);
	ClassDB::bind_static_method("G4MFMeshSurfaceBindingGeometry4D", D_METHOD("from_dictionary", "dict"), &G4MFMeshSurfaceBindingGeometry4D::from_dictionary);
	ClassDB::bind_method(D_METHOD("to_dictionary"), &G4MFMeshSurfaceBindingGeometry4D::to_dictionary);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "indices_accessor_index"), "set_indices_accessor_index", "get_indices_accessor_index");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "decompose_dimension"), "set_decompose_dimension", "get_decompose_dimension");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "geometry_dimension"), "set_geometry_dimension", "get_geometry_dimension");
}

void G4MFMeshSurfaceBinding4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_geometry_bindings"), &G4MFMeshSurfaceBinding4D::get_geometry_bindings);
	ClassDB::bind_method(D_METHOD("set_geometry_bindings", "geometry_bindings"), &G4MFMeshSurfaceBinding4D::set_geometry_bindings);
	ClassDB::bind_method(D_METHOD("get_per_simplex_accessor_index"), &G4MFMeshSurfaceBinding4D::get_per_simplex_accessor_index);
	ClassDB::bind_method(D_METHOD("set_per_simplex_accessor_index", "per_simplex_accessor_index"), &G4MFMeshSurfaceBinding4D::set_per_simplex_accessor_index);
	ClassDB::bind_method(D_METHOD("get_simplexes_accessor_index"), &G4MFMeshSurfaceBinding4D::get_simplexes_accessor_index);
	ClassDB::bind_method(D_METHOD("set_simplexes_accessor_index", "simplexes_accessor_index"), &G4MFMeshSurfaceBinding4D::set_simplexes_accessor_index);
	ClassDB::bind_method(D_METHOD("get_values_accessor_index"), &G4MFMeshSurfaceBinding4D::get_values_accessor_index);
	ClassDB::bind_method(D_METHOD("set_values_accessor_index", "values_accessor_index"), &G4MFMeshSurfaceBinding4D::set_values_accessor_index);

	ClassDB::bind_method(D_METHOD("load_per_simplex_indices", "g4mf_state"), &G4MFMeshSurfaceBinding4D::load_per_simplex_indices);
	ClassDB::bind_method(D_METHOD("load_simplex_indices", "g4mf_state"), &G4MFMeshSurfaceBinding4D::load_simplex_indices);

	ClassDB::bind_method(D_METHOD("load_values_as_variants", "g4mf_state", "variant_type"), &G4MFMeshSurfaceBinding4D::load_values_as_variants);
	ClassDB::bind_method(D_METHOD("load_values_as_colors", "g4mf_state"), &G4MFMeshSurfaceBinding4D::load_values_as_colors);
	ClassDB::bind_method(D_METHOD("load_values_as_vector3s", "g4mf_state"), &G4MFMeshSurfaceBinding4D::load_values_as_vector3s);
	ClassDB::bind_method(D_METHOD("load_values_as_vector4s", "g4mf_state"), &G4MFMeshSurfaceBinding4D::load_values_as_vector4s);

	ClassDB::bind_method(D_METHOD("is_equal_exact", "other"), &G4MFMeshSurfaceBinding4D::is_equal_exact);
	ClassDB::bind_static_method("G4MFMeshSurfaceBinding4D", D_METHOD("from_dictionary", "dict"), &G4MFMeshSurfaceBinding4D::from_dictionary);
	ClassDB::bind_method(D_METHOD("to_dictionary"), &G4MFMeshSurfaceBinding4D::to_dictionary);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "geometry_bindings", PROPERTY_HINT_ARRAY_TYPE, "G4MFMeshSurfaceBindingGeometry4D"), "set_geometry_bindings", "get_geometry_bindings");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "per_simplex_accessor_index"), "set_per_simplex_accessor_index", "get_per_simplex_accessor_index");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "simplexes_accessor_index"), "set_simplexes_accessor_index", "get_simplexes_accessor_index");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "values_accessor_index"), "set_values_accessor_index", "get_values_accessor_index");
}
