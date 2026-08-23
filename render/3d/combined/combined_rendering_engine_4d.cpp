#include "combined_rendering_engine_4d.h"

#include "../../../nodes/camera_4d.h"
#include "depth_capture.glsl.gen.h"

#if GDEXTENSION
#include <godot_cpp/classes/canvas_item_material.hpp>
#include <godot_cpp/classes/rd_sampler_state.hpp>
#include <godot_cpp/classes/rd_shader_source.hpp>
#include <godot_cpp/classes/rd_shader_spirv.hpp>
#include <godot_cpp/classes/rd_texture_format.hpp>
#include <godot_cpp/classes/rd_texture_view.hpp>
#include <godot_cpp/classes/rd_uniform.hpp>
#include <godot_cpp/classes/render_scene_buffers_rd.hpp>
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/uniform_set_cache_rd.hpp>
#include <godot_cpp/classes/viewport_texture.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#elif GODOT_MODULE
#include "scene/resources/canvas_item_material.h"
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

void CombinedRenderingEngine4D::set_inner_engines(const Ref<CrossSectionRenderingEngine4D> &p_cross_section_engine, const Ref<ProjectedRenderingEngine4D> &p_projected_engine) {
	_cross_section_engine = p_cross_section_engine;
	_projected_engine = p_projected_engine;
}

CombinedRenderingEngine4D::CombinedRenderingEngine4D() {
	_cross_section_depth_texture.instantiate();

	RenderingServer *rendering_server = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rendering_server);
	RenderingDevice *rd = rendering_server->get_rendering_device();
	if (rd == nullptr) {
		// The Compatibility renderer supports neither compositor effects nor the projected
		// pass this engine depends on, so there is nothing more to set up.
		return;
	}
	// Shader that copies just the depth values out of the cross-section pass's real (combined
	// depth+stencil, non-storage-capable) depth buffer into _depth_capture_output_texture, a
	// plain format Texture2DRD can wrap. See the constructor/callback of ProjectedRenderingEngine4D
	// for why this goes through Object::call() rather than calling these methods directly.
	Ref<RDShaderSource> shader_source;
	shader_source.instantiate();
	shader_source->set_stage_source(RenderingDevice::SHADER_STAGE_COMPUTE, depth_capture_shader_glsl);
	Ref<RDShaderSPIRV> shader_spirv = rd->call("shader_compile_spirv_from_source", shader_source);
	ERR_FAIL_COND(shader_spirv.is_null());
	const String compile_error = shader_spirv->get_stage_compile_error(RenderingDevice::SHADER_STAGE_COMPUTE);
	ERR_FAIL_COND_MSG(!compile_error.is_empty(), "CombinedRenderingEngine4D: Failed to compile depth_capture.glsl: " + compile_error);
	_depth_capture_shader = rd->call("shader_create_from_spirv", shader_spirv);
	_depth_capture_pipeline = rd->compute_pipeline_create(_depth_capture_shader);
#if GDEXTENSION
	Ref<RDSamplerState> sampler_state;
	sampler_state.instantiate();
	_depth_capture_sampler = rd->sampler_create(sampler_state);
#elif GODOT_MODULE
	_depth_capture_sampler = rd->sampler_create(RenderingDevice::SamplerState());
#endif

	_depth_capture_compositor_effect = rendering_server->compositor_effect_create();
	rendering_server->compositor_effect_set_callback(
			_depth_capture_compositor_effect,
			RenderingServer::COMPOSITOR_EFFECT_CALLBACK_TYPE_POST_OPAQUE,
			callable_mp(this, &CombinedRenderingEngine4D::_depth_capture_callback));
	rendering_server->compositor_effect_set_enabled(_depth_capture_compositor_effect, true);
	_depth_capture_compositor = rendering_server->compositor_create();
	TypedArray<RID> compositor_effects;
	compositor_effects.push_back(_depth_capture_compositor_effect);
	rendering_server->compositor_set_compositor_effects(_depth_capture_compositor, compositor_effects);
}

