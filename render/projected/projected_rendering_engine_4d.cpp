#include "projected_rendering_engine_4d.h"

#include "../../model/mesh/mesh_instance_4d.h"
#include "../../model/mesh/poly/poly_material_4d.h"
#include "../../nodes/camera_4d.h"
#include "normalize_transparency.glsl.gen.h"

#if GDEXTENSION
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/uniform_set_cache_rd.hpp>
#include <godot_cpp/classes/rd_shader_source.hpp>
#include <godot_cpp/classes/rd_shader_spirv.hpp>
#include <godot_cpp/classes/rd_uniform.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR < 4
#include <godot_cpp/classes/project_settings.hpp>
#endif
#elif GODOT_MODULE
#include "servers/rendering/rendering_device_binds.h"
#include "servers/rendering/renderer_rd/uniform_set_cache_rd.h"
#include "servers/rendering/renderer_rd/storage_rd/render_scene_buffers_rd.h"
#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR < 4
#include "core/config/project_settings.h"
#endif
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
			RenderingServer::COMPOSITOR_EFFECT_CALLBACK_TYPE_POST_TRANSPARENT,
			Callable(this, "normalize_image_callback"));
	rendering_server->compositor_effect_set_enabled(normalize_compositor_effect, true); // It's only ever attached when it should be enabled.
	normalize_compositor = rendering_server->compositor_create();
	rendering_server->compositor_set_compositor_effects(normalize_compositor, TypedArray<RID>({ normalize_compositor_effect }));
}

void ProjectedRenderingEngine4D::set_cross_section_depth_texture(const Variant &p_texture) {
	_cross_section_depth_texture = p_texture;
}

void ProjectedRenderingEngine4D::render_frame() {
	ERR_FAIL_NULL(RenderingServer::get_singleton());
	ERR_FAIL_NULL(get_camera());
	ERR_FAIL_NULL(get_viewport());
	update_camera();
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
	TypedArray<MeshInstance4D> mesh_instances = get_mesh_instances();
	TypedArray<Projection> modelview_basises = get_mesh_relative_basises();
	PackedVector4Array modelview_origins = get_mesh_relative_positions();
	int64_t instances_allocated = _instances_3d.size();
	if (mesh_instances.size() > instances_allocated) {
		_instances_3d.resize(mesh_instances.size());
	}
	int64_t instance_index = 0;
	for (int mesh_index = 0; mesh_index < mesh_instances.size(); mesh_index++) {
		MeshInstance4D *mesh_instance = Object::cast_to<MeshInstance4D>(mesh_instances[mesh_index]);
		ERR_CONTINUE(mesh_instance == nullptr);

		Ref<Mesh4D> mesh_4d = mesh_instance->get_mesh();
		if (!mesh_4d.is_valid()) {
			continue;
		}
		Ref<Mesh> mesh_3d = mesh_4d->get_cross_section_mesh(); // TODO: rename this method. It works for both.
		ERR_CONTINUE(!mesh_3d.is_valid());

		if (instances_allocated <= instance_index) {
			_instances_3d.set(instances_allocated++, create_instance());
		}
		RID instance_3d = _instances_3d[instance_index];

		RenderingServer::get_singleton()->instance_set_base(instance_3d, mesh_3d->get_rid());

		Ref<Material4D> material_4d = mesh_instance->get_active_material();
		if (!material_4d.is_valid()) {
			material_4d = mesh_4d->get_fallback_material();
		}
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
			RenderingServer::get_singleton()->instance_set_surface_override_material(instance_3d, 0, override_material_3d->get_rid());
			// Always called even when _cross_section_depth_texture is NIL so that it is cleared if this engine is used alone.
			override_material_3d->set_shader_parameter("cross_section_depth_texture", _cross_section_depth_texture);
		}

		Projection modelview_basis = modelview_basises[mesh_index];
		Vector4 modelview_origin = modelview_origins[mesh_index];
		// TODO Need to split out view matrix to support multiple viewports, currently the same for all viewports. Either instance per viewport or pack view matrix in camera attributes.
		RenderingServer::get_singleton()->instance_geometry_set_shader_parameter(instance_3d, "modelview_origin", modelview_origin);
		// Can't pass a mat4 through instance uniforms, need to break up into columns.
		RenderingServer::get_singleton()->instance_geometry_set_shader_parameter(instance_3d, "modelview_basis_x", modelview_basis.columns[0]);
		RenderingServer::get_singleton()->instance_geometry_set_shader_parameter(instance_3d, "modelview_basis_y", modelview_basis.columns[1]);
		RenderingServer::get_singleton()->instance_geometry_set_shader_parameter(instance_3d, "modelview_basis_z", modelview_basis.columns[2]);
		RenderingServer::get_singleton()->instance_geometry_set_shader_parameter(instance_3d, "modelview_basis_w", modelview_basis.columns[3]);
		RenderingServer::get_singleton()->instance_geometry_set_shader_parameter(instance_3d, "camera_slope", camera_slope);
		RenderingServer::get_singleton()->instance_geometry_set_shader_parameter(instance_3d, "camera_fade", camera_fade);
		RenderingServer::get_singleton()->instance_geometry_set_shader_parameter(instance_3d, "edge_falloff", edge_falloff);
		RenderingServer::get_singleton()->instance_geometry_set_shader_parameter(instance_3d, "plane_softness", plane_softness);
		RenderingServer::get_singleton()->instance_geometry_set_shader_parameter(instance_3d, "skewness", skewness);

		instance_index++;
	}
	for (int i = instance_index; i < _instances_3d.size(); i++) {
		RID instance = _instances_3d[i];
		RenderingServer::get_singleton()->free_rid(instance);
	}
	_instances_3d.resize(instance_index);
}

RID ProjectedRenderingEngine4D::create_instance() {
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

void ProjectedRenderingEngine4D::update_camera() {
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
			WARN_PRINT_ONCE("Dual-perspective is not supported by the Cross-section renderer. Use PESPECTIVE_3D, PERSPECTIVE_4D, or ORTHOGRAPHIC instead.");
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
		// Only Forward+ is supported. If a different renderer is selected, best avoid attaching
		// the compositor to prevent error spam.
#if GODOT_VERSION_MAJOR > 4 || (GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR >= 4)
		const String rendering_method = RenderingServer::get_singleton()->get_current_rendering_method();
#else
		const String rendering_method = ProjectSettings::get_singleton()->get_setting("rendering/renderer/rendering_method");
#endif
		if (rendering_method == "forward_plus") {
			RenderingServer::get_singleton()->scenario_set_compositor(_projected_world_3d->get_scenario(), normalize_compositor);
		} else {
			WARN_PRINT_ONCE("ProjectedRenderingEngine4D is only compatible with the Forward+ rendering method, not " + rendering_method + ".");
		}
	}
	viewport->set_transparent_background(true);
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
		_instances_3d.clear();
		_projected_camera = RID();
		return;
	}
	for (const RID &instance : _instances_3d) {
		if (instance.is_valid()) {
			rendering_server->free_rid(instance);
		}
	}
	_instances_3d.clear();
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
			push_constant_data[3] = 0;
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