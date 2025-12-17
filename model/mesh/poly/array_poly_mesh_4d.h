#pragma once

#include "../../../math/transform_4d.h"
#include "poly_mesh_4d.h"

class ArrayPolyMesh4D : public PolyMesh4D {
	GDCLASS(ArrayPolyMesh4D, PolyMesh4D);

public:
	enum ComputeNormalsMode {
		COMPUTE_NORMALS_MODE_CELL_ORIENTATION_ONLY,
		COMPUTE_NORMALS_MODE_FORCE_OUTWARD_FIX_CELL_ORIENTATION,
		COMPUTE_NORMALS_MODE_FORCE_OUTWARD_OVERRIDE_CELL_ORIENTATION,
	};

	enum UnwrapTextureMapMode {
		UNWRAP_MODE_AUTOMATIC,
		UNWRAP_MODE_EACH_CELL_FILLS,
		UNWRAP_MODE_TILE_CELLS,
		UNWRAP_MODE_ISLANDS_FROM_SEAMS,
	};

private:
	// 0: Each 2D face made up of 1D edge indices.
	// 1: Each 3D cell made up of 2D face indices (each 3D cell makes up the boundary/surface of the 4D mesh).
	// 2: Each 4D cell made up of 3D cell indices (optional, for encoding hypervolumes).
	Vector<Vector<PackedInt32Array>> _poly_cell_indices;
	PackedVector4Array _poly_cell_vertices;
	// Normals and UVW map always refer to 3D cells (the boundary/surface of the 4D mesh).
	PackedVector4Array _poly_cell_boundary_normals;
	Vector<PackedVector4Array> _poly_cell_vertex_normals;
	Vector<PackedVector3Array> _poly_cell_texture_map;
	// Seams always refer to 2D faces (the boundary between boundary 3D cells).
	HashSet<int32_t> _seam_face_indices;
	PackedInt32Array _edge_vertex_indices;

	static int64_t _append_edge_indices_internal(int32_t p_index_a, int32_t p_index_b, const bool p_deduplicate, PackedInt32Array &r_edge_indices);
	static int64_t _append_face_internal(const PackedInt32Array &p_face, Vector<PackedInt32Array> &r_all_face_edge_indices);
	static Vector4 _compute_cell_normal(const PackedInt32Array &p_cell_first_face, const PackedInt32Array &p_cell_second_face, const PackedInt32Array &p_edge_vertex_indices, const PackedVector4Array &p_vertices);
	static Vector<PackedInt32Array> _compose_triangles_into_faces(const PackedVector4Array &p_vertices, const Vector<PackedInt32Array> &p_triangle_vertex_indices, PackedInt32Array &r_edge_indices);
	static PackedInt32Array _save_triangle_vertex_indices_as_faces_and_cell(const Vector<PackedInt32Array> &p_last_triangle_vertex_indices, const Vector4 &p_last_simplex_normal, const PackedVector4Array &p_vertices, Vector<PackedInt32Array> &r_all_face_edge_indices, PackedInt32Array &r_edge_vertex_indices);
	PackedInt32Array _get_cell_4_vertices_starting_from_face(const int64_t p_cell, const int64_t p_start_face) const;
	void _get_cell_world_span_seed(const int64_t p_which_cell, Vector4 &r_world_x, Vector4 &r_world_y, Vector4 &r_world_z, int32_t &p_pivot) const;
	void _transform_cell_to_texture_space(const Transform4D &p_world_to_texcoord, const Vector<PackedInt32Array> &p_cell_vert, const int64_t p_cell_index, const int32_t p_pivot);
	Vector<PackedInt32Array> _get_face_to_cell_map() const;
	PackedInt32Array _collect_cells_in_island_internal(const int64_t p_start_cell, const Vector<PackedInt32Array> &p_face_to_cell_map);
	bool _unwrap_texture_map_island_cell(const PackedInt32Array &p_cells_in_island, const int64_t p_current_cell_index_index, const Vector<PackedInt32Array> &p_cell_vert);
	void _unwrap_texture_map_island_internal(const PackedInt32Array &p_cells_in_island, const bool p_keep_existing);
	void _fit_island_texture_map_into_aabb(const PackedInt32Array &p_cells_in_island, const AABB &p_target_aabb);
	static Vector3i _tiles_for_island_count(const int32_t p_island_count);
	static inline int32_t _ceil_div(int32_t p_a, int32_t p_b) {
		return (p_a + p_b - 1) / p_b;
	}

protected:
	static void _bind_methods();

public:
	int64_t append_edge_points(const Vector4 &p_point_a, const Vector4 &p_point_b, const bool p_deduplicate = true);
	int64_t append_edge_indices(int32_t p_index_a, int32_t p_index_b, const bool p_deduplicate = true);
	int append_vertex(const Vector4 &p_vertex, const bool p_deduplicate_vertices = true);
	PackedInt32Array append_vertices(const PackedVector4Array &p_vertices, const bool p_deduplicate_vertices = true);

