#include "projected_rendering_engine_4d.h"

#include "../../nodes/camera_4d.h"
#include "normalize_transparency.glsl.gen.h"

#if GDEXTENSION
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/rd_shader_source.hpp>
#include <godot_cpp/classes/rd_shader_spirv.hpp>
#include <godot_cpp/classes/rd_uniform.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/uniform_set_cache_rd.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#elif GODOT_MODULE
#include "core/config/project_settings.h"
#include "servers/rendering/renderer_rd/storage_rd/render_scene_buffers_rd.h"
#include "servers/rendering/renderer_rd/uniform_set_cache_rd.h"
#include "servers/rendering/rendering_device_binds.h"
#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR < 6
#include "servers/rendering_server.h"
#else
#include "servers/rendering/rendering_server.h"
#endif
#endif

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

bool ProjectedRenderingEngine4D::_update_light_3d_render_base(Light4D *p_light_4d, const Projection &p_relative_basis, const Vector4 &p_relative_position, const RID p_render_base) const {
	// The projection of a 4D light is never empty, unlike its cross section.
	p_light_4d->update_3d_projected_render_base(p_relative_basis, p_relative_position, p_render_base);
	return true;
}

Ref<Material> ProjectedRenderingEngine4D::_get_material_3d(const Ref<Material4D> &p_material_4d) {
	Ref<ShaderMaterial> material_3d = p_material_4d->get_projected_material();
	if (material_3d.is_valid()) {
		// Always set even when _cross_section_depth_texture is NIL so that it is cleared if this engine is used alone.
		material_3d->set_shader_parameter("cross_section_depth_texture", _cross_section_depth_texture);
	}
	return material_3d;
}

void ProjectedRenderingEngine4D::_update_extra_instance_shader_parameters(const RID p_instance) {
	Camera4D *camera = get_camera();
	ERR_FAIL_NULL(camera);
	RenderingServer *rendering_server = RenderingServer::get_singleton();
	rendering_server->instance_geometry_set_shader_parameter(p_instance, "camera_slope", camera->get_w_fade_slope());
	rendering_server->instance_geometry_set_shader_parameter(p_instance, "camera_fade", camera->get_w_fade_distance());
	// edge_falloff and plane_softness are converted from user-friendly form outside the shader
	// to computationally-efficient form inside it.
	rendering_server->instance_geometry_set_shader_parameter(p_instance, "edge_falloff", camera->get_edge_falloff() + 1.0);
	rendering_server->instance_geometry_set_shader_parameter(p_instance, "plane_softness", 1.0 - camera->get_plane_sharpness());
	rendering_server->instance_geometry_set_shader_parameter(p_instance, "skewness", camera->get_skewness());
}

void ProjectedRenderingEngine4D::_setup_world_3d() {
	if (normalize_compositor.is_valid()) {
		RenderingServer::get_singleton()->scenario_set_compositor(get_world_3d()->get_scenario(), normalize_compositor);
	}
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

void ProjectedRenderingEngine4D::_free_normalize_resources() {
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

ProjectedRenderingEngine4D::~ProjectedRenderingEngine4D() {
	free_shared_render_resources();
	_free_normalize_resources();
}
