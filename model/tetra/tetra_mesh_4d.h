#pragma once

#include "../mesh_4d.h"
#include "tetra_material_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/array_mesh.hpp>
#elif GODOT_MODULE
class ArrayMesh;
#endif

class ArrayTetraMesh4D;

class TetraMesh4D : public Mesh4D {
	GDCLASS(TetraMesh4D, Mesh4D);

protected:
	static void _bind_methods();
	PackedInt32Array _edge_indices_cache;
	PackedVector4Array _edge_positions_cache;

	Ref<ArrayMesh> convert_texture_map_to_mesh(const PackedVector3Array &p_texture_map);
	virtual void update_cross_section_mesh() override;

public:
	void tetra_mesh_clear_cache();
	virtual void validate_material_for_mesh(const Ref<Material4D> &p_material) override;
	Ref<ArrayTetraMesh4D> to_array_tetra_mesh();

	virtual Ref<ArrayMesh> export_uvw_map_mesh();
	virtual PackedInt32Array get_cell_indices();
	virtual PackedVector4Array get_cell_positions();
	virtual PackedVector4Array get_cell_normals();
	virtual PackedVector3Array get_cell_uvw_map();

	static PackedInt32Array calculate_edge_indices_from_cell_indices(const PackedInt32Array &p_cell_indices, const bool p_deduplicate = true);
	virtual PackedInt32Array get_edge_indices() override;
	virtual PackedVector4Array get_edge_positions() override;

	Ref<Material4D> get_default_material() override;
	static void init_default_material();
	static void cleanup_default_material();

	GDVIRTUAL0R(PackedInt32Array, _get_cell_indices);
	GDVIRTUAL0R(PackedVector4Array, _get_cell_positions);
	GDVIRTUAL0R(PackedVector4Array, _get_cell_normals);
	GDVIRTUAL0R(PackedVector3Array, _get_cell_uvw_map);

private:
	static Ref<TetraMaterial4D> _default_material;
};
