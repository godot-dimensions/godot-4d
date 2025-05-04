#pragma once

#include "../rendering_engine_4d.h"
#include <vector>

#if GDEXTENSION
#include <godot_cpp/variant/typed_array.hpp>
#elif GODOT_MODULE
#include "core/variant/typed_array.h"
#endif

class CrossSectionRenderingEngine4D : public RenderingEngine4D {
	GDCLASS(CrossSectionRenderingEngine4D, RenderingEngine4D);

private:
	std::vector<RID> _instances_3d;
	RID _cross_section_camera;

	void update_camera();
	bool cull_mesh(const MeshInstance4D &mesh, Projection basis, Vector4 position) const;

protected:
	static void _bind_methods() {}

public:
	virtual String get_friendly_name() const override { return "Cross-section"; }
	virtual void setup_for_viewport() override;
	virtual void render_frame() override;

	virtual ~CrossSectionRenderingEngine4D();
};
