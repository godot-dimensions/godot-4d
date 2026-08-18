#include "render_bridge_4d_to_3d.h"

#include "../../nodes/camera_4d.h"
#include "../../nodes/light/directional_light_4d.h"
#include "../rendering_server_4d.h"
#include "world_environment_4d.h"

static constexpr double CLOUD_NOISE_PERIOD = 289.0;
static constexpr double CLOUD_SHAPE_OCTAVE_2_LACUNARITY = 2.03;
static constexpr double CLOUD_SHAPE_OCTAVE_3_LACUNARITY = CLOUD_SHAPE_OCTAVE_2_LACUNARITY * 2.01;

static Vector4 calculate_cloud_wind_offset(const Vector4 &p_wind_velocity, double p_time, double p_noise_scale) {
	const double normalized_time = p_time / MAX(p_noise_scale, 0.0001);
	// Perform each modulo in double precision before constructing the float-backed Vector4.
	// Multiplying a Vector4 by an epoch-sized time before calling Vector4::posmod() would
	// already have discarded the time precision that these compact offsets are meant to retain.
	return Vector4(
			Math::fposmod(double(p_wind_velocity.x) * normalized_time, CLOUD_NOISE_PERIOD),
			Math::fposmod(double(p_wind_velocity.y) * normalized_time, CLOUD_NOISE_PERIOD),
			Math::fposmod(double(p_wind_velocity.z) * normalized_time, CLOUD_NOISE_PERIOD),
			Math::fposmod(double(p_wind_velocity.w) * normalized_time, CLOUD_NOISE_PERIOD));
}

