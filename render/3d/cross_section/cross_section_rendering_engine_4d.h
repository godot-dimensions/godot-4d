#pragma once

#include "../godot_3d_rendering_engine_4d.h"

class EnvironmentRenderBridge4DTo3D;

class CrossSectionRenderingEngine4D : public Godot3DRenderingEngine4D {
	GDCLASS(CrossSectionRenderingEngine4D, Godot3DRenderingEngine4D);

private:
	EnvironmentRenderBridge4DTo3D *_cross_section_environment_bridge = nullptr;

protected:
	static void _bind_methods() {}

	// RenderingEngine4D interface.
	virtual void _render_frame_callback() override;

	// Godot3DRenderingEngine4D interface.
	virtual Ref<Material> _get_material_3d(const Ref<Material4D> &p_material_4d) const override;
	virtual bool _update_light_3d_render_base(Light4D *p_light_4d, const Projection &p_relative_basis, const Vector4 &p_relative_position, const RID p_render_base) const override;
	virtual void _setup_specific_render_resources() override;
	virtual void _cleanup_specific_render_resources() override;

public:
	virtual String get_friendly_name() const override { return "Cross-section"; }
	virtual bool requires_transparent_background() const override { return false; }
	virtual bool supports_godot_rendering_method(const String &) const override { return true; }

	~CrossSectionRenderingEngine4D();
};
