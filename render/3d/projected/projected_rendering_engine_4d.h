#pragma once

#include "../../rendering_engine_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/render_data.hpp>
#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/variant/rid.hpp>
#elif GODOT_MODULE
#include "core/templates/rid.h"
#include "scene/resources/3d/world_3d.h"
#include "servers/rendering/storage/render_data.h"
#endif

class ProjectedRenderingEngine4D : public RenderingEngine4D {
	GDCLASS(ProjectedRenderingEngine4D, RenderingEngine4D);

private:
	Vector<RID> _instances_3d;
	RID _projected_camera = RID();
	Ref<World3D> _projected_world_3d;
	RID normalize_shader;
	RID normalize_pipeline;
	RID normalize_compositor;
	RID normalize_compositor_effect;

	// Sent from the combined rendering engine, or nil if projected rendering is used alone.
	Variant _cross_section_depth_texture;

	void update_camera();
	RID create_instance();
	void _cleanup_render_resources();
	void _normalize_image_callback(int64_t p_effect_callback_type, RenderData *p_render_data);

protected:
	static void _bind_methods() {}

	virtual void _render_frame_callback() override;

public:
	virtual String get_friendly_name() const override { return "Projected"; }
	virtual bool prefers_wireframe_meshes() const override { return false; }
	// This renderer does not create 3D light instances yet.
	virtual bool supports_lighting() const override { return false; }
	// The tetrahedra are accumulated with additive blending and then normalized by dividing out the
	// accumulated alpha, which only works if the buffer starts out fully transparent.
	virtual bool requires_transparent_background() const override { return true; }
	// The transparency normalization pass is a compositor effect writing to a storage image,
	// neither of which the Mobile or Compatibility rendering methods provide.
	// Also, Mobile's alpha bit depth is insufficient.
	virtual bool supports_godot_rendering_method(const String &p_godot_rendering_method) const override { return p_godot_rendering_method == "forward_plus"; }
	virtual void setup_for_viewport() override;
	virtual void cleanup_for_viewport() override;
	void set_cross_section_depth_texture(const Variant &p_texture);

	ProjectedRenderingEngine4D();
	~ProjectedRenderingEngine4D();
};
