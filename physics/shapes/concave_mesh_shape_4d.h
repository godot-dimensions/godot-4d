#pragma once

#include "shape_4d.h"

class ConcaveMeshShape4D : public Shape4D {
	GDCLASS(ConcaveMeshShape4D, Shape4D);

	PackedVector4Array _simplex_cells;
	PackedVector4Array _normals;

	void _calculate_normals();

protected:
	static void _bind_methods();

public:
	PackedVector4Array get_simplex_cells() const { return _simplex_cells; }
	void set_simplex_cells(const PackedVector4Array &p_simplex_cells);

	virtual bool is_equal_exact(const Ref<Shape4D> &p_shape) const override;

	virtual Ref<TetraMesh4D> to_tetra_mesh() const override;
	virtual Ref<WireMesh4D> to_wire_mesh() const override;
	static Ref<ConcaveMeshShape4D> create_from_mesh(const Ref<TetraMesh4D> &p_mesh);
};
