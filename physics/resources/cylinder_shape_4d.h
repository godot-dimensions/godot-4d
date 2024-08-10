#pragma once

#include "shape_4d.h"

class CylinderShape4D : public Shape4D {
	GDCLASS(CylinderShape4D, Shape4D);

	real_t _height = 2.0f;
	real_t _radius = 0.5f;

protected:
	static void _bind_methods();

public:
	real_t get_height() const;
	void set_height(const real_t p_height);

	real_t get_radius() const;
	void set_radius(const real_t p_radius);

	virtual bool has_point(const Vector4 &p_point) const override;
};
