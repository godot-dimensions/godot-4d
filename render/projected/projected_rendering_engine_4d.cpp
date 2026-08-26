#include "projected_rendering_engine_4d.h"

#include "../../model/mesh/mesh_instance_4d.h"
#include "../../model/mesh/poly/poly_material_4d.h"
#include "../../nodes/camera_4d.h"
#include "../../nodes/light/directional_light_4d.h"
#include "normalize_transparency.glsl.gen.h"

#if GDEXTENSION
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/uniform_set_cache_rd.hpp>
#include <godot_cpp/classes/rd_shader_source.hpp>
#include <godot_cpp/classes/rd_shader_spirv.hpp>
#include <godot_cpp/classes/rd_uniform.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#elif GODOT_MODULE
#include "servers/rendering/rendering_device_binds.h"
#include "servers/rendering/renderer_rd/uniform_set_cache_rd.h"
#include "servers/rendering/renderer_rd/storage_rd/render_scene_buffers_rd.h"
#include "core/config/project_settings.h"
#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR < 6
#include "servers/rendering_server.h"
#else
#include "servers/rendering/rendering_server.h"
#endif
#endif

/* notes for how to do the projection calculations in a compute shader:
// some sort of setup function:
	RenderingServer::get_singleton()->compositor_create();
	// Add compositor effect subclass. It should have COMPOSITOR_EFFECT_CALLBACK_TYPE_PRE_OPAQUE.
	RenderingDevice::get_singleton()->compute_pipeline_create(); //etc.

// in the compositor effect's main function:
	// for each 4D mesh:
		// Get the 3D mesh's buffers from RendererRD::MeshStorage::get_singleton()->mesh_surface_get_vertex_arrays_and_format
		// Bind the 3D vertex buffer and attribute buffer as outputs, and the 4D data buffer as the input.
		// Run the compute pipeline.
		// Maybe some sort of synchronization thing?
*/

void ProjectedRenderingEngine4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("normalize_image_callback", "effect_callback_type", "render_data"), &ProjectedRenderingEngine4D::normalize_image_callback);
}

ProjectedRenderingEngine4D::ProjectedRenderingEngine4D() {
	RenderingDevice *rd = RenderingDevice::get_singleton();
	if (rd == nullptr) {
		// The compatibility renderer isn't compatible with projected rendering anyway.
		return;
	}
	RenderingServer *rendering_server = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rendering_server);
	// shader_compile_spirv_from_source(Ref<RDShaderSource>)/shader_create_from_spirv(Ref<RDShaderSPIRV>)
	// are exposed to GDExtension directly, but in engine-module C++ they're private (named
	// _shader_compile_spirv_from_source/_shader_create_from_spirv; module code is expected to call
	// the raw stage/string/Vector<uint8_t> overloads of the same public names instead). Calling them
	// by name through the same Variant dispatch GDExtension and scripts use sidesteps that C++ access
	// specifier, so the exact same code works in both builds.
	Ref<RDShaderSource> shader_source;
	shader_source.instantiate();
	shader_source->set_stage_source(RenderingDevice::SHADER_STAGE_COMPUTE, normalize_transparency_shader_glsl);
	Ref<RDShaderSPIRV> shader_spirv = rd->call("shader_compile_spirv_from_source", shader_source);
	const String compile_error = shader_spirv->get_stage_compile_error(RenderingDevice::SHADER_STAGE_COMPUTE);
	ERR_FAIL_COND_MSG(!compile_error.is_empty(), "ProjectedRenderingEngine4D: Failed to compile normalize_transparency.glsl: " + compile_error);
	normalize_shader = rd->call("shader_create_from_spirv", shader_spirv);
	normalize_pipeline = rd->compute_pipeline_create(normalize_shader);
	normalize_compositor_effect = rendering_server->compositor_effect_create();
	rendering_server->compositor_effect_set_callback(
			normalize_compositor_effect,
			RSE::COMPOSITOR_EFFECT_CALLBACK_TYPE_POST_TRANSPARENT,
			Callable(this, "normalize_image_callback"));
	rendering_server->compositor_effect_set_enabled(normalize_compositor_effect, true); // It's only ever attached when it should be enabled.
	normalize_compositor = rendering_server->compositor_create();
	rendering_server->compositor_set_compositor_effects(normalize_compositor, TypedArray<RID>({ normalize_compositor_effect }));
}

