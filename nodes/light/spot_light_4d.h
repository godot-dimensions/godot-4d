#pragma once

#include "light_4d.h"

class SpotLight4D : public Light4D {
	GDCLASS(SpotLight4D, Light4D);

	double _spot_angle_degrees = 45.0; // Use double for angle to avoid loss when converting to radians or using trig functions.
	real_t _spot_angle_attenuation = 1.0f;
	real_t _spot_range_meters = 10.0f;
	real_t _spot_range_attenuation = 3.0f;
	real_t _spot_light_size_meters = 0.0f;

protected:
	static void _bind_methods();

public:
	virtual RID create_light_3d_render_base() const override;
	virtual bool update_light_3d_cross_section_render_base(const Projection &p_relative_to_camera_basis, const Vector4 &p_relative_to_camera_position, const RID p_light_3d_render_base) const override;
	virtual bool update_light_3d_projected_render_base(const Projection &p_relative_to_camera_basis, const Vector4 &p_relative_to_camera_position, const RID p_light_3d_render_base) const override;

	double get_spot_angle_degrees() const { return _spot_angle_degrees; }
	void set_spot_angle_degrees(const double p_spot_angle_degrees);

	real_t get_spot_angle_attenuation() const { return _spot_angle_attenuation; }
	void set_spot_angle_attenuation(const real_t p_spot_angle_attenuation);

	real_t get_spot_range_meters() const { return _spot_range_meters; }
	void set_spot_range_meters(const real_t p_spot_range_meters);

	real_t get_spot_range_attenuation() const { return _spot_range_attenuation; }
	void set_spot_range_attenuation(const real_t p_spot_range_attenuation);

	real_t get_spot_light_size_meters() const { return _spot_light_size_meters; }
	void set_spot_light_size_meters(const real_t p_spot_light_size_meters);
};
