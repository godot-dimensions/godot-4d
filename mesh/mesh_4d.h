#pragma once

#include "material_4d.h"

class ArrayWireMesh4D;
class WireMesh4D;

class Mesh4D : public Resource {
	GDCLASS(Mesh4D, Resource);

	Ref<Material4D> _material;
	bool _is_mesh_data_valid = false;

protected:
	static void _bind_methods();
	virtual bool validate_mesh_data();

public:
	static PackedInt32Array deduplicate_edge_indices(const PackedInt32Array &p_items);
	bool has_edge_indices(int p_first, int p_second);

	bool is_mesh_data_valid();
	void reset_mesh_data_validation();
	virtual void validate_material_for_mesh(const Ref<Material4D> &p_material);

	Ref<ArrayWireMesh4D> to_array_wire_mesh();
	virtual Ref<WireMesh4D> to_wire_mesh();

	Ref<Material4D> get_material() const;
	void set_material(const Ref<Material4D> &p_material);

	virtual PackedInt32Array get_edge_indices();
	virtual PackedVector4Array get_edge_positions();
	virtual PackedVector4Array get_vertices();

	GDVIRTUAL0R(PackedInt32Array, _get_edge_indices);
	GDVIRTUAL0R(PackedVector4Array, _get_edge_positions);
	GDVIRTUAL0R(PackedVector4Array, _get_vertices);
	GDVIRTUAL0R(bool, _validate_mesh_data);
	GDVIRTUAL1(_validate_material_for_mesh, const Ref<Material4D> &);
};
