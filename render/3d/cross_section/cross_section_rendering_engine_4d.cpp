#include "cross_section_rendering_engine_4d.h"

#include "../../environment/render_bridge_4d_to_3d.h"

#if GDEXTENSION
#include <godot_cpp/classes/rendering_server.hpp>
#elif GODOT_MODULE
#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR < 6
#include "servers/rendering_server.h"
#else
#include "servers/rendering/rendering_server.h"
#endif
#endif

// RenderingEngine4D interface.

void CrossSectionRenderingEngine4D::_render_frame_callback() {
	ERR_FAIL_NULL(RenderingServer::get_singleton());
	ERR_FAIL_NULL(get_camera());
	ERR_FAIL_NULL(get_viewport());
	_update_3d_camera();
	_update_3d_lights();
	if (_cross_section_environment_bridge != nullptr) {
		_cross_section_environment_bridge->update_environment(get_camera());
		_cross_section_environment_bridge->update_suns(get_light_object_ids(), get_light_relative_basises());
	}
	_update_3d_mesh_instances();
}

// Godot3DRenderingEngine4D interface.

Ref<Material> CrossSectionRenderingEngine4D::_get_material_3d(const Ref<Material4D> &p_material_4d) const {
	return p_material_4d->get_cross_section_material();
}

bool CrossSectionRenderingEngine4D::_update_light_3d_render_base(Light4D *p_light_4d, const Projection &p_relative_basis, const Vector4 &p_relative_position, const RID p_render_base) const {
	return p_light_4d->update_light_3d_cross_section_render_base(p_relative_basis, p_relative_position, p_render_base);
}

void CrossSectionRenderingEngine4D::_setup_specific_render_resources() {
	const Ref<World3D> &world_3d = get_world_3d();
	ERR_FAIL_COND(!world_3d.is_valid());
	if (_cross_section_environment_bridge == nullptr) {
		_cross_section_environment_bridge = memnew(EnvironmentRenderBridge4DTo3D);
	}
	_cross_section_environment_bridge->setup_environment_resources(world_3d);
}

void CrossSectionRenderingEngine4D::_cleanup_specific_render_resources() {
	if (_cross_section_environment_bridge != nullptr) {
		_cross_section_environment_bridge->cleanup_render_resources();
		memdelete(_cross_section_environment_bridge);
		_cross_section_environment_bridge = nullptr;
	}
}

CrossSectionRenderingEngine4D::~CrossSectionRenderingEngine4D() {
	_cleanup_specific_render_resources();
}
