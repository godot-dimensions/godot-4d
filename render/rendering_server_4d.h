#pragma once

#include "rendering_engine_4d.h"

#if GDEXTENSION
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>
#endif

class Light4D;
class WorldEnvironment4D;

class RenderingServer4D : public Object {
	GDCLASS(RenderingServer4D, Object);

	HashMap<String, Ref<RenderingEngine4D>> _rendering_engines;
	HashMap<Viewport *, Vector<Camera4D *>> _viewport_cameras;
	HashMap<Viewport *, Vector<WorldEnvironment4D *>> _viewport_world_environments;
	// For 3D, Godot has "World3D" which meshes are added to. Cameras in the same world can see the same meshes.
	// For 4D, we will use a simpler approach, just have one global array of meshes which all cameras can see.
	// We could add a "World4D" class in the future if we want to add this feature, but it's not necessary for now.
	Vector<Light4D *> _lights;
	Vector<MeshInstance4D *> _mesh_instances;

	PackedInt64Array _get_visible_light_object_ids() const;
	PackedInt64Array _get_visible_mesh_instance_object_ids() const;
	bool _are_render_frame_and_process_frame_connected = false;
	void _render_frame();
	void _request_godot_redraw();

protected:
	static RenderingServer4D *singleton;
	static void _bind_methods();

public:
	void register_camera(Camera4D *p_camera);
	void unregister_camera(Camera4D *p_camera);
	void make_camera_current(Camera4D *p_camera);
	void clear_camera_current(Camera4D *p_camera);
	Camera4D *get_current_camera(Viewport *p_viewport) const;

	void register_light(Light4D *p_light);
	void unregister_light(Light4D *p_light);

	void register_mesh_instance(MeshInstance4D *p_mesh_instance);
	void unregister_mesh_instance(MeshInstance4D *p_mesh_instance);

	void register_rendering_engine(const Ref<RenderingEngine4D> &p_engine);
	void unregister_rendering_engine(const String &p_friendly_name);
	PackedStringArray get_rendering_engine_names() const;
	Ref<RenderingEngine4D> get_rendering_engine_from_name(const String &p_friendly_name) const;

	void register_world_environment(WorldEnvironment4D *p_world_environment);
	void unregister_world_environment(WorldEnvironment4D *p_world_environment);
	void make_world_environment_current(WorldEnvironment4D *p_world_environment);
	void clear_world_environment_current(WorldEnvironment4D *p_world_environment);
	WorldEnvironment4D *get_current_world_environment(Viewport *p_viewport) const;
	WorldEnvironment4D *get_current_world_environment_for_camera(Camera4D *p_camera) const; // Internal use only, do not expose.

	static RenderingServer4D *get_singleton() { return singleton; }
	RenderingServer4D() { singleton = this; }
	~RenderingServer4D();
};
