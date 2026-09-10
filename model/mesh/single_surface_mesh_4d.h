#pragma once

#include "material_4d.h"
#include "mesh_4d.h"

class ArrayWireMesh4D;
class WireMesh4D;

class SingleSurfaceMesh4D : public Mesh4D {
	GDCLASS(SingleSurfaceMesh4D, Mesh4D);

	Ref<Material4D> _material;

protected:
	static void _bind_methods();

public:
	bool has_edge_indices(int p_first, int p_second);

	Ref<ArrayWireMesh4D> to_array_wire_mesh();
	virtual Ref<WireMesh4D> to_wire_mesh();

	virtual const Rect4 &get_rect_bounds() override;

	Ref<Material4D> get_material() const;
	void set_material(const Ref<Material4D> &p_material);
	virtual Ref<Material4D> get_fallback_material();
	virtual void validate_material_for_mesh(const Ref<Material4D> &p_material) override;

	virtual PackedInt32Array get_edge_indices();
	virtual PackedVector4Array get_edge_positions();
	virtual PackedVector4Array get_vertex_positions();
	virtual PackedVector4Array get_normal_values();
	virtual PackedVector3Array get_texture_map_values();

	GDVIRTUAL0R(PackedInt32Array, _get_edge_indices);
	GDVIRTUAL0R(PackedVector4Array, _get_edge_positions);
	GDVIRTUAL0R(PackedVector4Array, _get_vertex_positions);
	GDVIRTUAL0R(PackedVector4Array, _get_normal_values);
	GDVIRTUAL0R(PackedVector3Array, _get_texture_map_values);

	GDVIRTUAL0R(Ref<Material4D>, _get_fallback_material);
};
