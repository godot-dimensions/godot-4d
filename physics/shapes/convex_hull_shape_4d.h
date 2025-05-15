#pragma once

#include "shape_4d.h"

class ConvexHullShape4D : public Shape4D {
	GDCLASS(ConvexHullShape4D, Shape4D);

	PackedVector4Array _points;

protected:
	static void _bind_methods();

public:
	PackedVector4Array get_points() const { return _points; }
	void set_points(const PackedVector4Array &p_points);

	virtual bool is_equal_exact(const Ref<Shape4D> &p_shape) const override;

	virtual Ref<TetraMesh4D> to_tetra_mesh() const override;
	virtual Ref<WireMesh4D> to_wire_mesh() const override;
	static Ref<ConvexHullShape4D> create_from_mesh(const Ref<Mesh4D> &p_mesh);
};
