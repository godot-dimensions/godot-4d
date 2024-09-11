#pragma once

#include "rendering_engine_4d.h"

#if GDEXTENSION
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>
#endif

class RenderingServer4D : public Object {
	GDCLASS(RenderingServer4D, Object);

	HashMap<String, Ref<RenderingEngine4D>> _rendering_engines;
	HashMap<Viewport *, Vector<Camera4D *>> _viewport_cameras;
	// For 3D, Godot has "World3D" which meshes are added to. Cameras in the same world can see the same meshes.
	// For 4D, we will use a simpler approach, just have one global array of meshes which all cameras can see.
	// We could add a "World4D" class in the future if we want to add this feature, but it's not necessary for now.
	Vector<MeshInstance4D *> _mesh_instances;

	Ref<RenderingEngine4D> _get_rendering_engine(const String &p_name) const;
	TypedArray<MeshInstance4D> _get_visible_mesh_instances() const;
	bool _is_render_frame_connected = false;
	void _render_frame();

protected:
	static RenderingServer4D *singleton;
	static void _bind_methods();

public:
	void register_camera(Camera4D *p_camera);
	void unregister_camera(Camera4D *p_camera);
	void make_camera_current(Camera4D *p_camera);
	void clear_camera_current(Camera4D *p_camera);

	void register_mesh_instance(MeshInstance4D *p_mesh_instance);
	void unregister_mesh_instance(MeshInstance4D *p_mesh_instance);

	void register_rendering_engine(const String &p_name, const Ref<RenderingEngine4D> &p_engine);
	void unregister_rendering_engine(const String &p_name);
	PackedStringArray get_rendering_engine_names() const;

	static RenderingServer4D *get_singleton() { return singleton; }
	RenderingServer4D() { singleton = this; }
	~RenderingServer4D() { singleton = nullptr; }
};
