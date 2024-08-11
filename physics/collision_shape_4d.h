#pragma once

#include "../nodes/node_4d.h"
#include "shapes/shape_4d.h"

class CollisionShape4D : public Node4D {
	GDCLASS(CollisionShape4D, Node4D);

	Ref<Shape4D> _shape;

protected:
	static void _bind_methods();

public:
	Ref<Shape4D> get_shape() const;
	void set_shape(const Ref<Shape4D> &p_shape);

	bool has_global_point(const Vector4 &p_point) const;
};
