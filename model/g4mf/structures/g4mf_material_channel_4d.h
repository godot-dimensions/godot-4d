#pragma once

#include "g4mf_item_4d.h"

#include "../../mesh/tetra/array_tetra_mesh_4d.h"
#include "../../mesh/tetra/tetra_material_4d.h"
#include "../../mesh/wire/array_wire_mesh_4d.h"
#include "../../mesh/wire/wire_material_4d.h"

class G4MFState4D;

class G4MFMaterialChannel4D : public G4MFItem4D {
	GDCLASS(G4MFMaterialChannel4D, G4MFItem4D);

	Color _factor = Color(-1, -1, -1, -1);
	int _per_cell_accessor_index = -1;
	int _per_edge_accessor_index = -1;
	int _per_vertex_accessor_index = -1;
	int _cell_texture_map_accessor_index = -1;
	int _cell_texture_index = -1;

protected:
	static void _bind_methods();

public:
	Color get_factor() const { return _factor; }
	void set_factor(const Color &p_color) { _factor = p_color; }

	int get_per_cell_accessor_index() const { return _per_cell_accessor_index; }
	void set_per_cell_accessor_index(const int p_index) { _per_cell_accessor_index = p_index; }

	int get_per_edge_accessor_index() const { return _per_edge_accessor_index; }
	void set_edge_colors_accessor_index(const int p_index) { _per_edge_accessor_index = p_index; }

	int get_per_vertex_accessor_index() const { return _per_vertex_accessor_index; }
	void set_per_vertex_accessor_index(const int p_index) { _per_vertex_accessor_index = p_index; }

	int get_cell_texture_map_accessor_index() const { return _cell_texture_map_accessor_index; }
	void set_cell_texture_map_accessor_index(const int p_index) { _cell_texture_map_accessor_index = p_index; }

	int get_cell_texture_index() const { return _cell_texture_index; }
	void set_cell_texture_index(const int p_index) { _cell_texture_index = p_index; }

	bool is_equal_exact(const Ref<G4MFMaterialChannel4D> &p_other) const;
	PackedColorArray load_cell_colors(const Ref<G4MFState4D> &p_g4mf_state) const;
	PackedColorArray load_edge_colors(const Ref<G4MFState4D> &p_g4mf_state) const;
	PackedColorArray load_vertex_colors(const Ref<G4MFState4D> &p_g4mf_state) const;

	static Ref<G4MFMaterialChannel4D> from_dictionary(const Dictionary &p_dict);
	Dictionary to_dictionary() const;
};
