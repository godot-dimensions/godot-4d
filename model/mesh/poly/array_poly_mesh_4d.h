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
		UNWRAP_MODE_EACH_ISLAND_FILLS,
		UNWRAP_MODE_TILE_ISLANDS,
	};

private:
	// 0: Each 2D face made up of 1D edge indices.
	// 1: Each 3D cell made up of 2D face indices (each 3D cell makes up the boundary/surface of the 4D mesh).
	// 2: Each 4D cell made up of 3D cell indices (optional, for encoding hypervolumes).
	Vector<Vector<PackedInt32Array>> _poly_cell_indices;
	PackedVector4Array _poly_cell_vertex_positions;
	PackedVector4Array _poly_cell_normal_values;
	PackedVector3Array _poly_cell_texture_map_values;
	// The key's X is the geometry dimension, Y is the decomposition dimension.
	// See G4MFMeshSurfaceBindingGeometry4D for more details.
	HashMap<Vector2i, Vector<PackedInt32Array>> _all_poly_cell_normal_indices;
	HashMap<Vector2i, Vector<PackedInt32Array>> _all_poly_cell_texture_map_indices;
	PackedInt32Array _poly_cell_boundary_pivot_overrides;
	// Seams always refer to 2D faces (the border between boundary 3D cells).
	HashSet<int32_t> _seam_face_indices;
	PackedInt32Array _edge_vertex_indices;

	bool _validate_data_binding_shape_internal(const Vector2i p_key, const Vector<PackedInt32Array> &p_binding, const int64_t p_value_count, const String &p_binding_name) const;
	void _delete_data_bindings_internal(const int32_t p_dimension, const int32_t p_index);
	static void _delete_bindings_below_dimension_internal(HashMap<Vector2i, Vector<PackedInt32Array>> &r_bindings, const int p_dimension);

	PackedInt32Array _get_cell_4_vertices_starting_from_face(const int64_t p_which_cell, const int64_t p_start_face_in_cell) const;
	real_t _get_cell_extent(const PackedInt32Array &p_cell_vertices, const int32_t p_origin_vertex) const;
	void _get_cell_world_span_seed(const int64_t p_which_cell, Vector4 &r_world_x, Vector4 &r_world_y, Vector4 &r_world_z, int32_t &p_pivot) const;
	void _transform_cell_to_texture_space(const Transform4D &p_world_to_texcoord, const Vector<PackedInt32Array> &p_cell_vert, const int64_t p_cell_index, const int32_t p_pivot, Vector<PackedVector3Array> &r_poly_cell_texture_map) const;
	Vector<PackedInt32Array> _get_face_to_cell_map() const;
	PackedInt32Array _collect_cells_in_island_internal(const int64_t p_start_cell, const Vector<PackedInt32Array> &p_face_to_cell_map) const;
	static PackedInt32Array _deletion_remap_table(const int32_t p_element_count, const int32_t p_deleted_index);
	void _delete_edge_internal(const int32_t p_index);
	void _delete_vertex_internal(const int32_t p_index);
	void _delete_poly_cell_element_internal(const int32_t p_poly_dim_index, const int32_t p_index);
	bool _unwrap_texture_map_island_cell(const PackedInt32Array &p_cells_in_island, const int64_t p_current_cell_index_index, const Vector<PackedInt32Array> &p_cell_vert, Vector<PackedVector3Array> &r_poly_cell_texture_map) const;
	void _unwrap_texture_map_island_internal(const PackedInt32Array &p_cells_in_island, const bool p_keep_existing, Vector<PackedVector3Array> &r_poly_cell_texture_map);
	static void _fit_island_texture_map_into_aabb(const PackedInt32Array &p_cells_in_island, const AABB &p_target_aabb, const bool p_proportional, Vector<PackedVector3Array> &r_poly_cell_texture_map);
	static void _fit_or_tile_islands_internal(const Vector<PackedInt32Array> &p_islands, const UnwrapTextureMapMode p_mode, const double p_padding, const bool p_proportional, Vector<PackedVector3Array> &r_poly_cell_texture_map);
	static Vector3i _tiles_for_island_count(const int32_t p_island_count);
	static inline int32_t _ceil_div(int32_t p_a, int32_t p_b) {
		return (p_a + p_b - 1) / p_b;
	}

	// Internal helpers for the normal and texture map value pools.
	PackedInt32Array _normal_indices_for_values_internal(const PackedVector4Array &p_values);
	// Resamples one dense binding after `split_poly_element` changed the mesh. `p_pre_traversal` is the binding's
	// per-element traversal of its sub-elements before the split, or null for a per-element binding, and
	// `p_post_traversal` is the same after the split, which also tells how many elements there are now.
	template <typename TArray, typename TElement>
	static void _resample_dense_binding_after_split(const Vector2i &p_key, const int32_t p_dimension, const int32_t p_index, const PackedInt32Array &p_piece_indices, const Vector<PackedInt32Array> *p_pre_traversal, const Vector<PackedInt32Array> &p_post_traversal, Vector<TArray> &r_dense) {
		const int64_t element_count = p_post_traversal.size();
		if (p_key.x == p_dimension - 1) {
			// The dimension below: the pieces may use new elements, such as a cut face, which get zero values so that a
			// complete binding stays complete.
			if (p_key.y == p_key.x) {
				if (!r_dense.is_empty() && r_dense[0].size() < element_count) {
					r_dense.write[0].resize(element_count);
				}
			} else {
				while (r_dense.size() < element_count) {
					TArray zeros;
					zeros.resize(p_post_traversal[r_dense.size()].size());
					r_dense.push_back(zeros);
				}
			}
			return;
		}
		if (p_key.x == p_dimension) {
			// The split dimension: the pieces take the element's values, the per-element value as it is, and for each of
			// their sub-elements the value the element had for it, with zero for a sub-element the element did not have.
			if (p_key.y == p_key.x) {
				if (r_dense.is_empty() || p_index >= r_dense[0].size()) {
					return; // The element had no value, so neither do its pieces.
				}
				const TElement value = r_dense[0][p_index];
				if (r_dense[0].size() < element_count) {
					r_dense.write[0].resize(element_count);
				}
				for (const int32_t piece_index : p_piece_indices) {
					r_dense.write[0].set(piece_index, value);
				}
				return;
			}
			if (p_pre_traversal == nullptr || p_index >= r_dense.size() || p_index >= p_pre_traversal->size()) {
				return;
			}
			const PackedInt32Array element_sub_elements = (*p_pre_traversal)[p_index];
			const TArray element_values = r_dense[p_index];
			if (element_values.size() != element_sub_elements.size()) {
				return; // Malformed, so leave it for validation to report.
			}
			while (r_dense.size() < element_count) {
				r_dense.push_back(TArray());
			}
			for (const int32_t piece_index : p_piece_indices) {
				const PackedInt32Array &piece_sub_elements = p_post_traversal[piece_index];
				TArray piece_values;
				piece_values.resize(piece_sub_elements.size());
				for (int64_t i = 0; i < piece_sub_elements.size(); i++) {
					const int64_t found = element_sub_elements.find(piece_sub_elements[i]);
					piece_values.set(i, found >= 0 ? element_values[found] : TElement());
				}
				r_dense.write[piece_index] = piece_values;
			}
			return;
		}
		// Above the split dimension: an element that contains the pieces traverses its sub-elements in a new order, so
		// its values move to where their sub-elements are now, and a new sub-element gets zero.
		if (p_pre_traversal == nullptr) {
			return;
		}
		for (int64_t element_index = 0; element_index < r_dense.size() && element_index < p_pre_traversal->size() && element_index < element_count; element_index++) {
			const PackedInt32Array &pre = (*p_pre_traversal)[element_index];
			const PackedInt32Array &post = p_post_traversal[element_index];
			const TArray &values = r_dense[element_index];
			if (values.is_empty() || pre == post || values.size() != pre.size()) {
				continue;
			}
			TArray remapped;
			remapped.resize(post.size());
			for (int64_t i = 0; i < post.size(); i++) {
				const int64_t found = pre.find(post[i]);
				remapped.set(i, found >= 0 ? values[found] : TElement());
			}
			r_dense.write[element_index] = remapped;
		}
	}
	static bool _faces_share_edge(const PackedInt32Array &p_face_a, const PackedInt32Array &p_face_b);
	static bool _start_cell_with_adjacent_faces(const Vector<PackedInt32Array> &p_faces, PackedInt32Array &r_cell_faces);
	static int _induced_face_orientation_sign(const PackedInt32Array &p_face_vertices, const PackedVector4Array &p_positions, const Vector4 &p_cell_centroid, const Vector4 &p_cell_normal);
	PackedVector4Array _sample_normal_values_internal(const PackedInt32Array &p_indices) const;
	Vector<PackedVector3Array> _get_poly_cell_texture_map_dense_internal() const;
	Vector<PackedVector3Array> _get_poly_cell_texture_map_dense_resized_internal(const bool p_keep_existing) const;
	void _set_poly_cell_texture_map_dense_internal(const Vector<PackedVector3Array> &p_poly_cell_texture_map);
	void _compact_normal_values_internal();
	void _compact_texture_map_values_internal();

