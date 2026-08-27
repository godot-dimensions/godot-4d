#include "tetra_rendering_engine_4d.h"

#include "../model/mesh/mesh_instance_4d.h"
#include "../model/mesh/poly/poly_material_4d.h"
#include "../nodes/camera_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/rendering_server.hpp>
#elif GODOT_MODULE
#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR < 6
#include "servers/rendering_server.h"
#else
#include "servers/rendering/rendering_server.h"
#endif
#endif

void TetraRenderingEngine4D::_create_light_render_instance_3d(const ObjectID p_light_4d_node_object_id) {
	Light4D *light_4d = Object::cast_to<Light4D>(ObjectDB::get_instance(p_light_4d_node_object_id));
	ERR_FAIL_NULL(light_4d);
	RenderingServer *rendering_server = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rendering_server);
	if (!_world_3d.is_valid()) {
		_world_3d.instantiate();
	}
	LightRenderInstance3D light_render_instance_3d;
	light_render_instance_3d.base = light_4d->create_3d_render_base();
	ERR_FAIL_COND_MSG(!light_render_instance_3d.base.is_valid(), "Unable to create a Light3D render base RID for a Light4D node.");
	light_render_instance_3d.instance = rendering_server->instance_create();
	if (!light_render_instance_3d.instance.is_valid()) {
		rendering_server->free_rid(light_render_instance_3d.base);
		ERR_FAIL_MSG("Unable to create a Light3D render instance RID for a Light4D node.");
	}
	rendering_server->instance_set_visible(light_render_instance_3d.instance, false);
	rendering_server->instance_set_base(light_render_instance_3d.instance, light_render_instance_3d.base);
	rendering_server->instance_set_scenario(light_render_instance_3d.instance, _world_3d->get_scenario());
	_lights_3d[p_light_4d_node_object_id] = light_render_instance_3d;
}

RID TetraRenderingEngine4D::_create_mesh_render_instance_3d() {
	ERR_FAIL_NULL_V(RenderingServer::get_singleton(), RID());
	RID instance = RenderingServer::get_singleton()->instance_create();
	if (!_world_3d.is_valid()) {
		_world_3d.instantiate();
	}
	RenderingServer::get_singleton()->instance_set_scenario(instance, _world_3d->get_scenario());
	if (_ignore_mesh_culling()) {
		RenderingServer::get_singleton()->instance_set_ignore_culling(instance, true);
	}
	return instance;
}

void TetraRenderingEngine4D::_update_camera() {
	ERR_FAIL_NULL(RenderingServer::get_singleton());
	if (!_camera_3d.is_valid()) {
		ERR_FAIL_NULL(get_viewport());
		_camera_3d = RenderingServer::get_singleton()->camera_create();
		RenderingServer::get_singleton()->viewport_attach_camera(get_viewport()->get_viewport_rid(), _camera_3d);
	}
	// Only setting final 3D->2D projection stuff here. Can't pass the full Transform4D or even the basis as the camera's transform because it expects a Transform3D.
	ERR_FAIL_NULL(get_camera());
	Camera4D *camera = get_camera();
	const float clip_near = camera->get_clip_near();
	const float clip_far = camera->get_clip_far();
	switch (camera->get_projection_type()) {
		case Camera4D::PROJECTION4D_ORTHOGRAPHIC: {
			RenderingServer::get_singleton()->camera_set_orthogonal(_camera_3d, camera->get_orthographic_size(), clip_near, clip_far);
		} break;
		case Camera4D::PROJECTION4D_PERSPECTIVE_4D: {
			RenderingServer::get_singleton()->camera_set_perspective(_camera_3d, Math::rad_to_deg(camera->get_field_of_view_4d()), clip_near, clip_far);
		} break;
		case Camera4D::PROJECTION4D_PERSPECTIVE_3D: {
			RenderingServer::get_singleton()->camera_set_perspective(_camera_3d, Math::rad_to_deg(camera->get_field_of_view_3d()), clip_near, clip_far);
		} break;
		case Camera4D::PROJECTION4D_PERSPECTIVE_DUAL: {
			WARN_PRINT_ONCE("Dual-perspective is not supported by the " + get_friendly_name() + " renderer. Use PERSPECTIVE_3D, PERSPECTIVE_4D, or ORTHOGRAPHIC instead.");
			RenderingServer::get_singleton()->camera_set_perspective(_camera_3d, Math::rad_to_deg(camera->get_field_of_view_3d()), clip_near, clip_far);
		} break;
	}
}

