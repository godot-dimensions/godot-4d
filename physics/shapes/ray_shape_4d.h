#pragma once

#include "shape_4d.h"

class RayShape4D : public Shape4D {
	GDCLASS(RayShape4D, Shape4D);

	real_t _ray_target_y = -1.0;

protected:
	static void _bind_methods();

public:
	real_t get_ray_length() const { return -_ray_target_y; }
	void set_ray_length(const real_t p_length) { _ray_target_y = -p_length; }

	real_t get_ray_target_y() const { return _ray_target_y; }
	void set_ray_target_y(const real_t p_target_y) { _ray_target_y = p_target_y; }

	virtual real_t get_hypervolume() const override { return 0.0f; }
	virtual real_t get_surface_volume() const override { return 0.0f; }
	virtual Rect4 get_rect_bounds(const Transform4D &p_to_target = Transform4D()) const override;

	virtual Vector4 get_nearest_point(const Vector4 &p_point) const override;
	virtual Vector4 get_support_point(const Vector4 &p_direction) const override;
	virtual bool has_point(const Vector4 &p_point) const override;

	virtual bool is_equal_exact(const Ref<Shape4D> &p_shape) const override;

	virtual Ref<WireMesh4D> to_wire_mesh() const override;
};
