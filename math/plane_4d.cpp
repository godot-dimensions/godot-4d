#include "plane_4d.h"

#include "basis_4d.h"

real_t Plane4D::distance_to(const Vector4 &p_point) const {
	return normal.dot(p_point) - distance;
}

bool Plane4D::is_point_over(const Vector4 &p_point) const {
	return normal.dot(p_point) > distance;
}

bool Plane4D::has_point(const Vector4 &p_point) const {
	return Math::is_zero_approx(distance_to(p_point));
}

Vector4 Plane4D::project(const Vector4 &p_point) const {
	return p_point - normal * distance_to(p_point);
}

Variant Plane4D::intersect_ray(const Vector4 &p_ray_origin, const Vector4 &p_ray_direction) const {
	const real_t denominator = normal.dot(p_ray_direction);
	if (Math::is_zero_approx(denominator)) {
		return Variant();
	}
	const real_t factor = (distance - normal.dot(p_ray_origin)) / denominator;
	if (factor < 0.0) {
		return Variant();
	}
	return p_ray_origin + p_ray_direction * factor;
}

Variant Plane4D::intersect_line_segment(const Vector4 &p_begin, const Vector4 &p_end) const {
	const Vector4 direction = p_end - p_begin;
	const real_t denominator = normal.dot(direction);
	if (Math::is_zero_approx(denominator)) {
		return Variant();
	}
	const real_t factor = (distance - normal.dot(p_begin)) / denominator;
	if (factor < 0.0 || factor > 1.0) {
		return Variant();
	}
	return p_begin + direction * factor;
}

Plane4D Plane4D::normalized() const {
	const real_t len = normal.length();
	if (len == 0.0f) {
		return Plane4D();
	}
	return Plane4D(normal / len, distance * len);
}

// Plane comparison functions.

bool Plane4D::is_equal_approx_any_side(const Plane4D &p_plane) const {
	return (normal.is_equal_approx(p_plane.normal) && Math::is_equal_approx(distance, p_plane.distance)) || (normal.is_equal_approx(-p_plane.normal) && Math::is_equal_approx(distance, -p_plane.distance));
}

bool Plane4D::is_equal_approx(const Plane4D &p_plane) const {
	return normal.is_equal_approx(p_plane.normal) && Math::is_equal_approx(distance, p_plane.distance);
}

// Operators.

bool Plane4D::operator==(const Plane4D &p_other) const {
	return normal == p_other.normal && distance == p_other.distance;
}

bool Plane4D::operator!=(const Plane4D &p_other) const {
	return normal != p_other.normal || distance != p_other.distance;
}

Plane4D::operator String() const {
	return "[N: " + normal.operator String() + ", D: " + String::num_real(distance, false) + "]";
}

Plane4D::Plane4D(real_t p_x, real_t p_y, real_t p_z, real_t p_w, real_t p_distance) {
	normal = Vector4(p_x, p_y, p_z, p_w);
	distance = p_distance;
}

Plane4D::Plane4D(const Vector4 &p_normal, real_t p_distance) {
	normal = p_normal;
	distance = p_distance;
}

Plane4D::Plane4D(const Vector4 &p_normal, const Vector4 &p_point) {
	normal = p_normal;
	distance = p_normal.dot(p_point);
}

Plane4D::Plane4D(const Vector4 &p_point1, const Vector4 &p_point2, const Vector4 &p_point3, const Vector4 &p_point4) {
	const Vector4 added = p_point1 + p_point2 + p_point3 + p_point4;
	Basis4D basis = Basis4D(p_point2 - p_point1, p_point3 - p_point1, p_point4 - p_point1, added);
	basis.orthonormalize();
	normal = basis.w;
	distance = normal.dot(p_point1);
}

static_assert(sizeof(Plane4D) == 5 * sizeof(real_t));
