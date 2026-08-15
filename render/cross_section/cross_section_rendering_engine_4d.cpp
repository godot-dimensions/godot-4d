#include "cross_section_rendering_engine_4d.h"

#include "../../model/mesh/mesh_instance_4d.h"
#include "../../model/mesh/poly/poly_material_4d.h"
#include "../../nodes/camera_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/rendering_server.hpp>
#elif GODOT_MODULE
#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR < 6
#include "servers/rendering_server.h"
#else
#include "servers/rendering/rendering_server.h"
#endif
#endif

RID CrossSectionRenderingEngine4D::_create_mesh_render_instance_3d() {
	ERR_FAIL_NULL_V(RenderingServer::get_singleton(), RID());
	RID instance = RenderingServer::get_singleton()->instance_create();
	if (!_cross_section_world_3d.is_valid()) {
		_cross_section_world_3d.instantiate();
	}
	RenderingServer::get_singleton()->instance_set_scenario(instance, _cross_section_world_3d->get_scenario());
	return instance;
}

void CrossSectionRenderingEngine4D::_update_camera() {
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
			WARN_PRINT_ONCE("Dual-perspective is not supported by the Cross-section renderer. Use PERSPECTIVE_3D, PERSPECTIVE_4D, or ORTHOGRAPHIC instead.");
			RenderingServer::get_singleton()->camera_set_perspective(_cross_section_camera, Math::rad_to_deg(camera->get_field_of_view_3d()), clip_near, clip_far);
		} break;
	}
}

void CrossSectionRenderingEngine4D::_update_mesh_instances() {
	// Maps global to cameral-local, aka world space to view space.
	const PackedInt64Array mesh_instance_4d_object_ids = get_mesh_instance_object_ids();
	const TypedArray<Projection> modelview_basises = get_mesh_relative_basises();
	const PackedVector4Array modelview_origins = get_mesh_relative_positions();
	for (int64_t mesh_index = 0; mesh_index < mesh_instance_4d_object_ids.size(); mesh_index++) {
		// Get the MeshInstance4D and its Mesh4D, skipping if either is invalid.
		const ObjectID mesh_instance_4d_object_id = (ObjectID)mesh_instance_4d_object_ids[mesh_index];
		MeshInstance4D *mesh_instance_4d = Object::cast_to<MeshInstance4D>(ObjectDB::get_instance(mesh_instance_4d_object_id));
		ERR_CONTINUE(mesh_instance_4d == nullptr);

		Ref<Mesh4D> mesh_4d = mesh_instance_4d->get_mesh();
		if (!mesh_4d.is_valid()) {
			continue;
		}
		Ref<Mesh> mesh_3d = mesh_4d->get_cross_section_mesh();
		ERR_CONTINUE(!mesh_3d.is_valid());

		// This is a valid MeshInstance4D with a valid Mesh4D and a valid cross-section Mesh3D.
		// Get or create an MeshRenderInstance3D for this MeshInstance4D, and update if needed.
		if (!_mesh_instances_3d.has(mesh_instance_4d_object_id)) {
			_mesh_instances_3d[mesh_instance_4d_object_id] = MeshRenderInstance3D();
			_mesh_instances_3d[mesh_instance_4d_object_id].instance = _create_mesh_render_instance_3d();
		}
		MeshRenderInstance3D &mesh_render_instance_3d = _mesh_instances_3d[mesh_instance_4d_object_id];
		const RID base_3d_rid = mesh_3d->get_rid();
		const bool base_changed = mesh_render_instance_3d.base != base_3d_rid;
		if (base_changed) {
			mesh_render_instance_3d.base = base_3d_rid;
			RenderingServer::get_singleton()->instance_set_base(mesh_render_instance_3d.instance, base_3d_rid);
		}

		Ref<Material4D> material_4d = mesh_instance_4d->get_active_material();
		if (!material_4d.is_valid()) {
			material_4d = mesh_4d->get_fallback_material();
		}
		RID override_material_rid_3d = RID();
		if (material_4d.is_valid()) {
			Ref<PolyMaterial4D> poly_material_4d = material_4d;
			if (poly_material_4d.is_valid()) {
				Ref<TetraMesh4D> poly_mesh_4d_or_poly_derived_tetra_mesh_4d = mesh_4d;
				if (poly_mesh_4d_or_poly_derived_tetra_mesh_4d.is_valid()) {
					poly_material_4d->populate_albedo_color_array_for_poly_mesh(poly_mesh_4d_or_poly_derived_tetra_mesh_4d);
				}
			}
			Ref<Material> override_material_3d = material_4d->get_cross_section_material();
			ERR_CONTINUE(!override_material_3d.is_valid());
			override_material_rid_3d = override_material_3d->get_rid();
		}
		// Godot clears surface override materials when an instance's base changes.
		if (base_changed || mesh_render_instance_3d.material != override_material_rid_3d) {
			mesh_render_instance_3d.material = override_material_rid_3d;
			RenderingServer::get_singleton()->instance_set_surface_override_material(mesh_render_instance_3d.instance, 0, override_material_rid_3d);
		}

		Projection modelview_basis = modelview_basises[mesh_index];
		Vector4 modelview_origin = modelview_origins[mesh_index];
		// The proxy Mesh3D stores 4D data in its ordinary vertex channels, so its automatically
		// calculated AABB is unrelated to the positions produced by the cross-section shader.
		// Supply conservative camera-relative bounds for frustum culling and light pairing.
		const Rect4 bounds_4d = mesh_instance_4d->get_rect_bounds_local(Transform4D(modelview_basis, modelview_origin));
		const AABB bounds_3d = AABB(Vector3(bounds_4d.position.x, bounds_4d.position.y, bounds_4d.position.z), Vector3(bounds_4d.size.x, bounds_4d.size.y, bounds_4d.size.z));
		RenderingServer::get_singleton()->instance_set_custom_aabb(mesh_render_instance_3d.instance, bounds_3d);
		// TODO Need to split out view matrix to support multiple viewports, currently the same for all viewports. Either instance per viewport or pack view matrix in camera attributes.
		RenderingServer::get_singleton()->instance_geometry_set_shader_parameter(mesh_render_instance_3d.instance, "modelview_origin", modelview_origin);
		// Can't pass a mat4 through instance uniforms, need to break up into columns.
		RenderingServer::get_singleton()->instance_geometry_set_shader_parameter(mesh_render_instance_3d.instance, "modelview_basis_x", modelview_basis.columns[0]);
		RenderingServer::get_singleton()->instance_geometry_set_shader_parameter(mesh_render_instance_3d.instance, "modelview_basis_y", modelview_basis.columns[1]);
		RenderingServer::get_singleton()->instance_geometry_set_shader_parameter(mesh_render_instance_3d.instance, "modelview_basis_z", modelview_basis.columns[2]);
		RenderingServer::get_singleton()->instance_geometry_set_shader_parameter(mesh_render_instance_3d.instance, "modelview_basis_w", modelview_basis.columns[3]);

		mesh_render_instance_3d.last_used_pass = _current_pass;
	}
	// Delete any meshes that are not currently visible and active in the scene.
	Vector<ObjectID> mesh_instance_ids_to_erase;
	for (const KeyValue<ObjectID, CrossSectionRenderingEngine4D::MeshRenderInstance3D> &pair : _mesh_instances_3d) {
		const MeshRenderInstance3D &instance_3d = pair.value;
		if (instance_3d.last_used_pass != _current_pass) {
			RenderingServer::get_singleton()->free_rid(instance_3d.instance);
			mesh_instance_ids_to_erase.append(pair.key);
		}
	}
	for (const ObjectID &mesh_instance_id : mesh_instance_ids_to_erase) {
		_mesh_instances_3d.erase(mesh_instance_id);
	}
}

