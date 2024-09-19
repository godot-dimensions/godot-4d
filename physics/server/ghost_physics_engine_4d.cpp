#include "ghost_physics_engine_4d.h"

#include "../bodies/rigid_body_4d.h"

#include "../../math/geometric_algebra/rotor_4d.h"

void GhostPhysicsEngine4D::move_and_collide(PhysicsBody4D *p_body, Vector4 p_motion) {
	p_body->set_global_position(p_body->get_global_position() + p_motion);
}

void GhostPhysicsEngine4D::move_area(Area4D *p_area, Vector4 p_motion) {
	p_area->set_global_position(p_area->get_global_position() + p_motion);
}

void GhostPhysicsEngine4D::step_dynamic_rigid_body(RigidBody4D *p_body, double p_delta) {
	p_body->set_global_position(p_body->get_global_position() + p_body->get_linear_velocity() * p_delta);
	Rotor4D rotor = Rotor4D::rotation_bivector_magnitude(p_body->get_angular_velocity() * p_delta);
	p_body->set_global_basis(rotor.rotate_basis(p_body->get_global_basis()));
}
