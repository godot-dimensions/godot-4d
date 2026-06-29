#include "geometry_4d.h"

#include "vector_4d.h"

// Barycentric tetrahedra calculations. Don't expose these, it just needs to be efficient
// and shared between TetraMesh4D, ConcaveMeshShape4D, ConvexHullShape4D, etc.

bool Geometry4D::compute_inverse_metric_3x3(const real_t p_g00, const real_t p_g01, const real_t p_g02, const real_t p_g11, const real_t p_g12, const real_t p_g22, real_t r_inv_symmetric[6]) {
	// This is the 3x3 symmetric metric matrix G_ij = e_i · e_j for the tetrahedron basis.
	// We store only its six unique entries and invert that packed matrix.
	const real_t c00 = p_g11 * p_g22 - p_g12 * p_g12;
	const real_t c01 = p_g02 * p_g12 - p_g01 * p_g22;
	const real_t c02 = p_g01 * p_g12 - p_g02 * p_g11;
	const real_t c11 = p_g00 * p_g22 - p_g02 * p_g02;
	const real_t c12 = p_g02 * p_g01 - p_g00 * p_g12;
	const real_t c22 = p_g00 * p_g11 - p_g01 * p_g01;
	const real_t det = p_g00 * c00 + p_g01 * c01 + p_g02 * c02;
	if (!Math::is_finite(det) || Math::is_equal_approx(det, (real_t)0.0)) {
		return false;
	}
	const real_t inv_det = (real_t)1.0 / det;
	r_inv_symmetric[0] = c00 * inv_det;
	r_inv_symmetric[1] = c01 * inv_det;
	r_inv_symmetric[2] = c02 * inv_det;
	r_inv_symmetric[3] = c11 * inv_det;
	r_inv_symmetric[4] = c12 * inv_det;
	r_inv_symmetric[5] = c22 * inv_det;
	return true;
}

void Geometry4D::get_nearest_point_on_tetrahedron_barycentric(const Vector4 &p_vert0, const Vector4 &p_vert1, const Vector4 &p_vert2, const Vector4 &p_vert3, const Vector4 &p_point, const PackedFloat64Array &p_nearest_tetra_inverse_metric_cache, const int64_t p_tetrahedron_index, Vector4 &r_nearest_on_tet, real_t &r_distance_squared, bool &r_proj_inside) {
	ERR_FAIL_COND_MSG(p_nearest_tetra_inverse_metric_cache.size() < p_tetrahedron_index * 6 + 6, "Geometry4D::get_nearest_point_on_tetrahedron_barycentric: Inverse metric cache is too small for the given tetrahedron index.");
	// These indices are guaranteed to be within bounds due to mesh validation.
	const Vector4 edge1 = p_vert1 - p_vert0;
	const Vector4 edge2 = p_vert2 - p_vert0;
	const Vector4 edge3 = p_vert3 - p_vert0;
	// Solve for the barycentric coordinates, which implicitly solves local to the plane of the tetrahedron.
	real_t bary0, bary1, bary2, bary3;
	{
		const int64_t cache_offset = p_tetrahedron_index * 6;
		const Vector4 local = p_point - p_vert0;
		const real_t edge1_alignment = edge1.dot(local);
		const real_t edge2_alignment = edge2.dot(local);
		const real_t edge3_alignment = edge3.dot(local);
		const real_t inv00 = p_nearest_tetra_inverse_metric_cache[cache_offset + 0];
		const real_t inv01 = p_nearest_tetra_inverse_metric_cache[cache_offset + 1];
		const real_t inv02 = p_nearest_tetra_inverse_metric_cache[cache_offset + 2];
		const real_t inv11 = p_nearest_tetra_inverse_metric_cache[cache_offset + 3];
		const real_t inv12 = p_nearest_tetra_inverse_metric_cache[cache_offset + 4];
		const real_t inv22 = p_nearest_tetra_inverse_metric_cache[cache_offset + 5];
		bary1 = inv00 * edge1_alignment + inv01 * edge2_alignment + inv02 * edge3_alignment;
		bary2 = inv01 * edge1_alignment + inv11 * edge2_alignment + inv12 * edge3_alignment;
		bary3 = inv02 * edge1_alignment + inv12 * edge2_alignment + inv22 * edge3_alignment;
		bary0 = (real_t)1.0 - (bary1 + bary2 + bary3);
	}
	// Determine the nearest point and/or the min distance based on if it's inside or outside the tetrahedron.
	Vector4 nearest_on_tet;
	real_t min_dist_sq = Math_INF;
	if (bary0 >= -CMP_EPSILON && bary1 >= -CMP_EPSILON && bary2 >= -CMP_EPSILON && bary3 >= -CMP_EPSILON) {
		// In this case, the nearest point on the plane lands inside of the tetrahedron.
		r_proj_inside = true;
		nearest_on_tet = p_vert0 + edge1 * bary1 + edge2 * bary2 + edge3 * bary3;
	} else {
		r_proj_inside = false;
		// In this case, the nearest point on the plane lands outside, so we need to check the triangle borders.
		const Vector4 nearest_on_tri0 = Geometry4D::closest_point_on_triangle(p_vert1, p_vert2, p_vert3, p_point);
		const Vector4 nearest_on_tri1 = Geometry4D::closest_point_on_triangle(p_vert0, p_vert2, p_vert3, p_point);
		const Vector4 nearest_on_tri2 = Geometry4D::closest_point_on_triangle(p_vert0, p_vert1, p_vert3, p_point);
		const Vector4 nearest_on_tri3 = Geometry4D::closest_point_on_triangle(p_vert0, p_vert1, p_vert2, p_point);
		nearest_on_tet = nearest_on_tri0;
		min_dist_sq = nearest_on_tri0.distance_squared_to(p_point);
		const real_t dist_sq_tri1 = nearest_on_tri1.distance_squared_to(p_point);
		if (dist_sq_tri1 < min_dist_sq) {
			nearest_on_tet = nearest_on_tri1;
			min_dist_sq = dist_sq_tri1;
		}
		const real_t dist_sq_tri2 = nearest_on_tri2.distance_squared_to(p_point);
		if (dist_sq_tri2 < min_dist_sq) {
			nearest_on_tet = nearest_on_tri2;
			min_dist_sq = dist_sq_tri2;
		}
		const real_t dist_sq_tri3 = nearest_on_tri3.distance_squared_to(p_point);
		if (dist_sq_tri3 < min_dist_sq) {
			nearest_on_tet = nearest_on_tri3;
			min_dist_sq = dist_sq_tri3;
		}
	}
	// Write the outputs.
	r_nearest_on_tet = nearest_on_tet;
	if (min_dist_sq == Math_INF) {
		min_dist_sq = nearest_on_tet.distance_squared_to(p_point);
	}
	r_distance_squared = min_dist_sq;
}

