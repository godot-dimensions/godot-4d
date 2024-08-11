#pragma once

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
