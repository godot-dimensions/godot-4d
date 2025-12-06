#include "axis_aligned_box_physics_engine_4d.h"

#include "../../math/geometric_algebra/rotor_4d.h"
#include "../../math/vector_4d.h"
#include "../bodies/rigid_body_4d.h"
#include "../collision_shape_4d.h"
#include "physics_server_4d.h"

Vector<Rect4> _calculate_shape_rects(const TypedArray<CollisionShape4D> &p_shapes) {
	Vector<Rect4> shape_rects;
	const int shape_count = p_shapes.size();
	shape_rects.resize(shape_count);
	for (int shape_index = 0; shape_index < shape_count; shape_index++) {
		CollisionShape4D *shape = Object::cast_to<CollisionShape4D>(p_shapes[shape_index]);
		shape_rects.set(shape_index, shape->get_rect_bounds());
	}
	return shape_rects;
}

bool _do_moving_shapes_overlap(const Vector<Rect4> &p_moving_shape_rects, const Vector<Rect4> &p_obstacle_shape_rects, const Vector4 &p_motion) {
	const int shape_count_a = p_moving_shape_rects.size();
	const int shape_count_b = p_obstacle_shape_rects.size();
	for (int shape_index_a = 0; shape_index_a < shape_count_a; shape_index_a++) {
		const Rect4 &shape_rect_a = p_moving_shape_rects[shape_index_a];
		for (int shape_index_b = 0; shape_index_b < shape_count_b; shape_index_b++) {
			const Rect4 &shape_rect_b = p_obstacle_shape_rects[shape_index_b];
			if (shape_rect_a.continuous_collision_overlaps(p_motion, shape_rect_b)) {
				return true;
			}
		}
	}
	return false;
}

bool _do_static_shapes_overlap(const Vector<Rect4> &p_shape_rects_a, const Vector<Rect4> &p_shape_rects_b) {
	const int shape_count_a = p_shape_rects_a.size();
	const int shape_count_b = p_shape_rects_b.size();
	for (int shape_index_a = 0; shape_index_a < shape_count_a; shape_index_a++) {
		const Rect4 &shape_rect_a = p_shape_rects_a[shape_index_a];
		for (int shape_index_b = 0; shape_index_b < shape_count_b; shape_index_b++) {
			const Rect4 &shape_rect_b = p_shape_rects_b[shape_index_b];
			// Inclusive is required here to pair well with the behavior of the continuous collision function.
			if (shape_rect_a.intersects_inclusive(shape_rect_b)) {
				return true;
			}
		}
	}
	return false;
}

const Vector<Rect4> &AxisAlignedBoxPhysicsEngine4D::get_body_shape_rects(CollisionObject4D *p_moving_body, const TypedArray<CollisionShape4D> &p_body_shapes) {
	if (!_area_and_body_rects.has(p_moving_body) || _area_and_body_rects[p_moving_body].size() != p_body_shapes.size()) {
		_area_and_body_rects[p_moving_body] = _calculate_shape_rects(p_body_shapes);
	}
	return _area_and_body_rects[p_moving_body];
}

