#pragma once

#include "g4mf_item_4d.h"

#if GDEXTENSION
#include <godot_cpp/templates/hash_set.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#elif GODOT_MODULE
#include "core/variant/typed_array.h"
#endif

class G4MFState4D;

// https://github.com/godot-dimensions/g4mf/blob/main/specification/parts/mesh/bindings.md
class G4MFMeshSurfaceBindingGeometry4D : public G4MFItem4D {
	GDCLASS(G4MFMeshSurfaceBindingGeometry4D, G4MFItem4D);

	int _indices_accessor_index = -1;
	int _geometry_dimension = -1;
	int _decompose_dimension = 0;

protected:
	static void _bind_methods();

public:
	int get_indices_accessor_index() const { return _indices_accessor_index; }
	void set_indices_accessor_index(const int p_indices_accessor_index) { _indices_accessor_index = p_indices_accessor_index; }

	int get_decompose_dimension() const { return _decompose_dimension; }
	void set_decompose_dimension(const int p_decompose_dimension) { _decompose_dimension = p_decompose_dimension; }

	int get_geometry_dimension() const { return _geometry_dimension; }
	void set_geometry_dimension(const int p_geometry_dimension) { _geometry_dimension = p_geometry_dimension; }

	bool is_equal_exact(const Ref<G4MFMeshSurfaceBindingGeometry4D> &p_other) const;
	PackedInt32Array load_indices(const Ref<G4MFState4D> &p_g4mf_state) const;

	static Ref<G4MFMeshSurfaceBindingGeometry4D> from_dictionary(const Dictionary &p_dict);
	Dictionary to_dictionary() const;
};

class G4MFMeshSurfaceBinding4D : public G4MFItem4D {
	GDCLASS(G4MFMeshSurfaceBinding4D, G4MFItem4D);

	TypedArray<G4MFMeshSurfaceBindingGeometry4D> _geometry_bindings;
	int _per_simplex_accessor_index = -1;
	int _simplexes_accessor_index = -1;
	int _values_accessor_index = -1;

protected:
	static void _bind_methods();

public:
	TypedArray<G4MFMeshSurfaceBindingGeometry4D> get_geometry_bindings() const { return _geometry_bindings; }
	void set_geometry_bindings(const TypedArray<G4MFMeshSurfaceBindingGeometry4D> &p_geometry_bindings) {
		_geometry_bindings = p_geometry_bindings;
	}

	int get_per_simplex_accessor_index() const { return _per_simplex_accessor_index; }
	void set_per_simplex_accessor_index(const int p_per_simplex_accessor_index) { _per_simplex_accessor_index = p_per_simplex_accessor_index; }

	int get_simplexes_accessor_index() const { return _simplexes_accessor_index; }
	void set_simplexes_accessor_index(const int p_simplexes_accessor_index) { _simplexes_accessor_index = p_simplexes_accessor_index; }

	int get_values_accessor_index() const { return _values_accessor_index; }
	void set_values_accessor_index(const int p_values_accessor_index) { _values_accessor_index = p_values_accessor_index; }

	PackedInt32Array load_geometry_binding_indices(const Ref<G4MFState4D> &p_g4mf_state, const int p_geometry_dimension, const int p_decompose_dimension = 0) const;
	PackedInt32Array load_per_simplex_indices(const Ref<G4MFState4D> &p_g4mf_state) const;
	PackedInt32Array load_simplex_indices(const Ref<G4MFState4D> &p_g4mf_state) const;

	Array load_values_as_variants(const Ref<G4MFState4D> &p_g4mf_state, const Variant::Type p_variant_type) const;
	PackedColorArray load_values_as_colors(const Ref<G4MFState4D> &p_g4mf_state) const;
	PackedVector3Array load_values_as_vector3s(const Ref<G4MFState4D> &p_g4mf_state) const;
	PackedVector4Array load_values_as_vector4s(const Ref<G4MFState4D> &p_g4mf_state) const;

	bool is_equal_exact(const Ref<G4MFMeshSurfaceBinding4D> &p_other) const;
	static Ref<G4MFMeshSurfaceBinding4D> from_dictionary(const Dictionary &p_dict);
	Dictionary to_dictionary() const;
};
