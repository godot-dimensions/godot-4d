#pragma once

#include "../rendering_engine_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/render_data.hpp>
#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/rid.hpp>
#elif GODOT_MODULE
#include "core/templates/rid.h"
#include "scene/resources/3d/world_3d.h"
#include "servers/rendering/storage/render_data.h"
#endif

class ProjectedRenderingEngine4D : public RenderingEngine4D {
	GDCLASS(ProjectedRenderingEngine4D, RenderingEngine4D);

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

	RID _projected_camera = RID();
	Ref<World3D> _projected_world_3d;
	uint64_t _current_pass = 0;

	RID normalize_shader;
	RID normalize_pipeline;
	RID normalize_compositor;
	RID normalize_compositor_effect;

	// Sent from the combined rendering engine, or nil if projected rendering is used alone.
	Variant _cross_section_depth_texture;

	void _create_light_render_instance_3d(const ObjectID p_light_4d_node_object_id);
	RID _create_mesh_render_instance_3d();
	void _cleanup_render_resources();

	void _update_camera();
	void _update_lights();
	void _update_mesh_instances();

protected:
	static void _bind_methods();

public:
	ProjectedRenderingEngine4D();
	virtual String get_friendly_name() const override { return "Projected"; }
	virtual bool supports_lighting() const override { return true; }
	// The tetrahedra are accumulated with additive blending and then normalized by dividing out the
	// accumulated alpha, which only works if the buffer starts out fully transparent.
	virtual bool requires_transparent_background() const override { return true; }
	// The transparency normalization pass is a compositor effect writing to a storage image, neither
	// of which the Mobile or Compatibility rendering methods provide. Also Mobile's alpha bit depth is insufficient.
	virtual bool supports_godot_rendering_method(const String &p_godot_rendering_method) const override { return p_godot_rendering_method == "forward_plus"; }
	virtual void setup_for_viewport() override;
	virtual void cleanup_for_viewport() override;
	virtual void render_frame() override;
	void normalize_image_callback(int64_t p_effect_callback_type, RenderData *p_render_data);
	void set_cross_section_depth_texture(const Variant &p_texture);

	virtual ~ProjectedRenderingEngine4D();
};
