#pragma once

#include "../kinematic_collision_4d.h"
#include "collision_object_4d.h"

class PhysicsBody4D : public CollisionObject4D {
	GDCLASS(PhysicsBody4D, CollisionObject4D);

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	Vector4 get_gravity() const;
	Ref<KinematicCollision4D> move_and_collide(Vector4 p_motion, bool p_test_only = false);
};
