#pragma once

#if GDEXTENSION
#include <godot_cpp/core/gdvirtual.gen.inc>
#endif

#include "material_4d.h"

class ArrayWireMesh4D;
class WireMesh4D;

class Mesh4D : public Resource {
	GDCLASS(Mesh4D, Resource);

	Ref<Material4D> _material;

protected:
	static void _bind_methods();

public:
	static PackedInt32Array deduplicate_edge_indices(const PackedInt32Array &p_items);
	bool has_edge_indices(int p_first, int p_second);

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
};
