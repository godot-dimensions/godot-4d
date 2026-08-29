#pragma once

#include "../../model/mesh/material_4d.h"
#include "../../nodes/light/light_4d.h"
#include "../rendering_engine_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/rid.hpp>
#elif GODOT_MODULE
#include "core/templates/rid.h"
#include "scene/resources/3d/world_3d.h"
#endif

// Abstract base for the 4D rendering engines that use Godot's 3D renderer to render 4D scenes.
// These engines use a private World3D attached to the viewport to draw proxy Mesh3D resources, which
// carry 4D data in their ordinary vertex channels. Subclasses supply the 3D approximation of each
// 4D light, the 3D material carrying their shader, and any extra per-instance shader parameters.
class Godot3DRenderingEngine4D : public RenderingEngine4D {
	GDCLASS(Godot3DRenderingEngine4D, RenderingEngine4D);

private:
	// Terminology: "LightRenderInstance3D" is this struct. "Light3DRenderInstance" is what the struct holds.
	struct LightRenderInstance3D {
		// RenderingServer supports reuse of light RIDs, but we can't make use of this for 4D lights.
		// Each base + instance pair must be unique to the combination of rendering engine and 4D light node.
		RID base;
		RID instance;
		int64_t last_used_pass = 0;
	};
	HashMap<ObjectID, LightRenderInstance3D> _lights_3d;

	struct MeshRenderInstance3D {
		RID base;
		RID instance;
		RID material;
		int64_t last_used_pass = 0;
	};
	HashMap<ObjectID, MeshRenderInstance3D> _mesh_instances_3d;

	RID _camera_3d;
	Ref<World3D> _world_3d;

	void _create_light_render_instance_3d(const ObjectID p_light_4d_node_object_id);
	RID _create_mesh_render_instance_3d();

protected:
	static void _bind_methods() {}

	// Update functions called by _render_frame_callback() in derived classes.
	void _update_3d_camera();
	void _update_3d_lights();
	void _update_3d_mesh_instances();

	// The 3D material carrying this engine's shader, or an invalid Ref to use the proxy Mesh3D's
	// surface material without an instance override.
	virtual Ref<Material> _get_material_3d(const Ref<Material4D> &p_material_4d) const = 0;
	// Valid from setup_for_viewport() until cleanup_for_viewport(), and inside the hooks below.
	const Ref<World3D> &get_world_3d() const { return _world_3d; }

	// Per-instance shader parameters beyond the modelview transform that every shader takes.
	virtual void _update_extra_instance_shader_parameters(const RID p_instance) {}
	// Updates this engine's 3D approximation of the given 4D light, returning whether the light is
	// visible in this engine's view of the 4D scene at all.
	virtual bool _update_light_3d_render_base(Light4D *p_light_4d, const Projection &p_relative_basis, const Vector4 &p_relative_position, const RID p_render_base) const = 0;

	// Whether the positions this engine's shader produces can escape the proxy Mesh3D's custom AABB,
	// making Godot's frustum culling unusable.
	virtual bool _ignore_mesh_culling() const { return false; }

	// Called from setup_for_viewport() after the private World3D is attached to the viewport.
	virtual void _setup_specific_render_resources() = 0;
	// Clean up the render resources specific to the subclass, such as the cross-section environment bridge.
	// Subclasses must also call this from their destructor because virtual dispatch cannot reach the
	// derived implementation once the base destructor begins.
	virtual void _cleanup_specific_render_resources() = 0;
	// Frees the light and mesh render instances, the 3D camera, and the private World3D.
	void _cleanup_3d_render_resources();

public:
	virtual bool supports_lighting() const override { return true; }
	virtual void setup_for_viewport() override;
	virtual void cleanup_for_viewport() override;

	~Godot3DRenderingEngine4D();
};
