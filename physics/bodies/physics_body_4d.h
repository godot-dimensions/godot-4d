#pragma once

#include "collision_object_4d.h"

class PhysicsBody4D : public CollisionObject4D {
	GDCLASS(PhysicsBody4D, CollisionObject4D);

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void move_and_collide(Vector4 p_motion);
};
