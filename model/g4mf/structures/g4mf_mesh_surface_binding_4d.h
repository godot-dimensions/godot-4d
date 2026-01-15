#pragma once

#include "g4mf_item_4d.h"

#if GDEXTENSION
#include <godot_cpp/templates/hash_set.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#elif GODOT_MODULE
#include "core/variant/typed_array.h"
#endif

class G4MFState4D;

class G4MFMeshSurfaceBindingGeometry4D : public G4MFItem4D {
	GDCLASS(G4MFMeshSurfaceBindingGeometry4D, G4MFItem4D);

	int _indices_accessor_index = -1;
	int _decomposition_dimension = 0;
	int _geometry_index = -1;

protected:
	static void _bind_methods();

public:
	int get_indices_accessor_index() const { return _indices_accessor_index; }
	void set_indices_accessor_index(const int p_indices_accessor_index) { _indices_accessor_index = p_indices_accessor_index; }

	int get_decomposition_dimension() const { return _decomposition_dimension; }
	void set_decomposition_dimension(const int p_decomposition_dimension) { _decomposition_dimension = p_decomposition_dimension; }

	int get_geometry_index() const { return _geometry_index; }
	void set_geometry_index(const int p_geometry_index) { _geometry_index = p_geometry_index; }

	bool is_equal_exact(const Ref<G4MFMeshSurfaceBindingGeometry4D> &p_other) const;
	PackedInt32Array load_indices(const Ref<G4MFState4D> &p_g4mf_state) const;

	static Ref<G4MFMeshSurfaceBindingGeometry4D> from_dictionary(const Dictionary &p_dict);
	Dictionary to_dictionary() const;
};

class G4MFMeshSurfaceBinding4D : public G4MFItem4D {
	GDCLASS(G4MFMeshSurfaceBinding4D, G4MFItem4D);

	TypedArray<G4MFMeshSurfaceBindingGeometry4D> _geometry_decompositions;
	int _edges_accessor_index = -1;
	int _per_edge_accessor_index = -1;
	int _per_simplex_accessor_index = -1;
	int _simplexes_accessor_index = -1;
	int _values_accessor_index = -1;
	int _vertices_accessor_index = -1;

protected:
	static void _bind_methods();

public:
	TypedArray<G4MFMeshSurfaceBindingGeometry4D> get_geometry_decompositions() const { return _geometry_decompositions; }
	void set_geometry_decompositions(const TypedArray<G4MFMeshSurfaceBindingGeometry4D> &p_geometry_decompositions) {
		_geometry_decompositions = p_geometry_decompositions;
	}

	int get_edges_accessor_index() const { return _edges_accessor_index; }
	void set_edges_accessor_index(const int p_edges_accessor_index) { _edges_accessor_index = p_edges_accessor_index; }

	int get_per_edge_accessor_index() const { return _per_edge_accessor_index; }
	void set_per_edge_accessor_index(const int p_per_edge_accessor_index) { _per_edge_accessor_index = p_per_edge_accessor_index; }

	int get_per_simplex_accessor_index() const { return _per_simplex_accessor_index; }
	void set_per_simplex_accessor_index(const int p_per_simplex_accessor_index) { _per_simplex_accessor_index = p_per_simplex_accessor_index; }

	int get_simplexes_accessor_index() const { return _simplexes_accessor_index; }
	void set_simplexes_accessor_index(const int p_simplexes_accessor_index) { _simplexes_accessor_index = p_simplexes_accessor_index; }

	int get_values_accessor_index() const { return _values_accessor_index; }
	void set_values_accessor_index(const int p_values_accessor_index) { _values_accessor_index = p_values_accessor_index; }

	int get_vertices_accessor_index() const { return _vertices_accessor_index; }
	void set_vertices_accessor_index(const int p_vertices_accessor_index) { _vertices_accessor_index = p_vertices_accessor_index; }

	bool is_equal_exact(const Ref<G4MFMeshSurfaceBinding4D> &p_other) const;
	PackedInt32Array load_edge_indices(const Ref<G4MFState4D> &p_g4mf_state) const;
	PackedInt32Array load_per_edge_indices(const Ref<G4MFState4D> &p_g4mf_state) const;
	PackedInt32Array load_per_simplex_indices(const Ref<G4MFState4D> &p_g4mf_state) const;
	PackedInt32Array load_simplex_indices(const Ref<G4MFState4D> &p_g4mf_state) const;
	PackedInt32Array load_vertex_indices(const Ref<G4MFState4D> &p_g4mf_state) const;

	Array load_values_as_variants(const Ref<G4MFState4D> &p_g4mf_state, const Variant::Type p_variant_type) const;
	PackedColorArray load_values_as_colors(const Ref<G4MFState4D> &p_g4mf_state) const;

	static Ref<G4MFMeshSurfaceBinding4D> from_dictionary(const Dictionary &p_dict);
	Dictionary to_dictionary() const;
};
