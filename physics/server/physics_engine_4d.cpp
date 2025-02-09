#include "physics_engine_4d.h"

Ref<KinematicCollision4D> PhysicsEngine4D::move_and_collide(PhysicsBody4D *p_body, Vector4 p_motion, bool p_test_only) {
	Ref<KinematicCollision4D> ret;
	GDVIRTUAL_CALL(_move_and_collide, p_body, p_motion, p_test_only, ret);
	return ret;
}

void PhysicsEngine4D::move_area(Area4D *p_area, Vector4 p_motion) {
	GDVIRTUAL_CALL(_move_area, p_area, p_motion);
}

void PhysicsEngine4D::physics_process(double p_delta) {
	GDVIRTUAL_CALL(_physics_process, p_delta);
}

void PhysicsEngine4D::_bind_methods() {
	GDVIRTUAL_BIND(_move_and_collide, "body", "motion", "test_only");
	GDVIRTUAL_BIND(_move_area, "area", "motion");
	GDVIRTUAL_BIND(_physics_process, "delta");
}
