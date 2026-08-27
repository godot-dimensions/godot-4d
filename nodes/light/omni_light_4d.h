#pragma once

#include "light_4d.h"

// See Godot's OmniLight3D.
class OmniLight4D : public Light4D {
	GDCLASS(OmniLight4D, Light4D);

	real_t _omni_range_meters = 10.0f;
	real_t _omni_range_attenuation = 3.0f;
	real_t _omni_light_size_meters = 0.0f;

protected:
	static void _bind_methods();

public:
	virtual RID create_3d_render_base() const override;
	virtual bool update_3d_cross_section_render_base(const Projection &p_relative_to_camera_basis, const Vector4 &p_relative_to_camera_position, const RID p_light_3d_render_base) const override;
	virtual void update_3d_projected_render_base(const Projection &p_relative_to_camera_basis, const Vector4 &p_relative_to_camera_position, const RID p_light_3d_render_base) const override;

	real_t get_omni_range_meters() const { return _omni_range_meters; }
	void set_omni_range_meters(const real_t p_omni_range_meters);

	real_t get_omni_range_attenuation() const { return _omni_range_attenuation; }
	void set_omni_range_attenuation(const real_t p_omni_range_attenuation);

	real_t get_omni_light_size_meters() const { return _omni_light_size_meters; }
	void set_omni_light_size_meters(const real_t p_omni_light_size_meters);
};
