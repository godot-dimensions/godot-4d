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

	void update_camera();
	RID create_instance();
	void _cleanup_render_resources();

protected:
	static void _bind_methods();

	virtual void _render_frame_callback() override;

public:
	virtual String get_friendly_name() const override { return "Projected"; }
	virtual void setup_for_viewport() override;
	virtual void cleanup_for_viewport() override;
	void normalize_image_callback(int64_t p_effect_callback_type, RenderData *p_render_data);

	ProjectedRenderingEngine4D();
	~ProjectedRenderingEngine4D();
};
