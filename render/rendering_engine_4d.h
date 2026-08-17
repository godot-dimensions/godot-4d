#pragma once

#include "../godot_4d_defines.h"

#if GDEXTENSION
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/variant/typed_array.hpp>

#include <godot_cpp/core/gdvirtual.gen.inc>
#elif GODOT_MODULE
#include "core/object/ref_counted.h"
#include "core/variant/typed_array.h"
#include "scene/main/viewport.h"
#endif

class Camera4D;
class MeshInstance4D;

class RenderingEngine4D : public RefCounted {
	GDCLASS(RenderingEngine4D, RefCounted);

#if GDEXTENSION
	TypedArray<Viewport> _setup_viewports;
#elif GODOT_MODULE
	Vector<Viewport *> _setup_viewports;
#endif

	Viewport *_viewport = nullptr;
	Camera4D *_camera = nullptr;

	PackedInt64Array _light_object_ids;
	TypedArray<Projection> _light_relative_basises;
	PackedVector4Array _light_relative_positions;

	PackedInt64Array _mesh_instance_object_ids;
	TypedArray<Projection> _mesh_relative_basises;
	PackedVector4Array _mesh_relative_positions;

	void _sort_meshes_by_relative_z();

protected:
	static void _bind_methods();

public:
	void calculate_relative_transforms();

	Viewport *get_viewport() const { return _viewport; }
	void set_viewport(Viewport *p_viewport); // Internal use only, do not expose.

	Camera4D *get_camera() const { return _camera; }
	void set_camera(Camera4D *p_camera); // Internal use only, do not expose.

	PackedInt64Array get_light_object_ids() const { return _light_object_ids; }
	void set_light_object_ids(PackedInt64Array p_light_object_ids); // Internal use only, do not expose.
	TypedArray<Projection> get_light_relative_basises() const { return _light_relative_basises; }
	PackedVector4Array get_light_relative_positions() const { return _light_relative_positions; }

	PackedInt64Array get_mesh_instance_object_ids() const { return _mesh_instance_object_ids; }
	void set_mesh_instance_object_ids(PackedInt64Array p_mesh_instance_object_ids); // Internal use only, do not expose.
	TypedArray<Projection> get_mesh_relative_basises() const { return _mesh_relative_basises; }
	PackedVector4Array get_mesh_relative_positions() const { return _mesh_relative_positions; }

	void setup_for_viewport_if_needed(Viewport *p_for_viewport);
	void cleanup_for_viewport_if_needed(Viewport *p_for_viewport);

	virtual String get_friendly_name() const;
	virtual bool prefers_wireframe_meshes() const;
	virtual bool supports_lighting() const;

	virtual void setup_for_viewport();
	virtual void cleanup_for_viewport();
	virtual void render_frame();

	GDVIRTUAL0RC(String, _get_friendly_name);
	GDVIRTUAL0RC(bool, _prefers_wireframe_meshes);
	GDVIRTUAL0RC(bool, _supports_lighting);
	GDVIRTUAL0(_setup_for_viewport);
	GDVIRTUAL0(_cleanup_for_viewport);
	GDVIRTUAL0(_render_frame);
};
