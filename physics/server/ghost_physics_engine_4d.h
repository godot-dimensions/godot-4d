#pragma once

#include "physics_engine_4d.h"

// Trivial physics engine that does not perform any collision detection or resolution.
// All objects can pass through each other in this physics engine.
class GhostPhysicsEngine4D : public PhysicsEngine4D {
	GDCLASS(GhostPhysicsEngine4D, PhysicsEngine4D);

protected:
	static void _bind_methods() {}

public:
	virtual void move_and_collide(PhysicsBody4D *p_body, Vector4 p_motion) override;
	virtual void move_area(Area4D *p_area, Vector4 p_motion) override;
	virtual void step_dynamic_rigid_body(RigidBody4D *p_body, double p_delta) override;
};
