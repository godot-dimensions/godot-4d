#pragma once

#include "../tetra/tetra_material_4d.h"
#include "../tetra/tetra_mesh_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/templates/hash_set.hpp>
#elif GODOT_MODULE
class ArrayMesh;
#endif

class ArrayPolyMesh4D;

class PolyMesh4D : public TetraMesh4D {
	GDCLASS(PolyMesh4D, TetraMesh4D);

	PackedInt32Array _simplex_cell_indices_cache;
	PackedInt32Array _simplex_cell_indices_source_poly_cells;
	PackedVector4Array _simplex_cell_vertices_cache; // Superset of the polytope cell vertices.
	PackedVector4Array _simplex_cell_normals_cache;
	PackedVector3Array _simplex_cell_uvw_texture_map_cache;
	bool _is_poly_mesh_data_valid = false;

	static Vector3 _average_vector3(const PackedVector3Array &p_vector3_array);
	static int64_t _append_vertex_internal(PackedVector4Array &r_vertices, const Vector4 &p_vertex, const bool p_deduplicate);
	static int32_t _compare_triangulation_alignment(const PackedInt32Array &p_a, const PackedInt32Array &p_b);
	static inline bool _do_edges_have_common_vertex(const int32_t p_edge1_a, const int32_t p_edge1_b, const int32_t p_edge2_a, const int32_t p_edge2_b) {
		return (p_edge1_a == p_edge2_a) || (p_edge1_a == p_edge2_b) || (p_edge1_b == p_edge2_a) || (p_edge1_b == p_edge2_b);
	}

	static PackedInt32Array _get_canonical_span_vertex_index_sequence(const Vector<Vector<PackedInt32Array>> &p_poly_cell_indices, const PackedInt32Array &p_all_edge_indices, const int64_t p_indices_dim_index, const int64_t p_which_cell);
	static PackedInt32Array _get_cell_face_4_vertex_index_sequence(const PackedInt32Array &p_all_edge_indices, const PackedInt32Array &p_face1_edge_indices, const PackedInt32Array &p_face2_edge_indices);
	static PackedInt32Array _get_face_edge_3_vertex_index_sequence(const int32_t p_edge1_a, const int32_t p_edge1_b, const int32_t p_edge2_a, const int32_t p_edge2_b);
	static PackedInt32Array _get_edges_of_poly_cell(const Vector<Vector<PackedInt32Array>> &p_poly_cell_indices, const int64_t p_cell_dim_index, const int64_t p_which_cell);
	static PackedInt32Array _get_vertex_indices_of_poly_cell(const Vector<Vector<PackedInt32Array>> &p_poly_cell_indices, const PackedInt32Array &p_all_edge_indices, const int64_t p_cell_dim_index, const int64_t p_which_cell, const bool p_start_with_canonical_span);
	static PackedInt32Array _get_vertex_indices_of_face(const PackedInt32Array &p_all_edge_indices, const PackedInt32Array &p_face_edge_indices);
	static PackedInt32Array _triangulate_face_vertex_indices(const PackedInt32Array &p_face_vertex_indices, const int32_t p_pivot_attempt);
	void _decompose_boundary_cells_into_simplexes(const bool p_force_align_triangulations);

protected:
	// A decent margin under Mesh4D::MAX_VERTICES to avoid overflows.
	static constexpr int64_t MAX_POLY_VERTICES = Mesh4D::MAX_VERTICES * 3 / 4;

	static void _bind_methods();
	bool is_poly_mesh_data_valid();
	void reset_poly_mesh_data_validation();
	virtual bool validate_mesh_data() override;
	virtual bool _validate_poly_mesh_data_only();

	// Protected helper functions used by both PolyMesh4D and ArrayPolyMesh4D.
	Vector<PackedInt32Array> _get_vertex_indices_of_boundary_cells(const Vector<Vector<PackedInt32Array>> &p_poly_cell_indices, const PackedInt32Array &p_all_edge_indices, const bool p_start_with_canonical_span);
	PackedVector4Array _compute_boundary_normals_based_on_cell_orientation(const Vector<PackedInt32Array> &p_boundary_cell_vertex_indices, const bool p_keep_existing);

public:
	Vector<PackedInt32Array> get_all_face_vertex_indices();
	TypedArray<PackedInt32Array> get_all_face_vertex_indices_bind();
	TypedArray<PackedInt32Array> get_all_cell_vertex_indices_bind(const bool p_start_with_canonical_span);
	void poly_mesh_clear_cache(const bool p_normals_only = false);
	Ref<ArrayPolyMesh4D> to_array_poly_mesh();

	virtual Vector<Vector<PackedInt32Array>> get_poly_cell_indices();
	virtual PackedVector4Array get_poly_cell_vertices();
	virtual PackedVector4Array get_poly_cell_boundary_normals();
	virtual Vector<PackedVector4Array> get_poly_cell_vertex_normals();
	virtual Vector<PackedVector3Array> get_poly_cell_texture_map();
	virtual HashSet<int32_t> get_seam_face_indices() const { return HashSet<int32_t>(); }
	TypedArray<Array> get_poly_cell_indices_bind();
	TypedArray<PackedVector4Array> get_poly_cell_vertex_normals_bind();
	TypedArray<PackedVector3Array> get_poly_cell_texture_map_bind();

	virtual PackedInt32Array get_simplex_cell_indices() override;
	virtual PackedVector4Array get_simplex_cell_boundary_normals() override;
	virtual PackedVector3Array get_simplex_cell_texture_map() override;
	virtual PackedVector4Array get_vertices() override;

	GDVIRTUAL0R(TypedArray<Array>, _get_poly_cell_indices);
	GDVIRTUAL0R(PackedVector4Array, _get_poly_cell_vertices);
	GDVIRTUAL0R(PackedVector4Array, _get_poly_cell_boundary_normals);
	GDVIRTUAL0R(TypedArray<PackedVector4Array>, _get_poly_cell_vertex_normals);
	GDVIRTUAL0R(TypedArray<PackedVector3Array>, _get_poly_cell_texture_map);
};
