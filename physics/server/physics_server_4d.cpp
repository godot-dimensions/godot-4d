#include "physics_server_4d.h"

#include "../bodies/static_body_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>
#elif GODOT_MODULE
#include "core/core_bind.h"
#include "scene/main/scene_tree.h"
#include "scene/main/window.h"
#endif

void PhysicsServer4D::_raycast_shapes_fast_rect4(const Ref<RaycastParameters4D> &p_raycast_parameters, CollisionObject4D *p_col_obj_node, Vector<RaycastCandidate> &r_candidates) const {
	CRASH_COND_MSG(p_col_obj_node == nullptr, "PhysicsServer4D: CollisionObject4D node is null.");
	TypedArray<CollisionShape4D> collision_shapes = p_col_obj_node->get_collision_shapes();
	const Vector<ObjectID> &exclude_nodes = p_raycast_parameters->get_exclude_nodes();
	const Vector4 global_ray_direction = p_raycast_parameters->get_global_ray_direction();
	const Vector4 global_ray_origin = p_raycast_parameters->get_global_ray_origin();
	const double max_distance = p_raycast_parameters->get_max_distance();
	const uint32_t collision_mask = p_raycast_parameters->get_collision_mask();
	// We can't use `inside_is_skip` here because the Rect4 may be bigger than the actual shape.
	// We can't use `inside_is_zero` either because the exit point may be farther than the max distance.
	for (int shape_index = 0; shape_index < collision_shapes.size(); shape_index++) {
		CollisionShape4D *collision_shape = Object::cast_to<CollisionShape4D>(collision_shapes[shape_index]);
		CRASH_COND_MSG(collision_shape == nullptr, "PhysicsServer4D: CollisionShape4D node is null.");
		// Skip shapes whose collision layer does not match the collision mask.
		if ((collision_shape->get_collision_layer() & collision_mask) == 0) {
			continue;
		}
		const ObjectID col_shape_obj_id = ObjectID(collision_shape->get_instance_id());
		if (exclude_nodes.find(col_shape_obj_id) != -1) {
			continue;
		}
		// We could call the `get_rect_bounds_global` helper, but for efficiency,
		// it's better to calculate the global transform once and cache it for later use.
		const Transform4D global_transform = collision_shape->get_global_transform();
		const Rect4 rect_bounds_global = collision_shape->get_rect_bounds_local(global_transform);
		real_t distance = 0.0;
		// We MUST always pass `true` for `p_inside_is_zero` here to ensure we do not prematurely skip shapes on this broad phase check.
		const bool hit = rect_bounds_global.raycast_intersects(global_ray_origin, global_ray_direction, true, &distance, nullptr);
		if (!hit || distance > max_distance) {
			continue;
		}
		// Force `double` for distance storage to avoid precision loss when using Variant
		// later inside of a Dictionary, since Variant uses `double` for float types.
		const RaycastCandidate candidate = { global_transform, collision_shape, (double)distance };
		r_candidates.append(candidate);
	}
}

