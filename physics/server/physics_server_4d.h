#pragma once

#include "physics_engine_4d.h"

#include "../bodies/rigid_body_4d.h"

#if GDEXTENSION
#include <godot_cpp/templates/hash_map.hpp>
#elif GODOT_MODULE
#include "core/templates/hash_map.h"
#endif

class PhysicsServer4D : public Object {
	GDCLASS(PhysicsServer4D, Object);

	HashMap<String, Ref<PhysicsEngine4D>> _physics_engines;
	Ref<PhysicsEngine4D> _current_physics_engine;
	String _current_physics_engine_name;
	// For 3D, Godot has "World3D" which physics objects are added to. Objects in the same world can see each other.
	// For 4D, we will use a simpler approach, just have one global set of arrays for all physics objects.
	// We could add a "World4D" class in the future if we want to add this feature, but it's not necessary for now.
	TypedArray<Area4D> _area_nodes;
	TypedArray<PhysicsBody4D> _physics_body_nodes;
	SceneTree *_scene_tree = nullptr;

	Ref<PhysicsEngine4D> _get_physics_engine(const String &p_name) const;
	bool _is_physics_active = true;
	bool _is_physics_process_connected = false;
	void _physics_process();

protected:
	static PhysicsServer4D *singleton;
	static void _bind_methods();

public:
	void move_and_collide(PhysicsBody4D *p_body_node, Vector4 p_motion);
	void move_area(Area4D *p_area_node, Vector4 p_motion);

	void register_area(Area4D *p_area_node);
	void unregister_area(Area4D *p_area_node);

	void register_physics_body(PhysicsBody4D *p_physics_body_node);
	void unregister_physics_body(PhysicsBody4D *p_physics_body_node);

	void register_physics_engine(const String &p_name, const Ref<PhysicsEngine4D> &p_engine);
	void unregister_physics_engine(const String &p_name);
	void set_physics_engine(const String &p_name);
	PackedStringArray get_physics_engine_names() const;

	String get_current_physics_engine_name() const;
	void set_current_physics_engine_name(const String &p_name);

	bool get_active() const;
	void set_active(const bool p_active);

	TypedArray<Area4D> get_area_nodes() const;
	TypedArray<PhysicsBody4D> get_physics_body_nodes() const;

	static PhysicsServer4D *get_singleton() { return singleton; }
	PhysicsServer4D() { singleton = this; }
	~PhysicsServer4D() { singleton = nullptr; }
};
