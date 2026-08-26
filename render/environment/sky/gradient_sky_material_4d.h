#pragma once

#include "sky_material_4d.h"

class GradientSkyMaterial4D : public SkyMaterial4D {
	GDCLASS(GradientSkyMaterial4D, SkyMaterial4D);

	static Ref<Shader> _shader;

	// Keep these values in sync with `gradient_sky_shader.glsl`,
	// `EditorPreviewEnvironment4D`, and Godot's 3D `ProceduralSkyMaterial`.
	Color _horizon_color = Color(0.6463f, 0.6558f, 0.6708f);
	Color _top_color = Color(0.385f, 0.454f, 0.55f);
	Color _bottom_color = Color(0.2f, 0.169f, 0.133f);

	real_t _top_curve = 0.15f;
	real_t _bottom_curve = 0.02f;
	real_t _sun_angle_max = 30.0f;
	real_t _sun_curve = 0.15f;

protected:
	static void _bind_methods();
	void _validate_property(PropertyInfo &p_property) const;

public:
	Color get_horizon_color() const { return _horizon_color; }
	void set_horizon_color(const Color &p_horizon_color);

	Color get_top_color() const { return _top_color; }
	void set_top_color(const Color &p_top_color);

	real_t get_top_curve() const { return _top_curve; }
	void set_top_curve(const real_t p_top_curve);

	Color get_bottom_color() const { return _bottom_color; }
	void set_bottom_color(const Color &p_bottom_color);

	real_t get_bottom_curve() const { return _bottom_curve; }
	void set_bottom_curve(const real_t p_bottom_curve);

	real_t get_sun_angle_max() const { return _sun_angle_max; }
	void set_sun_angle_max(const real_t p_sun_angle_max);

	real_t get_sun_curve() const { return _sun_curve; }
	void set_sun_curve(const real_t p_sun_curve);

	static void init_shader();
	static void cleanup_shader();

	GradientSkyMaterial4D();
};