	void calculate_boundary_normals(const ComputeNormalsMode p_mode, const bool p_keep_existing = false);
	void set_flat_shading_normals(const ComputeNormalsMode p_mode, const bool p_recalculate_boundary_normals = true);
	void calculate_seam_faces(const double p_angle_threshold_radians = Math_TAU / 8.0, const bool p_discard_seams_within_islands = false);
	PackedInt32Array collect_cells_in_island(const int64_t p_start_cell);
	Vector<PackedInt32Array> collect_all_islands();
	void unwrap_texture_map_island(const PackedInt32Array &p_cells_in_island, const bool p_keep_existing = false);
	void unwrap_texture_map(const UnwrapTextureMapMode p_mode, const double p_padding = 0.0, const bool p_keep_existing = false);
	void transform_texture_map(const Transform3D &p_transform);

	void merge_with(const Ref<PolyMesh4D> &p_other, const Transform4D &p_transform = Transform4D());
	void merge_with_bind(const Ref<PolyMesh4D> &p_other, const Vector4 &p_offset = Vector4(), const Projection &p_basis = Projection());
	static Ref<ArrayPolyMesh4D> reconstruct_from_tetra_mesh(const Ref<TetraMesh4D> &p_tetra_mesh);

	virtual PackedInt32Array get_edge_indices() override;
	void set_edge_vertex_indices(const PackedInt32Array &p_edge_indices);

	virtual Vector<Vector<PackedInt32Array>> get_poly_cell_indices() override;
	void set_poly_cell_indices(const Vector<Vector<PackedInt32Array>> &p_poly_cell_indices);
	void set_poly_cell_indices_bind(const TypedArray<Array> &p_poly_cell_indices);

	virtual PackedVector4Array get_poly_cell_boundary_normals() override;
	void set_poly_cell_boundary_normals(const PackedVector4Array &p_poly_cell_boundary_normals);

	virtual Vector<PackedVector4Array> get_poly_cell_vertex_normals() override;
	void set_poly_cell_vertex_normals(const Vector<PackedVector4Array> &p_poly_cell_vertex_normals);
	void set_poly_cell_vertex_normals_bind(const TypedArray<PackedVector4Array> &p_poly_cell_vertex_normals);

	virtual Vector<PackedVector3Array> get_poly_cell_texture_map() override;
	void set_poly_cell_texture_map(const Vector<PackedVector3Array> &p_poly_cell_texture_map);
	void set_poly_cell_texture_map_bind(const TypedArray<PackedVector3Array> &p_poly_cell_texture_map);

	virtual HashSet<int32_t> get_seam_face_indices() const override { return _seam_face_indices; }
	PackedInt32Array get_seam_face_indices_bind() const;
	void set_seam_face_indices(const HashSet<int32_t> &p_seam_face_indices);
	void set_seam_face_indices_bind(const PackedInt32Array &p_seam_face_indices);

	virtual PackedVector4Array get_poly_cell_vertices() override;
	void set_poly_cell_vertices(const PackedVector4Array &p_vertices);
};

VARIANT_ENUM_CAST(ArrayPolyMesh4D::ComputeNormalsMode);
VARIANT_ENUM_CAST(ArrayPolyMesh4D::UnwrapTextureMapMode);
