#include "volumetric_cloud_material_4d.h"

#include "../sky/gradient_sky_material_4d.h"
#include "../sky/physical_sky_material_4d.h"
#include "../sky/plain_sky_material_4d.h"
#include "gradient_sky_with_volumetric_clouds_full_res_shader.glsl.gen.h"
#include "gradient_sky_with_volumetric_clouds_shader.glsl.gen.h"
#include "physical_sky_with_volumetric_clouds_full_res_shader.glsl.gen.h"
#include "physical_sky_with_volumetric_clouds_shader.glsl.gen.h"
#include "plain_sky_with_volumetric_clouds_full_res_shader.glsl.gen.h"
#include "plain_sky_with_volumetric_clouds_shader.glsl.gen.h"
#include "volumetric_cloud_full_res_shader.glsl.gen.h"
#include "volumetric_cloud_shader.glsl.gen.h"

#if GDEXTENSION
#include <godot_cpp/classes/rendering_server.hpp>
#elif GODOT_MODULE
#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR < 6
#include "servers/rendering_server.h"
#else
#include "servers/rendering/rendering_server.h"
#endif
#endif

Ref<Shader> VolumetricCloudMaterial4D::_cloud_only_shader;
Ref<Shader> VolumetricCloudMaterial4D::_gradient_sky_shader;
Ref<Shader> VolumetricCloudMaterial4D::_physical_sky_shader;
Ref<Shader> VolumetricCloudMaterial4D::_plain_sky_shader;
Ref<Shader> VolumetricCloudMaterial4D::_cloud_only_full_res_shader;
Ref<Shader> VolumetricCloudMaterial4D::_gradient_sky_full_res_shader;
Ref<Shader> VolumetricCloudMaterial4D::_physical_sky_full_res_shader;
Ref<Shader> VolumetricCloudMaterial4D::_plain_sky_full_res_shader;

static void initialize_cloud_shader(Ref<Shader> &r_shader, const String &p_name, const char *p_code) {
	r_shader.instantiate();
	r_shader->set_name(p_name);
	r_shader->set_code(p_code);
	if (RenderingServer::get_singleton() != nullptr) {
		RenderingServer::get_singleton()->shader_set_path_hint(r_shader->get_rid(), p_name);
	}
}

void VolumetricCloudMaterial4D::set_albedo_color(const Color &p_albedo_color) {
	_albedo_color = p_albedo_color;
	set_shader_parameter("cloud_albedo_color", _albedo_color);
}

void VolumetricCloudMaterial4D::set_ambient_color(const Color &p_ambient_color) {
	_ambient_color = p_ambient_color;
	set_shader_parameter("cloud_ambient_color", _ambient_color);
}

void VolumetricCloudMaterial4D::set_ambient_intensity(const float p_ambient_intensity) {
	_ambient_intensity = MAX(p_ambient_intensity, 0.0f);
	set_shader_parameter("cloud_ambient_intensity", _ambient_intensity);
}

void VolumetricCloudMaterial4D::set_sun_intensity(const float p_sun_intensity) {
	_sun_intensity = MAX(p_sun_intensity, 0.0f);
	set_shader_parameter("cloud_sun_intensity", _sun_intensity);
}

void VolumetricCloudMaterial4D::set_phase_anisotropy(const float p_phase_anisotropy) {
	_phase_anisotropy = CLAMP(p_phase_anisotropy, -0.95f, 0.95f);
	set_shader_parameter("cloud_phase_anisotropy", _phase_anisotropy);
}

void VolumetricCloudMaterial4D::set_bottom_height(const float p_bottom_height) {
	_bottom_height = p_bottom_height;
	set_shader_parameter("cloud_bottom_height", _bottom_height);
}

void VolumetricCloudMaterial4D::set_vertical_thickness(const float p_vertical_thickness) {
	_vertical_thickness = MAX(p_vertical_thickness, 0.001f);
	set_shader_parameter("cloud_vertical_thickness", _vertical_thickness);
}

void VolumetricCloudMaterial4D::set_shape_scale(const float p_shape_scale) {
	_shape_scale = MAX(p_shape_scale, 0.001f);
	set_shader_parameter("cloud_shape_scale", _shape_scale);
}

