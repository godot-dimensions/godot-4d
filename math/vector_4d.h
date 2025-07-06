#pragma once

#include "../godot_4d_defines.h"

// Stateless helper class to bind methods for Vector4.
class Vector4D : public Object {
	GDCLASS(Vector4D, Object);

protected:
	static Vector4D *singleton;
	static void _bind_methods();

public:
	// Cosmetic functions.
	static Color axis_color(int64_t p_axis);
	static String axis_letter(int64_t p_axis);

	// Vector math.
	static real_t angle_to(const Vector4 &p_from, const Vector4 &p_to);
	static Vector4 bounce(const Vector4 &p_vector, const Vector4 &p_normal);
	static Vector4 bounce_ratio(const Vector4 &p_vector, const Vector4 &p_normal, const real_t p_bounce_ratio);
	static real_t cross(const Vector4 &p_a, const Vector4 &p_b);
	static bool is_uniform(const Vector4 &p_vector);
	static Vector4 limit_length(const Vector4 &p_vector, const real_t p_len = 1.0);
	static Vector4 limit_length_taxicab(const Vector4 &p_vector, const real_t p_len = 1.0);
	static Vector4 move_toward(const Vector4 &p_from, const Vector4 &p_to, const real_t p_delta);
	static Vector4 perpendicular(const Vector4 &p_a, const Vector4 &p_b, const Vector4 &p_c);
	static Vector4 project(const Vector4 &p_vector, const Vector4 &p_on_normal);
	static Vector4 reflect(const Vector4 &p_vector, const Vector4 &p_normal);
	static Vector4 rotate_in_plane(const Vector4 &p_vector, const Vector4 &p_plane_from, const Vector4 &p_plane_to, const real_t p_angle);
	static Vector4 slide(const Vector4 &p_vector, const Vector4 &p_normal);

	// Generation.
	static Vector4 random_in_radius(const real_t p_radius = 1.0);
	static Vector4 random_in_range(const Vector4 &p_from = Vector4(0, 0, 0, 0), const Vector4 &p_to = Vector4(1, 1, 1, 1));

	// Conversion.
	static Vector4 from_color(const Color &p_color) { return Vector4(p_color.r, p_color.g, p_color.b, p_color.a); }
	static Color to_color(const Vector4 &p_vector) { return Color(p_vector.x, p_vector.y, p_vector.z, p_vector.w); }
	static Vector4 from_3d(const Vector3 &p_vector, const real_t p_w = 0.0) { return Vector4(p_vector.x, p_vector.y, p_vector.z, p_w); }
	static Vector3 to_3d(const Vector4 &p_vector) { return Vector3(p_vector.x, p_vector.y, p_vector.z); }
	static Vector4 from_json_array(const Array &p_json_array);
	static Array to_json_array(const Vector4 &p_vector);

	static Vector4D *get_singleton() { return singleton; }
	Vector4D() { singleton = this; }
	~Vector4D() { singleton = nullptr; }
};