// It is possible that in the future we will want to migrate this function to PhysicsEngine4D,
// to allow for different physics engines to customize the raycast behavior.
// However, there is really just one mathematically correct raycast result, so customization in a
// PhysicsEngine4D would just be for performance optimizations. And in that case, it's not clear
// what the API boundary should be, so for now we will keep this function only in PhysicsServer4D.
Dictionary PhysicsServer4D::raycast_physics_objects(const Ref<RaycastParameters4D> &p_raycast_parameters) {
	Dictionary nearest_raycast_result;
	ERR_FAIL_COND_V_MSG(p_raycast_parameters.is_null(), nearest_raycast_result, "PhysicsServer4D: The given raycast parameters object is null.");
	// Enforce that the ray direction is normalized.
	const Vector4 global_ray_direction = p_raycast_parameters->get_global_ray_direction();
	ERR_FAIL_COND_V_MSG(!global_ray_direction.is_normalized(), nearest_raycast_result, "PhysicsServer4D: The given raycast parameters have a non-normalized ray direction. The ray direction must be normalized.");
	// Enforce that this function is only called from the main thread, because it accesses the scene tree and nodes.
	CoreBind::OS *os = CoreBind::OS::get_singleton();
	ERR_FAIL_COND_V_MSG(os->get_thread_caller_id() != os->get_main_thread_id(), nearest_raycast_result, "PhysicsServer4D raycasts may only be performed from the main thread.");
	const Vector<ObjectID> &exclude_nodes = p_raycast_parameters->get_exclude_nodes();
	// First pass: Use Rect4 axis-aligned bounding boxes to perform a quick rough
	// check to see if it's a candidate for a more expensive raycast check.
	// The bulk of the logic is split into a separate function to avoid code duplication.
	Vector<RaycastCandidate> candidates;
	const bool collide_with_bodies = p_raycast_parameters->get_collide_with_bodies();
	if (collide_with_bodies) {
		for (int body_index = 0; body_index < _physics_body_nodes.size(); body_index++) {
			CollisionObject4D *col_obj_node = Object::cast_to<CollisionObject4D>(_physics_body_nodes[body_index]);
			const ObjectID col_obj_node_id = ObjectID(col_obj_node->get_instance_id());
			if (exclude_nodes.find(col_obj_node_id) != -1) {
				continue;
			}
			_raycast_shapes_fast_rect4(p_raycast_parameters, col_obj_node, candidates);
		}
	}
	const bool collide_with_areas = p_raycast_parameters->get_collide_with_areas();
	if (collide_with_areas) {
		for (int area_index = 0; area_index < _area_nodes.size(); area_index++) {
			CollisionObject4D *col_obj_node = Object::cast_to<CollisionObject4D>(_area_nodes[area_index]);
			const ObjectID col_obj_node_id = ObjectID(col_obj_node->get_instance_id());
			if (exclude_nodes.find(col_obj_node_id) != -1) {
				continue;
			}
			_raycast_shapes_fast_rect4(p_raycast_parameters, col_obj_node, candidates);
		}
	}
	if (candidates.is_empty()) {
		// Show error messages if the raycast parameters are invalid. We don't want this niche unusual
		// unhappy path to slow down normal raycasts, so only check this when there are no candidates.
		ERR_FAIL_COND_V_MSG(!(collide_with_bodies || collide_with_areas), nearest_raycast_result, "PhysicsServer4D: The given raycast parameters do not collide with bodies and areas, so the raycast will never hit anything.");
		ERR_FAIL_COND_V_MSG(p_raycast_parameters->get_collision_mask() == 0, nearest_raycast_result, "PhysicsServer4D: The given raycast parameters have a collision mask of zero, so the raycast will never hit anything.");
		ERR_FAIL_COND_V_MSG(p_raycast_parameters->get_max_distance() <= 0.0, nearest_raycast_result, "PhysicsServer4D: The given raycast parameters have a max distance of zero or less, so the raycast will never hit anything.");
		// No hit found. Return a Dictionary with "hit" set to false.
		nearest_raycast_result["hit"] = false;
		return nearest_raycast_result;
	}
	// Second pass: Sort by distance so that we process the nearest candidates first.
	struct RaycastCandidateComparator {
		bool operator()(const RaycastCandidate &a, const RaycastCandidate &b) const {
			return a.distance < b.distance;
		}
	};
	candidates.sort_custom<RaycastCandidateComparator>();
	// Third pass: Perform a more expensive raycast check on the candidates to find the nearest hit.
	// This will either return the same distance or farther. If the expensive raycast returns a distance
	// closer than the rest of the candidates, we can safely return it as the nearest hit.
	Transform4D nearest_inv_global_transform;
	const Vector4 global_ray_origin = p_raycast_parameters->get_global_ray_origin();
	int64_t nearest_candidate_index = -1;
	double nearest_global_distance = p_raycast_parameters->get_max_distance();
	const bool inside_is_skip = p_raycast_parameters->get_inside_is_skip();
	const bool inside_is_zero = p_raycast_parameters->get_inside_is_zero();
	for (int64_t candidate_index = 0; candidate_index < candidates.size(); candidate_index++) {
		const RaycastCandidate &candidate = candidates[candidate_index];
		if (candidate.distance >= nearest_global_distance) {
			// No need to check further candidates, as they are sorted by distance.
			break;
		}
		const Transform4D inv_global_transform = candidate.global_transform.inverse();
		const Vector4 local_ray_origin = inv_global_transform.xform(global_ray_origin);
		if (inside_is_skip && candidate.collision_shape->has_local_point(local_ray_origin)) {
			// Skip this candidate if the ray starts inside the shape and inside_is_skip is true.
			continue;
		}
		const Vector4 local_ray_direction_non_norm = inv_global_transform.basis.xform(global_ray_direction);
		const real_t local_units_per_global_unit = local_ray_direction_non_norm.length();
		const Vector4 local_ray_direction = local_ray_direction_non_norm / local_units_per_global_unit;
		const real_t local_nearest_distance = nearest_global_distance * local_units_per_global_unit;
		Dictionary raycast_result = candidate.collision_shape->raycast_intersects_local(local_ray_origin, local_ray_direction, local_nearest_distance, inside_is_zero);
		if (!raycast_result.has("hit") || !(bool)raycast_result["hit"] || !raycast_result.has("distance")) {
			continue;
		}
		const double global_distance = (double)raycast_result["distance"] / local_units_per_global_unit;
		// Due to `p_max_distance` being set to `nearest_distance`, the shape should not return a distance greater than nearest_distance.
		// It's possible that the distance will be slightly greater due to floating-point precision issues, in which case just allow it.
		// It's also possible that the shape's raycast function is not designed correctly, because these can be overridden
		// by user-defined shape classes, but in that case there is nothing we can do, so just assume the result is valid.
		nearest_raycast_result = raycast_result;
		nearest_candidate_index = candidate_index;
		nearest_global_distance = global_distance;
		nearest_inv_global_transform = inv_global_transform;
	}
	if (nearest_candidate_index == -1) {
		// No hit found. Return an empty dictionary with "hit" set to false.
		nearest_raycast_result["hit"] = false;
		return nearest_raycast_result;
	}
	// Hit found. First, transform the normal and point from local space to global space.
	// The shape's functions operate in local space, but this function is expected to be global, because the inputs are global.
	const Transform4D global_transform = candidates[nearest_candidate_index].global_transform;
	// To avoid ambiguity and avoid data loss, provide both, the global ones prefixed with "global_".
	// I considered renaming the shape-local ones to have a "shape_local_" prefix, but that would be unnecessary operations for a function
	// which should be fast, it would be an unnecessary breaking change compared to the shape API (maybe some code wants to handle both?),
	// and it would be a very long key anyway, so instead we'll just expect callers to understand what "distance", "normal", and "point" mean.
	nearest_raycast_result["global_distance"] = nearest_global_distance;
	if (nearest_raycast_result.has("normal")) {
		const Vector4 local_normal = nearest_raycast_result["normal"];
		nearest_raycast_result["global_normal"] = nearest_inv_global_transform.basis.transposed().xform(local_normal).normalized();
	}
	if (nearest_raycast_result.has("point")) {
		const Vector4 local_point = nearest_raycast_result["point"];
		nearest_raycast_result["global_point"] = global_transform.xform(local_point);
	}
	// Return the raycast result Dictionary with the body and shape added.
	CollisionShape4D *nearest_collision_shape = candidates[nearest_candidate_index].collision_shape;
	nearest_raycast_result["body"] = nearest_collision_shape->get_ancestor_collision_object();
	nearest_raycast_result["shape"] = nearest_collision_shape;
	// Note: The keys in the returned Dictionary are not guaranteed to be in any particular order.
	// If callers need a sorted order, they should call `.sort()` on the Dictionary after receiving it.
	return nearest_raycast_result;
}

