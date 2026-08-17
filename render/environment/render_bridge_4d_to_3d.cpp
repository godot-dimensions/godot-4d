#include "render_bridge_4d_to_3d.h"

#include "../../nodes/camera_4d.h"
#include "../rendering_server_4d.h"
#include "world_environment_4d.h"

bool EnvironmentRenderBridge4DTo3D::_is_renderer_sky_shader_parameter(const StringName &p_name) {
	const String name = p_name;
	return name == String("world_up_direction_4d") || name.begins_with("light0_") || name.begins_with("light1_") || name.begins_with("light2_") || name.begins_with("light3_");
}

void EnvironmentRenderBridge4DTo3D::_set_background_uses_sky(const bool p_enabled, const bool p_force_update) {
	ERR_FAIL_COND(!_cross_section_environment_3d.is_valid());
	if (!p_force_update && _background_uses_sky == p_enabled) {
		return;
	}
	_background_uses_sky = p_enabled;
	if (p_enabled) {
		_cross_section_environment_3d->set_background(Environment::BG_SKY);
		_cross_section_environment_3d->set_sky(_cross_section_sky_3d);
		_cross_section_environment_3d->set_ambient_source(Environment::AMBIENT_SOURCE_SKY);
		_cross_section_environment_3d->set_ambient_light_energy(1.0f);
		_cross_section_environment_3d->set_ambient_light_sky_contribution(1.0f);
		_cross_section_environment_3d->set_reflection_source(Environment::REFLECTION_SOURCE_SKY);
	} else {
		_cross_section_environment_3d->set_background(Environment::BG_COLOR);
		_cross_section_environment_3d->set_bg_color(Color(0.0f, 0.0f, 0.0f));
		_cross_section_environment_3d->set_ambient_source(Environment::AMBIENT_SOURCE_DISABLED);
		_cross_section_environment_3d->set_reflection_source(Environment::REFLECTION_SOURCE_DISABLED);
	}
}

void EnvironmentRenderBridge4DTo3D::_set_sky_shader_parameter(const StringName &p_name, const Variant &p_value) {
	ERR_FAIL_COND(!_sky_material_4d_to_3d.is_valid());
	const Variant *cached_value = _sky_shader_parameter_cache.getptr(p_name);
	if (cached_value != nullptr && *cached_value == p_value) {
		return;
	}
	_sky_shader_parameter_cache[p_name] = p_value;
	_sky_material_4d_to_3d->set_shader_parameter(p_name, p_value);
}

bool EnvironmentRenderBridge4DTo3D::_sync_sky_material(const Ref<SkyMaterial4D> &p_sky_material_source) {
	ERR_FAIL_COND_V(p_sky_material_source.is_null(), false);
	ERR_FAIL_COND_V(_sky_material_4d_to_3d.is_null(), false);
	const Ref<Shader> source_shader = p_sky_material_source->get_shader();
	if (_sky_material_source_4d != p_sky_material_source || _sky_material_4d_to_3d->get_shader() != source_shader) {
		_sky_material_source_4d = p_sky_material_source;
		_sky_shader_parameter_cache.clear();
		_sky_material_4d_to_3d->set_shader(source_shader);
	}
	if (source_shader.is_null()) {
		return false;
	}
#if GDEXTENSION
	const Array shader_uniforms = source_shader->get_shader_uniform_list();
	for (int uniform_index = 0; uniform_index < shader_uniforms.size(); uniform_index++) {
		const Dictionary uniform = shader_uniforms[uniform_index];
		const StringName uniform_name = uniform["name"];
		if (!_is_renderer_sky_shader_parameter(uniform_name)) {
			_set_sky_shader_parameter(uniform_name, p_sky_material_source->get_shader_parameter(uniform_name));
		}
	}
#elif GODOT_MODULE
	List<PropertyInfo> shader_uniforms;
	source_shader->get_shader_uniform_list(&shader_uniforms);
	for (const PropertyInfo &uniform : shader_uniforms) {
		if (!_is_renderer_sky_shader_parameter(uniform.name)) {
			_set_sky_shader_parameter(uniform.name, p_sky_material_source->get_shader_parameter(uniform.name));
		}
	}
#endif
	return true;
}

void EnvironmentRenderBridge4DTo3D::setup_environment_resources(const Ref<World3D> &p_cross_section_world_3d) {
	ERR_FAIL_COND_MSG(p_cross_section_world_3d.is_null(), "Cannot setup environment resources with a null World3D.");
	_cross_section_world_3d = p_cross_section_world_3d;
	if (_cross_section_environment_3d.is_null()) {
		_cross_section_sky_3d.instantiate();
		_sky_material_4d_to_3d.instantiate();
		_cross_section_sky_3d->set_material(_sky_material_4d_to_3d);
		_cross_section_sky_3d->set_process_mode(Sky::PROCESS_MODE_AUTOMATIC);
		_cross_section_environment_3d.instantiate();
	}
	_cross_section_world_3d->set_environment(_cross_section_environment_3d);
	_set_background_uses_sky(false, true); // Force update on setup.
}

