#pragma once

#include "../tetra/tetra_material_4d.h"
#include "poly_mesh_4d.h"

class PolyMaterial4D : public TetraMaterial4D {
	GDCLASS(PolyMaterial4D, Material4D);

	PackedColorArray _poly_albedo_color_array;

protected:
	static void _bind_methods();
	void _validate_property(PropertyInfo &p_property) const;

public:
	void populate_albedo_color_array_for_poly_mesh(const Ref<TetraMesh4D> &p_poly_mesh);

	PackedColorArray get_poly_albedo_color_array() const { return _poly_albedo_color_array; }
	void set_poly_albedo_color_array(const PackedColorArray &p_colors);

	PolyMaterial4D();
};
