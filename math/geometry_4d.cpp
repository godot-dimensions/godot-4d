#include "geometry_4d.h"

#include "vector_4d.h"

Vector4 Geometry4D::closest_point_on_line(const Vector4 &p_line_position, const Vector4 &p_line_direction, const Vector4 &p_point) {
	const Vector4 vector_to_point = p_point - p_line_position;
	return p_line_position + Vector4D::project(vector_to_point, p_line_direction);
}

Vector4 Geometry4D::closest_point_on_line_segment(const Vector4 &p_line_a, const Vector4 &p_line_b, const Vector4 &p_point) {
	const Vector4 line_direction = p_line_b - p_line_a;
	const Vector4 vector_to_point = p_point - p_line_a;
	const real_t projection_factor = vector_to_point.dot(line_direction) / line_direction.length_squared();
	if (projection_factor < 0.0) {
		return p_line_a;
	} else if (projection_factor > 1.0) {
		return p_line_b;
	}
	return p_line_a + line_direction * projection_factor;
}

Vector4 Geometry4D::closest_point_on_ray(const Vector4 &p_ray_origin, const Vector4 &p_ray_direction, const Vector4 &p_point) {
	const Vector4 vector_to_point = p_point - p_ray_origin;
	const real_t projection_factor = vector_to_point.dot(p_ray_direction) / p_ray_direction.length_squared();
	if (projection_factor < 0.0) {
		return p_ray_origin;
	}
	return p_ray_origin + p_ray_direction * projection_factor;
}

Vector4 Geometry4D::closest_point_between_lines(const Vector4 &p_line1_point, const Vector4 &p_line1_dir, const Vector4 &p_line2_point, const Vector4 &p_line2_dir) {
	const Vector4 difference_between_points = p_line1_point - p_line2_point;
	const real_t line1_len_sq = p_line1_dir.length_squared();
	const real_t line2_len_sq = p_line2_dir.length_squared();
	const real_t line1_projection = p_line1_dir.dot(difference_between_points);
	const real_t line2_projection = p_line2_dir.dot(difference_between_points);
	const real_t direction_dot = p_line1_dir.dot(p_line2_dir);
	const real_t denominator = line1_len_sq * line2_len_sq - direction_dot * direction_dot;
	if (Math::is_zero_approx(denominator)) {
		// Lines are parallel, handling it as a special case.
		return p_line1_point;
	}
	const real_t line1_factor = (direction_dot * line2_projection - line2_len_sq * line1_projection) / denominator;
	const Vector4 closest_point_on_line1 = p_line1_point + p_line1_dir * line1_factor;
	return closest_point_on_line1;
}

PackedVector4Array Geometry4D::closest_points_between_lines(const Vector4 &p_line1_point, const Vector4 &p_line1_dir, const Vector4 &p_line2_point, const Vector4 &p_line2_dir) {
	const Vector4 difference_between_points = p_line1_point - p_line2_point;
	const real_t line1_len_sq = p_line1_dir.length_squared();
	const real_t line2_len_sq = p_line2_dir.length_squared();
	const real_t line1_projection = p_line1_dir.dot(difference_between_points);
	const real_t line2_projection = p_line2_dir.dot(difference_between_points);
	const real_t direction_dot = p_line1_dir.dot(p_line2_dir);
	const real_t denominator = line1_len_sq * line2_len_sq - direction_dot * direction_dot;
	if (Math::is_zero_approx(denominator)) {
		// Lines are parallel, handling it as a special case.
		return PackedVector4Array{ p_line1_point, p_line2_point };
	}
	const real_t line1_factor = (direction_dot * line2_projection - line2_len_sq * line1_projection) / denominator;
	const real_t line2_factor = (line1_len_sq * line2_projection - direction_dot * line1_projection) / denominator;
	const Vector4 closest_point_on_line1 = p_line1_point + p_line1_dir * line1_factor;
	const Vector4 closest_point_on_line2 = p_line2_point + p_line2_dir * line2_factor;
	return PackedVector4Array{ closest_point_on_line1, closest_point_on_line2 };
}

Vector4 Geometry4D::closest_point_between_line_segments(const Vector4 &p_line1_a, const Vector4 &p_line1_b, const Vector4 &p_line2_a, const Vector4 &p_line2_b) {
	const Vector4 difference_between_points = p_line1_a - p_line2_a;
	const Vector4 line1_dir = p_line1_b - p_line1_a;
	const Vector4 line2_dir = p_line2_b - p_line2_a;
	const real_t line1_len_sq = line1_dir.length_squared();
	const real_t line2_len_sq = line2_dir.length_squared();
	const real_t line1_projection = line1_dir.dot(difference_between_points);
	const real_t line2_projection = line2_dir.dot(difference_between_points);
	const real_t direction_dot = line1_dir.dot(line2_dir);
	const real_t denominator = line1_len_sq * line2_len_sq - direction_dot * direction_dot;
	if (Math::is_zero_approx(denominator)) {
		// Lines are parallel, handling it as a special case.
		return p_line1_a;
	}
	const real_t line1_factor = (direction_dot * line2_projection - line2_len_sq * line1_projection) / denominator;
	const Vector4 closest_point_on_line1 = p_line1_a + line1_dir * CLAMP(line1_factor, (real_t)0.0, (real_t)1.0);
	return closest_point_on_line1;
}

