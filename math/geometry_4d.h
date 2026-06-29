#pragma once

#include "../godot_4d_defines.h"

// Static helper class for misc 4D geometry functions.
class Geometry4D : public Object {
	GDCLASS(Geometry4D, Object);

protected:
	static Geometry4D *singleton;
	static void _bind_methods();

public:
	// Barycentric tetrahedra calculations. Don't expose these, it just needs to be efficient
	// and shared between TetraMesh4D, ConcaveMeshShape4D, ConvexHullShape4D, etc.
	static bool compute_inverse_metric_3x3(const real_t p_g00, const real_t p_g01, const real_t p_g02, const real_t p_g11, const real_t p_g12, const real_t p_g22, real_t r_inv_symmetric[6]);
	static void get_nearest_point_on_tetrahedron_barycentric(const Vector4 &p_vert0, const Vector4 &p_vert1, const Vector4 &p_vert2, const Vector4 &p_vert3, const Vector4 &p_point, const PackedFloat64Array &p_nearest_tetra_inverse_metric_cache, const int64_t p_tetrahedron_index, Vector4 &r_nearest_on_tet, real_t &r_distance_squared, bool &r_proj_inside);
	static bool is_point_inside_tetrahedron_barycentric(const Vector4 &p_vert0, const Vector4 &p_vert1, const Vector4 &p_vert2, const Vector4 &p_vert3, const Vector4 &p_point, const PackedFloat64Array &p_nearest_tetra_inverse_metric_cache, const int64_t p_tetrahedron_index);

	// Point-line calculations.
	static Vector4 closest_point_on_line(const Vector4 &p_line_position, const Vector4 &p_line_direction, const Vector4 &p_point);
	static Vector4 closest_point_on_line_segment(const Vector4 &p_line_a, const Vector4 &p_line_b, const Vector4 &p_point);
	static Vector4 closest_point_on_ray(const Vector4 &p_ray_origin, const Vector4 &p_ray_direction, const Vector4 &p_point);
	// Point-shape calculations.
	static Vector4 closest_point_on_triangle(const Vector4 &p_triangle_a, const Vector4 &p_triangle_b, const Vector4 &p_triangle_c, const Vector4 &p_point);
	// Line-line calculations.
	static Vector4 closest_point_between_lines(const Vector4 &p_line1_point, const Vector4 &p_line1_dir, const Vector4 &p_line2_point, const Vector4 &p_line2_dir);
	static Vector4 closest_point_between_line_segments(const Vector4 &p_line1_a, const Vector4 &p_line1_b, const Vector4 &p_line2_a, const Vector4 &p_line2_b);
	static PackedVector4Array closest_points_between_lines(const Vector4 &p_line1_point, const Vector4 &p_line1_dir, const Vector4 &p_line2_point, const Vector4 &p_line2_dir);
	static PackedVector4Array closest_points_between_line_segments(const Vector4 &p_line1_a, const Vector4 &p_line1_b, const Vector4 &p_line2_a, const Vector4 &p_line2_b);
	static PackedVector4Array closest_points_between_line_and_segment(const Vector4 &p_line_point, const Vector4 &p_line_direction, const Vector4 &p_segment_a, const Vector4 &p_segment_b);

	static Geometry4D *get_singleton() { return singleton; }
	Geometry4D() { singleton = this; }
	~Geometry4D() { singleton = nullptr; }
};