void PhysicsServer4D::_physics_process() {
	if (!_is_physics_active) {
		return;
	}
	ERR_FAIL_COND_MSG(_current_physics_engine.is_null(), "PhysicsServer4D: No physics engine is set.");
	const double delta_time = _scene_tree->get_root()->get_physics_process_delta_time();
	_current_physics_engine->physics_process(delta_time);
}

Ref<KinematicCollision4D> PhysicsServer4D::move_and_collide(PhysicsBody4D *p_body_node, const Vector4 &p_motion, const bool p_test_only, const double p_delta_time) {
	ERR_FAIL_NULL_V_MSG(p_body_node, Ref<KinematicCollision4D>(), "PhysicsServer4D: Cannot move a null PhysicsBody4D node.");
	ERR_FAIL_COND_V_MSG(_current_physics_engine.is_null(), Ref<KinematicCollision4D>(), "PhysicsServer4D: No physics engine is set.");
	return _current_physics_engine->move_and_collide(p_body_node, p_motion, p_test_only, p_delta_time);
}

void PhysicsServer4D::move_area(Area4D *p_area_node, const Vector4 &p_motion) {
	ERR_FAIL_NULL_MSG(p_area_node, "PhysicsServer4D: Cannot move a null Area4D node.");
	ERR_FAIL_COND_MSG(_current_physics_engine.is_null(), "PhysicsServer4D: No physics engine is set.");
	_current_physics_engine->move_area(p_area_node, p_motion);
}