bool Geometry4D::is_point_inside_tetrahedron_barycentric(const Vector4 &p_vert0, const Vector4 &p_vert1, const Vector4 &p_vert2, const Vector4 &p_vert3, const Vector4 &p_point, const PackedFloat64Array &p_nearest_tetra_inverse_metric_cache, const int64_t p_tetrahedron_index) {
	ERR_FAIL_COND_V_MSG(p_nearest_tetra_inverse_metric_cache.size() < p_tetrahedron_index * 6 + 6, false, "Geometry4D::get_nearest_point_on_tetrahedron_barycentric: Inverse metric cache is too small for the given tetrahedron index.");
	const Vector4 edge1 = p_vert1 - p_vert0;
	const Vector4 edge2 = p_vert2 - p_vert0;
	const Vector4 edge3 = p_vert3 - p_vert0;
	// Solve for the barycentric coordinates, which implicitly solves local to the plane of the tetrahedron.
	real_t bary0, bary1, bary2, bary3;
	{
		const int64_t cache_offset = p_tetrahedron_index * 6;
		const Vector4 local = p_point - p_vert0;
		const real_t edge1_alignment = edge1.dot(local);
		const real_t edge2_alignment = edge2.dot(local);
		const real_t edge3_alignment = edge3.dot(local);
		const real_t inv00 = p_nearest_tetra_inverse_metric_cache[cache_offset + 0];
		const real_t inv01 = p_nearest_tetra_inverse_metric_cache[cache_offset + 1];
		const real_t inv02 = p_nearest_tetra_inverse_metric_cache[cache_offset + 2];
		const real_t inv11 = p_nearest_tetra_inverse_metric_cache[cache_offset + 3];
		const real_t inv12 = p_nearest_tetra_inverse_metric_cache[cache_offset + 4];
		const real_t inv22 = p_nearest_tetra_inverse_metric_cache[cache_offset + 5];
		bary1 = inv00 * edge1_alignment + inv01 * edge2_alignment + inv02 * edge3_alignment;
		bary2 = inv01 * edge1_alignment + inv11 * edge2_alignment + inv12 * edge3_alignment;
		bary3 = inv02 * edge1_alignment + inv12 * edge2_alignment + inv22 * edge3_alignment;
		bary0 = (real_t)1.0 - (bary1 + bary2 + bary3);
	}
	// The point is inside the tetrahedron if all barycentric coordinates are non-negative (allowing for a small epsilon).
	return bary0 >= -CMP_EPSILON && bary1 >= -CMP_EPSILON && bary2 >= -CMP_EPSILON && bary3 >= -CMP_EPSILON;
}

// Point-line calculations.

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

// Point-shape calculations.