void TetraRenderingEngine4D::_update_lights() {
	const PackedInt64Array light_object_ids = get_light_object_ids();
	const TypedArray<Projection> light_relative_basises = get_light_relative_basises();
	const PackedVector4Array light_relative_positions = get_light_relative_positions();
	RenderingServer *rendering_server = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rendering_server);
	ERR_FAIL_COND(light_relative_basises.size() != light_object_ids.size());
	ERR_FAIL_COND(light_relative_positions.size() != light_object_ids.size());
	for (int64_t light_index = 0; light_index < light_object_ids.size(); light_index++) {
		const ObjectID light_object_id = (ObjectID)light_object_ids[light_index];
		Light4D *light_4d = Object::cast_to<Light4D>(ObjectDB::get_instance(light_object_id));
		ERR_CONTINUE(light_4d == nullptr);
		if (!_lights_3d.has(light_object_id)) {
			_create_light_render_instance_3d(light_object_id);
			ERR_CONTINUE(!_lights_3d.has(light_object_id));
		}
		LightRenderInstance3D &light_render_instance_3d = _lights_3d[light_object_id]; // Mutable reference.
		const Projection light_relative_basis = light_relative_basises[light_index];
		const Vector4 light_relative_position = light_relative_positions[light_index];
		const bool visible = _update_light_3d_render_base(light_4d, light_relative_basis, light_relative_position, light_render_instance_3d.base);
		rendering_server->instance_set_visible(light_render_instance_3d.instance, visible);
		if (visible) {
			const Transform3D light_transform_3d = Transform3D(Basis4D(light_relative_basis).to_3d_orthonormalize_z_dominant(), Vector3(light_relative_position.x, light_relative_position.y, light_relative_position.z));
			rendering_server->instance_set_transform(light_render_instance_3d.instance, light_transform_3d);
		}
		light_render_instance_3d.last_used_pass = _current_pass;
	}
	// Delete any lights that are not currently visible and active in the scene.
	Vector<ObjectID> light_instance_ids_to_erase;
	for (const KeyValue<ObjectID, TetraRenderingEngine4D::LightRenderInstance3D> &light_pair : _lights_3d) {
		const LightRenderInstance3D &light_instance_3d = light_pair.value;
		if (light_instance_3d.last_used_pass != _current_pass) {
			rendering_server->free_rid(light_instance_3d.instance);
			rendering_server->free_rid(light_instance_3d.base);
			light_instance_ids_to_erase.append(light_pair.key);
		}
	}
	for (const ObjectID &light_object_id : light_instance_ids_to_erase) {
		_lights_3d.erase(light_object_id);
	}
}

void TetraRenderingEngine4D::_update_mesh_instances() {
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
		Ref<Mesh> mesh_3d = mesh_4d->get_proxy_mesh_3d();
		ERR_CONTINUE(!mesh_3d.is_valid());

		// This is a valid MeshInstance4D with a valid Mesh4D and a valid proxy Mesh3D.
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
			Ref<Material> override_material_3d = _get_material_3d(material_4d);
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
		// calculated AABB is unrelated to the positions the tetra shader produces. Supply conservative
		// camera-relative bounds instead, which frustum culling uses (unless _ignore_mesh_culling is
		// set) and which omni and spot lights are paired with geometry by, matching the space the
		// light transforms are in.
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
		_update_extra_instance_shader_parameters(mesh_render_instance_3d.instance);

		mesh_render_instance_3d.last_used_pass = _current_pass;
	}
	// Delete any meshes that are not currently visible and active in the scene.
	Vector<ObjectID> mesh_instance_ids_to_erase;
	for (const KeyValue<ObjectID, TetraRenderingEngine4D::MeshRenderInstance3D> &pair : _mesh_instances_3d) {
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

void TetraRenderingEngine4D::render_frame() {
	ERR_FAIL_NULL(RenderingServer::get_singleton());
	ERR_FAIL_NULL(get_camera());
	ERR_FAIL_NULL(get_viewport());
	_current_pass++;
	_update_camera();
	_update_lights();
	_update_environment();
	_update_mesh_instances();
}

void TetraRenderingEngine4D::setup_for_viewport() {
	ERR_FAIL_NULL(RenderingServer::get_singleton());
	ERR_FAIL_NULL(get_viewport());
	if (!_world_3d.is_valid()) {
		_world_3d.instantiate();
	}
	_setup_world_3d();
	Viewport *viewport = get_viewport();
	// Avoids a weird error from the current scenario on viewport not being initialized. Should ideally be handled by set_world_3d.
	RenderingServer::get_singleton()->viewport_set_scenario(viewport->get_viewport_rid(), _world_3d->get_scenario());
	viewport->set_world_3d(_world_3d);
	if (_camera_3d.is_valid()) {
		RenderingServer::get_singleton()->viewport_attach_camera(viewport->get_viewport_rid(), _camera_3d);
	}
}

void TetraRenderingEngine4D::free_shared_render_resources() {
	RenderingServer *rendering_server = RenderingServer::get_singleton();
	if (rendering_server == nullptr) {
		_lights_3d.clear();
		_mesh_instances_3d.clear();
		_camera_3d = RID();
		return;
	}
	for (const KeyValue<ObjectID, TetraRenderingEngine4D::LightRenderInstance3D> &pair : _lights_3d) {
		const LightRenderInstance3D &light_3d = pair.value;
		if (light_3d.instance.is_valid()) {
			rendering_server->free_rid(light_3d.instance);
		}
		if (light_3d.base.is_valid()) {
			rendering_server->free_rid(light_3d.base);
		}
	}
	_lights_3d.clear();
	for (const KeyValue<ObjectID, TetraRenderingEngine4D::MeshRenderInstance3D> &pair : _mesh_instances_3d) {
		const MeshRenderInstance3D &instance_3d = pair.value;
		if (instance_3d.instance.is_valid()) {
			rendering_server->free_rid(instance_3d.instance);
		}
	}
	_mesh_instances_3d.clear();
	if (_camera_3d.is_valid()) {
		rendering_server->free_rid(_camera_3d);
		_camera_3d = RID();
	}
	// Explicitly free the World3D so its scenario RID (and any remaining
	// instances inside it) are released while the RenderingServer is alive.
	_world_3d = Ref<World3D>();
}

void TetraRenderingEngine4D::cleanup_for_viewport() {
	// Detach the custom world from the viewport BEFORE freeing RS resources,
	// so any VisualInstance3D nodes in the viewport re-register in the
	// default world rather than our soon-to-be-freed scenario.
	Viewport *viewport = get_viewport();
	if (_world_3d.is_valid() && viewport != nullptr) {
		viewport->set_world_3d(Ref<World3D>());
		// same workaround as in setup_for_viewport (needed for when this is used in the combined renderer)
		RenderingServer::get_singleton()->viewport_set_scenario(viewport->get_viewport_rid(), RID());
	}
	free_shared_render_resources();
}
