#pragma once

#include "../godot_4d_defines.h"

// Static helper class for misc 4D geometry functions.
class Geometry4D : public Object {
	GDCLASS(Geometry4D, Object);

protected:
	static Geometry4D *singleton;
	static void _bind_methods();

public:
	static Vector4 closest_point_on_line(const Vector4 &p_line_position, const Vector4 &p_line_direction, const Vector4 &p_point);
	static Vector4 closest_point_on_line_segment(const Vector4 &p_line_a, const Vector4 &p_line_b, const Vector4 &p_point);
	static Vector4 closest_point_on_ray(const Vector4 &p_ray_origin, const Vector4 &p_ray_direction, const Vector4 &p_point);
	static Vector4 closest_point_between_lines(const Vector4 &p_line1_point, const Vector4 &p_line1_dir, const Vector4 &p_line2_point, const Vector4 &p_line2_dir);
	static Vector4 closest_point_between_line_segments(const Vector4 &p_line1_a, const Vector4 &p_line1_b, const Vector4 &p_line2_a, const Vector4 &p_line2_b);
	static PackedVector4Array closest_points_between_lines(const Vector4 &p_line1_point, const Vector4 &p_line1_dir, const Vector4 &p_line2_point, const Vector4 &p_line2_dir);
	static PackedVector4Array closest_points_between_line_segments(const Vector4 &p_line1_a, const Vector4 &p_line1_b, const Vector4 &p_line2_a, const Vector4 &p_line2_b);
	static PackedVector4Array closest_points_between_line_and_segment(const Vector4 &p_line_point, const Vector4 &p_line_direction, const Vector4 &p_segment_a, const Vector4 &p_segment_b);

	static Geometry4D *get_singleton() { return singleton; }
	Geometry4D() { singleton = this; }
	~Geometry4D() { singleton = nullptr; }
};
