#include "kinematic_collision_4d.h"

#include "collision_shape_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/physics_material.hpp>
#elif GODOT_MODULE
#include "scene/resources/physics_material.h"
#endif

real_t KinematicCollision4D::get_bounce_ratio() const {
	real_t moving_bounce = 0.0f;
	if (_moving_shape_node) {
		Ref<PhysicsMaterial> moving_mat = _moving_shape_node->get_physics_material();
		moving_bounce = moving_mat.is_valid() ? moving_mat->get_bounce() : 0.0f;
	}
	real_t obstacle_bounce = 0.0f;
	if (_obstacle_shape_node) {
		Ref<PhysicsMaterial> obstacle_mat = _obstacle_shape_node->get_physics_material();
		obstacle_bounce = obstacle_mat.is_valid() ? obstacle_mat->get_bounce() : 0.0f;
	}
	return MAX(moving_bounce, obstacle_bounce);
}

void KinematicCollision4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_bounce_ratio"), &KinematicCollision4D::get_bounce_ratio);

	ClassDB::bind_method(D_METHOD("get_moving_shape_node"), &KinematicCollision4D::get_moving_shape_node);
	ClassDB::bind_method(D_METHOD("set_moving_shape_node", "moving_shape_node"), &KinematicCollision4D::set_moving_shape_node);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "moving_shape_node"), "set_moving_shape_node", "get_moving_shape_node");

	ClassDB::bind_method(D_METHOD("get_obstacle_shape_node"), &KinematicCollision4D::get_obstacle_shape_node);
	ClassDB::bind_method(D_METHOD("set_obstacle_shape_node", "obstacle_shape_node"), &KinematicCollision4D::set_obstacle_shape_node);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "obstacle_shape_node"), "set_obstacle_shape_node", "get_obstacle_shape_node");

	ClassDB::bind_method(D_METHOD("get_normal"), &KinematicCollision4D::get_normal);
	ClassDB::bind_method(D_METHOD("set_normal", "layer"), &KinematicCollision4D::set_normal);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "normal"), "set_normal", "get_normal");

	ClassDB::bind_method(D_METHOD("get_travel_ratio"), &KinematicCollision4D::get_travel_ratio);
	ClassDB::bind_method(D_METHOD("set_travel_ratio", "travel_ratio"), &KinematicCollision4D::set_travel_ratio);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "travel_ratio"), "set_travel_ratio", "get_travel_ratio");
}
