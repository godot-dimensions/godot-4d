#include "projected_rendering_engine_4d.h"

#include "../../../nodes/camera_4d.h"
#include "normalize_transparency.glsl.gen.h"

#if GDEXTENSION
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/rd_shader_source.hpp>
#include <godot_cpp/classes/rd_shader_spirv.hpp>
#include <godot_cpp/classes/rd_uniform.hpp>
#include <godot_cpp/classes/render_scene_buffers_rd.hpp>
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/uniform_set_cache_rd.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#elif GODOT_MODULE
#include "core/config/project_settings.h"
#include "servers/rendering/renderer_rd/storage_rd/render_scene_buffers_rd.h"
#include "servers/rendering/renderer_rd/uniform_set_cache_rd.h"
#include "servers/rendering/rendering_device.h"
#include "servers/rendering/rendering_device_binds.h"
#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR < 6
#include "servers/rendering_server.h"
#else
#include "servers/rendering/rendering_server.h"
#endif
#endif

ProjectedRenderingEngine4D::ProjectedRenderingEngine4D() {
	RenderingServer *rendering_server = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rendering_server);
	RenderingDevice *rd = rendering_server->get_rendering_device();
	if (rd == nullptr) {
		// The Compatibility renderer isn't compatible with projected rendering anyway.
		return;
	}
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
	ERR_FAIL_COND(shader_spirv.is_null());
	const String compile_error = shader_spirv->get_stage_compile_error(RenderingDevice::SHADER_STAGE_COMPUTE);
	ERR_FAIL_COND_MSG(!compile_error.is_empty(), "ProjectedRenderingEngine4D: Failed to compile normalize_transparency.glsl: " + compile_error);
	_normalize_shader = rd->call("shader_create_from_spirv", shader_spirv);
	_normalize_pipeline = rd->compute_pipeline_create(_normalize_shader);
	_normalize_compositor_effect = rendering_server->compositor_effect_create();
	rendering_server->compositor_effect_set_callback(
			_normalize_compositor_effect,
			RSE::COMPOSITOR_EFFECT_CALLBACK_TYPE_POST_TRANSPARENT,
			callable_mp(this, &ProjectedRenderingEngine4D::_normalize_image_callback));
	rendering_server->compositor_effect_set_enabled(_normalize_compositor_effect, true); // It's only ever attached when it should be enabled.
	_normalize_compositor = rendering_server->compositor_create();
	TypedArray<RID> compositor_effects;
	compositor_effects.push_back(_normalize_compositor_effect);
	rendering_server->compositor_set_compositor_effects(_normalize_compositor, compositor_effects);
}

void ProjectedRenderingEngine4D::set_cross_section_depth_texture(const Variant &p_texture) {
	_cross_section_depth_texture = p_texture;
}

// RenderingEngine4D interface.

void ProjectedRenderingEngine4D::_render_frame_callback() {
	ERR_FAIL_NULL(RenderingServer::get_singleton());
	ERR_FAIL_NULL(get_camera());
	ERR_FAIL_NULL(get_viewport());
	_update_3d_camera();
	_update_3d_lights();
	_update_3d_mesh_instances();
}

// Godot3DRenderingEngine4D interface.

Ref<Material> ProjectedRenderingEngine4D::_get_material_3d(const Ref<Material4D> &p_material_4d) const {
	Ref<ShaderMaterial> material_3d = p_material_4d->get_projected_material_3d();
	if (material_3d.is_valid()) {
		// Always set even when _cross_section_depth_texture is NIL so that it is cleared if this engine is used alone.
		material_3d->set_shader_parameter("cross_section_depth_texture", _cross_section_depth_texture);
	}
	return material_3d;
}

bool ProjectedRenderingEngine4D::_update_light_3d_render_base(Light4D *p_light_4d, const Projection &p_relative_basis, const Vector4 &p_relative_position, const RID p_render_base) const {
	return p_light_4d->update_light_3d_projected_render_base(p_relative_basis, p_relative_position, p_render_base);
}

void ProjectedRenderingEngine4D::_update_extra_instance_shader_parameters(const RID p_instance) {
	Camera4D *camera = get_camera();
	ERR_FAIL_NULL(camera);
	RenderingServer *rendering_server = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rendering_server);
	rendering_server->instance_geometry_set_shader_parameter(p_instance, "camera_slope", camera->get_w_fade_slope());
	rendering_server->instance_geometry_set_shader_parameter(p_instance, "camera_fade", camera->get_w_fade_distance());
	// Convert the editor-facing ranges to the forms used by the shader.
	rendering_server->instance_geometry_set_shader_parameter(p_instance, "edge_falloff", camera->get_edge_falloff() + 1.0);
	rendering_server->instance_geometry_set_shader_parameter(p_instance, "plane_softness", 1.0 - camera->get_plane_sharpness());
	rendering_server->instance_geometry_set_shader_parameter(p_instance, "skewness", camera->get_skewness());
}