void CrossSectionRenderingEngine4D::render_frame() {
	ERR_FAIL_NULL(RenderingServer::get_singleton());
	ERR_FAIL_NULL(get_camera());
	ERR_FAIL_NULL(get_viewport());
	_current_pass++;
	_update_camera();
	_update_mesh_instances();
}

void CrossSectionRenderingEngine4D::setup_for_viewport() {
	ERR_FAIL_NULL(RenderingServer::get_singleton());
	ERR_FAIL_NULL(get_viewport());
	if (!_cross_section_world_3d.is_valid()) {
		_cross_section_world_3d.instantiate();
	}
	Viewport *viewport = get_viewport();
	// Avoids a weird error from the current scenario on viewport not being initialized. Should ideally be handled by set_world_3d.
	RenderingServer::get_singleton()->viewport_set_scenario(viewport->get_viewport_rid(), _cross_section_world_3d->get_scenario());
	viewport->set_world_3d(_cross_section_world_3d);
	if (_cross_section_camera.is_valid()) {
		RenderingServer::get_singleton()->viewport_attach_camera(viewport->get_viewport_rid(), _cross_section_camera);
	}
}

void CrossSectionRenderingEngine4D::_cleanup_render_resources() {
	RenderingServer *rendering_server = RenderingServer::get_singleton();
	if (rendering_server == nullptr) {
		_mesh_instances_3d.clear();
		_cross_section_camera = RID();
		return;
	}
	for (const KeyValue<ObjectID, CrossSectionRenderingEngine4D::MeshRenderInstance3D> &pair : _mesh_instances_3d) {
		const MeshRenderInstance3D &instance_3d = pair.value;
		if (instance_3d.instance.is_valid()) {
			rendering_server->free_rid(instance_3d.instance);
		}
	}
	_mesh_instances_3d.clear();
	if (_cross_section_camera.is_valid()) {
		rendering_server->free_rid(_cross_section_camera);
		_cross_section_camera = RID();
	}
	// Explicitly free the World3D so its scenario RID (and any remaining
	// instances inside it) are released while the RenderingServer is alive.
	_cross_section_world_3d = Ref<World3D>();
}

void CrossSectionRenderingEngine4D::cleanup_for_viewport() {
	// Detach the custom world from the viewport BEFORE freeing RS resources,
	// so any VisualInstance3D nodes in the viewport re-register in the
	// default world rather than our soon-to-be-freed scenario.
	Viewport *viewport = get_viewport();
	if (_cross_section_world_3d.is_valid() && viewport != nullptr) {
		viewport->set_world_3d(Ref<World3D>());
	}
	_cleanup_render_resources();
}

CrossSectionRenderingEngine4D::~CrossSectionRenderingEngine4D() {
	_cleanup_render_resources();
}