void CombinedRenderingEngine4D::_ensure_helpers_created() {
	Viewport *viewport = get_viewport();

	if (_projected_viewport == nullptr) {
		_projected_viewport = memnew(SubViewport);
		_projected_viewport->set_name("CombinedProjectedSubViewport4D");
		viewport->add_child(_projected_viewport);
	}
	if (_cross_section_viewport == nullptr) {
		_cross_section_viewport = memnew(SubViewport);
		_cross_section_viewport->set_name("CombinedCrossSectionSubViewport4D");
		_projected_viewport->add_child(_cross_section_viewport);
	}
	if (_combine_canvas_layer == nullptr) {
		_combine_canvas_layer = memnew(CanvasLayer);
		viewport->add_child(_combine_canvas_layer);

		TextureRect *cross_section_rect = memnew(TextureRect);
		cross_section_rect->set_texture(_cross_section_viewport->get_texture());
		cross_section_rect->set_anchors_preset(Control::PRESET_FULL_RECT);
		cross_section_rect->set_mouse_filter(Control::MOUSE_FILTER_IGNORE);
		_combine_canvas_layer->add_child(cross_section_rect);

		// Added after (and therefore drawn on top of) the cross-section rect, so the projected
		// pass's normalized result blends over the cross-section result.
		_projected_rect = memnew(TextureRect);
		_projected_rect->set_texture(_projected_viewport->get_texture());
		_projected_rect->set_anchors_preset(Control::PRESET_FULL_RECT);
		_projected_rect->set_mouse_filter(Control::MOUSE_FILTER_IGNORE);
		Ref<CanvasItemMaterial> projected_rect_material;
		projected_rect_material.instantiate();
		projected_rect_material->set_blend_mode(CanvasItemMaterial::BLEND_MODE_PREMULT_ALPHA);
		_projected_rect->set_material(projected_rect_material);
		_combine_canvas_layer->add_child(_projected_rect);
	}
}

void CombinedRenderingEngine4D::_ensure_depth_capture_output_texture(const Size2i &p_size) {
	if (_depth_capture_output_texture.is_valid() && _depth_capture_output_size == p_size) {
		return;
	}
	RenderingServer *rendering_server = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rendering_server);
	RenderingDevice *rd = rendering_server->get_rendering_device();
	ERR_FAIL_NULL(rd);
	const RID previous_output_texture = _depth_capture_output_texture;
#if GDEXTENSION
	Ref<RDTextureFormat> texture_format;
	texture_format.instantiate();
	texture_format->set_format(RenderingDevice::DATA_FORMAT_R32_SFLOAT);
	texture_format->set_width(p_size.x);
	texture_format->set_height(p_size.y);
	texture_format->set_usage_bits(RenderingDevice::TEXTURE_USAGE_SAMPLING_BIT | RenderingDevice::TEXTURE_USAGE_STORAGE_BIT);
	Ref<RDTextureView> texture_view;
	texture_view.instantiate();
	_depth_capture_output_texture = rd->texture_create(texture_format, texture_view);
#elif GODOT_MODULE
	RenderingDevice::TextureFormat texture_format;
	texture_format.format = RenderingDevice::DATA_FORMAT_R32_SFLOAT;
	texture_format.width = p_size.x;
	texture_format.height = p_size.y;
	texture_format.usage_bits = RenderingDevice::TEXTURE_USAGE_SAMPLING_BIT | RenderingDevice::TEXTURE_USAGE_STORAGE_BIT;
	_depth_capture_output_texture = rd->texture_create(texture_format, RenderingDevice::TextureView());
#endif
	_depth_capture_output_size = p_size;
	// _cross_section_depth_texture doesn't wrap _depth_capture_output_texture directly - internally,
	// TextureStorage::texture_rd_initialize() creates a separate "shared" RD texture that views it
	// without taking ownership of it.
	_cross_section_depth_texture->set_texture_rd_rid(_depth_capture_output_texture);
	if (previous_output_texture.is_valid()) {
		// set_texture_rd_rid() only tears down the old shared view immediately if this is already
		// running on the render thread - otherwise it just queues that teardown. sync() makes sure it has
		// actually happened before freeing the texture that view was pointing at.
#if GDEXTENSION
		rendering_server->force_sync();
#elif GODOT_MODULE
		rendering_server->sync();
#endif
		rd->free_rid(previous_output_texture);
	}
}

void CombinedRenderingEngine4D::_sync_viewport_sizes() {
	const Size2i size = Size2i(get_viewport()->get_visible_rect().size);
	_projected_viewport->set_size(size);
	_cross_section_viewport->set_size(size);
	// This has to be called before the depth capture callback, otherwise the projection
	// material will have a stale reference to the previously-sized texture, because of the order
	// these things are updated in.
	_ensure_depth_capture_output_texture(size);
}

