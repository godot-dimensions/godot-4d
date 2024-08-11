#pragma once

#include "shape_4d.h"

class DuocylinderShape4D : public Shape4D {
	GDCLASS(DuocylinderShape4D, Shape4D);

	real_t _radius_xy = 0.5f;
	real_t _radius_zw = 0.5f;

protected:
	static void _bind_methods();

public:
	real_t get_radius_xy() const;
	void set_radius_xy(const real_t p_radius_xy);

	real_t get_radius_zw() const;
	void set_radius_zw(const real_t p_radius_zw);

	virtual real_t get_hypervolume() const override;
	virtual Vector4 get_nearest_point(const Vector4 &p_point) const override;
	virtual Vector4 get_support_point(const Vector4 &p_direction) const override;
	virtual real_t get_surface_volume() const override;
	virtual bool has_point(const Vector4 &p_point) const override;
};
