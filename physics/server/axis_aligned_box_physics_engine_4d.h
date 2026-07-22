#pragma once

#include "physics_engine_4d.h"

#if GDEXTENSION
#include <godot_cpp/templates/hash_set.hpp>
#endif

// Axis-aligned bounding box physics engine with continuous collision detection using Rect4.
// This physics engine does not support rotation or angular velocity.
// Non-BoxShape4D shapes will work, but will be converted to boxes.
// Doesn't use any space partitioning, but could be added in the future.
class AxisAlignedBoxPhysicsEngine4D : public PhysicsEngine4D {
	GDCLASS(AxisAlignedBoxPhysicsEngine4D, PhysicsEngine4D);

	HashMap<CollisionObject4D *, Vector<Rect4>> _area_and_body_rects;
	HashMap<RigidBody4D *, Vector<Rect4>> _dynamic_rigid_body_rect_queue;
	double _most_recent_physics_delta_time = 0.0;

	static Vector<Rect4> _calculate_shape_rects(const TypedArray<CollisionShape4D> &p_shapes);
	static bool _do_moving_shapes_overlap(const Vector<Rect4> &p_moving_shape_rects, const TypedArray<CollisionShape4D> &p_moving_shapes, const Vector<Rect4> &p_obstacle_shape_rects, const TypedArray<CollisionShape4D> &p_obstacle_shapes, const Vector4 &p_motion, const bool p_moving_shapes_are_detector);
	static bool _do_static_shapes_overlap(const Vector<Rect4> &p_shape_rects_a, const TypedArray<CollisionShape4D> &p_shapes_a, const Vector<Rect4> &p_shape_rects_b, const TypedArray<CollisionShape4D> &p_shapes_b, const bool p_shapes_a_are_detector);

	const Vector<Rect4> &get_body_shape_rects(CollisionObject4D *p_body, const TypedArray<CollisionShape4D> &p_body_shapes);
	Ref<KinematicCollision4D> _check_motion_until_obstacle(CollisionObject4D *p_moving_body, const Vector<Rect4> &p_moving_rects, const Vector4 &p_motion, const double p_delta_time);
	HashSet<Area4D *> _step_dynamic_rigid_body(RigidBody4D *p_moving_body, const double p_delta_time);

protected:
	static void _bind_methods() {}

public:
	virtual Ref<KinematicCollision4D> move_and_collide(PhysicsBody4D *p_body, const Vector4 &p_motion, const bool p_test_only, const double p_delta_time) override;
	virtual void move_area(Area4D *p_area, const Vector4 &p_motion) override;
	virtual void physics_process(const double p_delta_time) override;
};