void PhysicsServer4D::register_area(Area4D *p_area_node) {
	ERR_FAIL_NULL_MSG(p_area_node, "PhysicsServer4D: Cannot register a null Area4D node.");
	_area_nodes.append(p_area_node);
	if (likely(_is_physics_process_connected)) {
		return;
	}
	_scene_tree = p_area_node->get_tree();
	if (_scene_tree == nullptr) {
		return;
	}
	_scene_tree->connect(StringName("physics_frame"), callable_mp(this, &PhysicsServer4D::_physics_process));
	_is_physics_process_connected = true;
}

void PhysicsServer4D::unregister_area(Area4D *p_area_node) {
	ERR_FAIL_NULL_MSG(p_area_node, "PhysicsServer4D: Cannot unregister a null Area4D node.");
	_area_nodes.erase(p_area_node);
}

void PhysicsServer4D::register_physics_body(PhysicsBody4D *p_physics_body_node) {
	ERR_FAIL_NULL_MSG(p_physics_body_node, "PhysicsServer4D: Cannot register a null PhysicsBody4D node.");
	_physics_body_nodes.append(p_physics_body_node);
	// The first time a physics body in the tree is registered, connect to the scene tree's "physics_frame" signal to process physics.
	if (likely(_is_physics_process_connected)) {
		return;
	}
	if (!p_physics_body_node->is_inside_tree()) {
		return;
	}
	_scene_tree = p_physics_body_node->get_tree();
	if (_scene_tree == nullptr) {
		return;
	}
	_scene_tree->connect(StringName("physics_frame"), callable_mp(this, &PhysicsServer4D::_physics_process));
	_is_physics_process_connected = true;
}

void PhysicsServer4D::unregister_physics_body(PhysicsBody4D *p_physics_body_node) {
	ERR_FAIL_NULL_MSG(p_physics_body_node, "PhysicsServer4D: Cannot unregister a null PhysicsBody4D node.");
	_physics_body_nodes.erase(p_physics_body_node);
}

Ref<PhysicsEngine4D> PhysicsServer4D::_get_physics_engine(const String &p_name) const {
	if (_physics_engines.is_empty()) {
		ERR_PRINT("PhysicsServer4D: No physics engines are registered.");
		return Ref<PhysicsEngine4D>();
	}
	if (_physics_engines.has(p_name)) {
		return _physics_engines[p_name];
	}
	// Fallback to the first registered physics engine. If the name is empty,
	// treat it as "auto" and do not print a warning. Else, print a warning.
	if (!p_name.is_empty()) {
		WARN_PRINT("Physics engine '" + p_name + "' not registered. Using the first registered engine.");
	}
	return _physics_engines.begin()->value;
}

void PhysicsServer4D::register_physics_engine(const String &p_name, const Ref<PhysicsEngine4D> &p_engine) {
	if (_physics_engines.has(p_name)) {
		WARN_PRINT("Physics engine '" + p_name + "' already registered. The existing engine will be replaced.");
	}
	_physics_engines[p_name] = p_engine;
	if (_current_physics_engine.is_null()) {
		set_current_physics_engine_name(p_name);
	}
}

void PhysicsServer4D::unregister_physics_engine(const String &p_name) {
	_physics_engines.erase(p_name);
	if (_current_physics_engine_name == p_name) {
		if (_physics_engines.is_empty()) {
			_current_physics_engine.unref();
			_current_physics_engine_name = "";
		} else {
			_current_physics_engine_name = _physics_engines.begin()->key;
			_current_physics_engine = _physics_engines.begin()->value;
		}
	}
}

PackedStringArray PhysicsServer4D::get_physics_engine_names() const {
	// HashMap doesn't have a keys() method, so we have to do this manually.
	PackedStringArray engine_names;
	if (_physics_engines.is_empty()) {
		return engine_names;
	}
	engine_names.resize(_physics_engines.size());
	int i = 0;
	for (const KeyValue<String, Ref<PhysicsEngine4D>> &E : _physics_engines) {
		engine_names.set(i, String(E.key));
		i++;
	}
	return engine_names;
}