void VolumetricCloudMaterial4D::set_shape_fractal_strength(const float p_shape_fractal_strength) {
	_shape_fractal_strength = CLAMP(p_shape_fractal_strength, 0.0f, 1.0f);
	set_shader_parameter("cloud_shape_fractal_strength", _shape_fractal_strength);
}

void VolumetricCloudMaterial4D::set_detail_scale(const float p_detail_scale) {
	_detail_scale = MAX(p_detail_scale, 0.001f);
	set_shader_parameter("cloud_detail_scale", _detail_scale);
}

void VolumetricCloudMaterial4D::set_detail_strength(const float p_detail_strength) {
	_detail_strength = CLAMP(p_detail_strength, 0.0f, 1.0f);
	set_shader_parameter("cloud_detail_strength", _detail_strength);
}

void VolumetricCloudMaterial4D::set_coverage(const float p_coverage) {
	_coverage = CLAMP(p_coverage, 0.0f, 1.0f);
	set_shader_parameter("cloud_coverage", _coverage);
}

void VolumetricCloudMaterial4D::set_density(const float p_density) {
	_density = MAX(p_density, 0.0f);
	set_shader_parameter("cloud_density", _density);
}

void VolumetricCloudMaterial4D::set_wind_velocity(const Vector4 &p_wind_velocity) {
	_wind_velocity = p_wind_velocity;
}

void VolumetricCloudMaterial4D::set_evolution_speed(const float p_evolution_speed) {
	_evolution_speed = p_evolution_speed;
}

void VolumetricCloudMaterial4D::set_extinction_coefficient(const float p_extinction_coefficient) {
	_extinction_coefficient = MAX(p_extinction_coefficient, 0.0f);
	set_shader_parameter("cloud_extinction_coefficient", _extinction_coefficient);
}

void VolumetricCloudMaterial4D::set_shadow_distance(const float p_shadow_distance) {
	_shadow_distance = MAX(p_shadow_distance, 0.0f);
	set_shader_parameter("cloud_shadow_distance", _shadow_distance);
}

void VolumetricCloudMaterial4D::set_shadow_strength(const float p_shadow_strength) {
	_shadow_strength = MAX(p_shadow_strength, 0.0f);
	set_shader_parameter("cloud_shadow_strength", _shadow_strength);
}

void VolumetricCloudMaterial4D::set_maximum_ray_distance(const float p_maximum_ray_distance) {
	_maximum_ray_distance = MAX(p_maximum_ray_distance, 0.001f);
	set_shader_parameter("cloud_maximum_ray_distance", _maximum_ray_distance);
}

void VolumetricCloudMaterial4D::set_distance_fade_start(const float p_distance_fade_start) {
	_distance_fade_start = CLAMP(p_distance_fade_start, 0.0f, 0.99f);
	set_shader_parameter("cloud_distance_fade_start", _distance_fade_start);
}

void VolumetricCloudMaterial4D::set_distance_fade_curve(const real_t p_distance_fade_curve) {
	_distance_fade_curve = p_distance_fade_curve;
	set_shader_parameter("cloud_inv_distance_fade_curve", 0.6f / MAX(_distance_fade_curve, (real_t)CMP_EPSILON));
}

void VolumetricCloudMaterial4D::set_sampling_method(const SamplingMethod p_sampling_method) {
	ERR_FAIL_INDEX(p_sampling_method, SAMPLING_METHOD_MAX);
	_sampling_method = p_sampling_method;
	set_shader_parameter("cloud_sampling_method", (int32_t)_sampling_method);
}

void VolumetricCloudMaterial4D::set_sampling_steps(const int32_t p_sampling_steps) {
	_sampling_steps = CLAMP(p_sampling_steps, 4, 128);
	set_shader_parameter("cloud_sampling_steps", _sampling_steps);
}

void VolumetricCloudMaterial4D::set_affect_radiance(const bool p_affect_radiance) {
	_affect_radiance = p_affect_radiance;
	set_shader_parameter("cloud_affect_radiance", _affect_radiance);
}

bool VolumetricCloudMaterial4D::is_animated() const {
	return _animation_enabled && (!_wind_velocity.is_zero_approx() || !Math::is_zero_approx(_evolution_speed));
}

