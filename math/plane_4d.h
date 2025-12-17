#pragma once

#include "../godot_4d_defines.h"

// Represents a triplane, a 4D hyperplane, whose surface is 3-dimensional.
struct _NO_DISCARD_ Plane4D {
	Vector4 normal = Vector4(0, 1, 0, 0);
	real_t distance = 0.0f;

	real_t get_distance() const { return distance; }
	void set_distance(const real_t p_distance) { distance = p_distance; }

	Vector4 get_normal() const { return normal; };
	void set_normal(const Vector4 &p_normal) { normal = p_normal; }

	// Point functions.
	real_t distance_to(const Vector4 &p_point) const;
	bool is_point_over(const Vector4 &p_point) const;
	bool has_point(const Vector4 &p_point) const;
	Vector4 project(const Vector4 &p_point) const;

	// Plane math functions.
	Vector4 get_center() const { return normal * distance; }
	Variant intersect_ray(const Vector4 &p_ray_origin, const Vector4 &p_ray_direction) const;
	Variant intersect_line_segment(const Vector4 &p_begin, const Vector4 &p_end) const;
	Plane4D normalized() const;

	// Plane comparison functions.
	bool is_equal_approx(const Plane4D &p_other) const;
	bool is_equal_approx_any_side(const Plane4D &p_Plane4D) const;
	bool is_finite() const;

	// Operators.
	Plane4D operator-() const { return Plane4D(-normal, -distance); }
	bool operator==(const Plane4D &p_other) const;
	bool operator!=(const Plane4D &p_other) const;
	operator String() const;

	Plane4D() {}
	Plane4D(real_t p_x, real_t p_y, real_t p_z, real_t p_w, real_t p_distance = 0.0f);
	Plane4D(const Vector4 &p_normal, real_t p_distance = 0.0f);
	Plane4D(const Vector4 &p_normal, const Vector4 &p_point);
	Plane4D(const Vector4 &p_point1, const Vector4 &p_point2, const Vector4 &p_point3, const Vector4 &p_point4);
	static Plane4D from_coplanar_directions(const Vector4 &p_dir1, const Vector4 &p_dir2, const Vector4 &p_dir3, const Vector4 &p_point = Vector4());
};

static_assert(sizeof(Plane4D) == sizeof(real_t) * 5);