bool EnvironmentRenderBridge4DTo3D::_is_renderer_sky_shader_parameter(const StringName &p_name) {
	const String name = p_name;
	return name == String("world_up_direction_4d") ||
			name == String("cloud_camera_position_4d") ||
			name == String("cloud_camera_basis_4d") ||
			name == String("cloud_evolution_cos_sin") ||
			name == String("cloud_shape_wind_offset_4d") ||
			name == String("cloud_shape_octave_2_wind_offset_4d") ||
			name == String("cloud_shape_octave_3_wind_offset_4d") ||
			name == String("cloud_detail_wind_offset_4d") ||
			name.begins_with("light0_") || name.begins_with("light1_") || name.begins_with("light2_") || name.begins_with("light3_");
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

void EnvironmentRenderBridge4DTo3D::_copy_material_shader_parameters(const Ref<ShaderMaterial> &p_source_material) {
	ERR_FAIL_COND(p_source_material.is_null());
	const Ref<Shader> source_shader = p_source_material->get_shader();
	ERR_FAIL_COND(source_shader.is_null());
#if GDEXTENSION
	const Array shader_uniforms = source_shader->get_shader_uniform_list();
	for (int uniform_index = 0; uniform_index < shader_uniforms.size(); uniform_index++) {
		const Dictionary uniform = shader_uniforms[uniform_index];
		const StringName uniform_name = uniform["name"];
		if (!_is_renderer_sky_shader_parameter(uniform_name)) {
			_set_sky_shader_parameter(uniform_name, p_source_material->get_shader_parameter(uniform_name));
		}
	}
#elif GODOT_MODULE
	List<PropertyInfo> shader_uniforms;
	source_shader->get_shader_uniform_list(&shader_uniforms);
	for (const PropertyInfo &uniform : shader_uniforms) {
		if (!_is_renderer_sky_shader_parameter(uniform.name)) {
			_set_sky_shader_parameter(uniform.name, p_source_material->get_shader_parameter(uniform.name));
		}
	}
#endif
}

bool EnvironmentRenderBridge4DTo3D::_sync_environment_materials(const Ref<SkyMaterial4D> &p_sky_material_source, const Ref<VolumetricCloudMaterial4D> &p_cloud_material_source) {
	ERR_FAIL_COND_V(_sky_material_4d_to_3d.is_null(), false);
	Ref<Shader> combined_shader;
	bool clouds_supported = false;
	if (p_cloud_material_source.is_valid()) {
		combined_shader = p_cloud_material_source->get_shader_for_sky_material(p_sky_material_source);
		clouds_supported = combined_shader.is_valid();
		if (!clouds_supported && p_sky_material_source.is_valid()) {
			WARN_PRINT_ONCE("VolumetricCloudMaterial4D currently supports GradientSkyMaterial4D, PhysicalSkyMaterial4D, PlainSkyMaterial4D, or no base sky. Rendering the custom SkyMaterial4D without clouds.");
		}
	}
	if (combined_shader.is_null() && p_sky_material_source.is_valid()) {
		combined_shader = p_sky_material_source->get_shader();
	}
	if (_sky_material_source_4d != p_sky_material_source || _cloud_material_source_4d != p_cloud_material_source || _sky_material_4d_to_3d->get_shader() != combined_shader) {
		_sky_material_source_4d = p_sky_material_source;
		_cloud_material_source_4d = p_cloud_material_source;
		_sky_shader_parameter_cache.clear();
		_sky_material_4d_to_3d->set_shader(combined_shader);
	}
	if (combined_shader.is_null()) {
		return false;
	}
	if (p_sky_material_source.is_valid()) {
		_copy_material_shader_parameters(p_sky_material_source);
	}
	if (clouds_supported) {
		_copy_material_shader_parameters(p_cloud_material_source);
	}
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
		if (_cross_section_sky_3d->get_process_mode() != Sky::PROCESS_MODE_AUTOMATIC) {
			_cross_section_sky_3d->set_process_mode(Sky::PROCESS_MODE_AUTOMATIC);
		}
		// _set_background_uses_sky() may return early if the previous environment also had no sky,
		// but a missing WorldEnvironment4D must disable the previous environment's ambient color.
		if (_cross_section_environment_3d->get_ambient_source() != Environment::AMBIENT_SOURCE_DISABLED) {
			_cross_section_environment_3d->set_ambient_source(Environment::AMBIENT_SOURCE_DISABLED);
		}
		if (_sky_material_source_4d.is_valid() || _cloud_material_source_4d.is_valid() || _sky_material_4d_to_3d->get_shader().is_valid()) {
			_sky_material_source_4d.unref();
			_cloud_material_source_4d.unref();
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
	const Ref<SkyMaterial4D> sky_material = world_environment_4d->get_sky_material();
	const Ref<VolumetricCloudMaterial4D> cloud_material = world_environment_4d->get_cloud_material();
	const bool clouds_supported = cloud_material.is_valid() && cloud_material->get_shader_for_sky_material(sky_material).is_valid();
	const bool environment_materials_synced = _sync_environment_materials(sky_material, cloud_material);
	_set_background_uses_sky(environment_materials_synced);
	const Sky::ProcessMode sky_process_mode = clouds_supported && cloud_material->is_animated() ? Sky::PROCESS_MODE_REALTIME : Sky::PROCESS_MODE_AUTOMATIC;
	if (_cross_section_sky_3d->get_process_mode() != sky_process_mode) {
		_cross_section_sky_3d->set_process_mode(sky_process_mode);
	}
	if (_cross_section_environment_3d->get_ambient_light_color() != world_environment_4d->get_ambient_light_color()) {
		_cross_section_environment_3d->set_ambient_light_color(world_environment_4d->get_ambient_light_color());
	}
	const float ambient_sky_contribution = world_environment_4d->get_ambient_light_sky_contribution();
	if (_cross_section_environment_3d->get_ambient_light_sky_contribution() != ambient_sky_contribution) {
		_cross_section_environment_3d->set_ambient_light_sky_contribution(ambient_sky_contribution);
	}
	const Environment::AmbientSource ambient_source = environment_materials_synced ? Environment::AMBIENT_SOURCE_SKY : Environment::AMBIENT_SOURCE_COLOR;
	if (_cross_section_environment_3d->get_ambient_source() != ambient_source) {
		_cross_section_environment_3d->set_ambient_source(ambient_source);
	}
	// With no sky material, reproduce the ordinary ambient color/sky blend by treating the missing sky as black.
	// Scaling the energy applies the blend after the ambient color has been converted to linear color space.
	const float ambient_light_energy = environment_materials_synced ? 1.0f : 1.0f - ambient_sky_contribution;
	if (_cross_section_environment_3d->get_ambient_light_energy() != ambient_light_energy) {
		_cross_section_environment_3d->set_ambient_light_energy(ambient_light_energy);
	}
	if (!environment_materials_synced) {
		if (_sky_material_source_4d.is_valid() || _cloud_material_source_4d.is_valid() || _sky_material_4d_to_3d->get_shader().is_valid()) {
			_sky_material_source_4d.unref();
			_cloud_material_source_4d.unref();
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
	if (clouds_supported) {
		const Transform4D camera_relative_to_environment = world_environment_4d->get_global_transform().inverse() * p_camera->get_global_transform();
		_set_sky_shader_parameter("cloud_camera_position_4d", camera_relative_to_environment.origin);
		_set_sky_shader_parameter("cloud_camera_basis_4d", (Projection)camera_relative_to_environment.basis);
		const double cloud_time = cloud_material->is_animated() ? rendering_server_4d->get_render_time() : 0.0;
		const double evolution_angle = Math::fposmod(cloud_time * double(cloud_material->get_evolution_speed()), double(Math_TAU));
		const Vector2 evolution_cos_sin = Vector2((real_t)Math::cos(evolution_angle), (real_t)Math::sin(evolution_angle));
		const Vector4 wind_velocity = cloud_material->get_wind_velocity();
		const double shape_scale = cloud_material->get_shape_scale();
		_set_sky_shader_parameter("cloud_evolution_cos_sin", evolution_cos_sin);
		_set_sky_shader_parameter("cloud_shape_wind_offset_4d", calculate_cloud_wind_offset(wind_velocity, cloud_time, shape_scale));
		_set_sky_shader_parameter("cloud_shape_octave_2_wind_offset_4d", calculate_cloud_wind_offset(wind_velocity, cloud_time, shape_scale / CLOUD_SHAPE_OCTAVE_2_LACUNARITY));
		_set_sky_shader_parameter("cloud_shape_octave_3_wind_offset_4d", calculate_cloud_wind_offset(wind_velocity, cloud_time, shape_scale / CLOUD_SHAPE_OCTAVE_3_LACUNARITY));
		_set_sky_shader_parameter("cloud_detail_wind_offset_4d", calculate_cloud_wind_offset(wind_velocity, cloud_time, cloud_material->get_detail_scale()));
	}
}

void EnvironmentRenderBridge4DTo3D::update_suns(const PackedInt64Array &p_light_object_ids, const TypedArray<Projection> &p_light_relative_basises) {
	if (_sky_material_4d_to_3d.is_null() || _sky_material_4d_to_3d->get_shader().is_null()) {
		return; // Having no sky material is an expected state, so return silently without logging an error.
	}
	for (int64_t light_index = 0; light_index < MAX_SUNS; light_index++) {
		_set_sky_shader_parameter(StringName("light" + itos(light_index) + "_enabled"), false);
	}
	ERR_FAIL_COND(p_light_object_ids.size() != p_light_relative_basises.size());
	int64_t sky_light_index = 0;
	for (int64_t light_index = 0; light_index < p_light_object_ids.size() && sky_light_index < MAX_SUNS; light_index++) {
		const ObjectID light_object_id = (ObjectID)p_light_object_ids[light_index];
		DirectionalLight4D *directional_light_4d = Object::cast_to<DirectionalLight4D>(ObjectDB::get_instance(light_object_id));
		if (directional_light_4d == nullptr) {
			continue;
		}
		Vector4 light_direction_4d = Basis4D(p_light_relative_basises[light_index]).z;
		const real_t direction_length = light_direction_4d.length();
		if (direction_length <= CMP_EPSILON) {
			continue;
		}
		light_direction_4d /= direction_length;
		const String parameter_prefix = "light" + itos(sky_light_index);
		_set_sky_shader_parameter(StringName(parameter_prefix + String("_enabled")), true);
		_set_sky_shader_parameter(StringName(parameter_prefix + String("_direction_4d")), light_direction_4d);
		_set_sky_shader_parameter(StringName(parameter_prefix + String("_color")), directional_light_4d->get_light_color());
		_set_sky_shader_parameter(StringName(parameter_prefix + String("_energy")), directional_light_4d->get_light_energy());
		// This parameter uses the original 4D light's angular radius, not the cross-section 3D light's angular radius.
		_set_sky_shader_parameter(StringName(parameter_prefix + String("_angular_radius_radians")), directional_light_4d->get_angular_radius_radians());
		sky_light_index++;
	}
}

void EnvironmentRenderBridge4DTo3D::cleanup_render_resources() {
	if (_cross_section_world_3d.is_valid()) {
		_cross_section_world_3d->set_environment(Ref<Environment>());
	}
	_cross_section_environment_3d = Ref<Environment>();
	_cross_section_sky_3d = Ref<Sky>();
	_sky_material_source_4d = Ref<SkyMaterial4D>();
	_cloud_material_source_4d = Ref<VolumetricCloudMaterial4D>();
	_sky_material_4d_to_3d = Ref<ShaderMaterial>();
	_sky_shader_parameter_cache.clear();
	_background_uses_sky = false;
	// Explicitly free the World3D so its scenario RID (and any remaining
	// instances inside it) are released while the RenderingServer is alive.
	_cross_section_world_3d = Ref<World3D>();
}
