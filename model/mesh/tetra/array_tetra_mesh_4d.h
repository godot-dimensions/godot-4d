#pragma once

#include "../../../math/transform_4d.h"
#include "tetra_mesh_4d.h"

class ArrayTetraMesh4D : public TetraMesh4D {
	GDCLASS(ArrayTetraMesh4D, TetraMesh4D);

	PackedInt32Array _simplex_cell_indices;
	PackedVector4Array _simplex_cell_boundary_normals;
	PackedVector4Array _simplex_cell_vertex_normals;
	PackedVector3Array _simplex_cell_texture_map;
	PackedVector4Array _vertices;

	void _clear_cache();

protected:
	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	static void _bind_methods();
	virtual bool validate_mesh_data() override;

public:
	void append_tetra_cell_points(const Vector4 &p_a, const Vector4 &p_b, const Vector4 &p_c, const Vector4 &p_d, const bool p_deduplicate_vertices = true);
	void append_tetra_cell_indices(const int p_index_a, const int p_index_b, const int p_index_c, const int p_index_d);
	int append_vertex(const Vector4 &p_vertex, const bool p_deduplicate_vertices = true);
	PackedInt32Array append_vertices(const PackedVector4Array &p_vertices, const bool p_deduplicate_vertices = true);

	void calculate_boundary_normals(const bool p_keep_existing = false);
	void set_flat_shading_normals(const bool p_recalculate_boundary_normals = false);
	void merge_with(const Ref<ArrayTetraMesh4D> &p_other, const Transform4D &p_transform = Transform4D());
	void merge_with_bind(const Ref<ArrayTetraMesh4D> &p_other, const Vector4 &p_offset = Vector4(), const Projection &p_basis = Projection());

	virtual PackedInt32Array get_simplex_cell_indices() override;
	void set_simplex_cell_indices(const PackedInt32Array &p_simplex_cell_indices);

	virtual PackedVector4Array get_simplex_cell_boundary_normals() override;
	void set_simplex_cell_boundary_normals(const PackedVector4Array &p_simplex_cell_boundary_normals);

	virtual PackedVector4Array get_simplex_cell_vertex_normals() override;
	void set_simplex_cell_vertex_normals(const PackedVector4Array &p_simplex_cell_vertex_normals);

	virtual PackedVector3Array get_simplex_cell_texture_map() override;
	void set_simplex_cell_texture_map(const PackedVector3Array &p_simplex_cell_texture_map);

	virtual PackedVector4Array get_vertices() override;
	void set_vertices(const PackedVector4Array &p_vertices);
};
