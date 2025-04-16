#pragma once

#include "g4mf_item_4d.h"

#include "../../tetra/array_tetra_mesh_4d.h"
#include "../../wire/array_wire_mesh_4d.h"

#if GDEXTENSION
#include <godot_cpp/templates/hash_set.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#elif GODOT_MODULE
#include "core/variant/typed_array.h"
#endif

class G4MFState4D;

class G4MFMeshSurface4D : public G4MFItem4D {
	GDCLASS(G4MFMeshSurface4D, G4MFItem4D);

	int _cells_accessor_index = -1;
	int _cell_normals_accessor_index = -1;
	int _edges_accessor_index = -1;
	int _vertices_accessor_index = -1;
	bool _polytope_cells = false;

protected:
	static void _bind_methods();

public:
	int get_cells_accessor_index() const { return _cells_accessor_index; }
	void set_cells_accessor_index(const int p_cells_accessor_index) { _cells_accessor_index = p_cells_accessor_index; }

	int get_cell_normals_accessor_index() const { return _cell_normals_accessor_index; }
	void set_cell_normals_accessor_index(const int p_cell_normals_accessor_index) { _cell_normals_accessor_index = p_cell_normals_accessor_index; }

	int get_edges_accessor_index() const { return _edges_accessor_index; }
	void set_edges_accessor_index(const int p_edges_accessor_index) { _edges_accessor_index = p_edges_accessor_index; }

	int get_vertices_accessor_index() const { return _vertices_accessor_index; }
	void set_vertices_accessor_index(const int p_vertices_accessor_index) { _vertices_accessor_index = p_vertices_accessor_index; }

	bool get_polytope_cells() const { return _polytope_cells; }
	void set_polytope_cells(const bool p_polytope_cells) { _polytope_cells = p_polytope_cells; }

	bool is_equal_exact(const Ref<G4MFMeshSurface4D> &p_other) const;
	PackedInt32Array load_cell_indices(const Ref<G4MFState4D> &p_g4mf_state) const;
	PackedVector4Array load_cell_normals(const Ref<G4MFState4D> &p_g4mf_state) const;
	PackedInt32Array load_edge_indices(const Ref<G4MFState4D> &p_g4mf_state) const;
	PackedVector4Array load_vertices(const Ref<G4MFState4D> &p_g4mf_state) const;

	Ref<ArrayTetraMesh4D> generate_tetra_mesh_surface(const Ref<G4MFState4D> &p_g4mf_state) const;
	Ref<ArrayWireMesh4D> generate_wire_mesh_surface(const Ref<G4MFState4D> &p_g4mf_state) const;
	static Ref<G4MFMeshSurface4D> convert_mesh_surface_for_state(Ref<G4MFState4D> p_g4mf_state, const Ref<Mesh4D> &p_mesh, const bool p_deduplicate = true);

	static Ref<G4MFMeshSurface4D> from_dictionary(const Dictionary &p_dict);
	Dictionary to_dictionary() const;
};
