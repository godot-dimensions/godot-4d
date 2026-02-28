#pragma once

#include "g4mf_item_4d.h"

class G4MFState4D;
class MeshInstance4D;

class G4MFMeshInstance4D : public G4MFItem4D {
	GDCLASS(G4MFMeshInstance4D, G4MFItem4D);

	PackedInt32Array _material_indices;
	int _mesh_index = -1;

protected:
	static void _bind_methods();

public:
	PackedInt32Array get_material_indices() const { return _material_indices; }
	void set_material_indices(const PackedInt32Array &p_material_indices) { _material_indices = p_material_indices; }

	int get_mesh_index() const { return _mesh_index; }
	void set_mesh_index(const int p_mesh_index) { _mesh_index = p_mesh_index; }

	MeshInstance4D *import_generate_mesh_instance(const Ref<G4MFState4D> &p_g4mf_state) const;
	static Ref<G4MFMeshInstance4D> export_convert_mesh_instance(const Ref<G4MFState4D> &p_g4mf_state, const MeshInstance4D *p_mesh_instance);

	static Ref<G4MFMeshInstance4D> from_dictionary(const Dictionary &p_dict);
	Dictionary to_dictionary() const;
};
