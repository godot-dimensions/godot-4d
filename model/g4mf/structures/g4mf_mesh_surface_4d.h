#pragma once

#include "g4mf_mesh_surface_binding_4d.h"

#include "../../mesh/tetra/array_tetra_mesh_4d.h"
#include "../../mesh/wire/array_wire_mesh_4d.h"

#if GDEXTENSION
#include <godot_cpp/templates/hash_set.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#elif GODOT_MODULE
#include "core/variant/typed_array.h"
#endif

class G4MFState4D;

class G4MFMeshSurface4D : public G4MFItem4D {
	GDCLASS(G4MFMeshSurface4D, G4MFItem4D);

	Ref<G4MFMeshSurfaceBinding4D> _normals_binding;
	Ref<G4MFMeshSurfaceBinding4D> _texture_map_binding;
	int _simplexes_accessor_index = -1;
	int _edges_accessor_index = -1;
	int _material_index = -1;
	bool _polytope_simplexes = false;

protected:
	static void _bind_methods();

public:
	int get_simplexes_accessor_index() const { return _simplexes_accessor_index; }
	void set_simplexes_accessor_index(const int p_simplexes_accessor_index) { _simplexes_accessor_index = p_simplexes_accessor_index; }

	int get_edges_accessor_index() const { return _edges_accessor_index; }
	void set_edges_accessor_index(const int p_edges_accessor_index) { _edges_accessor_index = p_edges_accessor_index; }

	int get_material_index() const { return _material_index; }
	void set_material_index(const int p_material_index) { _material_index = p_material_index; }

	Ref<G4MFMeshSurfaceBinding4D> get_normals_binding() const { return _normals_binding; }
	void set_normals_binding(const Ref<G4MFMeshSurfaceBinding4D> &p_normals_binding) { _normals_binding = p_normals_binding; }

	bool get_polytope_simplexes() const { return _polytope_simplexes; }
	void set_polytope_simplexes(const bool p_polytope_simplexes) { _polytope_simplexes = p_polytope_simplexes; }

	Ref<G4MFMeshSurfaceBinding4D> get_texture_map_binding() const { return _texture_map_binding; }
	void set_texture_map_binding(const Ref<G4MFMeshSurfaceBinding4D> &p_texture_map_binding) { _texture_map_binding = p_texture_map_binding; }

	bool is_equal_exact(const Ref<G4MFMeshSurface4D> &p_other) const;
	PackedInt32Array load_simplex_indices(const Ref<G4MFState4D> &p_g4mf_state) const;
	PackedInt32Array load_edge_indices(const Ref<G4MFState4D> &p_g4mf_state) const;

	Ref<ArrayTetraMesh4D> generate_tetra_mesh_surface(const Ref<G4MFState4D> &p_g4mf_state, const PackedVector4Array &p_vertices) const;
	Ref<ArrayWireMesh4D> generate_wire_mesh_surface(const Ref<G4MFState4D> &p_g4mf_state, const PackedVector4Array &p_vertices) const;
	static Ref<G4MFMeshSurface4D> convert_mesh_surface_for_state(Ref<G4MFState4D> p_g4mf_state, const Ref<Mesh4D> &p_mesh, const bool p_deduplicate = true);

	static Ref<G4MFMeshSurface4D> from_dictionary(const Dictionary &p_dict);
	Dictionary to_dictionary() const;
};
