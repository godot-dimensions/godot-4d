#pragma once

#include "../model/mesh_instance_4d.h"
#include "../nodes/camera_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#elif GODOT_MODULE
#include "core/object/ref_counted.h"
#include "core/variant/typed_array.h"
#include "scene/main/viewport.h"
#endif

class RenderingEngine4D : public RefCounted {
	GDCLASS(RenderingEngine4D, RefCounted);

#if GDEXTENSION
	TypedArray<Viewport> _setup_viewports;
#elif GODOT_MODULE
	Vector<Viewport *> _setup_viewports;
#endif

	Viewport *_viewport = nullptr;
	Camera4D *_camera = nullptr;
	TypedArray<MeshInstance4D> _mesh_instances;
	TypedArray<Projection> _mesh_relative_basises;
	PackedVector4Array _mesh_relative_positions;

	void _sort_meshes_by_relative_z();

protected:
	static void _bind_methods();

public:
	void calculate_relative_transforms();

	Viewport *get_viewport() const;
	void set_viewport(Viewport *p_viewport);

	Camera4D *get_camera() const;
	void set_camera(Camera4D *p_camera);

	TypedArray<MeshInstance4D> get_mesh_instances() const;
	void set_mesh_instances(TypedArray<MeshInstance4D> p_mesh_instances);

	TypedArray<Projection> get_mesh_relative_basises() const;
	void set_mesh_relative_basises(TypedArray<Projection> p_mesh_relative_basises);

	PackedVector4Array get_mesh_relative_positions() const;
	void set_mesh_relative_positions(PackedVector4Array p_mesh_relative_positions);

	void setup_for_viewport_if_needed(Viewport *p_for_viewport);
	virtual bool prefers_wireframe_meshes();
	virtual void setup_for_viewport();
	virtual void render_frame();

	GDVIRTUAL0R(bool, _prefers_wireframe_meshes);
	GDVIRTUAL0(_setup_for_viewport);
	GDVIRTUAL0(_render_frame);
};
