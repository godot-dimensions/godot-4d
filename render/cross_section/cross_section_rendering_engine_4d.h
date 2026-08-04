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

class CrossSectionRenderingEngine4D : public RenderingEngine4D {
	GDCLASS(CrossSectionRenderingEngine4D, RenderingEngine4D);

private:
	struct Instance3D {
		RID instance;
		RID base;
		RID material;
		uint64_t last_used_pass = 0;
	};
	HashMap<ObjectID, Instance3D> _instances_3d;

	RID _cross_section_camera = RID();
	Ref<World3D> _cross_section_world_3d;
	uint64_t _current_pass = 0;

	void update_camera();
	RID create_instance();
	void _cleanup_render_resources();

protected:
	static void _bind_methods() {}

public:
	virtual String get_friendly_name() const override { return "Cross-section"; }
	virtual void setup_for_viewport() override;
	virtual void cleanup_for_viewport() override;
	virtual void render_frame() override;

	virtual ~CrossSectionRenderingEngine4D();
};