Ref<Shader> VolumetricCloudMaterial4D::get_shader_for_sky_material(const Ref<SkyMaterial4D> &p_sky_material) const {
	const RenderingServer *rendering_server = RenderingServer::get_singleton();
	// Compatibility has no RenderingDevice and, in older supported Godot versions, its sky
	// subpasses do not compile reliably. Use a full-resolution fallback without engine changes.
	const bool use_full_resolution = rendering_server != nullptr && rendering_server->get_rendering_device() == nullptr;
	if (p_sky_material.is_null()) {
		return use_full_resolution ? _cloud_only_full_res_shader : _cloud_only_shader;
	}
	Ref<GradientSkyMaterial4D> gradient_sky = p_sky_material;
	if (gradient_sky.is_valid()) {
		return use_full_resolution ? _gradient_sky_full_res_shader : _gradient_sky_shader;
	}
	Ref<PhysicalSkyMaterial4D> physical_sky = p_sky_material;
	if (physical_sky.is_valid()) {
		return use_full_resolution ? _physical_sky_full_res_shader : _physical_sky_shader;
	}
	Ref<PlainSkyMaterial4D> plain_sky = p_sky_material;
	if (plain_sky.is_valid()) {
		return use_full_resolution ? _plain_sky_full_res_shader : _plain_sky_shader;
	}
	return Ref<Shader>();
}

void VolumetricCloudMaterial4D::_validate_property(PropertyInfo &p_property) const {
	const String property_name = p_property.name;
	if (property_name == String("shader") || property_name.begins_with("shader_parameter/")) {
		p_property.usage = PROPERTY_USAGE_NONE;
	}
}

