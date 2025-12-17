#pragma once

#include "material_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/array_mesh.hpp>
#elif GODOT_MODULE
#include "scene/resources/mesh.h"
#endif

class ArrayWireMesh4D;
class WireMesh4D;

class Mesh4D : public Resource {
	GDCLASS(Mesh4D, Resource);

	Ref<Material4D> _material;
	bool _is_mesh_data_valid = false;
	bool _is_cross_section_mesh_dirty = true;

protected:
	// Slightly under the 32-bit integer limit to avoid overflows.
	static constexpr int64_t MAX_VERTICES = 2147483640;
	Ref<ArrayMesh> _cross_section_mesh;

	static void _bind_methods();
	virtual bool validate_mesh_data();
	// Call when the mesh is modified to indicate that
	// the 3D mesh used for cross-section rendering needs to be updated.
	void mark_cross_section_mesh_dirty() { _is_cross_section_mesh_dirty = true; };
	// Called when the cross-section mesh is requested and the cross-section mesh has been marked dirty.
	// Update the mesh referenced by _cross_section_mesh to match the current state of the mesh.
	virtual void update_cross_section_mesh();

public:
	static PackedInt32Array deduplicate_edge_indices(const PackedInt32Array &p_items);
	bool has_edge_indices(int p_first, int p_second);

	bool is_mesh_data_valid();
	void reset_mesh_data_validation();
	virtual void validate_material_for_mesh(const Ref<Material4D> &p_material);

	Ref<ArrayWireMesh4D> to_array_wire_mesh();
	virtual Ref<WireMesh4D> to_wire_mesh();
	// Returns a reference to the mesh used for cross-section rendering.
	Ref<ArrayMesh> get_cross_section_mesh();

	Ref<Material4D> get_material() const;
	void set_material(const Ref<Material4D> &p_material);
	virtual Ref<Material4D> get_fallback_material();

	virtual PackedInt32Array get_edge_indices();
	virtual PackedVector4Array get_edge_positions();
	virtual PackedVector4Array get_vertices();

	GDVIRTUAL0R(PackedInt32Array, _get_edge_indices);
	GDVIRTUAL0R(PackedVector4Array, _get_edge_positions);
	GDVIRTUAL0R(PackedVector4Array, _get_vertices);
	GDVIRTUAL0R(bool, _validate_mesh_data);
	GDVIRTUAL0R(Ref<Material4D>, _get_fallback_material);
	GDVIRTUAL0(_update_cross_section_mesh);
	GDVIRTUAL1(_validate_material_for_mesh, const Ref<Material4D> &);
};
