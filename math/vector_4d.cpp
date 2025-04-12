#include "vector_4d.h"

#include <algorithm>

real_t Vector4D::angle_to(const Vector4 &p_from, const Vector4 &p_to) {
	return Math::acos(p_from.dot(p_to) / (p_from.length() * p_to.length()));
}

Vector4 Vector4D::bounce(const Vector4 &p_vector, const Vector4 &p_normal) {
	return p_vector + (-2.0f) * p_vector.dot(p_normal) * p_normal;
}

Vector4 Vector4D::bounce_ratio(const Vector4 &p_vector, const Vector4 &p_normal, const real_t p_bounce_ratio) {
	return p_vector + (-1.0f - p_bounce_ratio) * p_vector.dot(p_normal) * p_normal;
}

real_t Vector4D::cross(const Vector4 &p_a, const Vector4 &p_b) {
	const real_t diagonal = p_a.length_squared() * p_b.length_squared();
	const real_t non_diagonal = p_a.dot(p_b);
	return sqrt(diagonal - non_diagonal * non_diagonal);
}

Vector4 Vector4D::limit_length(const Vector4 &p_vector, const real_t p_len) {
	const real_t l = p_vector.length();
	Vector4 v = p_vector;
	if (l > 0 && p_len < l) {
		v /= l;
		v *= p_len;
	}
	return v;
}

void _sort_axes_max_to_min(const Vector4 &p_vector, Vector4::Axis *r_axes) {
	std::sort(r_axes, r_axes + 4, [&p_vector](Vector4::Axis a, Vector4::Axis b) {
		return p_vector[a] > p_vector[b];
	});
}

Vector4 Vector4D::limit_length_taxicab(const Vector4 &p_vector, const real_t p_len) {
	Vector4 abs_vector = p_vector.abs();
	real_t taxicab_length = abs_vector.x + abs_vector.y + abs_vector.z + abs_vector.w;
	if (taxicab_length <= p_len) {
		return p_vector;
	}
	// Else, we need to take away length from each axis, as equally as possible.
	// But we need to start with the shortest axis because it will be the first to reach 0.
	Vector4::Axis axes[4] = { Vector4::Axis::AXIS_X, Vector4::Axis::AXIS_Y, Vector4::Axis::AXIS_Z, Vector4::Axis::AXIS_W };
	_sort_axes_max_to_min(abs_vector, axes);
	taxicab_length -= p_len;
	Vector4 limited = p_vector;
	for (int i = 4; i > 0; i--) {
		const Vector4::Axis axis = axes[i - 1];
		const real_t takeaway = taxicab_length / i;
		if (abs_vector[axis] <= takeaway) {
			limited[axis] = 0.0f;
			// Since this axis reached zero, we need to take away more from the other axes.
			taxicab_length -= abs_vector[axis];
		} else {
			taxicab_length -= takeaway;
			if (limited[axis] < 0.0f) {
				limited[axis] += takeaway;
			} else {
				limited[axis] -= takeaway;
			}
		}
	}
	return limited;
}

Vector4 Vector4D::move_toward(const Vector4 &p_from, const Vector4 &p_to, const real_t p_delta) {
	const Vector4 offset = p_to - p_from;
	const real_t len = offset.length();
	return len <= p_delta || len < (real_t)CMP_EPSILON ? p_to : p_from + offset / len * p_delta;
}

Vector4 Vector4D::perpendicular(const Vector4 &p_a, const Vector4 &p_b, const Vector4 &p_c) {
	Vector4 perp;
	/* clang-format off */
	perp.x = - p_a.y * (p_b.z * p_c.w - p_b.w * p_c.z)
	         + p_a.z * (p_b.y * p_c.w - p_b.w * p_c.y)
	         - p_a.w * (p_b.y * p_c.z - p_b.z * p_c.y);
	perp.y = + p_a.x * (p_b.z * p_c.w - p_b.w * p_c.z)
	         - p_a.z * (p_b.x * p_c.w - p_b.w * p_c.x)
	         + p_a.w * (p_b.x * p_c.z - p_b.z * p_c.x);
	perp.z = - p_a.x * (p_b.y * p_c.w - p_b.w * p_c.y)
	         + p_a.y * (p_b.x * p_c.w - p_b.w * p_c.x)
	         - p_a.w * (p_b.x * p_c.y - p_b.y * p_c.x);
	perp.w = + p_a.x * (p_b.y * p_c.z - p_b.z * p_c.y)
	         - p_a.y * (p_b.x * p_c.z - p_b.z * p_c.x)
	         + p_a.z * (p_b.x * p_c.y - p_b.y * p_c.x);
	/* clang-format on */
	return perp;
}

Vector4 Vector4D::project(const Vector4 &p_vector, const Vector4 &p_on_normal) {
	return p_on_normal * (p_vector.dot(p_on_normal) / p_on_normal.length_squared());
}

Vector4 Vector4D::reflect(const Vector4 &p_vector, const Vector4 &p_normal) {
#ifdef MATH_CHECKS
	ERR_FAIL_COND_V_MSG(!p_normal.is_normalized(), Vector4(), "The normal Vector4 must be normalized.");
#endif
	return 2.0f * p_vector.dot(p_normal) * p_normal - p_vector;
}

