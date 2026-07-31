#pragma once

#include "../../godot_4d_defines.h"

#if GDEXTENSION
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#elif GODOT_MODULE
#include "core/object/ref_counted.h"
#include "core/variant/typed_array.h"
#endif

class Node4D;

class RaycastParameters4D : public RefCounted {
	GDCLASS(RaycastParameters4D, RefCounted);

	// This is performance-critical, so store `Vector<>` internally, not `TypedArray<>`.
	// Also, we expect that the number of excluded nodes will be small, so prefer `Vector<>` over `HashSet<>`.
	Vector<ObjectID> _exclude_nodes;

	Vector4 _global_ray_direction;
	Vector4 _global_ray_origin;
	double _max_distance = Math_INF;
	uint32_t _collision_mask = 0xFFFFFFFFu;

	bool _collide_with_areas = false;
	bool _collide_with_bodies = true;
	bool _inside_is_skip = true;
	bool _inside_is_zero = false;

protected:
	static void _bind_methods();

public:
	const Vector<ObjectID> &get_exclude_nodes() const { return _exclude_nodes; }
	void set_exclude_nodes(const Vector<ObjectID> &p_exclude_nodes) { _exclude_nodes = p_exclude_nodes; }

	TypedArray<Node4D> get_exclude_nodes_bind() const;
	void set_exclude_nodes_bind(const TypedArray<Node4D> &p_exclude_nodes);

	Vector4 get_global_ray_direction() const { return _global_ray_direction; }
	void set_global_ray_direction(const Vector4 &p_global_ray_direction) { _global_ray_direction = p_global_ray_direction; }

	Vector4 get_global_ray_origin() const { return _global_ray_origin; }
	void set_global_ray_origin(const Vector4 &p_global_ray_origin) { _global_ray_origin = p_global_ray_origin; }

	double get_max_distance() const { return _max_distance; }
	void set_max_distance(const double p_max_distance) { _max_distance = p_max_distance; }

	uint32_t get_collision_mask() const { return _collision_mask; }
	void set_collision_mask(const uint32_t p_collision_mask) { _collision_mask = p_collision_mask; }

	bool get_collide_with_areas() const { return _collide_with_areas; }
	void set_collide_with_areas(const bool p_collide_with_areas) { _collide_with_areas = p_collide_with_areas; }

	bool get_collide_with_bodies() const { return _collide_with_bodies; }
	void set_collide_with_bodies(const bool p_collide_with_bodies) { _collide_with_bodies = p_collide_with_bodies; }

	bool get_inside_is_skip() const { return _inside_is_skip; }
	void set_inside_is_skip(const bool p_inside_is_skip) { _inside_is_skip = p_inside_is_skip; }

	bool get_inside_is_zero() const { return _inside_is_zero; }
	void set_inside_is_zero(const bool p_inside_is_zero) { _inside_is_zero = p_inside_is_zero; }
};
