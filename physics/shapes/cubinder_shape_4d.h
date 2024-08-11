#pragma once

#include "shape_4d.h"

class CubinderShape4D : public Shape4D {
	GDCLASS(CubinderShape4D, Shape4D);

	real_t _height = 2.0f;
	real_t _radius = 0.5f;
	real_t _thickness = 2.0f;

protected:
	static void _bind_methods();

public:
	real_t get_height() const;
	void set_height(const real_t p_height);

	real_t get_radius() const;
	void set_radius(const real_t p_radius);

	real_t get_thickness() const;
	void set_thickness(const real_t p_thickness);

	Vector2 get_size_wy() const;
	void set_size_wy(const Vector2 &p_size_wy);

	virtual real_t get_hypervolume() const override;
	virtual Vector4 get_nearest_point(const Vector4 &p_point) const override;
	virtual Vector4 get_support_point(const Vector4 &p_direction) const override;
	virtual real_t get_surface_volume() const override;
	virtual bool has_point(const Vector4 &p_point) const override;
};