void EnvironmentRenderBridge4DTo3D::update_environment(Camera4D *p_camera) {
	ERR_FAIL_NULL(p_camera);
	ERR_FAIL_COND_MSG(_cross_section_world_3d.is_null(), "EnvironmentRenderBridge4DTo3D: The setup function must be called with a valid World3D before updating the environment.");
	ERR_FAIL_COND_MSG(_cross_section_environment_3d.is_null(), "EnvironmentRenderBridge4DTo3D: The setup function must be called with a valid World3D before updating the environment.");
	RenderingServer4D *rendering_server_4d = RenderingServer4D::get_singleton();
	ERR_FAIL_NULL(rendering_server_4d);
	WorldEnvironment4D *world_environment_4d = rendering_server_4d->get_current_world_environment_for_camera(p_camera);
	if (world_environment_4d == nullptr) {
		_set_background_uses_sky(false);
		// _set_background_uses_sky() may return early if the previous environment also had no sky,
		// but a missing WorldEnvironment4D must disable the previous environment's ambient color.
		if (_cross_section_environment_3d->get_ambient_source() != Environment::AMBIENT_SOURCE_DISABLED) {
			_cross_section_environment_3d->set_ambient_source(Environment::AMBIENT_SOURCE_DISABLED);
		}
		if (_sky_material_source_4d.is_valid()) {
			_sky_material_source_4d.unref();
			_sky_shader_parameter_cache.clear();
			_sky_material_4d_to_3d->set_shader(Ref<Shader>());
		}
		if (_cross_section_environment_3d->get_tonemapper() != Environment::TONE_MAPPER_LINEAR) {
			_cross_section_environment_3d->set_tonemapper(Environment::TONE_MAPPER_LINEAR);
		}
		if (_cross_section_environment_3d->get_tonemap_exposure() != 1.0f) {
			_cross_section_environment_3d->set_tonemap_exposure(1.0f);
		}
		if (_cross_section_environment_3d->get_tonemap_white() != 1.0f) {
			_cross_section_environment_3d->set_tonemap_white(1.0f);
		}
		return;
	}
	const Environment::ToneMapper tone_mapper = static_cast<Environment::ToneMapper>(world_environment_4d->get_tonemapper());
	if (_cross_section_environment_3d->get_tonemapper() != tone_mapper) {
		_cross_section_environment_3d->set_tonemapper(tone_mapper);
	}
	if (_cross_section_environment_3d->get_tonemap_exposure() != world_environment_4d->get_tonemap_exposure()) {
		_cross_section_environment_3d->set_tonemap_exposure(world_environment_4d->get_tonemap_exposure());
	}
	if (_cross_section_environment_3d->get_tonemap_white() != world_environment_4d->get_tonemap_white()) {
		_cross_section_environment_3d->set_tonemap_white(world_environment_4d->get_tonemap_white());
	}
	Ref<SkyMaterial4D> sky_material = world_environment_4d->get_sky_material();
	bool sky_material_synced = false;
	if (sky_material.is_valid()) {
		sky_material_synced = _sync_sky_material(sky_material);
	}
	_set_background_uses_sky(sky_material_synced);
	if (_cross_section_environment_3d->get_ambient_light_color() != world_environment_4d->get_ambient_light_color()) {
		_cross_section_environment_3d->set_ambient_light_color(world_environment_4d->get_ambient_light_color());
	}
	const float ambient_sky_contribution = world_environment_4d->get_ambient_light_sky_contribution();
	if (_cross_section_environment_3d->get_ambient_light_sky_contribution() != ambient_sky_contribution) {
		_cross_section_environment_3d->set_ambient_light_sky_contribution(ambient_sky_contribution);
	}
	const Environment::AmbientSource ambient_source = sky_material_synced ? Environment::AMBIENT_SOURCE_SKY : Environment::AMBIENT_SOURCE_COLOR;
	if (_cross_section_environment_3d->get_ambient_source() != ambient_source) {
		_cross_section_environment_3d->set_ambient_source(ambient_source);
	}
	// With no sky material, reproduce the ordinary ambient color/sky blend by treating the missing sky as black.
	// Scaling the energy applies the blend after the ambient color has been converted to linear color space.
	const float ambient_light_energy = sky_material_synced ? 1.0f : 1.0f - ambient_sky_contribution;
	if (_cross_section_environment_3d->get_ambient_light_energy() != ambient_light_energy) {
		_cross_section_environment_3d->set_ambient_light_energy(ambient_light_energy);
	}
	if (!sky_material_synced) {
		if (_sky_material_source_4d.is_valid() || _sky_material_4d_to_3d->get_shader().is_valid()) {
			_sky_material_source_4d.unref();
			_sky_shader_parameter_cache.clear();
			_sky_material_4d_to_3d->set_shader(Ref<Shader>());
		}
		return;
	}
	const Transform4D environment_relative_transform = p_camera->get_global_transform().inverse() * world_environment_4d->get_global_transform();
	Vector4 world_up_direction_4d = environment_relative_transform.basis.y;
	const real_t world_up_length = world_up_direction_4d.length();
	if (world_up_length > 0.0f) {
		world_up_direction_4d /= world_up_length;
	} else {
		world_up_direction_4d = Vector4(0.0f, 1.0f, 0.0f, 0.0f);
	}
	_set_sky_shader_parameter("world_up_direction_4d", world_up_direction_4d);
}

void EnvironmentRenderBridge4DTo3D::cleanup_render_resources() {
	if (_cross_section_world_3d.is_valid()) {
		_cross_section_world_3d->set_environment(Ref<Environment>());
	}
	_cross_section_environment_3d = Ref<Environment>();
	_cross_section_sky_3d = Ref<Sky>();
	_sky_material_source_4d = Ref<SkyMaterial4D>();
	_sky_material_4d_to_3d = Ref<ShaderMaterial>();
	_sky_shader_parameter_cache.clear();
	_background_uses_sky = false;
	// Explicitly free the World3D so its scenario RID (and any remaining
	// instances inside it) are released while the RenderingServer is alive.
	_cross_section_world_3d = Ref<World3D>();
}