Vector4 Vector4D::rotate_in_plane(const Vector4 &p_vector, const Vector4 &p_plane_from, const Vector4 &p_plane_to, const real_t p_angle) {
	const Vector4 plane_cos = p_plane_from.normalized();
	const Vector4 plane_sin = (p_plane_to - plane_cos * p_plane_from.dot(p_plane_to)).normalized();
	// Determine the destination plane vectors.
	const real_t angle_cos = Math::cos(p_angle);
	const real_t angle_sin = Math::sin(p_angle);
	const Vector4 dest_cos = plane_cos * angle_cos + plane_sin * angle_sin;
	const Vector4 dest_sin = plane_cos * -angle_sin + plane_sin * angle_cos;
	// Reproject the vector onto the plane.
	const real_t dot_cos = p_vector.dot(plane_cos);
	const real_t dot_sin = p_vector.dot(plane_sin);
	return p_vector - (plane_cos * dot_cos + plane_sin * dot_sin) + (dest_cos * dot_cos + dest_sin * dot_sin);
}

// Slide returns the component of the vector along the given plane, specified by its normal vector.
Vector4 Vector4D::slide(const Vector4 &p_vector, const Vector4 &p_normal) {
#ifdef MATH_CHECKS
	ERR_FAIL_COND_V_MSG(!p_normal.is_normalized(), Vector4(), "The normal Vector4 must be normalized.");
#endif
	return p_vector - p_normal * p_vector.dot(p_normal);
}

// Generation.

Vector4 Vector4D::random_in_radius(const real_t p_radius) {
	while (true) {
		Vector4 random_point = Vector4(Math::random(-p_radius, p_radius), Math::random(-p_radius, p_radius), Math::random(-p_radius, p_radius), Math::random(-p_radius, p_radius));
		if (random_point.length_squared() <= 1.0f) {
			return random_point * p_radius;
		}
	}
}

Vector4 Vector4D::random_in_range(const Vector4 &p_from, const Vector4 &p_to) {
	return Vector4(Math::random(p_from.x, p_to.x), Math::random(p_from.y, p_to.y), Math::random(p_from.z, p_to.z), Math::random(p_from.w, p_to.w));
}

// Conversion.

Vector4 Vector4D::from_3d(const Vector3 &p_vector, const real_t p_w) {
	return Vector4(p_vector.x, p_vector.y, p_vector.z, p_w);
}

Vector3 Vector4D::to_3d(const Vector4 &p_vector) {
	return Vector3(p_vector.x, p_vector.y, p_vector.z);
}

Vector4D *Vector4D::singleton = nullptr;

void Vector4D::_bind_methods() {
	// Vector math.
	ClassDB::bind_static_method("Vector4D", D_METHOD("angle_to", "from", "to"), &Vector4D::angle_to);
	ClassDB::bind_static_method("Vector4D", D_METHOD("bounce", "vector", "normal"), &Vector4D::bounce);
	ClassDB::bind_static_method("Vector4D", D_METHOD("bounce_ratio", "vector", "normal", "bounce_ratio"), &Vector4D::bounce_ratio);
	ClassDB::bind_static_method("Vector4D", D_METHOD("cross", "a", "b"), &Vector4D::cross);
	ClassDB::bind_static_method("Vector4D", D_METHOD("limit_length", "vector", "length"), &Vector4D::limit_length, DEFVAL(1.0));
	ClassDB::bind_static_method("Vector4D", D_METHOD("move_toward", "from", "to", "delta"), &Vector4D::move_toward);
	ClassDB::bind_static_method("Vector4D", D_METHOD("perpendicular", "a", "b", "c"), &Vector4D::perpendicular);
	ClassDB::bind_static_method("Vector4D", D_METHOD("project", "vector", "on_normal"), &Vector4D::project);
	ClassDB::bind_static_method("Vector4D", D_METHOD("reflect", "vector", "normal"), &Vector4D::reflect);
	ClassDB::bind_static_method("Vector4D", D_METHOD("rotate_in_plane", "vector", "plane_from", "plane_to", "angle"), &Vector4D::rotate_in_plane);
	ClassDB::bind_static_method("Vector4D", D_METHOD("slide", "vector", "normal"), &Vector4D::slide);
	// Generation.
	ClassDB::bind_static_method("Vector4D", D_METHOD("random_in_radius", "radius"), &Vector4D::random_in_radius, DEFVAL(1.0));
	ClassDB::bind_static_method("Vector4D", D_METHOD("random_in_range", "from", "to"), &Vector4D::random_in_range, DEFVAL(Vector4(0, 0, 0, 0)), DEFVAL(Vector4(1, 1, 1, 1)));
	// Conversion.
	ClassDB::bind_static_method("Vector4D", D_METHOD("from_3d", "vector", "w"), &Vector4D::from_3d, DEFVAL(0.0));
	ClassDB::bind_static_method("Vector4D", D_METHOD("to_3d", "vector"), &Vector4D::to_3d);
}
