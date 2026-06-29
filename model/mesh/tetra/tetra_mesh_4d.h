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
	PackedVector4Array _simplex_positions_cache;
	PackedFloat64Array _nearest_tetra_inverse_metric_cache;

	Ref<ArrayMesh> convert_texture_map_to_mesh(const PackedVector3Array &p_texture_map);
	virtual void update_cross_section_mesh() override;

public:
	// Nearest point and signed distance.
	void populate_inverse_metric_cache();
	real_t get_signed_distance_to_mesh(const Vector4 &p_local_point, Vector4 *r_nearest_point_on_tet, int *r_tetrahedron_index);
	real_t get_signed_distance_to_mesh_bind(const Vector4 &p_local_point);

	// Raycast.
	bool raycast_intersects_fast(const Vector4 &p_local_from, const Vector4 &p_local_direction);
	Dictionary raycast_intersects(const Vector4 &p_local_from, const Vector4 &p_local_direction);

	// Cache and validation.
	void tetra_mesh_clear_cache();
	virtual bool validate_mesh_data() override;
	virtual void validate_material_for_mesh(const Ref<Material4D> &p_material) override;

	// Conversion.
	Ref<ArrayTetraMesh4D> to_array_tetra_mesh();
	virtual Ref<TetraMesh4D> to_tetra_mesh();
	virtual Ref<ArrayMesh> export_texture_map_mesh();

	// Getters.
	virtual PackedInt32Array get_simplex_cell_indices();
	virtual PackedVector4Array get_simplex_cell_boundary_normals();
	virtual PackedVector4Array get_simplex_cell_vertex_normals();
	virtual PackedVector3Array get_simplex_cell_texture_map();
	PackedVector4Array get_simplex_cell_positions();

	// Edges.
	static PackedInt32Array calculate_edge_indices_from_simplex_cell_indices(const PackedInt32Array &p_simplex_cell_indices, const bool p_deduplicate = true);
	virtual PackedInt32Array get_edge_indices() override;
	virtual PackedVector4Array get_edge_positions() override;

	// Fallback material.
	Ref<Material4D> get_fallback_material() override;
	static void init_fallback_material();
	static void cleanup_fallback_material();

	GDVIRTUAL0R(PackedInt32Array, _get_simplex_cell_indices);
	GDVIRTUAL0R(PackedVector4Array, _get_simplex_cell_boundary_normals);
	GDVIRTUAL0R(PackedVector4Array, _get_simplex_cell_vertex_normals);
	GDVIRTUAL0R(PackedVector3Array, _get_simplex_cell_texture_map);

private:
	static Ref<TetraMaterial4D> _fallback_material;
};
