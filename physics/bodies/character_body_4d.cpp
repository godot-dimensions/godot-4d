#include "character_body_4d.h"

#include "../../math/vector_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/engine.hpp>
#elif GODOT_MODULE
#include "core/config/engine.h"
#endif

constexpr int CHARACTER_BODY_4D_MAX_MOVE_AND_SLIDE_ITERATIONS = 10;

CharacterBody4D::SurfaceType CharacterBody4D::_determine_surface_type(const Vector4 &p_global_up_direction, const Vector4 &p_surface_normal) const {
	ERR_FAIL_COND_V_MSG(!p_surface_normal.is_normalized(), SURFACE_TYPE_ANY, "CharacterBody4D: The surface normal vector is not normalized.");
	if (p_global_up_direction == Vector4()) {
		return SURFACE_TYPE_ANY;
	}
	const real_t angle = Vector4D::angle_to(p_global_up_direction, p_surface_normal);
	if (angle < _floor_max_angle) {
		return SURFACE_TYPE_FLOOR;
	} else if (angle > Math_PI - _floor_max_angle) {
		return SURFACE_TYPE_CEILING;
	} else {
		return SURFACE_TYPE_WALL;
	}
}

TypedArray<KinematicCollision4D> CharacterBody4D::move_and_slide(const double p_delta_time) {
	const Basis4D global_basis = get_global_basis();
	ERR_FAIL_COND_V_MSG(!global_basis.is_orthonormal(), TypedArray<KinematicCollision4D>(), "CharacterBody4D: The global basis is not orthonormal. Physics bodies are required to have an orthonormal basis, with no scaling or shearing, on itself and all ancestor nodes.");
	const Vector4 global_up_direction = global_basis.xform(_local_up_direction);
	double delta_time;
	if (p_delta_time < 0.0) {
		// Hack in order to work with calling from _process as well as from _physics_process. Copied from CharacterBody3D.
		delta_time = Engine::get_singleton()->is_in_physics_frame() ? get_physics_process_delta_time() : get_process_delta_time();
	} else {
		delta_time = p_delta_time;
	}
	TypedArray<KinematicCollision4D> collisions;
	const bool was_on_ceiling = _is_on_ceiling;
	const bool was_on_floor = _is_on_floor;
	const bool was_on_wall = _is_on_wall;
	_is_on_ceiling = false;
	_is_on_floor = false;
	_is_on_wall = false;
	{
		Vector4 desired_motion = _linear_velocity * delta_time;
		for (int iteration = 0; iteration < CHARACTER_BODY_4D_MAX_MOVE_AND_SLIDE_ITERATIONS; iteration++) {
			Ref<KinematicCollision4D> collision = move_and_collide(desired_motion, false, delta_time);
			const real_t travel_ratio = collision->get_travel_ratio();
			if (travel_ratio == 1.0f) {
				// This last move_and_collide call moved us the rest of the way, with no collisions.
				break;
			}
			// This last move_and_collide call moved us part of the way. We need to deal with the collision.
			const Vector4 normal = collision->get_normal();
			desired_motion = desired_motion * (1.0f - travel_ratio);
			// Is on floor/ceiling/wall checks.
			const SurfaceType surface_type = _determine_surface_type(global_up_direction, normal);
			switch (surface_type) {
				case SURFACE_TYPE_ANY: // Treat "any" surface as a wall for the purposes of _is_on_wall / _snap_to_wall / etc.
				case SURFACE_TYPE_WALL: {
					_last_wall_normal = normal;
					_is_on_wall = true;
				} break;
				case SURFACE_TYPE_CEILING: {
					_is_on_ceiling = true;
				} break;
				case SURFACE_TYPE_FLOOR: {
					_is_on_floor = true;
				} break;
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
	// Snapping to surfaces.
	for (SurfaceType type = SURFACE_TYPE_WALL; type <= SURFACE_TYPE_FLOOR; type = SurfaceType(type + 1)) {
		Vector4 snap_direction;
		SurfaceType type_to_try = type;
		// The snap_to_* flags determine which directions to use, but the results will depend on the normal of the surface we hit.
		if (type == SURFACE_TYPE_WALL) {
			if (!_snap_to_wall || !was_on_wall || _is_on_wall) {
				continue;
			}
			if (global_up_direction == Vector4()) {
				type_to_try = SURFACE_TYPE_ANY;
			}
			snap_direction = -_last_wall_normal;
		} else if (type == SURFACE_TYPE_CEILING) {
			if (!_snap_to_ceiling || !was_on_ceiling || _is_on_ceiling) {
				continue;
			}
			snap_direction = global_up_direction;
		} else if (type == SURFACE_TYPE_FLOOR) {
			if (!_snap_to_floor || !was_on_floor || _is_on_floor) {
				continue;
			}
			snap_direction = -global_up_direction;
		}
		// Only snap if the velocity is pointing towards or along the surface (dot positive or near-zero).
		// For example, we don't want to snap when jumping off the floor.
		if (snap_direction.dot(_linear_velocity) > -CMP_EPSILON) {
			const bool snapped = try_snap_to_surface(snap_direction * _snap_distance, type_to_try, delta_time);
			if (snapped) {
				switch (type) {
					case SURFACE_TYPE_ANY: // Treat "any" surface as a wall for the purposes of _is_on_wall / _snap_to_wall / etc.
					case SURFACE_TYPE_WALL: {
						_is_on_wall = true;
					} break;
					case SURFACE_TYPE_CEILING: {
						_is_on_ceiling = true;
					} break;
					case SURFACE_TYPE_FLOOR: {
						_is_on_floor = true;
					} break;
				}
			}
		}
	}
	return collisions;
}

bool CharacterBody4D::try_snap_to_surface(const Vector4 &p_snap_motion, const SurfaceType p_allowed_surface_type, const double p_delta_time) {
	double delta_time;
	if (p_delta_time < 0.0) {
		// Hack in order to work with calling from _process as well as from _physics_process. Copied from CharacterBody3D.
		delta_time = Engine::get_singleton()->is_in_physics_frame() ? get_physics_process_delta_time() : get_process_delta_time();
	} else {
		delta_time = p_delta_time;
	}
	Ref<KinematicCollision4D> test = move_and_collide(p_snap_motion, true, delta_time);
	if (test.is_null()) {
		return false;
	}
	const real_t travel_ratio = test->get_travel_ratio();
	if (travel_ratio >= 1.0f) {
		// This last move_and_collide call moved us the rest of the way, with no collisions.
		return false;
	}
	const Vector4 normal = test->get_normal();
	const Vector4 global_up_direction = get_global_basis().xform(_local_up_direction);
	const SurfaceType surface_type = _determine_surface_type(global_up_direction, normal);
	if (p_allowed_surface_type != SURFACE_TYPE_ANY) {
		if (surface_type != p_allowed_surface_type) {
			return false;
		}
	}
	if (surface_type == SURFACE_TYPE_ANY || surface_type == SURFACE_TYPE_WALL) {
		_last_wall_normal = normal;
	}
	if (travel_ratio <= 0.0f) {
		// Already overlapping or zero distance, let's consider this a successful snap.
		return true;
	}
	// The test hit something, so apply it. We need to re-run `move_and_collide` to actually move the body,
	// not just set the position directly, because the motion needs to check area nodes along the way.
	// DO NOT pre-multiply the travel ratio, it breaks things, we must let `move_and_collide` handle the full motion.
	move_and_collide(p_snap_motion, false, delta_time);
	return true;
}

void CharacterBody4D::set_snap_distance(const real_t p_snap_distance) {
	ERR_FAIL_COND_MSG(p_snap_distance < 0.0, "CharacterBody4D: Snap distance must not be negative. Refusing to set.");
	_snap_distance = p_snap_distance;
}

void CharacterBody4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_on_ceiling"), &CharacterBody4D::is_on_ceiling);
	ClassDB::bind_method(D_METHOD("is_on_floor"), &CharacterBody4D::is_on_floor);
	ClassDB::bind_method(D_METHOD("is_on_wall"), &CharacterBody4D::is_on_wall);

	ClassDB::bind_method(D_METHOD("move_and_slide", "delta_time"), &CharacterBody4D::move_and_slide, DEFVAL(-1.0));
	ClassDB::bind_method(D_METHOD("try_snap_to_surface", "snap_motion", "allowed_surface_type", "delta_time"), &CharacterBody4D::try_snap_to_surface, DEFVAL(SURFACE_TYPE_ANY), DEFVAL(-1.0));

	ClassDB::bind_method(D_METHOD("get_floor_max_angle"), &CharacterBody4D::get_floor_max_angle);
	ClassDB::bind_method(D_METHOD("set_floor_max_angle", "floor_max_angle"), &CharacterBody4D::set_floor_max_angle);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "floor_max_angle", PROPERTY_HINT_RANGE, "0,180,0.1,radians_as_degrees"), "set_floor_max_angle", "get_floor_max_angle");

	ClassDB::bind_method(D_METHOD("get_snap_distance"), &CharacterBody4D::get_snap_distance);
	ClassDB::bind_method(D_METHOD("set_snap_distance", "snap_distance"), &CharacterBody4D::set_snap_distance);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "snap_distance", PROPERTY_HINT_RANGE, "0,100,0.01,or_greater"), "set_snap_distance", "get_snap_distance");

	ClassDB::bind_method(D_METHOD("get_linear_velocity"), &CharacterBody4D::get_linear_velocity);
	ClassDB::bind_method(D_METHOD("set_linear_velocity", "linear_velocity"), &CharacterBody4D::set_linear_velocity);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "linear_velocity", PROPERTY_HINT_NONE, "suffix:m/s"), "set_linear_velocity", "get_linear_velocity");

	ClassDB::bind_method(D_METHOD("get_local_up_direction"), &CharacterBody4D::get_local_up_direction);
	ClassDB::bind_method(D_METHOD("set_local_up_direction", "local_up_direction"), &CharacterBody4D::set_local_up_direction);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "local_up_direction"), "set_local_up_direction", "get_local_up_direction");

	ClassDB::bind_method(D_METHOD("get_snap_to_ceiling"), &CharacterBody4D::get_snap_to_ceiling);
	ClassDB::bind_method(D_METHOD("set_snap_to_ceiling", "snap_to_ceiling"), &CharacterBody4D::set_snap_to_ceiling);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "snap_to_ceiling"), "set_snap_to_ceiling", "get_snap_to_ceiling");

	ClassDB::bind_method(D_METHOD("get_snap_to_floor"), &CharacterBody4D::get_snap_to_floor);
	ClassDB::bind_method(D_METHOD("set_snap_to_floor", "snap_to_floor"), &CharacterBody4D::set_snap_to_floor);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "snap_to_floor"), "set_snap_to_floor", "get_snap_to_floor");

	ClassDB::bind_method(D_METHOD("get_snap_to_wall"), &CharacterBody4D::get_snap_to_wall);
	ClassDB::bind_method(D_METHOD("set_snap_to_wall", "snap_to_wall"), &CharacterBody4D::set_snap_to_wall);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "snap_to_wall"), "set_snap_to_wall", "get_snap_to_wall");

	ClassDB::bind_method(D_METHOD("get_use_bounce"), &CharacterBody4D::get_use_bounce);
	ClassDB::bind_method(D_METHOD("set_use_bounce", "use_bounce"), &CharacterBody4D::set_use_bounce);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_bounce"), "set_use_bounce", "get_use_bounce");

	BIND_ENUM_CONSTANT(SURFACE_TYPE_ANY);
	BIND_ENUM_CONSTANT(SURFACE_TYPE_WALL);
	BIND_ENUM_CONSTANT(SURFACE_TYPE_CEILING);
	BIND_ENUM_CONSTANT(SURFACE_TYPE_FLOOR);
}
