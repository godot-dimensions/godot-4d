#pragma once

#include "shape_4d.h"

class CapsuleShape4D : public Shape4D {
	GDCLASS(CapsuleShape4D, Shape4D);

	real_t _height = 2.0f;
	real_t _radius = 0.5f;

protected:
	static void _bind_methods();

public:
	real_t get_height() const;
	void set_height(const real_t p_height);

	real_t get_mid_height() const;
	void set_mid_height(const real_t p_mid_height);

	real_t get_radius() const;
	void set_radius(const real_t p_radius);

	virtual real_t get_hypervolume() const override;
	virtual real_t get_surface_volume() const override;

	virtual Vector4 get_nearest_point(const Vector4 &p_point) const override;
	virtual Vector4 get_support_point(const Vector4 &p_direction) const override;
	virtual bool has_point(const Vector4 &p_point) const override;

	virtual bool is_equal_exact(const Ref<Shape4D> &p_shape) const override;
};
