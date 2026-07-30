#pragma once

#include "physics_body_4d.h"

class CharacterBody4D : public PhysicsBody4D {
	GDCLASS(CharacterBody4D, PhysicsBody4D);

public:
	enum SurfaceType {
		SURFACE_TYPE_ANY,
		SURFACE_TYPE_WALL,
		SURFACE_TYPE_CEILING,
		SURFACE_TYPE_FLOOR,
	};

private:
	Vector4 _last_wall_normal = Vector4();
	Vector4 _linear_velocity = Vector4();
	Vector4 _local_up_direction = Vector4D::DIR_UP;

	real_t _floor_max_angle = Math_TAU / 8.0;
	real_t _snap_distance = 0.1;

	bool _is_on_ceiling = false;
	bool _is_on_floor = false;
	bool _is_on_wall = false;
	bool _snap_to_ceiling = false;
	bool _snap_to_floor = true;
	bool _snap_to_wall = false;
	bool _use_bounce = true;

	SurfaceType _determine_surface_type(const Vector4 &p_global_up_direction, const Vector4 &p_surface_normal) const;

protected:
	static void _bind_methods();

public:
	bool is_on_ceiling() const { return _is_on_ceiling; }
	bool is_on_floor() const { return _is_on_floor; }
	bool is_on_wall() const { return _is_on_wall; }

	TypedArray<KinematicCollision4D> move_and_slide(const double p_delta_time = -1.0);
	bool try_snap_to_surface(const Vector4 &p_snap_motion, const SurfaceType p_allowed_surface_type = SURFACE_TYPE_ANY, const double p_delta_time = -1.0);

	real_t get_floor_max_angle() const { return _floor_max_angle; }
	void set_floor_max_angle(const real_t p_floor_max_angle) { _floor_max_angle = p_floor_max_angle; }

	real_t get_snap_distance() const { return _snap_distance; }
	void set_snap_distance(const real_t p_snap_distance);

	Vector4 get_linear_velocity() const { return _linear_velocity; }
	void set_linear_velocity(const Vector4 &p_linear_velocity) { _linear_velocity = p_linear_velocity; }

	Vector4 get_local_up_direction() const { return _local_up_direction; }
	void set_local_up_direction(const Vector4 &p_local_up_direction) { _local_up_direction = p_local_up_direction; }

	bool get_snap_to_ceiling() const { return _snap_to_ceiling; }
	void set_snap_to_ceiling(const bool p_snap_to_ceiling) { _snap_to_ceiling = p_snap_to_ceiling; }

	bool get_snap_to_floor() const { return _snap_to_floor; }
	void set_snap_to_floor(const bool p_snap_to_floor) { _snap_to_floor = p_snap_to_floor; }

	bool get_snap_to_wall() const { return _snap_to_wall; }
	void set_snap_to_wall(const bool p_snap_to_wall) { _snap_to_wall = p_snap_to_wall; }

	bool get_use_bounce() const { return _use_bounce; }
	void set_use_bounce(const bool p_use_bounce) { _use_bounce = p_use_bounce; }
};

VARIANT_ENUM_CAST(CharacterBody4D::SurfaceType);