void CombinedRenderingEngine4D::setup_for_viewport() {
	ERR_FAIL_NULL(RenderingServer::get_singleton());
	ERR_FAIL_NULL(get_viewport());
	ERR_FAIL_COND_MSG(_cross_section_engine.is_null() || _projected_engine.is_null(), "CombinedRenderingEngine4D: set_inner_engines() was never called.");
	_ensure_helpers_created();
	_sync_viewport_sizes();

	_cross_section_engine->setup_for_viewport_if_needed(_cross_section_viewport);
	_projected_engine->setup_for_viewport_if_needed(_projected_viewport);

	if (_depth_capture_compositor.is_valid()) {
		RenderingServer::get_singleton()->scenario_set_compositor(_cross_section_viewport->get_world_3d()->get_scenario(), _depth_capture_compositor);
	}
	_projected_engine->set_cross_section_depth_texture(_cross_section_depth_texture);

	_combine_canvas_layer->show();
	_projected_viewport->set_update_mode(SubViewport::UPDATE_ALWAYS);
	_cross_section_viewport->set_update_mode(SubViewport::UPDATE_ALWAYS);
}

void CombinedRenderingEngine4D::cleanup_for_viewport() {
	// Stop the sub-viewports from continuing to render every frame (and hide the composited
	// result) while some other engine is active, without tearing down and recreating them.
	if (_combine_canvas_layer != nullptr) {
		_combine_canvas_layer->hide();
	}
	if (_projected_viewport != nullptr) {
		_projected_viewport->set_update_mode(SubViewport::UPDATE_DISABLED);
	}
	if (_cross_section_viewport != nullptr) {
		_cross_section_viewport->set_update_mode(SubViewport::UPDATE_DISABLED);
	}
	if (_cross_section_engine.is_valid() && _cross_section_viewport != nullptr) {
		_cross_section_engine->cleanup_for_viewport_if_needed(_cross_section_viewport);
	}
	if (_projected_engine.is_valid() && _projected_viewport != nullptr) {
		_projected_engine->cleanup_for_viewport_if_needed(_projected_viewport);
	}
	if (_projected_engine.is_valid()) {
		// Clear it so the projected engine, if used alone, doesn't refer to the stale texture.
		_projected_engine->set_cross_section_depth_texture(Variant());
	}
}

void CombinedRenderingEngine4D::_render_frame_callback() {
	ERR_FAIL_NULL(get_camera());
	ERR_FAIL_NULL(get_viewport());
	ERR_FAIL_COND(_cross_section_engine.is_null() || _projected_engine.is_null());
	_sync_viewport_sizes();

	Camera4D *camera = get_camera();
	_projected_rect->set_modulate(Color(1.0, 1.0, 1.0, camera->get_projection_opacity()));
	const PackedInt64Array light_object_ids = get_light_object_ids();
	const PackedInt64Array mesh_instance_object_ids = get_mesh_instance_object_ids();

	// The relative transforms are derived from the camera and the object IDs rather than forwarded,
	// since each inner engine computes and caches its own copy in calculate_relative_transforms().
	_cross_section_engine->set_camera(camera);
	_cross_section_engine->set_light_object_ids(light_object_ids);
	_cross_section_engine->set_mesh_instance_object_ids(mesh_instance_object_ids);
	_cross_section_engine->calculate_relative_transforms();
	_cross_section_engine->render_frame();

	_projected_engine->set_camera(camera);
	_projected_engine->set_light_object_ids(light_object_ids);
	_projected_engine->set_mesh_instance_object_ids(mesh_instance_object_ids);
	_projected_engine->calculate_relative_transforms();
	_projected_engine->render_frame();
}

