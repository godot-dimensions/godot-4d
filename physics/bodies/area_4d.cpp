#include "area_4d.h"

#include "../server/physics_server_4d.h"

void Area4D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			PhysicsServer4D::get_singleton()->register_area(this);
		} break;
		case NOTIFICATION_EXIT_TREE: {
			PhysicsServer4D::get_singleton()->unregister_area(this);
		} break;
	}
}

void Area4D::move_area(Vector4 p_motion) {
	PhysicsServer4D::get_singleton()->move_area(this, p_motion);
}

void Area4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("move_area", "motion"), &Area4D::move_area);

	ADD_SIGNAL(MethodInfo("area_entered_self", PropertyInfo(Variant::OBJECT, "area", PROPERTY_HINT_RESOURCE_TYPE, "Area4D")));
	ADD_SIGNAL(MethodInfo("area_exited_self", PropertyInfo(Variant::OBJECT, "area", PROPERTY_HINT_RESOURCE_TYPE, "Area4D")));
	ADD_SIGNAL(MethodInfo("body_entered_area", PropertyInfo(Variant::OBJECT, "body", PROPERTY_HINT_RESOURCE_TYPE, "PhysicsBody4D")));
	ADD_SIGNAL(MethodInfo("body_exited_area", PropertyInfo(Variant::OBJECT, "body", PROPERTY_HINT_RESOURCE_TYPE, "PhysicsBody4D")));
}
