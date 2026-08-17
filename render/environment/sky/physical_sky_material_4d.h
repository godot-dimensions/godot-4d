#pragma once

#include "sky_material_4d.h"

class PhysicalSkyMaterial4D : public SkyMaterial4D {
	GDCLASS(PhysicalSkyMaterial4D, SkyMaterial4D);

	static Ref<Shader> _shader;

	// These colors are based on Godot's 3D `PhysicalSkyMaterial`, while the
	// coefficients use physical inverse-meter units instead of legacy tuning values.
	Color _ground_color = Color(0.1f, 0.07f, 0.034f);
	Color _rayleigh_color = Color(0.3f, 0.405f, 0.6f);
	Color _mie_color = Color(0.69f, 0.729f, 0.812f);

	// The scattering coefficients are expressed in inverse meters.
	real_t _rayleigh_coefficient = 2.0e-5f;
	real_t _mie_coefficient = 5.0e-6f;
	real_t _mie_anisotropy = 0.8f;

	real_t _sun_glow_intensity = 1.0f;
	real_t _sun_glow_half_width_radians = 0.05f;

protected:
	static void _bind_methods();
	void _validate_property(PropertyInfo &p_property) const;

public:
	Color get_ground_color() const { return _ground_color; }
	void set_ground_color(const Color &p_ground_color);

	real_t get_sun_glow_intensity() const { return _sun_glow_intensity; }
	void set_sun_glow_intensity(const real_t p_sun_glow_intensity);
	real_t get_sun_glow_half_width_radians() const { return _sun_glow_half_width_radians; }
	void set_sun_glow_half_width_radians(const real_t p_sun_glow_half_width_radians);

	Color get_rayleigh_color() const { return _rayleigh_color; }
	void set_rayleigh_color(const Color &p_rayleigh_color);

	real_t get_rayleigh_coefficient() const { return _rayleigh_coefficient; }
	void set_rayleigh_coefficient(const real_t p_rayleigh_coefficient);

	Color get_mie_color() const { return _mie_color; }
	void set_mie_color(const Color &p_mie_color);

	real_t get_mie_coefficient() const { return _mie_coefficient; }
	void set_mie_coefficient(const real_t p_mie_coefficient);

	real_t get_mie_anisotropy() const { return _mie_anisotropy; }
	void set_mie_anisotropy(const real_t p_mie_anisotropy);

	static void init_shader();
	static void cleanup_shader();

	PhysicalSkyMaterial4D();
};
