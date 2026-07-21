#pragma once

#include "shape_4d.h"

class CapsuleShape4D : public Shape4D {
	GDCLASS(CapsuleShape4D, Shape4D);

	real_t _radius = 0.5f;
	real_t _height = 2.0f;
	// If true, _height is the full height of the capsule. If false, _height is the mid-height of the capsule.
	bool _height_is_full = true;

protected:
	static void _bind_methods();
	void _validate_property(PropertyInfo &p_property) const;

public:
	real_t get_radius() const;
	void set_radius(const real_t p_radius);

	real_t get_full_height() const;
	void set_full_height(const real_t p_full_height);

	real_t get_mid_height() const;
	void set_mid_height(const real_t p_mid_height);

	bool get_height_is_full() const;
	void set_height_is_full(const bool p_height_is_full);

	virtual real_t get_hypervolume() const override;
	virtual real_t get_surface_volume() const override;

	virtual Dictionary raycast_intersects(const Vector4 &p_local_from, const Vector4 &p_local_direction, const real_t p_max_distance = Math_INF, const bool p_inside_is_zero = false) const override;

	virtual real_t get_signed_distance_to_surface(const Vector4 &p_local_point, Vector4 *r_nearest_point_on_surface = nullptr) const override;

	virtual Vector4 get_nearest_point(const Vector4 &p_local_point) const override;
	virtual Vector4 get_support_point(const Vector4 &p_local_direction) const override;
	virtual bool has_point(const Vector4 &p_local_point) const override;

	virtual bool is_equal_exact(const Ref<Shape4D> &p_shape) const override;
};