PackedVector4Array Geometry4D::closest_points_between_line_segments(const Vector4 &p_line1_a, const Vector4 &p_line1_b, const Vector4 &p_line2_a, const Vector4 &p_line2_b) {
	const Vector4 difference_between_points = p_line1_a - p_line2_a;
	const Vector4 line1_dir = p_line1_b - p_line1_a;
	const Vector4 line2_dir = p_line2_b - p_line2_a;
	const real_t line1_len_sq = line1_dir.length_squared();
	const real_t line2_len_sq = line2_dir.length_squared();
	const real_t line1_projection = line1_dir.dot(difference_between_points);
	const real_t line2_projection = line2_dir.dot(difference_between_points);
	const real_t direction_dot = line1_dir.dot(line2_dir);
	const real_t denominator = line1_len_sq * line2_len_sq - direction_dot * direction_dot;
	if (Math::is_zero_approx(denominator)) {
		// Lines are parallel, handling it as a special case.
		return PackedVector4Array{ p_line1_a, p_line2_a };
	}
	const real_t line1_factor = (direction_dot * line2_projection - line2_len_sq * line1_projection) / denominator;
	const real_t line2_factor = (line1_len_sq * line2_projection - direction_dot * line1_projection) / denominator;
	const Vector4 closest_point_on_line1 = p_line1_a + line1_dir * CLAMP(line1_factor, (real_t)0.0, (real_t)1.0);
	const Vector4 closest_point_on_line2 = p_line2_a + line2_dir * CLAMP(line2_factor, (real_t)0.0, (real_t)1.0);
	return PackedVector4Array{ closest_point_on_line1, closest_point_on_line2 };
}

PackedVector4Array Geometry4D::closest_points_between_line_and_segment(const Vector4 &p_line_point, const Vector4 &p_line_direction, const Vector4 &p_segment_a, const Vector4 &p_segment_b) {
	const Vector4 difference_between_points = p_line_point - p_segment_a;
	const Vector4 segment_dir = p_segment_b - p_segment_a;
	const real_t segment_len_sq = segment_dir.length_squared();
	const real_t line_projection = p_line_direction.dot(difference_between_points);
	const real_t segment_projection = segment_dir.dot(difference_between_points);
	const real_t direction_dot = p_line_direction.dot(segment_dir);
	const real_t denominator = p_line_direction.length_squared() * segment_len_sq - direction_dot * direction_dot;
	if (Math::is_zero_approx(denominator)) {
		// Lines are parallel, handling it as a special case.
		return PackedVector4Array{ p_line_point, p_segment_a };
	}
	const real_t line_factor = (direction_dot * segment_projection - segment_len_sq * line_projection) / denominator;
	const real_t segment_factor = (p_line_direction.length_squared() * segment_projection - direction_dot * line_projection) / denominator;
	const Vector4 closest_point_on_line = p_line_point + p_line_direction * line_factor;
	const Vector4 closest_point_on_segment = p_segment_a + segment_dir * CLAMP(segment_factor, (real_t)0.0, (real_t)1.0);
	return PackedVector4Array{ closest_point_on_line, closest_point_on_segment };
}

Vector4 Geometry4D::perpendicular_to_three_vectors(const Vector4 &p_a, const Vector4 &p_b, const Vector4 &p_c) {
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

Geometry4D *Geometry4D::singleton = nullptr;

void Geometry4D::_bind_methods() {
	ClassDB::bind_static_method("Geometry4D", D_METHOD("closest_point_on_line", "line_position", "line_direction", "point"), &Geometry4D::closest_point_on_line);
	ClassDB::bind_static_method("Geometry4D", D_METHOD("closest_point_on_line_segment", "line_a", "line_b", "point"), &Geometry4D::closest_point_on_line_segment);
	ClassDB::bind_static_method("Geometry4D", D_METHOD("closest_point_on_ray", "ray_origin", "ray_direction", "point"), &Geometry4D::closest_point_on_ray);
	ClassDB::bind_static_method("Geometry4D", D_METHOD("closest_point_between_lines", "line1_point", "line1_dir", "line2_point", "line2_dir"), &Geometry4D::closest_point_between_lines);
	ClassDB::bind_static_method("Geometry4D", D_METHOD("closest_point_between_line_segments", "line1_a", "line1_b", "line2_a", "line2_b"), &Geometry4D::closest_point_between_line_segments);
	ClassDB::bind_static_method("Geometry4D", D_METHOD("closest_points_between_lines", "line1_point", "line1_dir", "line2_point", "line2_dir"), &Geometry4D::closest_points_between_lines);
	ClassDB::bind_static_method("Geometry4D", D_METHOD("closest_points_between_line_segments", "line1_a", "line1_b", "line2_a", "line2_b"), &Geometry4D::closest_points_between_line_segments);
	ClassDB::bind_static_method("Geometry4D", D_METHOD("closest_points_between_line_and_segment", "line_point", "line_direction", "segment_a", "segment_b"), &Geometry4D::closest_points_between_line_and_segment);
	ClassDB::bind_static_method("Geometry4D", D_METHOD("perpendicular_to_three_vectors", "a", "b", "c"), &Geometry4D::perpendicular_to_three_vectors);
}
