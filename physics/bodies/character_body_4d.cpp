#include "character_body_4d.h"

#include "../../math/vector_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/engine.hpp>
#endif

constexpr int MAX_MOVE_AND_SLIDE_ITERATIONS = 10;

TypedArray<KinematicCollision4D> CharacterBody4D::move_and_slide() {
	// Hack in order to work with calling from _process as well as from _physics_process. Copied from CharacterBody3D.
	const double delta = Engine::get_singleton()->is_in_physics_frame() ? get_physics_process_delta_time() : get_process_delta_time();
	TypedArray<KinematicCollision4D> collisions;
	_is_on_ceiling = false;
	_is_on_floor = false;
	_is_on_wall = false;
	{
		Vector4 desired_motion = _linear_velocity * delta;
		for (int iteration = 0; iteration < MAX_MOVE_AND_SLIDE_ITERATIONS; iteration++) {
			Ref<KinematicCollision4D> collision = move_and_collide(desired_motion);
			const real_t travel_ratio = collision->get_travel_ratio();
			if (travel_ratio == 1.0f) {
				// This last move_and_collide call moved us the rest of the way, with no collisions.
				break;
			}
			// This last move_and_collide call moved us part of the way. We need to deal with the collision.
			const Vector4 normal = collision->get_normal();
			desired_motion = desired_motion * (1.0f - travel_ratio);
			// Is on floor/ceiling/wall checks.
			if (_up_direction == Vector4()) {
				_is_on_wall = true;
			} else {
				const real_t angle = Vector4D::angle_to(_up_direction, normal);
				if (angle < _floor_max_angle) {
					_is_on_floor = true;
				} else if (angle > Math_TAU - _floor_max_angle) {
					_is_on_ceiling = true;
				} else {
					_is_on_wall = true;
				}
			}
			// Bouncing.
			if (_use_bounce) {
				const real_t bounce_ratio = collision->get_bounce_ratio();
				desired_motion = Vector4D::bounce_ratio(desired_motion, collision->get_normal(), bounce_ratio);
				_linear_velocity = Vector4D::bounce_ratio(_linear_velocity, collision->get_normal(), bounce_ratio);
			} else {
				desired_motion = Vector4D::slide(desired_motion, collision->get_normal());
				_linear_velocity = Vector4D::slide(_linear_velocity, collision->get_normal());
			}
			// Final checks.
			collisions.append(collision);
			if (desired_motion.is_zero_approx()) {
				break;
			}
		}
	}
	return collisions;
}

void CharacterBody4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_on_ceiling"), &CharacterBody4D::is_on_ceiling);
	ClassDB::bind_method(D_METHOD("is_on_floor"), &CharacterBody4D::is_on_floor);
	ClassDB::bind_method(D_METHOD("is_on_wall"), &CharacterBody4D::is_on_wall);

	ClassDB::bind_method(D_METHOD("move_and_slide"), &CharacterBody4D::move_and_slide);

	ClassDB::bind_method(D_METHOD("get_floor_max_angle"), &CharacterBody4D::get_floor_max_angle);
	ClassDB::bind_method(D_METHOD("set_floor_max_angle", "floor_max_angle"), &CharacterBody4D::set_floor_max_angle);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "floor_max_angle", PROPERTY_HINT_RANGE, "0,180,0.1,radians_as_degrees"), "set_floor_max_angle", "get_floor_max_angle");

	ClassDB::bind_method(D_METHOD("get_linear_velocity"), &CharacterBody4D::get_linear_velocity);
	ClassDB::bind_method(D_METHOD("set_linear_velocity", "linear_velocity"), &CharacterBody4D::set_linear_velocity);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "linear_velocity", PROPERTY_HINT_NONE, "suffix:m/s"), "set_linear_velocity", "get_linear_velocity");

	ClassDB::bind_method(D_METHOD("get_up_direction"), &CharacterBody4D::get_up_direction);
	ClassDB::bind_method(D_METHOD("set_up_direction", "up_direction"), &CharacterBody4D::set_up_direction);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "up_direction"), "set_up_direction", "get_up_direction");

	ClassDB::bind_method(D_METHOD("get_use_bounce"), &CharacterBody4D::get_use_bounce);
	ClassDB::bind_method(D_METHOD("set_use_bounce", "use_bounce"), &CharacterBody4D::set_use_bounce);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_bounce"), "set_use_bounce", "get_use_bounce");
}