void CombinedRenderingEngine4D::_depth_capture_callback(int64_t p_effect_callback_type, RenderData *p_render_data) {
	RenderingServer *rendering_server = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rendering_server);
	RenderingDevice *rd = rendering_server->get_rendering_device();
	ERR_FAIL_NULL(rd);
	ERR_FAIL_NULL(p_render_data);
	RenderSceneBuffersRD *buffers = Object::cast_to<RenderSceneBuffersRD>(p_render_data->get_render_scene_buffers().ptr());
	if (buffers == nullptr) {
		return;
	}
	const Size2i size = buffers->get_internal_size();
	// Matches the shader's local_size of 8x8: round up so a partial workgroup still covers
	// the last few rows or columns. The shader bounds-checks those extra invocations.
	const uint32_t x_groups = (uint32_t(size.x) + 7) / 8;
	const uint32_t y_groups = (uint32_t(size.y) + 7) / 8;

	Ref<RDUniform> depth_input_uniform;
	depth_input_uniform.instantiate();
	depth_input_uniform->set_uniform_type(RenderingDevice::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE);
	depth_input_uniform->set_binding(0);
	depth_input_uniform->add_id(_depth_capture_sampler);
	depth_input_uniform->add_id(buffers->get_depth_texture());

	Ref<RDUniform> depth_output_uniform;
	depth_output_uniform.instantiate();
	depth_output_uniform->set_uniform_type(RenderingDevice::UNIFORM_TYPE_IMAGE);
	depth_output_uniform->set_binding(1);
	depth_output_uniform->add_id(_depth_capture_output_texture);

	TypedArray<RDUniform> uniforms;
	uniforms.push_back(depth_input_uniform);
	uniforms.push_back(depth_output_uniform);
	// UniformSetCacheRD's C++ template get_cache() isn't reachable from GDExtension, so this uses
	// its non-template, TypedArray-based equivalent instead - exposed under a different name in
	// each build (see normalize_image_callback in ProjectedRenderingEngine4D for the same split).
#if GDEXTENSION
	RID uniform_set = UniformSetCacheRD::get_cache(_depth_capture_shader, 0, uniforms);
#elif GODOT_MODULE
	RID uniform_set = UniformSetCacheRD::get_cache_array(_depth_capture_shader, 0, uniforms);
#endif

	PackedByteArray push_constant;
	push_constant.resize(16);
	{
		int32_t *push_constant_data = reinterpret_cast<int32_t *>(push_constant.ptrw());
		push_constant_data[0] = size.x;
		push_constant_data[1] = size.y;
		push_constant_data[2] = 0;
		push_constant_data[3] = 0;
	}

	const int64_t compute_list = rd->compute_list_begin();
	rd->compute_list_bind_compute_pipeline(compute_list, _depth_capture_pipeline);
	rd->compute_list_bind_uniform_set(compute_list, uniform_set, 0);
#if GDEXTENSION
	rd->compute_list_set_push_constant(compute_list, push_constant, push_constant.size());
#elif GODOT_MODULE
	rd->compute_list_set_push_constant(compute_list, push_constant.ptr(), push_constant.size());
#endif
	rd->compute_list_dispatch(compute_list, x_groups, y_groups, 1);
	rd->compute_list_end();
}

void CombinedRenderingEngine4D::_cleanup_render_resources() {
	RenderingServer *rendering_server = RenderingServer::get_singleton();
	if (rendering_server != nullptr && _depth_capture_compositor.is_valid()) {
		rendering_server->free_rid(_depth_capture_compositor);
		_depth_capture_compositor = RID();
		rendering_server->free_rid(_depth_capture_compositor_effect);
		_depth_capture_compositor_effect = RID();
	}
	RenderingDevice *rd = rendering_server == nullptr ? nullptr : rendering_server->get_rendering_device();
	if (rd == nullptr) {
		return;
	}
	if (_cross_section_depth_texture.is_valid()) {
		// Tear down the non-owning "shared" RD texture _cross_section_depth_texture wraps
		// _depth_capture_output_texture with (see _ensure_depth_capture_output_texture()) before
		// freeing the texture it views below - otherwise the view is left dangling on a texture
		// that's already gone, which leaks both RIDs rather than freeing either cleanly. This is
		// called from module shutdown, not the render thread, so set_texture_rd_rid()'s free is only
		// queued rather than applied immediately - sync() makes sure it has actually run first.
		_cross_section_depth_texture->set_texture_rd_rid(RID());
#if GDEXTENSION
		rendering_server->force_sync();
#elif GODOT_MODULE
		rendering_server->sync();
#endif
	}
	if (_depth_capture_output_texture.is_valid()) {
		rd->free_rid(_depth_capture_output_texture);
		_depth_capture_output_texture = RID();
	}
	if (_depth_capture_sampler.is_valid()) {
		rd->free_rid(_depth_capture_sampler);
		_depth_capture_sampler = RID();
	}
	if (_depth_capture_pipeline.is_valid()) {
		rd->free_rid(_depth_capture_pipeline);
		_depth_capture_pipeline = RID();
	}
	if (_depth_capture_shader.is_valid()) {
		rd->free_rid(_depth_capture_shader);
		_depth_capture_shader = RID();
	}
}

CombinedRenderingEngine4D::~CombinedRenderingEngine4D() {
	_cleanup_render_resources();
}