void ProjectedRenderingEngine4D::_setup_specific_render_resources() {
	const Ref<World3D> &world_3d = get_world_3d();
	ERR_FAIL_COND(!world_3d.is_valid());
	if (_normalize_compositor.is_valid()) {
		RenderingServer::get_singleton()->scenario_set_compositor(world_3d->get_scenario(), _normalize_compositor);
	}
}

void ProjectedRenderingEngine4D::_cleanup_specific_render_resources() {
	RenderingServer *rendering_server = RenderingServer::get_singleton();
	const Ref<World3D> &world_3d = get_world_3d();
	if (rendering_server != nullptr && world_3d.is_valid()) {
		rendering_server->scenario_set_compositor(world_3d->get_scenario(), RID());
	}
}

void ProjectedRenderingEngine4D::_normalize_image_callback(int64_t p_effect_callback_type, RenderData *p_render_data) {
	RenderingServer *rendering_server = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rendering_server);
	RenderingDevice *rd = rendering_server->get_rendering_device();
	ERR_FAIL_NULL(rd);
	ERR_FAIL_NULL(p_render_data);
	Camera4D *camera = get_camera();
	ERR_FAIL_NULL(camera);
	RenderSceneBuffersRD *buffers = Object::cast_to<RenderSceneBuffersRD>(p_render_data->get_render_scene_buffers().ptr());
	if (buffers) {
		const Vector2i size = buffers->get_internal_size();
		// Matches the shader's local_size of 8x8: round up so a partial workgroup still covers
		// the last few rows or columns. The shader bounds-checks those extra invocations.
		const uint32_t x_groups = (uint32_t(size.x) + 7) / 8;
		const uint32_t y_groups = (uint32_t(size.y) + 7) / 8;
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
			// 3 is transparency. `_cross_section_depth_texture` is set iff this is running as part of the combined renderer.
			push_constant_data[3] = (_cross_section_depth_texture.get_type() != Variant::NIL || project_transparent_background) ? 1 : 0;
		}
		for (uint32_t view = 0; view < buffers->get_view_count(); view++) {
			Ref<RDUniform> uniform;
			uniform.instantiate();
			uniform->set_uniform_type(RenderingDevice::UNIFORM_TYPE_IMAGE);
			uniform->set_binding(0);
#if GDEXTENSION
			uniform->add_id(buffers->get_color_layer(view));
#elif GODOT_MODULE
			uniform->add_id(buffers->get_internal_texture(view));
#endif
			TypedArray<RDUniform> uniforms;
			uniforms.push_back(uniform);
			// UniformSetCacheRD's C++ template get_cache() isn't reachable from GDExtension, so
			// this uses its non-template, TypedArray-based equivalent instead - exposed under a
			// different name in each build.
#if GDEXTENSION
			RID uniform_set = UniformSetCacheRD::get_cache(_normalize_shader, 0, uniforms);
#elif GODOT_MODULE
			RID uniform_set = UniformSetCacheRD::get_cache_array(_normalize_shader, 0, uniforms);
#endif
			const int64_t compute_list = rd->compute_list_begin();
			rd->compute_list_bind_compute_pipeline(compute_list, _normalize_pipeline);
			rd->compute_list_bind_uniform_set(compute_list, uniform_set, 0);
			// compute_list_set_push_constant takes a PackedByteArray in GDExtension, but only a
			// raw pointer is exposed to engine-module C++ (the PackedByteArray-based overload is
			// private there, same split as the shader-compile methods in the constructor).
#if GDEXTENSION
			rd->compute_list_set_push_constant(compute_list, push_constant, push_constant.size());
#elif GODOT_MODULE
			rd->compute_list_set_push_constant(compute_list, push_constant.ptr(), push_constant.size());
#endif
			rd->compute_list_dispatch(compute_list, x_groups, y_groups, 1);
			rd->compute_list_end();
		}
	}
}

void ProjectedRenderingEngine4D::_free_normalize_resources() {
	RenderingServer *rendering_server = RenderingServer::get_singleton();
	if (rendering_server != nullptr) {
		if (_normalize_compositor.is_valid()) {
			rendering_server->free_rid(_normalize_compositor);
		}
		if (_normalize_compositor_effect.is_valid()) {
			rendering_server->free_rid(_normalize_compositor_effect);
		}
		RenderingDevice *rd = rendering_server->get_rendering_device();
		if (rd != nullptr) {
			if (_normalize_pipeline.is_valid()) {
				rd->free_rid(_normalize_pipeline);
			}
			if (_normalize_shader.is_valid()) {
				rd->free_rid(_normalize_shader);
			}
		}
	}
	_normalize_compositor = RID();
	_normalize_compositor_effect = RID();
	_normalize_pipeline = RID();
	_normalize_shader = RID();
}

ProjectedRenderingEngine4D::~ProjectedRenderingEngine4D() {
	_cleanup_specific_render_resources();
	_free_normalize_resources();
}
