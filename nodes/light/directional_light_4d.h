#pragma once

#include "light_4d.h"

class DirectionalLight4D : public Light4D {
	GDCLASS(DirectionalLight4D, Light4D);

	// Default to 0.005 radians, which is about 0.286 degrees, similar to the angular radius of the sun as seen from Earth.
	// Note that GradientSkyMaterial4D and PhysicalSkyMaterial4D have sun angle/glow properties that make the visible sun appear larger than this.
	double _angular_radius_radians = 0.005;

protected:
	static void _bind_methods();

public:
	virtual RID create_3d_render_base() const override;
	virtual bool update_3d_cross_section_render_base(const Projection &p_relative_to_camera_basis, const Vector4 &p_relative_to_camera_position, const RID p_light_3d_render_base) const override;
	virtual void update_3d_projected_render_base(const Projection &p_relative_to_camera_basis, const Vector4 &p_relative_to_camera_position, const RID p_light_3d_render_base) const override;

	double get_angular_radius_degrees() const { return Math::rad_to_deg(_angular_radius_radians); }
	void set_angular_radius_degrees(const double p_angular_radius_degrees);

	double get_angular_radius_radians() const { return _angular_radius_radians; }
	void set_angular_radius_radians(const double p_angular_radius_radians);
};
