#pragma once

#include "../../math/transform_4d.h"
#include "tetra_mesh_4d.h"

class ArrayTetraMesh4D : public TetraMesh4D {
	GDCLASS(ArrayTetraMesh4D, TetraMesh4D);

	PackedInt32Array _cell_indices;
	PackedVector4Array _cell_positions_cache;
	PackedVector4Array _cell_normals;
	PackedVector3Array _cell_uvw_map;
	PackedVector4Array _vertices;

	void _clear_cache();

protected:
	static void _bind_methods();

public:
	void calculate_normals(const bool p_keep_existing = false);
	void merge_with(const Ref<ArrayTetraMesh4D> &p_other, const Transform4D &p_transform = Transform4D());
	void merge_with_bind(const Ref<ArrayTetraMesh4D> &p_other, const Vector4 &p_offset = Vector4(), const Projection &p_basis = Projection());

	virtual PackedInt32Array get_cell_indices() override;
	void set_cell_indices(const PackedInt32Array &p_cell_indices);

	virtual PackedVector4Array get_cell_positions() override;

	virtual PackedVector4Array get_cell_normals() override;
	void set_cell_normals(const PackedVector4Array &p_cell_normals);

	virtual PackedVector3Array get_cell_uvw_map() override;
	void set_cell_uvw_map(const PackedVector3Array &p_cell_uvw_map);

	virtual PackedVector4Array get_vertices() override;
	void set_vertices(const PackedVector4Array &p_vertices);
};