void ProjectedRenderingEngine4D::set_cross_section_depth_texture(const Variant &p_texture) {
	_cross_section_depth_texture = p_texture;
}

void ProjectedRenderingEngine4D::_create_light_render_instance_3d(const ObjectID p_light_4d_node_object_id) {
	Light4D *light_4d = Object::cast_to<Light4D>(ObjectDB::get_instance(p_light_4d_node_object_id));
	ERR_FAIL_NULL(light_4d);
	RenderingServer *rendering_server = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rendering_server);
	if (!_projected_world_3d.is_valid()) {
		_projected_world_3d.instantiate();
	}
	LightRenderInstance3D light_render_instance_3d;
	light_render_instance_3d.base = light_4d->create_light_3d_render_base();
	ERR_FAIL_COND_MSG(!light_render_instance_3d.base.is_valid(), "Unable to create a Light3D render base RID for a Light4D node.");
	light_render_instance_3d.instance = rendering_server->instance_create();
	if (!light_render_instance_3d.instance.is_valid()) {
		rendering_server->free_rid(light_render_instance_3d.base);
		ERR_FAIL_MSG("Unable to create a Light3D render instance RID for a Light4D node.");
	}
	rendering_server->instance_set_visible(light_render_instance_3d.instance, false);
	rendering_server->instance_set_base(light_render_instance_3d.instance, light_render_instance_3d.base);
	rendering_server->instance_set_scenario(light_render_instance_3d.instance, _projected_world_3d->get_scenario());
	_lights_3d[p_light_4d_node_object_id] = light_render_instance_3d;
}

RID ProjectedRenderingEngine4D::_create_mesh_render_instance_3d() {
	ERR_FAIL_NULL_V(RenderingServer::get_singleton(), RID());
	RID instance = RenderingServer::get_singleton()->instance_create();
	if (!_projected_world_3d.is_valid()) {
		_projected_world_3d.instantiate();
	}
	RenderingServer::get_singleton()->instance_set_scenario(instance, _projected_world_3d->get_scenario());

	// Vertex data on the mesh is wack. Culling will not work.
	RenderingServer::get_singleton()->instance_set_ignore_culling(instance, true);
	return instance;
}

