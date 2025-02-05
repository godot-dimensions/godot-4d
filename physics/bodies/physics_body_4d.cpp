#include "physics_body_4d.h"

#include "../server/physics_server_4d.h"

void PhysicsBody4D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			PhysicsServer4D::get_singleton()->register_physics_body(this);
		} break;
		case NOTIFICATION_EXIT_TREE: {
			PhysicsServer4D::get_singleton()->unregister_physics_body(this);
		} break;
	}
}

void PhysicsBody4D::move_and_collide(Vector4 p_motion) {
	PhysicsServer4D::get_singleton()->move_and_collide(this, p_motion);
}

void PhysicsBody4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("move_and_collide", "motion"), &PhysicsBody4D::move_and_collide);
}
