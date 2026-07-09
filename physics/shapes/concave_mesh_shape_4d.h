#pragma once

#include "shape_4d.h"

class ConcaveMeshShape4D : public Shape4D {
	GDCLASS(ConcaveMeshShape4D, Shape4D);

	PackedVector4Array _simplex_cells;
	PackedVector4Array _normals;
	PackedFloat64Array _inverse_metric_cache;

	static PackedVector4Array _calculate_normals(const PackedVector4Array &p_simplex_cells);
	static PackedFloat64Array _calculate_inverse_metric_cache(const PackedVector4Array &p_simplex_cells);

protected:
	static void _bind_methods();

public:
	PackedVector4Array get_simplex_cells() const { return _simplex_cells; }
	void set_simplex_cells(const PackedVector4Array &p_simplex_cells);

	virtual bool is_equal_exact(const Ref<Shape4D> &p_shape) const override;

	virtual Dictionary raycast_intersects(const Vector4 &p_local_from, const Vector4 &p_local_direction) const override;

	virtual Vector4 get_nearest_point(const Vector4 &p_local_point) const override;
	virtual Vector4 get_support_point(const Vector4 &p_local_direction) const override;
	// Concave shapes do not have any points by definition, they are not solid and not necessarily manifold.
	virtual bool has_point(const Vector4 &p_local_point) const override { return false; }

	virtual Ref<TetraMesh4D> to_tetra_mesh(const Dictionary &p_options = Dictionary()) const override;
	virtual Ref<WireMesh4D> to_wire_mesh(const Dictionary &p_options = Dictionary()) const override;
	static Ref<ConcaveMeshShape4D> create_from_mesh(const Ref<TetraMesh4D> &p_mesh);
};