Ref<KinematicCollision4D> AxisAlignedBoxPhysicsEngine4D::_check_motion_until_obstacle(CollisionObject4D *p_moving_body, const Vector<Rect4> &p_moving_rects, const Vector4 &p_motion) {
	Ref<KinematicCollision4D> ret;
	ret.instantiate();
	// Get information about bodies.
	const TypedArray<CollisionShape4D> &moving_shapes = p_moving_body->get_collision_shapes();
	const Vector<Rect4> &moving_shape_rects = get_body_shape_rects(p_moving_body, moving_shapes);
	const TypedArray<PhysicsBody4D> &obstacle_body_nodes = PhysicsServer4D::get_singleton()->get_physics_body_nodes();
	const int obstacle_body_count = obstacle_body_nodes.size();
	// Calculation the ratio of the desired motion that can be taken before hitting an obstacle.
	real_t travel_ratio = 1.0f;
	Vector4 r_out_normal;
	for (int obstacle_body_index = 0; obstacle_body_index < obstacle_body_count; obstacle_body_index++) {
		PhysicsBody4D *obstacle_body_node = Object::cast_to<PhysicsBody4D>(obstacle_body_nodes[obstacle_body_index]);
		if (obstacle_body_node == p_moving_body) {
			continue; // Don't collide with ourself.
		}
		RigidBody4D *obstacle_rigid_body = Object::cast_to<RigidBody4D>(obstacle_body_node);
		const TypedArray<CollisionShape4D> &obstacle_body_shapes = obstacle_body_node->get_collision_shapes();
		const Vector<Rect4> &obstacle_shape_rects = get_body_shape_rects(obstacle_body_node, obstacle_body_shapes);
		// Iterate through both sets of shapes and check for collisions.
		for (int moving_shape_index = 0; moving_shape_index < moving_shape_rects.size(); moving_shape_index++) {
			CollisionShape4D *moving_shape = Object::cast_to<CollisionShape4D>(moving_shapes[moving_shape_index]);
			const Rect4 &moving_shape_rect = moving_shape_rects[moving_shape_index];
			for (int obstacle_shape_index = 0; obstacle_shape_index < obstacle_shape_rects.size(); obstacle_shape_index++) {
				CollisionShape4D *obstacle_shape = Object::cast_to<CollisionShape4D>(obstacle_body_shapes[obstacle_shape_index]);
				if (moving_shape->get_collision_mask() & obstacle_shape->get_collision_layer()) {
					const Rect4 &obstacle_shape_rect = obstacle_shape_rects[obstacle_shape_index];
					// The CCD function operates with a relative velocity, so we can check for rigid bodies and use their velocity.
					const Vector4 relative_motion = obstacle_rigid_body ? (p_motion - obstacle_rigid_body->get_linear_velocity() * _physics_delta_time) : p_motion;
					Vector4 this_normal;
					const real_t this_ratio = moving_shape_rect.continuous_collision_depth(relative_motion, obstacle_shape_rect, &this_normal);
					if (this_ratio < travel_ratio) {
						travel_ratio = this_ratio;
						r_out_normal = this_normal; // We remember the normal only from the nearest collision.
						ret->set_moving_shape_node(moving_shape);
						ret->set_obstacle_shape_node(obstacle_shape);
						ret->set_relative_velocity(relative_motion / _physics_delta_time);
					}
				}
			}
		}
	}
	ret->set_normal(r_out_normal);
	ret->set_travel_ratio(travel_ratio);
	return ret;
}

HashSet<Area4D *> AxisAlignedBoxPhysicsEngine4D::_step_dynamic_rigid_body(RigidBody4D *p_moving_body, double p_delta) {
	const Vector4 start_global_position = p_moving_body->get_global_position();
	// Used for determining overlaps with Area4D nodes.
	const TypedArray<Area4D> &area_nodes = PhysicsServer4D::get_singleton()->get_area_nodes();
	const int area_count = area_nodes.size();
	HashSet<Area4D *> overlapping_areas;
	// Calculate the motion.
	Vector4 actual_motion = Vector4();
	{
		Vector4 linear_velocity = p_moving_body->get_linear_velocity();
		Vector4 desired_motion = linear_velocity * p_delta;
		Vector<Rect4> current_moving_shape_rects = _area_and_body_rects[p_moving_body];
		constexpr int MAX_RIGID_BODY_ITERATIONS = 10;
		for (int iteration = 0; iteration < MAX_RIGID_BODY_ITERATIONS; iteration++) {
			Ref<KinematicCollision4D> collision = _check_motion_until_obstacle(p_moving_body, current_moving_shape_rects, desired_motion);
			Vector4 motion_this_iteration;
			real_t travel_ratio = collision->get_travel_ratio();
			if (travel_ratio == 1.0f) {
				// Move the rest of the way.
				motion_this_iteration = desired_motion;
				actual_motion += desired_motion;
				desired_motion = Vector4();
			} else {
				// Move part of the way, since we hit an obstacle.
				// The safe_margin tries to prevent objects from getting too close, but is not enough by itself.
				// We still need a proper depenetration algorithm to fix already-overlapping objects.
				const Vector4 safe_margin = collision->get_normal() * PHYSICS_4D_SAFE_MARGIN;
				motion_this_iteration = desired_motion * travel_ratio + safe_margin;
				actual_motion += motion_this_iteration;
				desired_motion = desired_motion * (1.0f - travel_ratio);
				// Emit signals.
				PhysicsBody4D *obstacle_body = collision->get_obstacle_body_node();
				if (obstacle_body) {
					obstacle_body->emit_signal(StringName("body_impacted_self"), p_moving_body, collision);
					p_moving_body->emit_signal(StringName("self_impacted_body"), obstacle_body, collision);
				}
				// Bouncing.
				const real_t bounce_ratio = collision->get_bounce_ratio();
				desired_motion = Vector4D::bounce_ratio(desired_motion, collision->get_normal(), bounce_ratio);
				linear_velocity = Vector4D::bounce_ratio(linear_velocity, collision->get_normal(), bounce_ratio);
			}
			// Check for overlaps with areas.
			for (int area_index = 0; area_index < area_count; area_index++) {
				Area4D *area_node = Object::cast_to<Area4D>(area_nodes[area_index]);
				if (overlapping_areas.has(area_node)) {
					continue; // Already overlapping, no need to check again.
				}
				const Vector<Rect4> &area_rects = _area_and_body_rects[area_node];
				if (_do_moving_shapes_overlap(current_moving_shape_rects, area_rects, motion_this_iteration)) {
					overlapping_areas.insert(area_node);
				}
			}
			// Update the position and current shape rects for next iteration.
			p_moving_body->set_global_position(start_global_position + actual_motion);
			current_moving_shape_rects = _calculate_shape_rects(p_moving_body->get_collision_shapes());
			if (desired_motion.is_zero_approx()) {
				break;
			}
		}
		p_moving_body->set_linear_velocity(linear_velocity);
		_rigid_body_rect_queue[p_moving_body] = current_moving_shape_rects;
	}
	return overlapping_areas;
}

