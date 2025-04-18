#pragma once

#include "g4mf_material_channel_4d.h"

#include "../../tetra/array_tetra_mesh_4d.h"
#include "../../tetra/tetra_material_4d.h"
#include "../../wire/array_wire_mesh_4d.h"
#include "../../wire/wire_material_4d.h"

class G4MFState4D;

class G4MFMaterial4D : public G4MFItem4D {
	GDCLASS(G4MFMaterial4D, G4MFItem4D);

	Ref<G4MFMaterialChannel4D> _base_color_channel;
	Ref<G4MFMaterialChannel4D> _emissive_channel;
	Ref<G4MFMaterialChannel4D> _normal_channel;
	Ref<G4MFMaterialChannel4D> _orm_channel;

protected:
	static void _bind_methods();

public:
	Ref<G4MFMaterialChannel4D> get_base_color_channel() const { return _base_color_channel; }
	void set_base_color_channel(const Ref<G4MFMaterialChannel4D> &p_channel) { _base_color_channel = p_channel; }

	Ref<G4MFMaterialChannel4D> get_emissive_channel() const { return _emissive_channel; }
	void set_emissive_channel(const Ref<G4MFMaterialChannel4D> &p_channel) { _emissive_channel = p_channel; }

	Ref<G4MFMaterialChannel4D> get_normal_channel() const { return _normal_channel; }
	void set_normal_channel(const Ref<G4MFMaterialChannel4D> &p_channel) { _normal_channel = p_channel; }

	Ref<G4MFMaterialChannel4D> get_orm_channel() const { return _orm_channel; }
	void set_orm_channel(const Ref<G4MFMaterialChannel4D> &p_channel) { _orm_channel = p_channel; }

	bool is_equal_exact(const Ref<G4MFMaterial4D> &p_other) const;

	Ref<TetraMaterial4D> generate_tetra_material(const Ref<G4MFState4D> &p_g4mf_state) const;
	Ref<WireMaterial4D> generate_wire_material(const Ref<G4MFState4D> &p_g4mf_state) const;
	static int convert_material_into_state(Ref<G4MFState4D> p_g4mf_state, const Ref<Material4D> &p_material, const bool p_deduplicate = true);

	static Ref<G4MFMaterial4D> from_dictionary(const Dictionary &p_dict);
	Dictionary to_dictionary() const;
};
