#pragma once

#include "g4mf_mesh_surface_4d.h"

class G4MFState4D;

class G4MFMesh4D : public G4MFItem4D {
	GDCLASS(G4MFMesh4D, G4MFItem4D);

	TypedArray<G4MFMeshSurface4D> _surfaces;

	bool _can_generate_tetra_meshes_for_all_surfaces() const;
	Ref<ArrayTetraMesh4D> _generate_tetra_mesh_surface(const Ref<G4MFState4D> &p_g4mf_state, const int p_surface) const;
	Ref<ArrayWireMesh4D> _generate_wire_mesh_surface(const Ref<G4MFState4D> &p_g4mf_state, const int p_surface) const;

protected:
	static void _bind_methods();

public:
	TypedArray<G4MFMeshSurface4D> get_surfaces() const { return _surfaces; }
	void set_surfaces(const TypedArray<G4MFMeshSurface4D> &p_surfaces) { _surfaces = p_surfaces; }

	bool is_equal_exact(const Ref<G4MFMesh4D> &p_other) const;

	Ref<Mesh4D> generate_mesh(const Ref<G4MFState4D> &p_g4mf_state, const bool p_force_wireframe = false) const;
	static int convert_mesh_into_state(Ref<G4MFState4D> p_g4mf_state, const Ref<Mesh4D> &p_mesh, const bool p_deduplicate = true);

	static Ref<G4MFMesh4D> from_dictionary(const Dictionary &p_dict);
	Dictionary to_dictionary() const;
};
