#pragma once

#include "g4mf_mesh_surface_binding_4d.h"

#include "../../mesh/tetra/array_tetra_mesh_4d.h"
#include "../../mesh/tetra/tetra_material_4d.h"
#include "../../mesh/wire/array_wire_mesh_4d.h"
#include "../../mesh/wire/wire_material_4d.h"

class G4MFState4D;

class G4MFMaterialChannel4D : public G4MFItem4D {
	GDCLASS(G4MFMaterialChannel4D, G4MFItem4D);

	Ref<G4MFMeshSurfaceBinding4D> _element_map_binding;
	Ref<G4MFMeshSurfaceBinding4D> _texture_map_binding;
	Color _factor = Color(-1, -1, -1, -1);
	int _texture_index = -1;

protected:
	static void _bind_methods();

public:
	Ref<G4MFMeshSurfaceBinding4D> get_element_map_binding() const { return _element_map_binding; }
	void set_element_map_binding(const Ref<G4MFMeshSurfaceBinding4D> &p_element_map_binding) { _element_map_binding = p_element_map_binding; }

	Color get_factor() const { return _factor; }
	void set_factor(const Color &p_color) { _factor = p_color; }

	int get_texture_index() const { return _texture_index; }
	void set_texture_index(const int p_index) { _texture_index = p_index; }

	Ref<G4MFMeshSurfaceBinding4D> get_texture_map_binding() const { return _texture_map_binding; }
	void set_texture_map_binding(const Ref<G4MFMeshSurfaceBinding4D> &p_texture_map_binding) { _texture_map_binding = p_texture_map_binding; }

	bool is_equal_exact(const Ref<G4MFMaterialChannel4D> &p_other) const;

	static Ref<G4MFMaterialChannel4D> from_dictionary(const Dictionary &p_dict);
	Dictionary to_dictionary() const;
};
