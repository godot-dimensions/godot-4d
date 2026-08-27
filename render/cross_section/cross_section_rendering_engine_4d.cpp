#include "cross_section_rendering_engine_4d.h"

#include "../environment/render_bridge_4d_to_3d.h"

bool CrossSectionRenderingEngine4D::_update_light_3d_render_base(Light4D *p_light_4d, const Projection &p_relative_basis, const Vector4 &p_relative_position, const RID p_render_base) const {
	return p_light_4d->update_3d_cross_section_render_base(p_relative_basis, p_relative_position, p_render_base);
}

Ref<Material> CrossSectionRenderingEngine4D::_get_material_3d(const Ref<Material4D> &p_material_4d) {
	return p_material_4d->get_cross_section_material();
}

void CrossSectionRenderingEngine4D::_setup_world_3d() {
	if (_environment_bridge == nullptr) {
		_environment_bridge = memnew(EnvironmentRenderBridge4DTo3D);
	}
	_environment_bridge->setup_environment_resources(get_world_3d());
}

void CrossSectionRenderingEngine4D::_update_environment() {
	if (_environment_bridge != nullptr) {
		_environment_bridge->update_environment(get_camera());
		_environment_bridge->update_suns(get_light_object_ids(), get_light_relative_basises());
	}
}

void CrossSectionRenderingEngine4D::_free_environment_bridge() {
	if (_environment_bridge != nullptr) {
		_environment_bridge->cleanup_render_resources();
		memdelete(_environment_bridge);
		_environment_bridge = nullptr;
	}
}

void CrossSectionRenderingEngine4D::cleanup_for_viewport() {
	TetraRenderingEngine4D::cleanup_for_viewport();
	_free_environment_bridge();
}

CrossSectionRenderingEngine4D::~CrossSectionRenderingEngine4D() {
	free_shared_render_resources();
	_free_environment_bridge();
}
