#include "physics_body_4d.h"

#include "../server/physics_server_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/project_settings.hpp>
#elif GODOT_MODULE
#include "core/config/project_settings.h"
#endif

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

Vector4 PhysicsBody4D::get_gravity() const {
	// For now use the 3D gravity amount for 4D.
	real_t global_gravity_amount = ProjectSettings::get_singleton()->get_setting("physics/3d/default_gravity", 9.8);
	Vector4 gravity = Vector4(0, -global_gravity_amount, 0, 0);
	// TODO: Add gravity from Area4D nodes.
	return gravity;
}

Ref<KinematicCollision4D> PhysicsBody4D::move_and_collide(Vector4 p_motion, bool p_test_only) {
	return PhysicsServer4D::get_singleton()->move_and_collide(this, p_motion, p_test_only);
}

void PhysicsBody4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_gravity"), &PhysicsBody4D::get_gravity);
	ClassDB::bind_method(D_METHOD("move_and_collide", "motion", "test_only"), &PhysicsBody4D::move_and_collide, DEFVAL(false));

	ADD_SIGNAL(MethodInfo("body_impacted_self", PropertyInfo(Variant::OBJECT, "body", PROPERTY_HINT_RESOURCE_TYPE, "PhysicsBody4D"), PropertyInfo(Variant::OBJECT, "kinematic_collision", PROPERTY_HINT_RESOURCE_TYPE, "KinematicCollision4D")));
	ADD_SIGNAL(MethodInfo("self_impacted_body", PropertyInfo(Variant::OBJECT, "body", PROPERTY_HINT_RESOURCE_TYPE, "PhysicsBody4D"), PropertyInfo(Variant::OBJECT, "kinematic_collision", PROPERTY_HINT_RESOURCE_TYPE, "KinematicCollision4D")));
}
