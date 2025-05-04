#include "cross_section_rendering_engine_4d.h"

#include "../../model/mesh_instance_4d.h"
#include "../../nodes/camera_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/rendering_server.hpp>
#elif GODOT_MODULE
#include "servers/rendering_server.h"
#endif

RID create_instance(const Viewport &viewport) {
	ERR_FAIL_NULL_V(RenderingServer::get_singleton(), RID());
	RID instance = RenderingServer::get_singleton()->instance_create();
	// Vertex data on the mesh is wack. Culling will not work.
	RenderingServer::get_singleton()->instance_set_ignore_culling(instance, true);
	Ref<World3D> world = viewport.find_world_3d();
	if (world.is_valid()) {
		RenderingServer::get_singleton()->instance_set_scenario(instance, world->get_scenario());
	}

	return instance;
}

void CrossSectionRenderingEngine4D::render_frame() {
	ERR_FAIL_NULL(RenderingServer::get_singleton());
	ERR_FAIL_NULL(get_camera());
	ERR_FAIL_NULL(get_viewport());
	update_camera();
	// Maps global to cameral-local, aka world space to view space.
	TypedArray<MeshInstance4D> mesh_instances = get_mesh_instances();
	TypedArray<Projection> modelview_basises = get_mesh_relative_basises();
	PackedVector4Array modelview_origins = get_mesh_relative_positions();
	size_t instance_index = 0;
	if (mesh_instances.size() > _instances_3d.size()) {
		_instances_3d.reserve(mesh_instances.size() - _instances_3d.size());
	}
	for (int mesh_index = 0; mesh_index < mesh_instances.size(); mesh_index++) {
		MeshInstance4D *mesh_instance = Object::cast_to<MeshInstance4D>(mesh_instances[mesh_index]);
		ERR_CONTINUE(mesh_instance == nullptr);

		Ref<Mesh4D> mesh = mesh_instance->get_mesh();
		if (!mesh.is_valid()) {
			continue;
		}
		Ref<Mesh> mesh_3d = mesh->get_cross_section_mesh();
		ERR_CONTINUE(!mesh_3d.is_valid());

		while (_instances_3d.size() <= instance_index) {
			_instances_3d.push_back(create_instance(*get_viewport()));
		}
		RID instance_3d = _instances_3d[instance_index];

		RenderingServer::get_singleton()->instance_set_base(instance_3d, mesh_3d->get_rid());

		Ref<Material4D> override_material = mesh_instance->get_material_override();
		if (override_material.is_valid()) {
			Ref<Material> override_material_3d = override_material->get_cross_section_material();
			ERR_CONTINUE(!override_material_3d.is_valid());
			RenderingServer::get_singleton()->instance_set_surface_override_material(instance_3d, 0, override_material_3d->get_rid());
		}

		Projection modelview_basis = modelview_basises[mesh_index];
		Vector4 modelview_origin = modelview_origins[mesh_index];
		RenderingServer::get_singleton()->instance_geometry_set_shader_parameter(instance_3d, "modelview_origin", modelview_origin);
		// Can't pass a mat4 through instance uniforms, need to break up into columns.
		RenderingServer::get_singleton()->instance_geometry_set_shader_parameter(instance_3d, "modelview_basis_x", modelview_basis.columns[0]);
		RenderingServer::get_singleton()->instance_geometry_set_shader_parameter(instance_3d, "modelview_basis_y", modelview_basis.columns[1]);
		RenderingServer::get_singleton()->instance_geometry_set_shader_parameter(instance_3d, "modelview_basis_z", modelview_basis.columns[2]);
		RenderingServer::get_singleton()->instance_geometry_set_shader_parameter(instance_3d, "modelview_basis_w", modelview_basis.columns[3]);

		instance_index++;
	}
	for (int i = instance_index; i < _instances_3d.size(); i++) {
		RID instance = _instances_3d[i];
		RenderingServer::get_singleton()->free_rid(instance);
	}
	_instances_3d.resize(instance_index);
}

void CrossSectionRenderingEngine4D::update_camera() {
	ERR_FAIL_NULL(RenderingServer::get_singleton());
	if (!_cross_section_camera.is_valid()) {
		ERR_FAIL_NULL(get_viewport());
		_cross_section_camera = RenderingServer::get_singleton()->camera_create();
		RenderingServer::get_singleton()->viewport_attach_camera(get_viewport()->get_viewport_rid(), _cross_section_camera);
	}
	// Only setting final 3D->2D projection stuff here. Can't pass the full Transform4D or even the basis as the camera's transform because it expects a Transform3D.
	ERR_FAIL_NULL(get_camera());
	Camera4D *camera = get_camera();
	const float clip_near = camera->get_clip_near();
	const float clip_far = camera->get_clip_far();
	switch (camera->get_projection_type()) {
		case Camera4D::PROJECTION4D_ORTHOGRAPHIC: {
			RenderingServer::get_singleton()->camera_set_orthogonal(_cross_section_camera, camera->get_orthographic_size(), clip_near, clip_far);
		} break;
		case Camera4D::PROJECTION4D_PERSPECTIVE_4D: {
			RenderingServer::get_singleton()->camera_set_perspective(_cross_section_camera, Math::rad_to_deg(camera->get_field_of_view_4d()), clip_near, clip_far);
		} break;
		case Camera4D::PROJECTION4D_PERSPECTIVE_3D: {
			RenderingServer::get_singleton()->camera_set_perspective(_cross_section_camera, Math::rad_to_deg(camera->get_field_of_view_3d()), clip_near, clip_far);
		} break;
		case Camera4D::PROJECTION4D_PERSPECTIVE_DUAL: {
			WARN_PRINT_ONCE("Dual-perspective is not supported by the Cross-section renderer. Use PESPECTIVE_3D, PERSPECTIVE_4D, or ORTHOGRAPHIC instead.");
			RenderingServer::get_singleton()->camera_set_perspective(_cross_section_camera, Math::rad_to_deg(camera->get_field_of_view_3d()), clip_near, clip_far);
		} break;
	}
}

void CrossSectionRenderingEngine4D::setup_for_viewport() {
	ERR_FAIL_NULL(RenderingServer::get_singleton());
	ERR_FAIL_NULL(get_viewport());
	Ref<World3D> world = get_viewport()->find_world_3d();
	// This will move instances between World3Ds, so adding a new viewport will remove all the instances from the previous viewport.
	// TODO Per viewport instance cache?
	if (world.is_valid()) {
		RID scenario = world->get_scenario();
		for (RID instance : _instances_3d) {
			RenderingServer::get_singleton()->instance_set_scenario(instance, scenario);
		}
	}
	if (_cross_section_camera.is_valid()) {
		RenderingServer::get_singleton()->viewport_attach_camera(get_viewport()->get_viewport_rid(), _cross_section_camera);
	}
}

CrossSectionRenderingEngine4D::~CrossSectionRenderingEngine4D() {
	ERR_FAIL_NULL(RenderingServer::get_singleton());
	for (RID instance : _instances_3d) {
		RenderingServer::get_singleton()->free_rid(instance);
	}
	RenderingServer::get_singleton()->free_rid(_cross_section_camera);
}