void VolumetricCloudMaterial4D::_bind_methods() {
	ADD_GROUP("Quality", "");
	ClassDB::bind_method(D_METHOD("get_sampling_method"), &VolumetricCloudMaterial4D::get_sampling_method);
	ClassDB::bind_method(D_METHOD("set_sampling_method", "method"), &VolumetricCloudMaterial4D::set_sampling_method);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "sampling_method", PROPERTY_HINT_ENUM, "World Grid,Jittered Raymarch"), "set_sampling_method", "get_sampling_method");
	ClassDB::bind_method(D_METHOD("get_sampling_steps"), &VolumetricCloudMaterial4D::get_sampling_steps);
	ClassDB::bind_method(D_METHOD("set_sampling_steps", "steps"), &VolumetricCloudMaterial4D::set_sampling_steps);
	// Since sampling steps are very costly, limit the property hint's maximum to 64, but allow setting up to 128 steps via code.
	ADD_PROPERTY(PropertyInfo(Variant::INT, "sampling_steps", PROPERTY_HINT_RANGE, "4,64,1"), "set_sampling_steps", "get_sampling_steps");

	ADD_GROUP("Size", "");
	ClassDB::bind_method(D_METHOD("get_bottom_height"), &VolumetricCloudMaterial4D::get_bottom_height);
	ClassDB::bind_method(D_METHOD("set_bottom_height", "height"), &VolumetricCloudMaterial4D::set_bottom_height);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "bottom_height", PROPERTY_HINT_RANGE, "-20000,20000,1,or_greater,or_less,suffix:m"), "set_bottom_height", "get_bottom_height");
	ClassDB::bind_method(D_METHOD("get_vertical_thickness"), &VolumetricCloudMaterial4D::get_vertical_thickness);
	ClassDB::bind_method(D_METHOD("set_vertical_thickness", "thickness"), &VolumetricCloudMaterial4D::set_vertical_thickness);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "vertical_thickness", PROPERTY_HINT_RANGE, "10,10000,1,exp,or_greater,suffix:m"), "set_vertical_thickness", "get_vertical_thickness");
	ClassDB::bind_method(D_METHOD("get_maximum_ray_distance"), &VolumetricCloudMaterial4D::get_maximum_ray_distance);
	ClassDB::bind_method(D_METHOD("set_maximum_ray_distance", "distance"), &VolumetricCloudMaterial4D::set_maximum_ray_distance);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "maximum_ray_distance", PROPERTY_HINT_RANGE, "1000,500000,1,exp,or_greater,or_less,suffix:m"), "set_maximum_ray_distance", "get_maximum_ray_distance");
	ClassDB::bind_method(D_METHOD("get_distance_fade_curve"), &VolumetricCloudMaterial4D::get_distance_fade_curve);
	ClassDB::bind_method(D_METHOD("set_distance_fade_curve", "curve"), &VolumetricCloudMaterial4D::set_distance_fade_curve);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "distance_fade_curve", PROPERTY_HINT_EXP_EASING), "set_distance_fade_curve", "get_distance_fade_curve");
	ClassDB::bind_method(D_METHOD("get_distance_fade_start"), &VolumetricCloudMaterial4D::get_distance_fade_start);
	ClassDB::bind_method(D_METHOD("set_distance_fade_start", "start"), &VolumetricCloudMaterial4D::set_distance_fade_start);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "distance_fade_start", PROPERTY_HINT_RANGE, "0,0.99,0.001"), "set_distance_fade_start", "get_distance_fade_start");

	ADD_GROUP("Shape", "");
	ClassDB::bind_method(D_METHOD("get_coverage"), &VolumetricCloudMaterial4D::get_coverage);
	ClassDB::bind_method(D_METHOD("set_coverage", "coverage"), &VolumetricCloudMaterial4D::set_coverage);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "coverage", PROPERTY_HINT_RANGE, "0,1,0.001"), "set_coverage", "get_coverage");
	ClassDB::bind_method(D_METHOD("get_density"), &VolumetricCloudMaterial4D::get_density);
	ClassDB::bind_method(D_METHOD("set_density", "density"), &VolumetricCloudMaterial4D::set_density);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "density", PROPERTY_HINT_RANGE, "0,8,0.001,or_greater"), "set_density", "get_density");
	ClassDB::bind_method(D_METHOD("get_shape_scale"), &VolumetricCloudMaterial4D::get_shape_scale);
	ClassDB::bind_method(D_METHOD("set_shape_scale", "scale"), &VolumetricCloudMaterial4D::set_shape_scale);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "shape_scale", PROPERTY_HINT_RANGE, "100,100000,1,exp,or_greater,or_less,suffix:m"), "set_shape_scale", "get_shape_scale");
	ClassDB::bind_method(D_METHOD("get_shape_fractal_strength"), &VolumetricCloudMaterial4D::get_shape_fractal_strength);
	ClassDB::bind_method(D_METHOD("set_shape_fractal_strength", "strength"), &VolumetricCloudMaterial4D::set_shape_fractal_strength);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "shape_fractal_strength", PROPERTY_HINT_RANGE, "0,1,0.001"), "set_shape_fractal_strength", "get_shape_fractal_strength");
	ClassDB::bind_method(D_METHOD("get_detail_scale"), &VolumetricCloudMaterial4D::get_detail_scale);
	ClassDB::bind_method(D_METHOD("set_detail_scale", "scale"), &VolumetricCloudMaterial4D::set_detail_scale);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "detail_scale", PROPERTY_HINT_RANGE, "10,100000,0.1,exp,or_greater,or_less,suffix:m"), "set_detail_scale", "get_detail_scale");
	ClassDB::bind_method(D_METHOD("get_detail_strength"), &VolumetricCloudMaterial4D::get_detail_strength);
	ClassDB::bind_method(D_METHOD("set_detail_strength", "strength"), &VolumetricCloudMaterial4D::set_detail_strength);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "detail_strength", PROPERTY_HINT_RANGE, "0,1,0.001"), "set_detail_strength", "get_detail_strength");

	ADD_GROUP("Animation", "");
	ClassDB::bind_method(D_METHOD("is_animation_enabled"), &VolumetricCloudMaterial4D::is_animation_enabled);
	ClassDB::bind_method(D_METHOD("set_animation_enabled", "enabled"), &VolumetricCloudMaterial4D::set_animation_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "animation_enabled", PROPERTY_HINT_GROUP_ENABLE), "set_animation_enabled", "is_animation_enabled");
	ClassDB::bind_method(D_METHOD("get_wind_velocity"), &VolumetricCloudMaterial4D::get_wind_velocity);
	ClassDB::bind_method(D_METHOD("set_wind_velocity", "velocity"), &VolumetricCloudMaterial4D::set_wind_velocity);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "wind_velocity", PROPERTY_HINT_NONE, "suffix:m/s"), "set_wind_velocity", "get_wind_velocity");
	ClassDB::bind_method(D_METHOD("get_evolution_speed"), &VolumetricCloudMaterial4D::get_evolution_speed);
	ClassDB::bind_method(D_METHOD("set_evolution_speed", "speed"), &VolumetricCloudMaterial4D::set_evolution_speed);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "evolution_speed", PROPERTY_HINT_RANGE, "-1,1,0.001,or_greater,or_less,radians_as_degrees"), "set_evolution_speed", "get_evolution_speed");

	ADD_GROUP("Lighting", "");
	ClassDB::bind_method(D_METHOD("get_albedo_color"), &VolumetricCloudMaterial4D::get_albedo_color);
	ClassDB::bind_method(D_METHOD("set_albedo_color", "color"), &VolumetricCloudMaterial4D::set_albedo_color);
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "albedo_color", PROPERTY_HINT_COLOR_NO_ALPHA), "set_albedo_color", "get_albedo_color");
	ClassDB::bind_method(D_METHOD("get_ambient_color"), &VolumetricCloudMaterial4D::get_ambient_color);
	ClassDB::bind_method(D_METHOD("set_ambient_color", "color"), &VolumetricCloudMaterial4D::set_ambient_color);
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "ambient_color", PROPERTY_HINT_COLOR_NO_ALPHA), "set_ambient_color", "get_ambient_color");
	ClassDB::bind_method(D_METHOD("get_ambient_intensity"), &VolumetricCloudMaterial4D::get_ambient_intensity);
	ClassDB::bind_method(D_METHOD("set_ambient_intensity", "intensity"), &VolumetricCloudMaterial4D::set_ambient_intensity);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "ambient_intensity", PROPERTY_HINT_RANGE, "0,4,0.001,or_greater"), "set_ambient_intensity", "get_ambient_intensity");
	ClassDB::bind_method(D_METHOD("get_sun_intensity"), &VolumetricCloudMaterial4D::get_sun_intensity);
	ClassDB::bind_method(D_METHOD("set_sun_intensity", "intensity"), &VolumetricCloudMaterial4D::set_sun_intensity);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "sun_intensity", PROPERTY_HINT_RANGE, "0,4,0.001,or_greater"), "set_sun_intensity", "get_sun_intensity");
	ClassDB::bind_method(D_METHOD("get_phase_anisotropy"), &VolumetricCloudMaterial4D::get_phase_anisotropy);
	ClassDB::bind_method(D_METHOD("set_phase_anisotropy", "anisotropy"), &VolumetricCloudMaterial4D::set_phase_anisotropy);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "phase_anisotropy", PROPERTY_HINT_RANGE, "-0.95,0.95,0.001"), "set_phase_anisotropy", "get_phase_anisotropy");
	ClassDB::bind_method(D_METHOD("is_affecting_radiance"), &VolumetricCloudMaterial4D::is_affecting_radiance);
	ClassDB::bind_method(D_METHOD("set_affect_radiance", "affect"), &VolumetricCloudMaterial4D::set_affect_radiance);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "affect_radiance"), "set_affect_radiance", "is_affecting_radiance");
	ClassDB::bind_method(D_METHOD("get_extinction_coefficient"), &VolumetricCloudMaterial4D::get_extinction_coefficient);
	ClassDB::bind_method(D_METHOD("set_extinction_coefficient", "coefficient"), &VolumetricCloudMaterial4D::set_extinction_coefficient);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "extinction_coefficient", PROPERTY_HINT_RANGE, U"0,0.1,0.00001,or_greater,suffix:m⁻¹"), "set_extinction_coefficient", "get_extinction_coefficient");
	ClassDB::bind_method(D_METHOD("get_shadow_distance"), &VolumetricCloudMaterial4D::get_shadow_distance);
	ClassDB::bind_method(D_METHOD("set_shadow_distance", "distance"), &VolumetricCloudMaterial4D::set_shadow_distance);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "shadow_distance", PROPERTY_HINT_RANGE, "0,100000,1,or_greater,suffix:m"), "set_shadow_distance", "get_shadow_distance");
	ClassDB::bind_method(D_METHOD("get_shadow_strength"), &VolumetricCloudMaterial4D::get_shadow_strength);
	ClassDB::bind_method(D_METHOD("set_shadow_strength", "strength"), &VolumetricCloudMaterial4D::set_shadow_strength);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "shadow_strength", PROPERTY_HINT_RANGE, "0,8,0.001,or_greater"), "set_shadow_strength", "get_shadow_strength");

	BIND_ENUM_CONSTANT(SAMPLING_METHOD_WORLD_GRID);
	BIND_ENUM_CONSTANT(SAMPLING_METHOD_JITTERED_RAYMARCH);
}

