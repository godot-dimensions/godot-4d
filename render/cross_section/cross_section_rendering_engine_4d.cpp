#include "cross_section_rendering_engine_4d.h"

#include "../../math/transform_4d.h"
#include "../../model/mesh_instance_4d.h"
#include "../../nodes/camera_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/material.h>
#include <godot_cpp/classes/world_3d.h>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/core/error_macros.hpp">
#include <godot_cpp/core/variant/typed_array.hpp"
#include <godot_cpp/core/variant/variant.hpp>
#elif GODOT_MODULE
#include "core/error/error_macros.h"
#include "core/variant/array.h"
#include "core/variant/variant.h"
#include "scene/main/viewport.h"
#include "scene/resources/3d/world_3d.h"
#include "scene/resources/material.h"
#include "servers/rendering_server.h"
#endif

RID create_instance(const Viewport &viewport) {
	ERR_FAIL_NULL_V(RenderingServer::get_singleton(), RID());
	RID instance = RS::get_singleton()->instance_create();
	// Vertex data on the mesh is wack. Culling will not work.
	RS::get_singleton()->instance_set_ignore_culling(instance, true);
	Ref<World3D> world = viewport.find_world_3d();
	if (world.is_valid()) {
		RS::get_singleton()->instance_set_scenario(instance, world->get_scenario());
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
	for (int mesh_index = 0; mesh_index < mesh_instances.size(); mesh_index++) {
		MeshInstance4D *mesh_instance = Object::cast_to<MeshInstance4D>(mesh_instances[mesh_index]);
		ERR_CONTINUE(mesh_instance == nullptr);
		// TODO Enable culling
		// if (cull_mesh(*mesh_instance)) {
		// 	continue;
		// }

		Ref<Mesh4D> mesh = mesh_instance->get_mesh();
		if (mesh.is_null()) {
			continue;
		}
		Ref<Mesh> mesh_3d = mesh->get_cross_section_mesh();
		ERR_CONTINUE(mesh_3d.is_null());

		while (_instances_3d.size() <= instance_index) {
			_instances_3d.push_back(create_instance(*get_viewport()));
		}
		RID instance_3d = _instances_3d[instance_index];

		RS::get_singleton()->instance_set_base(instance_3d, mesh_3d->get_rid());

		Ref<Material4D> override_material = mesh_instance->get_material_override();
		if (override_material.is_valid()) {
			Ref<Material> override_material_3d = override_material->get_cross_section_material();
			ERR_CONTINUE(override_material_3d.is_null());
			RS::get_singleton()->instance_set_surface_override_material(instance_3d, 0, override_material_3d->get_rid());
		}

		Projection modelview_basis = modelview_basises[mesh_index];
		Vector4 modelview_origin = modelview_origins[mesh_index];
		RS::get_singleton()->instance_geometry_set_shader_parameter(instance_3d, "modelview_origin", modelview_origin);
		// Can't pass a mat4 through instance uniforms, need to break up into columns.
		RS::get_singleton()->instance_geometry_set_shader_parameter(instance_3d, "modelview_basis_x", modelview_basis.columns[0]);
		RS::get_singleton()->instance_geometry_set_shader_parameter(instance_3d, "modelview_basis_y", modelview_basis.columns[1]);
		RS::get_singleton()->instance_geometry_set_shader_parameter(instance_3d, "modelview_basis_z", modelview_basis.columns[2]);
		RS::get_singleton()->instance_geometry_set_shader_parameter(instance_3d, "modelview_basis_w", modelview_basis.columns[3]);

		instance_index++;
	}
	while (_instances_3d.size() > instance_index) {
		RS::get_singleton()->free(_instances_3d.pop_back());
	}
}

void CrossSectionRenderingEngine4D::update_camera() {
	// TODO 3D preview is broken, mesh is not drawn relative to preview camera. 
	ERR_FAIL_NULL(RenderingServer::get_singleton());
	if (_cross_section_camera.is_null()) {
		ERR_FAIL_NULL(get_viewport());
		_cross_section_camera = RS::get_singleton()->camera_create();
		RS::get_singleton()->viewport_attach_camera(get_viewport()->get_viewport_rid(), _cross_section_camera);
	}
	// Only setting final 3D->2D projection stuff here. Can't pass the full Transform4D or even the basis as the camera's transform because it expects a Transform3D.
	ERR_FAIL_NULL(get_camera());
	Camera4D *camera = get_camera();
	// Projection type is a bitmask of 3D and 4D projection. Ignoring 4D projection because we're cross-sectioning instead.
	if (camera->get_projection_type() == Camera4D::PROJECTION4D_ORTHOGRAPHIC) {
		RS::get_singleton()->camera_set_orthogonal(_cross_section_camera, camera->get_orthographic_size(), camera->get_clip_near(), camera->get_clip_far());
	} else if (bool(camera->get_projection_type() & Camera4D::PROJECTION4D_PERSPECTIVE_3D)) {
		RS::get_singleton()->camera_set_perspective(_cross_section_camera, Math::rad_to_deg(camera->get_field_of_view_3d()), camera->get_clip_near(), camera->get_clip_far());
	} else if (bool(camera->get_projection_type() & Camera4D::PROJECTION4D_PERSPECTIVE_4D)) {
		WARN_PRINT_ONCE("4D Perspective settings are ignored when using the Cross-section renderer. Use PESPECTIVE_3D or ORTHOGRAPHIC.");
	}
}

bool CrossSectionRenderingEngine4D::cull_mesh(const MeshInstance4D &mesh_instance, Projection basis, Vector4 position) const {
	// TODO verify, this is probably wrong. Also scans the mesh every time so may not be faster.
	Rect4 rect = mesh_instance.get_rect_bounds(get_camera()->get_transform().inverse());
	// If the mesh doesn't cross the cross-section plane, don't bother drawing it.
	if (rect.get_position().w > 0) {
		return true;
	}
	if (rect.get_end().w < 0) {
		return true;
	}
	// TODO frustum culling
	return false;
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
			RS::get_singleton()->instance_set_scenario(instance, scenario);
		}
	}
	if (_cross_section_camera.is_valid()) {
		RS::get_singleton()->viewport_attach_camera(get_viewport()->get_viewport_rid(), _cross_section_camera);
	}
}

CrossSectionRenderingEngine4D::~CrossSectionRenderingEngine4D() {
	ERR_FAIL_NULL(RenderingServer::get_singleton());
	for (RID instance : _instances_3d) {
		RS::get_singleton()->free(instance);
	}
	RS::get_singleton()->free(_cross_section_camera);
}