protected:
	bool _set(const StringName &p_name, const Variant &p_value);
	static void _bind_methods();
	bool _validate_poly_mesh_data_only() override;

public:
	// Append and delete functions.
	int64_t append_edge_points(const Vector4 &p_point_a, const Vector4 &p_point_b, const bool p_deduplicate = true);
	int64_t append_edge_indices(int32_t p_index_a, int32_t p_index_b, const bool p_deduplicate = true);
	int64_t append_poly_cell(const int32_t p_dimension, const PackedInt32Array &p_cell, const bool p_deduplicate = true);
	int64_t append_poly_hierarchy(const Vector<Vector<PackedInt32Array>> &p_poly_cell_indices, const PackedInt32Array &p_edge_vertex_indices);
	int32_t append_vertex(const Vector4 &p_vertex, const bool p_deduplicate_vertices = true);
	PackedInt32Array append_vertices(const PackedVector4Array &p_vertices, const bool p_deduplicate_vertices = true);
	void delete_poly_element(const int32_t p_dimension, const int32_t p_index);
	PackedInt32Array split_poly_element(const int32_t p_dimension, const int32_t p_index, const Vector<PackedInt32Array> &p_pieces);
	PackedInt32Array split_poly_element_bind(const int32_t p_dimension, const int32_t p_index, const TypedArray<PackedInt32Array> &p_pieces);

	// Explicit compaction functions for removing unreferenced or duplicate data.
	void compact_normal_values();
	void compact_texture_map_values();

	// Normal calculation functions.
	void calculate_boundary_normals(const ComputeNormalsMode p_mode = COMPUTE_NORMALS_MODE_CELL_ORIENTATION_ONLY, const bool p_keep_existing = false);
	void calculate_face_normals(const Vector4 &p_hyperplane_normal = Vector4(0, 0, 0, 1), const bool p_keep_existing = false);
	void orient_cells_to_boundary_normals(const PackedVector4Array &p_desired_boundary_normals);
	void set_flat_shading_normals(const ComputeNormalsMode p_mode = COMPUTE_NORMALS_MODE_CELL_ORIENTATION_ONLY, const bool p_recalculate_boundary_normals = true);
	void set_smooth_shading_normals(const ComputeNormalsMode p_mode = COMPUTE_NORMALS_MODE_CELL_ORIENTATION_ONLY, const bool p_recalculate_boundary_normals = true);
	void make_double_sided(const bool p_idempotent = true);
	void delete_normals_below_dimension(const int p_dimension);
	PackedInt32Array make_single_cell_from_all_faces() const;
	PackedInt32Array make_single_volume_from_all_cells() const;

	// Texture map and seam functions.
	void calculate_seam_faces(const double p_angle_threshold_radians = Math_TAU / 8.0, const bool p_discard_seams_within_islands = false);
	PackedInt32Array collect_cells_in_island(const int64_t p_start_cell);
	Vector<PackedInt32Array> collect_all_islands();
	void project_texture_map(const PackedInt32Array &p_cells, const Basis4D &p_mesh_to_texture);
	void project_texture_map_bind(const PackedInt32Array &p_cells, const Projection &p_mesh_to_texture);
	void unwrap_texture_map_island(const PackedInt32Array &p_cells_in_island, const bool p_keep_existing = false);
	void fit_texture_map_island(const PackedInt32Array &p_cells_in_island, const AABB &p_target_aabb = AABB(Vector3(), Vector3(1, 1, 1)), const bool p_proportional = true);
	void unwrap_texture_map(const UnwrapTextureMapMode p_mode, const double p_padding = 0.0, const bool p_proportional = true, const bool p_keep_existing = false);
	void unwrap_texture_map_upright(const Vector4 &p_up = Vector4(0, 1, 0, 0), const UnwrapTextureMapMode p_mode = UNWRAP_MODE_TILE_ISLANDS, const double p_padding = 0.0, const bool p_proportional = true, const bool p_keep_existing = false);
	void transform_texture_map(const Transform3D &p_transform);
	void delete_texture_maps_below_dimension(const int p_dimension);

	// Misc functions.
	void deduplicate_all_elements(const int64_t p_max_dimension = 1000000000);
	void transform_mesh(const Transform4D &p_transform);
	void transform_mesh_bind(const Vector4 &p_offset, const Projection &p_basis = Projection());
	void merge_with(const Ref<PolyMesh4D> &p_other, const Transform4D &p_transform = Transform4D());
	void merge_with_bind(const Ref<PolyMesh4D> &p_other, const Vector4 &p_offset = Vector4(), const Projection &p_basis = Projection());

	// Getters and setters.
	virtual HashMap<Vector2i, Vector<PackedInt32Array>> get_all_poly_cell_normal_indices() override;
	void set_all_poly_cell_normal_indices(const HashMap<Vector2i, Vector<PackedInt32Array>> &p_all_poly_cell_normal_indices);
	virtual HashMap<Vector2i, Vector<PackedInt32Array>> get_all_poly_cell_texture_map_indices() override;
	void set_all_poly_cell_texture_map_indices(const HashMap<Vector2i, Vector<PackedInt32Array>> &p_all_poly_cell_texture_map_indices);

	// Setters for dense views of the indexed data bindings, the counterparts of the getters on PolyMesh4D.
	// They deduplicate the values into the value pool and store indices.
	void set_poly_cell_dense_normals(const Vector2i &p_key, const Vector<PackedVector4Array> &p_dense_normals);
	void set_poly_cell_dense_texture_map(const Vector2i &p_key, const Vector<PackedVector3Array> &p_dense_texture_map);

	void set_all_poly_cell_normal_indices_bind(const PolyDataDictionary &p_all_poly_cell_normal_indices);
	void set_all_poly_cell_texture_map_indices_bind(const PolyDataDictionary &p_all_poly_cell_texture_map_indices);

	virtual PackedInt32Array get_edge_indices() override { return _edge_vertex_indices; }
	void set_edge_vertex_indices(const PackedInt32Array &p_edge_indices);

	virtual Vector<Vector<PackedInt32Array>> get_poly_cell_indices() override { return _poly_cell_indices; }
	void set_poly_cell_indices(const Vector<Vector<PackedInt32Array>> &p_poly_cell_indices);
	void set_poly_cell_indices_bind(const TypedArray<Array> &p_poly_cell_indices);

	virtual PackedVector4Array get_poly_cell_boundary_normals() override;
	void set_poly_cell_boundary_normals(const PackedVector4Array &p_poly_cell_boundary_normals);

	virtual PackedInt32Array get_poly_cell_boundary_pivot_overrides() override;
	void set_poly_cell_boundary_pivot_overrides(const PackedInt32Array &p_poly_cell_boundary_pivot_overrides);

	virtual Vector<PackedInt32Array> get_poly_cell_normal_indices() override;
	void set_poly_cell_normal_indices(const Vector<PackedInt32Array> &p_poly_cell_normal_indices);
	void set_poly_cell_normal_indices_bind(const TypedArray<PackedInt32Array> &p_poly_cell_normal_indices);

	virtual Vector<PackedInt32Array> get_poly_cell_texture_map_indices() override;
	void set_poly_cell_texture_map_indices(const Vector<PackedInt32Array> &p_poly_cell_texture_map_indices);
	void set_poly_cell_texture_map_indices_bind(const TypedArray<PackedInt32Array> &p_poly_cell_texture_map_indices);

	virtual HashSet<int32_t> get_seam_face_indices() const override { return HashSet<int32_t>(_seam_face_indices); }
	PackedInt32Array get_seam_face_indices_bind() const;
	void set_seam_face_indices(const HashSet<int32_t> &p_seam_face_indices);
	void set_seam_face_indices_bind(const PackedInt32Array &p_seam_face_indices);

	virtual PackedVector4Array get_poly_cell_vertex_positions() override;
	void set_poly_cell_vertex_positions(const PackedVector4Array &p_vertex_positions);

	virtual PackedVector4Array get_poly_cell_normal_values() override;
	void set_poly_cell_normal_values(const PackedVector4Array &p_normal_values);

	virtual PackedVector3Array get_poly_cell_texture_map_values() override;
	void set_poly_cell_texture_map_values(const PackedVector3Array &p_tex_map_values);
};

VARIANT_ENUM_CAST(ArrayPolyMesh4D::ComputeNormalsMode);
VARIANT_ENUM_CAST(ArrayPolyMesh4D::UnwrapTextureMapMode);