void ProjectedRenderingEngine4D::_update_lights() {
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
		// NOTE: update_light_3d_render_base() describes the light as it intersects the W = 0 slice
		// (for example OmniLight4D reports sqrt(range^2 - w^2), the radius of that cross-section, and
		// reports not-visible when the light misses the slice entirely). That is exactly right for
		// the cross-section pass, but the projected pass shows the whole 4D scene rather than one
		// slice, so lights are currently slice-culled here too even though geometry away from the
		// slice is visible and ought to be lit. Lighting the projected view on its own terms needs
		// light math that integrates over W rather than sampling it at 0.
		const bool visible_in_slice = light_4d->update_light_3d_render_base(light_relative_basis, light_relative_position, light_render_instance_3d.base);
		rendering_server->instance_set_visible(light_render_instance_3d.instance, visible_in_slice);
		if (visible_in_slice) {
			const Transform3D light_transform_3d = Transform3D(Basis4D(light_relative_basis).to_3d_orthonormalize_z_dominant(), Vector3(light_relative_position.x, light_relative_position.y, light_relative_position.z));
			rendering_server->instance_set_transform(light_render_instance_3d.instance, light_transform_3d);
		}
		light_render_instance_3d.last_used_pass = _current_pass;
	}
	// Delete any lights that are not currently visible and active in the scene.
	Vector<ObjectID> light_instance_ids_to_erase;
	for (const KeyValue<ObjectID, ProjectedRenderingEngine4D::LightRenderInstance3D> &light_pair : _lights_3d) {
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

void ProjectedRenderingEngine4D::_update_mesh_instances() {
	Camera4D *camera = get_camera();
	const double camera_slope = camera->get_w_fade_slope();
	const double camera_fade = camera->get_w_fade_distance();
	// The shader's edge_falloff and plane_softness uniforms keep their original ranges
	// ([1, inf) and (0, 1] respectively) - edge_falloff and plane_sharpness are the more
	// user-friendly camera-facing versions ([0, inf) and [0, 1) respectively), converted here.
	const double edge_falloff = camera->get_edge_falloff() + 1.0;
	const double plane_softness = 1.0 - camera->get_plane_sharpness();
	const double skewness = camera->get_skewness();
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
		Ref<Mesh> mesh_3d = mesh_4d->get_cross_section_mesh(); // TODO: rename this method. It works for both.
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
			Ref<ShaderMaterial> override_material_3d = material_4d->get_projected_material();
			ERR_CONTINUE(!override_material_3d.is_valid());
			override_material_rid_3d = override_material_3d->get_rid();
			// Always called even when _cross_section_depth_texture is NIL so that it is cleared if this engine is used alone.
			override_material_3d->set_shader_parameter("cross_section_depth_texture", _cross_section_depth_texture);
		}
		// Godot clears surface override materials when an instance's base changes.
		if (base_changed || mesh_render_instance_3d.material != override_material_rid_3d) {
			mesh_render_instance_3d.material = override_material_rid_3d;
			RenderingServer::get_singleton()->instance_set_surface_override_material(mesh_render_instance_3d.instance, 0, override_material_rid_3d);
		}

		Projection modelview_basis = modelview_basises[mesh_index];
		Vector4 modelview_origin = modelview_origins[mesh_index];
		// The proxy Mesh3D stores 4D data in its ordinary vertex channels, so its automatically
		// calculated AABB is unrelated to the positions produced by the projected shader. Frustum
		// culling is disabled outright for these instances (see _create_mesh_render_instance_3d),
		// but omni and spot lights are still paired with geometry by AABB overlap, so supply
		// conservative camera-relative bounds for that, matching the space the light transforms
		// above are in.
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
		RenderingServer::get_singleton()->instance_geometry_set_shader_parameter(mesh_render_instance_3d.instance, "camera_slope", camera_slope);
		RenderingServer::get_singleton()->instance_geometry_set_shader_parameter(mesh_render_instance_3d.instance, "camera_fade", camera_fade);
		RenderingServer::get_singleton()->instance_geometry_set_shader_parameter(mesh_render_instance_3d.instance, "edge_falloff", edge_falloff);
		RenderingServer::get_singleton()->instance_geometry_set_shader_parameter(mesh_render_instance_3d.instance, "plane_softness", plane_softness);
		RenderingServer::get_singleton()->instance_geometry_set_shader_parameter(mesh_render_instance_3d.instance, "skewness", skewness);

		mesh_render_instance_3d.last_used_pass = _current_pass;
	}
	// Delete any meshes that are not currently visible and active in the scene.
	Vector<ObjectID> mesh_instance_ids_to_erase;
	for (const KeyValue<ObjectID, ProjectedRenderingEngine4D::MeshRenderInstance3D> &pair : _mesh_instances_3d) {
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

void ProjectedRenderingEngine4D::render_frame() {
	ERR_FAIL_NULL(RenderingServer::get_singleton());
	ERR_FAIL_NULL(get_camera());
	ERR_FAIL_NULL(get_viewport());
	_current_pass++;
	_update_camera();
	_update_lights();
	_update_mesh_instances();
}

void ProjectedRenderingEngine4D::_update_camera() {
	ERR_FAIL_NULL(RenderingServer::get_singleton());
	if (!_projected_camera.is_valid()) {
		ERR_FAIL_NULL(get_viewport());
		_projected_camera = RenderingServer::get_singleton()->camera_create();
		RenderingServer::get_singleton()->viewport_attach_camera(get_viewport()->get_viewport_rid(), _projected_camera);
	}
	// Only setting final 3D->2D projection stuff here. Can't pass the full Transform4D or even the basis as the camera's transform because it expects a Transform3D.
	ERR_FAIL_NULL(get_camera());
	Camera4D *camera = get_camera();
	const float clip_near = camera->get_clip_near();
	const float clip_far = camera->get_clip_far();
	switch (camera->get_projection_type()) {
		case Camera4D::PROJECTION4D_ORTHOGRAPHIC: {
			RenderingServer::get_singleton()->camera_set_orthogonal(_projected_camera, camera->get_orthographic_size(), clip_near, clip_far);
		} break;
		case Camera4D::PROJECTION4D_PERSPECTIVE_4D: {
			RenderingServer::get_singleton()->camera_set_perspective(_projected_camera, Math::rad_to_deg(camera->get_field_of_view_4d()), clip_near, clip_far);
		} break;
		case Camera4D::PROJECTION4D_PERSPECTIVE_3D: {
			RenderingServer::get_singleton()->camera_set_perspective(_projected_camera, Math::rad_to_deg(camera->get_field_of_view_3d()), clip_near, clip_far);
		} break;
		case Camera4D::PROJECTION4D_PERSPECTIVE_DUAL: {
			WARN_PRINT_ONCE("Dual-perspective is not supported by the Projected renderer. Use PERSPECTIVE_3D, PERSPECTIVE_4D, or ORTHOGRAPHIC instead.");
			RenderingServer::get_singleton()->camera_set_perspective(_projected_camera, Math::rad_to_deg(camera->get_field_of_view_3d()), clip_near, clip_far);
		} break;
	}
}

void ProjectedRenderingEngine4D::setup_for_viewport() {
	ERR_FAIL_NULL(RenderingServer::get_singleton());
	ERR_FAIL_NULL(get_viewport());
	if (!_projected_world_3d.is_valid()) {
		_projected_world_3d.instantiate();
	}
	Viewport *viewport = get_viewport();
	// Avoids a weird error from the current scenario on viewport not being initialized. Should ideally be handled by set_world_3d.
	RenderingServer::get_singleton()->viewport_set_scenario(viewport->get_viewport_rid(), _projected_world_3d->get_scenario());
	viewport->set_world_3d(_projected_world_3d);
	if (_projected_camera.is_valid()) {
		RenderingServer::get_singleton()->viewport_attach_camera(viewport->get_viewport_rid(), _projected_camera);
	}
	if (normalize_compositor.is_valid()) {
		RenderingServer::get_singleton()->scenario_set_compositor(_projected_world_3d->get_scenario(), normalize_compositor);
	}
}

void ProjectedRenderingEngine4D::_cleanup_render_resources() {
	cleanup_for_viewport();
	RenderingServer *rendering_server = RenderingServer::get_singleton();
	if (normalize_compositor.is_valid()) {
		rendering_server->free_rid(normalize_compositor);
		normalize_compositor = RID();
		rendering_server->free_rid(normalize_compositor_effect);
		normalize_compositor_effect = RID();
	}
	RenderingDevice *rd = RenderingDevice::get_singleton();
	if (normalize_pipeline.is_valid()) {
		rd->free_rid(normalize_pipeline);
		normalize_pipeline = RID();
	}
	if (normalize_shader.is_valid()) {
		rd->free_rid(normalize_shader);
		normalize_shader = RID();
	}
}

void ProjectedRenderingEngine4D::cleanup_for_viewport() {
	// Detach the custom world from the viewport BEFORE freeing RS resources,
	// so any VisualInstance3D nodes in the viewport re-register in the
	// default world rather than our soon-to-be-freed scenario.
	Viewport *viewport = get_viewport();
	if (_projected_world_3d.is_valid() && viewport != nullptr) {
		viewport->set_world_3d(Ref<World3D>());
		// same workaround as in setup_for_viewport (needed for when this is used in the combined renderer)
		RenderingServer::get_singleton()->viewport_set_scenario(viewport->get_viewport_rid(), RID());
	}
	RenderingServer *rendering_server = RenderingServer::get_singleton();
	if (rendering_server == nullptr) {
		_lights_3d.clear();
		_mesh_instances_3d.clear();
		_projected_camera = RID();
		return;
	}
	for (const KeyValue<ObjectID, ProjectedRenderingEngine4D::LightRenderInstance3D> &pair : _lights_3d) {
		const LightRenderInstance3D &light_3d = pair.value;
		if (light_3d.instance.is_valid()) {
			rendering_server->free_rid(light_3d.instance);
		}
		if (light_3d.base.is_valid()) {
			rendering_server->free_rid(light_3d.base);
		}
	}
	_lights_3d.clear();
	for (const KeyValue<ObjectID, ProjectedRenderingEngine4D::MeshRenderInstance3D> &pair : _mesh_instances_3d) {
		const MeshRenderInstance3D &instance_3d = pair.value;
		if (instance_3d.instance.is_valid()) {
			rendering_server->free_rid(instance_3d.instance);
		}
	}
	_mesh_instances_3d.clear();
	if (_projected_camera.is_valid()) {
		rendering_server->free_rid(_projected_camera);
		_projected_camera = RID();
	}
	// Explicitly free the World3D so its scenario RID (and any remaining
	// instances inside it) are released while the RenderingServer is alive.
	_projected_world_3d = Ref<World3D>();
}

void ProjectedRenderingEngine4D::normalize_image_callback(int64_t p_effect_callback_type, RenderData *p_render_data) {
	RenderingDevice *rd = RenderingDevice::get_singleton();
	ERR_FAIL_NULL(rd);
	ERR_FAIL_NULL(p_render_data);
	Camera4D *camera = get_camera();
	ERR_FAIL_NULL(camera);
	RenderSceneBuffersRD *buffers = Object::cast_to<RenderSceneBuffersRD>(p_render_data->get_render_scene_buffers().ptr());
	if (buffers) {
		const Vector2i size = buffers->get_internal_size();
		PackedByteArray push_constant;
		push_constant.resize(16);
		{
			int32_t *push_constant_data = reinterpret_cast<int32_t *>(push_constant.ptrw());
			push_constant_data[0] = size.x;
			push_constant_data[1] = size.y;
			float *push_constant_float_data = reinterpret_cast<float *>(push_constant.ptrw());
			push_constant_float_data[2] = (float)camera->get_projection_opacity_base();
			// Whether to leave the output transparent. The input is always transparent because the
			// normalization requires it, but the output only needs to be when something is going to
			// composite it over something else: either the combined renderer (_cross_section_depth_texture
			// is set iff this is running as part of it) or whatever the project setting is for.
			const bool project_transparent_background = ProjectSettings::get_singleton()->get_setting("rendering/viewport/transparent_background");
			push_constant_data[3] = (!_cross_section_depth_texture.is_null() || project_transparent_background) ? 1 : 0;
		}
		for (uint32_t view = 0; view < buffers->get_view_count(); view++) {
			Ref<RDUniform> uniform;
			uniform.instantiate();
			uniform->set_uniform_type(RenderingDevice::UNIFORM_TYPE_IMAGE);
			uniform->set_binding(0);
			uniform->add_id(buffers->get_internal_texture(view));
			const TypedArray<RDUniform> uniforms({ uniform });
			// UniformSetCacheRD's C++ template get_cache() isn't reachable from GDExtension, so
			// this uses its non-template, TypedArray-based equivalent instead - exposed under a
			// different name in each build.
#if GDEXTENSION
			RID uniform_set = UniformSetCacheRD::get_cache(normalize_shader, 0, uniforms);
#elif GODOT_MODULE
			RID uniform_set = UniformSetCacheRD::get_cache_array(normalize_shader, 0, uniforms);
#endif
			RenderingDevice::ComputeListID compute_list = rd->compute_list_begin();
			rd->compute_list_bind_compute_pipeline(compute_list, normalize_pipeline);
			rd->compute_list_bind_uniform_set(compute_list, uniform_set, 0);
			// compute_list_set_push_constant takes a PackedByteArray in GDExtension, but only a
			// raw pointer is exposed to engine-module C++ (the PackedByteArray-based overload is
			// private there, same split as the shader-compile methods in the constructor).
#if GDEXTENSION
			rd->compute_list_set_push_constant(compute_list, push_constant, push_constant.size());
#elif GODOT_MODULE
			rd->compute_list_set_push_constant(compute_list, push_constant.ptr(), push_constant.size());
#endif
			rd->compute_list_dispatch_threads(compute_list, size.x, size.y, 1);
			rd->compute_list_end();
		}
	}
}

ProjectedRenderingEngine4D::~ProjectedRenderingEngine4D() {
	_cleanup_render_resources();
}