Vector4 Geometry4D::closest_point_on_triangle(const Vector4 &p_triangle_a, const Vector4 &p_triangle_b, const Vector4 &p_triangle_c, const Vector4 &p_point) {
	const Vector4 a_to_b = p_triangle_b - p_triangle_a;
	const Vector4 a_to_c = p_triangle_c - p_triangle_a;
	const Vector4 a_to_point = p_point - p_triangle_a;
	const real_t ab_ap = a_to_b.dot(a_to_point);
	const real_t ac_ap = a_to_c.dot(a_to_point);
	if (ab_ap <= (real_t)0.0 && ac_ap <= (real_t)0.0) {
		return p_triangle_a;
	}
	const Vector4 b_to_point = p_point - p_triangle_b;
	const real_t ab_bp = a_to_b.dot(b_to_point);
	const real_t ac_bp = a_to_c.dot(b_to_point);
	if (ab_bp >= (real_t)0.0 && ac_bp <= ab_bp) {
		return p_triangle_b;
	}
	const real_t vc = ab_ap * ac_bp - ab_bp * ac_ap;
	if (vc <= (real_t)0.0 && ab_ap >= (real_t)0.0 && ab_bp <= (real_t)0.0) {
		const real_t bary_edge_ab = ab_ap / (ab_ap - ab_bp);
		return p_triangle_a + a_to_b * bary_edge_ab;
	}
	const Vector4 c_to_point = p_point - p_triangle_c;
	const real_t ab_cp = a_to_b.dot(c_to_point);
	const real_t ac_cp = a_to_c.dot(c_to_point);
	if (ac_cp >= (real_t)0.0 && ab_cp <= ac_cp) {
		return p_triangle_c;
	}
	const real_t vb = ab_cp * ac_ap - ab_ap * ac_cp;
	if (vb <= (real_t)0.0 && ac_ap >= (real_t)0.0 && ac_cp <= (real_t)0.0) {
		const real_t bary_edge_ac = ac_ap / (ac_ap - ac_cp);
		return p_triangle_a + a_to_c * bary_edge_ac;
	}
	const real_t va = ab_bp * ac_cp - ab_cp * ac_bp;
	if (va <= (real_t)0.0 && (ac_bp - ab_bp) >= (real_t)0.0 && (ab_cp - ac_cp) >= (real_t)0.0) {
		const Vector4 b_to_c = p_triangle_c - p_triangle_b;
		const real_t bary_edge_bc = (ac_bp - ab_bp) / ((ac_bp - ab_bp) + (ab_cp - ac_cp));
		return p_triangle_b + b_to_c * bary_edge_bc;
	}
	const real_t denom = (real_t)1.0 / (va + vb + vc);
	const real_t bary_ab = vb * denom;
	const real_t bary_ac = vc * denom;
	return p_triangle_a + a_to_b * bary_ab + a_to_c * bary_ac;
}

// Line-line calculations.

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

Geometry4D *Geometry4D::singleton = nullptr;

void Geometry4D::_bind_methods() {
	// Point-line calculations.
	ClassDB::bind_static_method("Geometry4D", D_METHOD("closest_point_on_line", "line_position", "line_direction", "point"), &Geometry4D::closest_point_on_line);
	ClassDB::bind_static_method("Geometry4D", D_METHOD("closest_point_on_line_segment", "line_a", "line_b", "point"), &Geometry4D::closest_point_on_line_segment);
	ClassDB::bind_static_method("Geometry4D", D_METHOD("closest_point_on_ray", "ray_origin", "ray_direction", "point"), &Geometry4D::closest_point_on_ray);
	// Point-shape calculations.
	ClassDB::bind_static_method("Geometry4D", D_METHOD("closest_point_on_triangle", "triangle_a", "triangle_b", "triangle_c", "point"), &Geometry4D::closest_point_on_triangle);
	// Line-line calculations.
	ClassDB::bind_static_method("Geometry4D", D_METHOD("closest_point_between_lines", "line1_point", "line1_dir", "line2_point", "line2_dir"), &Geometry4D::closest_point_between_lines);
	ClassDB::bind_static_method("Geometry4D", D_METHOD("closest_point_between_line_segments", "line1_a", "line1_b", "line2_a", "line2_b"), &Geometry4D::closest_point_between_line_segments);
	ClassDB::bind_static_method("Geometry4D", D_METHOD("closest_points_between_lines", "line1_point", "line1_dir", "line2_point", "line2_dir"), &Geometry4D::closest_points_between_lines);
	ClassDB::bind_static_method("Geometry4D", D_METHOD("closest_points_between_line_segments", "line1_a", "line1_b", "line2_a", "line2_b"), &Geometry4D::closest_points_between_line_segments);
	ClassDB::bind_static_method("Geometry4D", D_METHOD("closest_points_between_line_and_segment", "line_point", "line_direction", "segment_a", "segment_b"), &Geometry4D::closest_points_between_line_and_segment);
}
