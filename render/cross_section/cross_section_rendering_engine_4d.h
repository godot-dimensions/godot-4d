#pragma once

#include "../rendering_engine_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/rid.hpp>
#elif GODOT_MODULE
#include "core/templates/rid.h"
#include "scene/resources/3d/world_3d.h"
#endif

class EnvironmentRenderBridge4DTo3D;

class CrossSectionRenderingEngine4D : public RenderingEngine4D {
	GDCLASS(CrossSectionRenderingEngine4D, RenderingEngine4D);

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

	RID _cross_section_camera = RID();
	Ref<World3D> _cross_section_world_3d;
	EnvironmentRenderBridge4DTo3D *_cross_section_environment_bridge = nullptr;
	uint64_t _current_pass = 0;

	void _create_light_render_instance_3d(const ObjectID p_light_4d_node_object_id);
	RID _create_mesh_render_instance_3d();
	void _cleanup_render_resources();

	void _update_camera();
	void _update_lights();
	void _update_mesh_instances();

protected:
	static void _bind_methods() {}

public:
	virtual String get_friendly_name() const override { return "Cross-section"; }
	virtual bool requires_transparent_background() const override { return false; }
	virtual bool supports_lighting() const override { return true; }
	virtual bool supports_godot_rendering_method(const String &) const override { return true; }
	virtual void setup_for_viewport() override;
	virtual void cleanup_for_viewport() override;
	virtual void render_frame() override;

	virtual ~CrossSectionRenderingEngine4D();
};