void VolumetricCloudMaterial4D::init_shaders() {
	if (_cloud_only_shader.is_valid()) {
		return;
	}
	initialize_cloud_shader(_cloud_only_shader, String("4D Volumetric Cloud Shader"), volumetric_cloud_shader_shader_glsl);
	initialize_cloud_shader(_gradient_sky_shader, String("4D Gradient Sky with Volumetric Clouds Shader"), gradient_sky_with_volumetric_clouds_shader_shader_glsl);
	initialize_cloud_shader(_physical_sky_shader, String("4D Physical Sky with Volumetric Clouds Shader"), physical_sky_with_volumetric_clouds_shader_shader_glsl);
	initialize_cloud_shader(_plain_sky_shader, String("4D Plain Sky with Volumetric Clouds Shader"), plain_sky_with_volumetric_clouds_shader_shader_glsl);
	initialize_cloud_shader(_cloud_only_full_res_shader, String("4D Full-Resolution Volumetric Cloud Shader"), volumetric_cloud_full_res_shader_shader_glsl);
	initialize_cloud_shader(_gradient_sky_full_res_shader, String("4D Full-Resolution Gradient Sky with Volumetric Clouds Shader"), gradient_sky_with_volumetric_clouds_full_res_shader_shader_glsl);
	initialize_cloud_shader(_physical_sky_full_res_shader, String("4D Full-Resolution Physical Sky with Volumetric Clouds Shader"), physical_sky_with_volumetric_clouds_full_res_shader_shader_glsl);
	initialize_cloud_shader(_plain_sky_full_res_shader, String("4D Full-Resolution Plain Sky with Volumetric Clouds Shader"), plain_sky_with_volumetric_clouds_full_res_shader_shader_glsl);
}

