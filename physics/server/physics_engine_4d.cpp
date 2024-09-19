#include "physics_engine_4d.h"

void PhysicsEngine4D::move_and_collide(PhysicsBody4D *p_body, Vector4 p_motion) {
	GDVIRTUAL_CALL(_move_and_collide, p_body, p_motion);
}

void PhysicsEngine4D::move_area(Area4D *p_area, Vector4 p_motion) {
	GDVIRTUAL_CALL(_move_area, p_area, p_motion);
}

void PhysicsEngine4D::step_dynamic_rigid_body(RigidBody4D *p_body, double p_delta) {
	GDVIRTUAL_CALL(_step_dynamic_rigid_body, p_body, p_delta);
}

void PhysicsEngine4D::_bind_methods() {
	GDVIRTUAL_BIND(_move_and_collide, "body", "motion");
	GDVIRTUAL_BIND(_move_area, "area", "motion");
	GDVIRTUAL_BIND(_step_dynamic_rigid_body, "body", "delta");
}
