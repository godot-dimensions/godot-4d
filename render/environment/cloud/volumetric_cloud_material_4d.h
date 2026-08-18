#pragma once

#include "../../../godot_4d_defines.h"

#if GDEXTENSION
#include <godot_cpp/classes/shader.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#elif GODOT_MODULE
#include "scene/resources/material.h"
#endif

class SkyMaterial4D;

class VolumetricCloudMaterial4D : public ShaderMaterial {
	GDCLASS(VolumetricCloudMaterial4D, ShaderMaterial);

public:
	enum SamplingMethod : uint8_t {
		SAMPLING_METHOD_WORLD_GRID,
		SAMPLING_METHOD_JITTERED_RAYMARCH,
		SAMPLING_METHOD_MAX,
	};

private:
	static Ref<Shader> _cloud_only_shader;
	static Ref<Shader> _gradient_sky_shader;
	static Ref<Shader> _physical_sky_shader;
	static Ref<Shader> _plain_sky_shader;
	static Ref<Shader> _cloud_only_full_res_shader;
	static Ref<Shader> _gradient_sky_full_res_shader;
	static Ref<Shader> _physical_sky_full_res_shader;
	static Ref<Shader> _plain_sky_full_res_shader;

	Vector4 _wind_velocity = Vector4(12.0f, 0.0f, 4.0f, 2.0f);
	Color _albedo_color = Color(1.0f, 1.0f, 1.0f);
	Color _ambient_color = Color(0.42f, 0.48f, 0.58f);

	real_t _distance_fade_curve = 0.25f;
	float _distance_fade_start = 0.0f;
	float _ambient_intensity = 0.45f;
	float _sun_intensity = 1.0f;
	float _phase_anisotropy = 0.55f;

	float _bottom_height = 1000.0f;
	float _vertical_thickness = 2500.0f;
	float _maximum_ray_distance = 20000.0f;
	float _shape_scale = 1800.0f;
	float _shape_fractal_strength = 0.5f;
	float _detail_scale = 350.0f;
	float _detail_strength = 0.35f;
	float _coverage = 0.5f;
	float _density = 1.0f;

	float _evolution_speed = 0.002f;
	float _extinction_coefficient = 0.002f;
	float _shadow_distance = 600.0f;
	float _shadow_strength = 1.0f;
	int32_t _sampling_steps = 64;
	SamplingMethod _sampling_method = SAMPLING_METHOD_JITTERED_RAYMARCH;
	bool _animation_enabled = true;
	bool _affect_radiance = true;

protected:
	static void _bind_methods();
	void _validate_property(PropertyInfo &p_property) const;

public:
	Color get_albedo_color() const { return _albedo_color; }
	void set_albedo_color(const Color &p_albedo_color);

	Color get_ambient_color() const { return _ambient_color; }
	void set_ambient_color(const Color &p_ambient_color);

	float get_ambient_intensity() const { return _ambient_intensity; }
	void set_ambient_intensity(const float p_ambient_intensity);

	float get_sun_intensity() const { return _sun_intensity; }
	void set_sun_intensity(const float p_sun_intensity);

	float get_phase_anisotropy() const { return _phase_anisotropy; }
	void set_phase_anisotropy(const float p_phase_anisotropy);

	float get_bottom_height() const { return _bottom_height; }
	void set_bottom_height(const float p_bottom_height);

	float get_vertical_thickness() const { return _vertical_thickness; }
	void set_vertical_thickness(const float p_vertical_thickness);

	float get_shape_scale() const { return _shape_scale; }
	void set_shape_scale(const float p_shape_scale);
	float get_shape_fractal_strength() const { return _shape_fractal_strength; }
	void set_shape_fractal_strength(const float p_shape_fractal_strength);

	float get_detail_scale() const { return _detail_scale; }
	void set_detail_scale(const float p_detail_scale);

	float get_detail_strength() const { return _detail_strength; }
	void set_detail_strength(const float p_detail_strength);

	float get_coverage() const { return _coverage; }
	void set_coverage(const float p_coverage);

	float get_density() const { return _density; }
	void set_density(const float p_density);

	Vector4 get_wind_velocity() const { return _wind_velocity; }
	void set_wind_velocity(const Vector4 &p_wind_velocity);

	float get_evolution_speed() const { return _evolution_speed; }
	void set_evolution_speed(const float p_evolution_speed);
	bool is_animation_enabled() const { return _animation_enabled; }
	void set_animation_enabled(const bool p_enabled) { _animation_enabled = p_enabled; }

	float get_extinction_coefficient() const { return _extinction_coefficient; }
	void set_extinction_coefficient(const float p_extinction_coefficient);

	float get_shadow_distance() const { return _shadow_distance; }
	void set_shadow_distance(const float p_shadow_distance);

	float get_shadow_strength() const { return _shadow_strength; }
	void set_shadow_strength(const float p_shadow_strength);

	float get_maximum_ray_distance() const { return _maximum_ray_distance; }
	void set_maximum_ray_distance(const float p_maximum_ray_distance);
	float get_distance_fade_start() const { return _distance_fade_start; }
	void set_distance_fade_start(const float p_distance_fade_start);
	real_t get_distance_fade_curve() const { return _distance_fade_curve; }
	void set_distance_fade_curve(const real_t p_distance_fade_curve);

	SamplingMethod get_sampling_method() const { return _sampling_method; }
	void set_sampling_method(const SamplingMethod p_sampling_method);
	int32_t get_sampling_steps() const { return _sampling_steps; }
	void set_sampling_steps(const int32_t p_sampling_steps);

	bool is_affecting_radiance() const { return _affect_radiance; }
	void set_affect_radiance(const bool p_affect_radiance);

	bool is_animated() const;
	Ref<Shader> get_shader_for_sky_material(const Ref<SkyMaterial4D> &p_sky_material) const;

	static void init_shaders();
	static void cleanup_shaders();

	VolumetricCloudMaterial4D();
};

VARIANT_ENUM_CAST(VolumetricCloudMaterial4D::SamplingMethod);