String PhysicsServer4D::get_current_physics_engine_name() const {
	return _current_physics_engine_name;
}

void PhysicsServer4D::set_current_physics_engine_name(const String &p_name) {
	if (p_name == _current_physics_engine_name) {
		return;
	}
	if (p_name.is_empty()) {
		_current_physics_engine_name = "";
		_current_physics_engine = Ref<PhysicsEngine4D>();
		return;
	}
	if (_physics_engines.is_empty()) {
		_current_physics_engine_name = "";
		_current_physics_engine = Ref<PhysicsEngine4D>();
		ERR_PRINT("PhysicsServer4D: No physics engines are registered.");
		return;
	}
	if (_physics_engines.has(p_name)) {
		_current_physics_engine_name = p_name;
		_current_physics_engine = _physics_engines[p_name];
		return;
	}
	_current_physics_engine_name = _physics_engines.begin()->key;
	_current_physics_engine = _physics_engines.begin()->value;
	WARN_PRINT("Physics engine '" + p_name + "' not registered. The first registered engine will be used as a fallback.");
}

bool PhysicsServer4D::get_active() const {
	return _is_physics_active;
}

void PhysicsServer4D::set_active(const bool p_active) {
	_is_physics_active = p_active;
}

TypedArray<Area4D> PhysicsServer4D::get_area_nodes() const {
	return _area_nodes;
}

TypedArray<PhysicsBody4D> PhysicsServer4D::get_physics_body_nodes() const {
	return _physics_body_nodes;
}

CollisionObject4D *PhysicsServer4D::get_or_create_global_static_body() {
	if (_global_static_body_for_bodyless_shapes != nullptr) {
		return _global_static_body_for_bodyless_shapes;
	}
	StaticBody4D *global_static_body = memnew(StaticBody4D);
	global_static_body->set_name("_GlobalStaticBody4D");
	PhysicsServer4D::get_singleton()->register_physics_body(global_static_body);
	_global_static_body_for_bodyless_shapes = global_static_body;
	return _global_static_body_for_bodyless_shapes;
}

CollisionObject4D *PhysicsServer4D::_global_static_body_for_bodyless_shapes = nullptr;
PhysicsServer4D *PhysicsServer4D::_singleton = nullptr;

PhysicsServer4D::~PhysicsServer4D() {
	_singleton = nullptr;
	if (_global_static_body_for_bodyless_shapes != nullptr) {
		memdelete(_global_static_body_for_bodyless_shapes);
		_global_static_body_for_bodyless_shapes = nullptr;
	}
	_area_nodes.clear();
	_physics_body_nodes.clear();
	_physics_engines.clear();
	_current_physics_engine.unref();
	_scene_tree = nullptr;
}

void PhysicsServer4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("raycast_physics_objects", "raycast_parameters"), &PhysicsServer4D::raycast_physics_objects);

	ClassDB::bind_method(D_METHOD("register_physics_engine", "name", "engine"), &PhysicsServer4D::register_physics_engine);
	ClassDB::bind_method(D_METHOD("unregister_physics_engine", "name"), &PhysicsServer4D::unregister_physics_engine);
	ClassDB::bind_method(D_METHOD("get_physics_engine_names"), &PhysicsServer4D::get_physics_engine_names);

	ClassDB::bind_method(D_METHOD("get_current_physics_engine_name"), &PhysicsServer4D::get_current_physics_engine_name);
	ClassDB::bind_method(D_METHOD("set_current_physics_engine_name", "name"), &PhysicsServer4D::set_current_physics_engine_name);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "current_physics_engine_name"), "set_current_physics_engine_name", "get_current_physics_engine_name");

	ClassDB::bind_method(D_METHOD("get_active"), &PhysicsServer4D::get_active);
	ClassDB::bind_method(D_METHOD("set_active", "active"), &PhysicsServer4D::set_active);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "active"), "set_active", "get_active");

	ClassDB::bind_method(D_METHOD("get_area_nodes"), &PhysicsServer4D::get_area_nodes);
	ClassDB::bind_method(D_METHOD("get_physics_body_nodes"), &PhysicsServer4D::get_physics_body_nodes);
}
