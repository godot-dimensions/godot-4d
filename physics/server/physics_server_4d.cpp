#include "physics_server_4d.h"

#include "../bodies/static_body_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>
#elif GODOT_MODULE
#include "scene/main/scene_tree.h"
#include "scene/main/window.h"
#endif

void PhysicsServer4D::_physics_process() {
	if (!_is_physics_active) {
		return;
	}
	ERR_FAIL_COND_MSG(_current_physics_engine.is_null(), "PhysicsServer4D: No physics engine is set.");
	const double p_delta = _scene_tree->get_root()->get_physics_process_delta_time();
	_current_physics_engine->physics_process(p_delta);
}

Ref<KinematicCollision4D> PhysicsServer4D::move_and_collide(PhysicsBody4D *p_body_node, Vector4 p_motion, bool p_test_only) {
	ERR_FAIL_NULL_V_MSG(p_body_node, Ref<KinematicCollision4D>(), "PhysicsServer4D: Cannot move a null PhysicsBody4D node.");
	ERR_FAIL_COND_V_MSG(_current_physics_engine.is_null(), Ref<KinematicCollision4D>(), "PhysicsServer4D: No physics engine is set.");
	return _current_physics_engine->move_and_collide(p_body_node, p_motion, p_test_only);
}

void PhysicsServer4D::move_area(Area4D *p_area_node, Vector4 p_motion) {
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
	if (likely(_is_physics_process_connected)) {
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
	if (_physics_engines.has(p_name)) {
		_current_physics_engine_name = p_name;
		_current_physics_engine = _physics_engines[p_name];
	} else {
		WARN_PRINT("Physics engine '" + p_name + "' not registered. The first registered engine will be used as a fallback.");
		_current_physics_engine_name = _physics_engines.begin()->key;
		_current_physics_engine = _physics_engines.begin()->value;
	}
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
}

void PhysicsServer4D::_bind_methods() {
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
