#pragma once

#include "g4mf_mesh_surface_binding_4d.h"

#include "../../mesh/poly/array_poly_mesh_4d.h"
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
	PackedInt32Array _geometry_accessor_indices;
	int _edges_accessor_index = -1;
	int _material_index = -1;
	int _seams_accessor_index = -1;
	int _simplexes_accessor_index = -1;
	bool _polytope_simplexes = false;

	void _convert_poly_mesh_surface_for_state(const Ref<G4MFState4D> &p_g4mf_state, const Ref<PolyMesh4D> &p_poly_mesh, const bool p_deduplicate, PackedVector4Array &r_unique_normal_values, PackedVector3Array &r_unique_texture_map_values);
	void _convert_tetra_mesh_surface_for_state(const Ref<G4MFState4D> &p_g4mf_state, const Ref<TetraMesh4D> &p_tetra_mesh, const bool p_deduplicate, PackedVector4Array &r_unique_normal_values, PackedVector3Array &r_unique_texture_map_values);

protected:
	static void _bind_methods();

public:
	int get_edges_accessor_index() const { return _edges_accessor_index; }
	void set_edges_accessor_index(const int p_edges_accessor_index) { _edges_accessor_index = p_edges_accessor_index; }

	PackedInt32Array get_geometry_accessor_indices() const { return _geometry_accessor_indices; }
	void set_geometry_accessor_indices(const PackedInt32Array &p_geometry_accessor_indices) { _geometry_accessor_indices = p_geometry_accessor_indices; }

	int get_material_index() const { return _material_index; }
	void set_material_index(const int p_material_index) { _material_index = p_material_index; }

	Ref<G4MFMeshSurfaceBinding4D> get_normals_binding() const { return _normals_binding; }
	void set_normals_binding(const Ref<G4MFMeshSurfaceBinding4D> &p_normals_binding) { _normals_binding = p_normals_binding; }

	bool get_polytope_simplexes() const { return _polytope_simplexes; }
	void set_polytope_simplexes(const bool p_polytope_simplexes) { _polytope_simplexes = p_polytope_simplexes; }

	int get_seams_accessor_index() const { return _seams_accessor_index; }
	void set_seams_accessor_index(const int p_seams_accessor_index) { _seams_accessor_index = p_seams_accessor_index; }

	int get_simplexes_accessor_index() const { return _simplexes_accessor_index; }
	void set_simplexes_accessor_index(const int p_simplexes_accessor_index) { _simplexes_accessor_index = p_simplexes_accessor_index; }

	Ref<G4MFMeshSurfaceBinding4D> get_texture_map_binding() const { return _texture_map_binding; }
	void set_texture_map_binding(const Ref<G4MFMeshSurfaceBinding4D> &p_texture_map_binding) { _texture_map_binding = p_texture_map_binding; }

	bool is_equal_exact(const Ref<G4MFMeshSurface4D> &p_other) const;
	void convert_separated_geometry_into_packed(const Ref<G4MFState4D> &p_g4mf_state, const Vector<Vector<PackedInt32Array>> &_separated_geometry, const bool p_deduplicate);
	void convert_separated_geometry_into_packed_bind(const Ref<G4MFState4D> &p_g4mf_state, const TypedArray<Array> &p_separated_geometry, const bool p_deduplicate);
	Vector<Vector<PackedInt32Array>> load_geometry_separated(const Ref<G4MFState4D> &p_g4mf_state) const;
	TypedArray<Array> load_geometry_separated_bind(const Ref<G4MFState4D> &p_g4mf_state) const;
	PackedInt32Array load_edge_indices(const Ref<G4MFState4D> &p_g4mf_state) const;
	PackedInt32Array load_seam_indices(const Ref<G4MFState4D> &p_g4mf_state) const;
	PackedInt32Array load_simplex_indices(const Ref<G4MFState4D> &p_g4mf_state) const;

	Ref<ArrayPolyMesh4D> generate_poly_mesh_surface(const Ref<G4MFState4D> &p_g4mf_state, const PackedVector4Array &p_vertices) const;
	Ref<ArrayTetraMesh4D> generate_tetra_mesh_surface(const Ref<G4MFState4D> &p_g4mf_state, const PackedVector4Array &p_vertices) const;
	Ref<ArrayWireMesh4D> generate_wire_mesh_surface(const Ref<G4MFState4D> &p_g4mf_state, const PackedVector4Array &p_vertices) const;
	static Ref<G4MFMeshSurface4D> convert_mesh_surface_for_state(Ref<G4MFState4D> p_g4mf_state, const Ref<Mesh4D> &p_mesh, const bool p_deduplicate = true);

	static Ref<G4MFMeshSurface4D> from_dictionary(const Dictionary &p_dict);
	Dictionary to_dictionary() const;
};