Ref<KinematicCollision4D> AxisAlignedBoxPhysicsEngine4D::move_and_collide(PhysicsBody4D *p_moving_body, Vector4 p_motion, bool p_test_only) {
	// Calculate the motion.
	const Vector<Rect4> &moving_shape_rects = get_body_shape_rects(p_moving_body, p_moving_body->get_collision_shapes());
	Ref<KinematicCollision4D> collision = _check_motion_until_obstacle(p_moving_body, moving_shape_rects, p_motion);
	if (p_test_only) {
		return collision;
	}
	real_t travel_ratio = collision->get_travel_ratio();
	// A negative travel ratio means we're inside the obstacle.
	// TODO: Implement a proper depenetration algorithm. For now, just stop moving.
	if (travel_ratio < 0.0f) {
		collision->set_travel_ratio(0.0f);
		travel_ratio = 0.0f;
	}
	// The safe_margin tries to prevent objects from getting too close, but is not enough by itself.
	// We still need a proper depenetration algorithm to fix already-overlapping objects.
	Vector4 safe_margin = collision->get_normal() * PHYSICS_4D_SAFE_MARGIN;
	p_moving_body->set_global_position(p_moving_body->get_global_position() + p_motion * travel_ratio + safe_margin);
	Vector<Rect4> moving_rects_after = _calculate_shape_rects(p_moving_body->get_collision_shapes());
	// Check for overlaps with areas.
	const TypedArray<Area4D> &area_nodes = PhysicsServer4D::get_singleton()->get_area_nodes();
	for (int area_index = 0; area_index < area_nodes.size(); area_index++) {
		Area4D *area_node = Object::cast_to<Area4D>(area_nodes[area_index]);
		const Vector<Rect4> &static_area_rects = _area_and_body_rects[area_node];
		if (_do_moving_shapes_overlap(moving_shape_rects, static_area_rects, p_motion * travel_ratio)) {
			if (!_do_static_shapes_overlap(moving_shape_rects, static_area_rects)) {
				// If they overlap during the motion, but didn't before, emit the entered signals.
				area_node->emit_signal(StringName("body_entered_area"), p_moving_body);
				p_moving_body->emit_signal(StringName("self_entered_area"), area_node);
			}
			if (!_do_static_shapes_overlap(moving_rects_after, static_area_rects)) {
				// If they overlap during the motion, but don't after, emit the exited signals.
				area_node->emit_signal(StringName("body_exited_area"), p_moving_body);
				p_moving_body->emit_signal(StringName("self_exited_area"), area_node);
			}
		}
	}
	_area_and_body_rects[p_moving_body] = moving_rects_after;
	return collision;
}

