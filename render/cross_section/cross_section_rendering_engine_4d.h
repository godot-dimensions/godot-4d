#pragma once

#include "../tetra_rendering_engine_4d.h"

class EnvironmentRenderBridge4DTo3D;

class CrossSectionRenderingEngine4D : public TetraRenderingEngine4D {
	GDCLASS(CrossSectionRenderingEngine4D, TetraRenderingEngine4D);

private:
	EnvironmentRenderBridge4DTo3D *_environment_bridge = nullptr;

	void _free_environment_bridge();

protected:
	static void _bind_methods() {}

	virtual bool _update_light_3d_render_base(Light4D *p_light_4d, const Projection &p_relative_basis, const Vector4 &p_relative_position, const RID p_render_base) const override;
	virtual Ref<Material> _get_material_3d(const Ref<Material4D> &p_material_4d) override;
	virtual void _update_environment() override;
	virtual void _setup_world_3d() override;

public:
	virtual String get_friendly_name() const override { return "Cross-section"; }
	virtual void cleanup_for_viewport() override;

	virtual ~CrossSectionRenderingEngine4D();
};
