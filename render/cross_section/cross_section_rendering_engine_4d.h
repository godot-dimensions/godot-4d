#pragma once

#include "../rendering_engine_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/variant/rid.hpp>
#elif GODOT_MODULE
#include "core/templates/rid.h"
#include "scene/resources/3d/world_3d.h"
#endif

class CrossSectionRenderingEngine4D : public RenderingEngine4D {
	GDCLASS(CrossSectionRenderingEngine4D, RenderingEngine4D);

private:
	Vector<RID> _instances_3d;
	RID _cross_section_camera;
	Ref<World3D> _cross_section_world_3d;

	void update_camera();
	RID create_instance();

protected:
	static void _bind_methods() {}

public:
	virtual String get_friendly_name() const override { return "Cross-section"; }
	virtual void setup_for_viewport() override;
	virtual void render_frame() override;

	virtual ~CrossSectionRenderingEngine4D();
};
