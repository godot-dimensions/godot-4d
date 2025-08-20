#pragma once

#include "physics_body_4d.h"

class CharacterBody4D : public PhysicsBody4D {
	GDCLASS(CharacterBody4D, PhysicsBody4D);

	Vector4 _linear_velocity = Vector4();
	Vector4 _up_direction = Vector4D::DIR_UP;
	real_t _floor_max_angle = Math_TAU / 4.0;
	bool _is_on_ceiling = false;
	bool _is_on_floor = false;
	bool _is_on_wall = false;
	bool _use_bounce = true;

protected:
	static void _bind_methods();

public:
	bool is_on_ceiling() const { return _is_on_ceiling; }
	bool is_on_floor() const { return _is_on_floor; }
	bool is_on_wall() const { return _is_on_wall; }

	TypedArray<KinematicCollision4D> move_and_slide();

	real_t get_floor_max_angle() const { return _floor_max_angle; }
	void set_floor_max_angle(const real_t p_floor_max_angle) { _floor_max_angle = p_floor_max_angle; }

	Vector4 get_linear_velocity() const { return _linear_velocity; }
	void set_linear_velocity(const Vector4 &p_linear_velocity) { _linear_velocity = p_linear_velocity; }

	Vector4 get_up_direction() const { return _up_direction; }
	void set_up_direction(const Vector4 &p_up_direction) { _up_direction = p_up_direction; }

	bool get_use_bounce() const { return _use_bounce; }
	void set_use_bounce(const bool p_use_bounce) { _use_bounce = p_use_bounce; }
};
