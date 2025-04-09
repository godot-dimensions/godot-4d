#include "ghost_physics_engine_4d.h"

#include "../../math/geometric_algebra/rotor_4d.h"
#include "../bodies/rigid_body_4d.h"
#include "physics_server_4d.h"

void GhostPhysicsEngine4D::_step_dynamic_rigid_body(RigidBody4D *p_moving_body, double p_delta) {
	p_moving_body->set_global_position(p_moving_body->get_global_position() + p_moving_body->get_linear_velocity() * p_delta);
	Rotor4D rotor = Rotor4D::from_bivector_magnitude(p_moving_body->get_angular_velocity() * p_delta);
	p_moving_body->set_global_basis(rotor.rotate_basis(p_moving_body->get_global_basis()));
}

Ref<KinematicCollision4D> GhostPhysicsEngine4D::move_and_collide(PhysicsBody4D *p_body, Vector4 p_motion, bool p_test_only) {
	Ref<KinematicCollision4D> ret;
	ret.instantiate();
	if (p_test_only) {
		return ret;
	}
	p_body->set_global_position(p_body->get_global_position() + p_motion);
	return ret;
}

void GhostPhysicsEngine4D::move_area(Area4D *p_area, Vector4 p_motion) {
	p_area->set_global_position(p_area->get_global_position() + p_motion);
}

void GhostPhysicsEngine4D::physics_process(double p_delta) {
	const TypedArray<PhysicsBody4D> &body_nodes = PhysicsServer4D::get_singleton()->get_physics_body_nodes();
	for (int body_index = 0; body_index < body_nodes.size(); body_index++) {
		RigidBody4D *rigid_body = Object::cast_to<RigidBody4D>(body_nodes[body_index]);
		if (rigid_body) {
			_step_dynamic_rigid_body(rigid_body, p_delta);
		}
	}
}
