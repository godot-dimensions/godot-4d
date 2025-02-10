#include "kinematic_collision_4d.h"

#include "bodies/physics_body_4d.h"
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

PhysicsBody4D *KinematicCollision4D::get_moving_body_node() const {
	Node *moving_ancestor = _moving_shape_node;
	while (moving_ancestor) {
		moving_ancestor = moving_ancestor->get_parent();
		CollisionObject4D *moving_body = Object::cast_to<CollisionObject4D>(moving_ancestor);
		if (moving_body) {
			return Object::cast_to<PhysicsBody4D>(moving_body);
		}
	}
	return nullptr;
}

PhysicsBody4D *KinematicCollision4D::get_obstacle_body_node() const {
	Node *obstacle_ancestor = _obstacle_shape_node;
	while (obstacle_ancestor) {
		obstacle_ancestor = obstacle_ancestor->get_parent();
		CollisionObject4D *obstacle_body = Object::cast_to<CollisionObject4D>(obstacle_ancestor);
		if (obstacle_body) {
			return Object::cast_to<PhysicsBody4D>(obstacle_body);
		}
	}
	return nullptr;
}

void KinematicCollision4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_bounce_ratio"), &KinematicCollision4D::get_bounce_ratio);
	ClassDB::bind_method(D_METHOD("get_moving_body_node"), &KinematicCollision4D::get_moving_body_node);
	ClassDB::bind_method(D_METHOD("get_obstacle_body_node"), &KinematicCollision4D::get_obstacle_body_node);

	ClassDB::bind_method(D_METHOD("get_moving_shape_node"), &KinematicCollision4D::get_moving_shape_node);
	ClassDB::bind_method(D_METHOD("set_moving_shape_node", "moving_shape_node"), &KinematicCollision4D::set_moving_shape_node);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "moving_shape_node", PROPERTY_HINT_NODE_TYPE, "CollisionShape4D"), "set_moving_shape_node", "get_moving_shape_node");

	ClassDB::bind_method(D_METHOD("get_obstacle_shape_node"), &KinematicCollision4D::get_obstacle_shape_node);
	ClassDB::bind_method(D_METHOD("set_obstacle_shape_node", "obstacle_shape_node"), &KinematicCollision4D::set_obstacle_shape_node);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "obstacle_shape_node", PROPERTY_HINT_NODE_TYPE, "CollisionShape4D"), "set_obstacle_shape_node", "get_obstacle_shape_node");

	ClassDB::bind_method(D_METHOD("get_normal"), &KinematicCollision4D::get_normal);
	ClassDB::bind_method(D_METHOD("set_normal", "layer"), &KinematicCollision4D::set_normal);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "normal"), "set_normal", "get_normal");

	ClassDB::bind_method(D_METHOD("get_travel_ratio"), &KinematicCollision4D::get_travel_ratio);
	ClassDB::bind_method(D_METHOD("set_travel_ratio", "travel_ratio"), &KinematicCollision4D::set_travel_ratio);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "travel_ratio"), "set_travel_ratio", "get_travel_ratio");

	ClassDB::bind_method(D_METHOD("get_relative_velocity"), &KinematicCollision4D::get_relative_velocity);
	ClassDB::bind_method(D_METHOD("set_relative_velocity", "relative_velocity"), &KinematicCollision4D::set_relative_velocity);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "relative_velocity", PROPERTY_HINT_NONE, "suffix:m/s"), "set_relative_velocity", "get_relative_velocity");
}