void VolumetricCloudMaterial4D::cleanup_shaders() {
	_plain_sky_full_res_shader.unref();
	_physical_sky_full_res_shader.unref();
	_gradient_sky_full_res_shader.unref();
	_cloud_only_full_res_shader.unref();
	_plain_sky_shader.unref();
	_physical_sky_shader.unref();
	_gradient_sky_shader.unref();
	_cloud_only_shader.unref();
}

VolumetricCloudMaterial4D::VolumetricCloudMaterial4D() {
	init_shaders();
	set_shader(get_shader_for_sky_material(Ref<SkyMaterial4D>()));
	set_albedo_color(_albedo_color);
	set_ambient_color(_ambient_color);
	set_ambient_intensity(_ambient_intensity);
	set_sun_intensity(_sun_intensity);
	set_phase_anisotropy(_phase_anisotropy);
	set_bottom_height(_bottom_height);
	set_vertical_thickness(_vertical_thickness);
	set_shape_scale(_shape_scale);
	set_shape_fractal_strength(_shape_fractal_strength);
	set_detail_scale(_detail_scale);
	set_detail_strength(_detail_strength);
	set_coverage(_coverage);
	set_density(_density);
	set_wind_velocity(_wind_velocity);
	set_evolution_speed(_evolution_speed);
	set_extinction_coefficient(_extinction_coefficient);
	set_shadow_distance(_shadow_distance);
	set_shadow_strength(_shadow_strength);
	set_maximum_ray_distance(_maximum_ray_distance);
	set_distance_fade_start(_distance_fade_start);
	set_distance_fade_curve(_distance_fade_curve);
	set_sampling_method(_sampling_method);
	set_sampling_steps(_sampling_steps);
	set_affect_radiance(_affect_radiance);
}
