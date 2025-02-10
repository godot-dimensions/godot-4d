#pragma once

#include "../godot_4d_defines.h"

#if GDEXTENSION
#include <godot_cpp/classes/ref_counted.hpp>
#elif GODOT_MODULE
#include "core/object/ref_counted.h"
#endif

class CollisionShape4D;
class PhysicsBody4D;

class KinematicCollision4D : public RefCounted {
	GDCLASS(KinematicCollision4D, RefCounted);

	CollisionShape4D *_moving_shape_node;
	CollisionShape4D *_obstacle_shape_node;
	Vector4 _normal;
	Vector4 _relative_velocity;
	real_t _travel_ratio;

protected:
	static void _bind_methods();

public:
	real_t get_bounce_ratio() const;
	PhysicsBody4D *get_moving_body_node() const;
	PhysicsBody4D *get_obstacle_body_node() const;

	CollisionShape4D *get_moving_shape_node() const { return _moving_shape_node; }
	void set_moving_shape_node(CollisionShape4D *p_moving_shape_node) { _moving_shape_node = p_moving_shape_node; }

	CollisionShape4D *get_obstacle_shape_node() const { return _obstacle_shape_node; }
	void set_obstacle_shape_node(CollisionShape4D *p_obstacle_shape_node) { _obstacle_shape_node = p_obstacle_shape_node; }

	Vector4 get_normal() const { return _normal; }
	void set_normal(const Vector4 &p_normal) { _normal = p_normal; }

	real_t get_travel_ratio() const { return _travel_ratio; }
	void set_travel_ratio(const real_t p_travel_ratio) { _travel_ratio = p_travel_ratio; }

	Vector4 get_relative_velocity() const { return _relative_velocity; }
	void set_relative_velocity(const Vector4 &p_relative_velocity) { _relative_velocity = p_relative_velocity; }
};
