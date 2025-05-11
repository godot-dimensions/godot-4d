#pragma once

#include "../../nodes/node_4d.h"

class CollisionShape4D;

class CollisionObject4D : public Node4D {
	GDCLASS(CollisionObject4D, Node4D);

	TypedArray<CollisionShape4D> _collision_shapes;

protected:
	static void _bind_methods();

public:
	void register_collision_shape(CollisionShape4D *p_shape);
	void unregister_collision_shape(CollisionShape4D *p_shape);

	TypedArray<CollisionShape4D> get_collision_shapes() const;
};

// Due to the usage of TypedArray, we need CollisionShape4D to be always
// included for all users of CollisionObject4D, but it's not needed for the
// header of CollisionObject4D itself. So we put this at the bottom instead.
#include "../collision_shape_4d.h"
