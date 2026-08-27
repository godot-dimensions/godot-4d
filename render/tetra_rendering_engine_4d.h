#pragma once

#include "../model/mesh/material_4d.h"
#include "../nodes/light/light_4d.h"
#include "rendering_engine_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/rid.hpp>
#elif GODOT_MODULE
#include "core/templates/rid.h"
#include "scene/resources/3d/world_3d.h"
#endif

// Abstract base for the 4D rendering engines that draw tetrahedral meshes by handing a proxy Mesh3D
// (which carries 4D data in its ordinary vertex channels) to Godot's own 3D renderer, inside a
// private World3D attached to the viewport. Subclasses supply the 3D approximation of each 4D light,
// the 3D material carrying their shader, and any extra per-instance shader parameters it needs.
class TetraRenderingEngine4D : public RenderingEngine4D {
	GDCLASS(TetraRenderingEngine4D, RenderingEngine4D);

private:
	// Terminology: "LightRenderInstance3D" is this struct. "Light3DRenderInstance" is what the struct holds.
	struct LightRenderInstance3D {
		// RenderingServer supports reuse of light RIDs, but we can't make use of this for 4D lights.
		// Each base + instance pair must be unique to the combination of rendering engine and 4D light node.
		RID base;
		RID instance;
		uint64_t last_used_pass = 0;
	};
	HashMap<ObjectID, LightRenderInstance3D> _lights_3d;

	struct MeshRenderInstance3D {
		RID base;
		RID instance;
		RID material;
		uint64_t last_used_pass = 0;
	};
	HashMap<ObjectID, MeshRenderInstance3D> _mesh_instances_3d;

	RID _camera_3d;
	Ref<World3D> _world_3d;
	uint64_t _current_pass = 0;

	void _create_light_render_instance_3d(const ObjectID p_light_4d_node_object_id);
	RID _create_mesh_render_instance_3d();

	void _update_camera();
	void _update_lights();
	void _update_mesh_instances();

protected:
	static void _bind_methods() {}

	// Valid from setup_for_viewport() until cleanup_for_viewport(), and inside the hooks below.
	Ref<World3D> get_world_3d() const { return _world_3d; }
	// Frees the light and mesh render instances, the 3D camera and the private World3D. Subclasses
	// must call this from their own destructor, since a base destructor cannot reach their overrides.
	void free_shared_render_resources();

	// Updates this engine's 3D approximation of the given 4D light, returning whether the light is
	// visible in this engine's view of the 4D scene at all.
	virtual bool _update_light_3d_render_base(Light4D *p_light_4d, const Projection &p_relative_basis, const Vector4 &p_relative_position, const RID p_render_base) const = 0;
	// The 3D material carrying this engine's shader, or null to leave the mesh's own material in place.
	virtual Ref<Material> _get_material_3d(const Ref<Material4D> &p_material_4d) = 0;
	// Whether the positions this engine's shader produces can escape the proxy Mesh3D's custom AABB,
	// making Godot's frustum culling unusable.
	virtual bool _ignore_mesh_culling() const { return false; }
	// Per-instance shader parameters beyond the modelview transform that every tetra shader takes.
	virtual void _update_extra_instance_shader_parameters(const RID p_instance) {}
	// Called once per frame between the lights and the meshes, for engines that render a background.
	virtual void _update_environment() {}
	// Called from setup_for_viewport() once the private World3D exists but before it is attached.
	virtual void _setup_world_3d() {}

public:
	virtual bool supports_lighting() const override { return true; }
	virtual void setup_for_viewport() override;
	virtual void cleanup_for_viewport() override;
	virtual void render_frame() override;
};