void AxisAlignedBoxPhysicsEngine4D::move_area(Area4D *p_moving_area, Vector4 p_motion) {
	p_moving_area->set_global_position(p_moving_area->get_global_position() + p_motion);
	const Vector<Rect4> &moving_shape_rects_before = _area_and_body_rects[p_moving_area];
	Vector<Rect4> moving_area_rects_after = _calculate_shape_rects(p_moving_area->get_collision_shapes());
	// Check for overlaps with bodies and other areas.
	const TypedArray<Area4D> &area_nodes = PhysicsServer4D::get_singleton()->get_area_nodes();
	for (int area_index = 0; area_index < area_nodes.size(); area_index++) {
		Area4D *static_area_node = Object::cast_to<Area4D>(area_nodes[area_index]);
		if (static_area_node == p_moving_area) {
			continue; // Don't collide with ourself.
		}
		const Vector<Rect4> &static_area_rects = _area_and_body_rects[static_area_node];
		if (_do_moving_shapes_overlap(moving_shape_rects_before, static_area_rects, p_motion)) {
			if (!_do_static_shapes_overlap(moving_shape_rects_before, static_area_rects)) {
				// If they overlap during the motion, but didn't before, emit the entered signals.
				static_area_node->emit_signal(StringName("area_entered_self"), p_moving_area);
				p_moving_area->emit_signal(StringName("self_entered_area"), static_area_node);
			}
			if (!_do_static_shapes_overlap(moving_area_rects_after, static_area_rects)) {
				// If they overlap during the motion, but don't after, emit the exited signals.
				static_area_node->emit_signal(StringName("area_exited_self"), p_moving_area);
				p_moving_area->emit_signal(StringName("self_exited_area"), static_area_node);
			}
		}
	}
	const TypedArray<PhysicsBody4D> &body_nodes = PhysicsServer4D::get_singleton()->get_physics_body_nodes();
	for (int body_index = 0; body_index < body_nodes.size(); body_index++) {
		PhysicsBody4D *static_body_node = Object::cast_to<PhysicsBody4D>(body_nodes[body_index]);
		const Vector<Rect4> &static_body_rects = _area_and_body_rects[static_body_node];
		if (_do_moving_shapes_overlap(moving_shape_rects_before, static_body_rects, p_motion)) {
			if (!_do_static_shapes_overlap(moving_shape_rects_before, static_body_rects)) {
				// If they overlap during the motion, but didn't before, emit the entered signals.
				static_body_node->emit_signal(StringName("self_entered_area"), p_moving_area);
				p_moving_area->emit_signal(StringName("body_entered_area"), static_body_node);
			}
			if (!_do_static_shapes_overlap(moving_area_rects_after, static_body_rects)) {
				// If they overlap during the motion, but don't after, emit the exited signals.
				static_body_node->emit_signal(StringName("self_exited_area"), p_moving_area);
				p_moving_area->emit_signal(StringName("body_exited_area"), static_body_node);
			}
		}
	}
	_area_and_body_rects[p_moving_area] = moving_area_rects_after;
}

void AxisAlignedBoxPhysicsEngine4D::physics_process(double p_delta) {
	_physics_delta_time = p_delta;
	// Recalculate rects for all areas and bodies each frame, just in case they changed.
	_area_and_body_rects.clear();
	const TypedArray<Area4D> &area_nodes = PhysicsServer4D::get_singleton()->get_area_nodes();
	const TypedArray<PhysicsBody4D> &body_nodes = PhysicsServer4D::get_singleton()->get_physics_body_nodes();
	for (int area_index = 0; area_index < area_nodes.size(); area_index++) {
		CollisionObject4D *area_node = Object::cast_to<CollisionObject4D>(area_nodes[area_index]);
		_area_and_body_rects[area_node] = _calculate_shape_rects(area_node->get_collision_shapes());
	}
	for (int body_index = 0; body_index < body_nodes.size(); body_index++) {
		CollisionObject4D *body_node = Object::cast_to<CollisionObject4D>(body_nodes[body_index]);
		_area_and_body_rects[body_node] = _calculate_shape_rects(body_node->get_collision_shapes());
		// Also since we're in a loop with bodies, apply gravity here, before stepping.
		RigidBody4D *rigid_body = Object::cast_to<RigidBody4D>(body_node);
		if (rigid_body) {
			rigid_body->apply_acceleration(rigid_body->get_scaled_gravity());
		}
	}
	// Step dynamic rigid bodies with the calculated rects.
	for (int body_index = 0; body_index < body_nodes.size(); body_index++) {
		RigidBody4D *rigid_body = Object::cast_to<RigidBody4D>(body_nodes[body_index]);
		if (rigid_body) {
			HashSet<Area4D *> overlapping_areas = _step_dynamic_rigid_body(rigid_body, p_delta);
			// Check for overlaps with areas. The HashSet tell us which areas overlap
			// via the whole CCD motion, but we need to check before and after.
			for (Area4D *area_node : overlapping_areas) {
				const Vector<Rect4> &area_rects = _area_and_body_rects[area_node];
				if (!_do_static_shapes_overlap(_area_and_body_rects[rigid_body], area_rects)) {
					// If they overlap during the motion, but didn't before, emit the entered signals.
					area_node->emit_signal(StringName("body_entered_area"), rigid_body);
					rigid_body->emit_signal(StringName("self_entered_area"), area_node);
				}
				if (!_do_static_shapes_overlap(_rigid_body_rect_queue[rigid_body], area_rects)) {
					// If they overlap during the motion, but don't after, emit the exited signals.
					area_node->emit_signal(StringName("body_exited_area"), rigid_body);
					rigid_body->emit_signal(StringName("self_exited_area"), area_node);
				}
			}
		}
	}
	// Update rects for rigid bodies from queue. Doing this separately from stepping
	// allows rigid bodies to move next to each other and hit each other, with the
	// same behavior regardless of the order they are processed in.
	for (const KeyValue<RigidBody4D *, Vector<Rect4>> &item : _rigid_body_rect_queue) {
		_area_and_body_rects[item.key] = item.value;
	}
	_rigid_body_rect_queue.clear();
}
