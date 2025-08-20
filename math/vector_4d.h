#pragma once

#include "../godot_4d_defines.h"

// Stateless helper class to bind methods for Vector4.
class Vector4D : public Object {
	GDCLASS(Vector4D, Object);

protected:
	static Vector4D *singleton;
	static void _bind_methods();

public:
	// These are a superset of the directions found in Godot's Vector3 type.
	// These align with the G4MF specification: https://github.com/godot-dimensions/g4mf/blob/main/specification/parts/coordinate_system.md
#if USE_CONST_NOT_CONSTEXPR_FOR_VECTORS
	static const Vector4 ZERO;
	static const Vector4 ONE;

	static const Vector4 DIR_RIGHT;
	static const Vector4 DIR_LEFT;
	static const Vector4 DIR_UP;
	static const Vector4 DIR_DOWN;
	static const Vector4 DIR_BACK;
	static const Vector4 DIR_FORWARD;
	static const Vector4 DIR_ANA;
	static const Vector4 DIR_KATA;

	static const Vector4 MODEL_LEFT_SIDE;
	static const Vector4 MODEL_RIGHT_SIDE;
	static const Vector4 MODEL_TOP_SIDE;
	static const Vector4 MODEL_BOTTOM_SIDE;
	static const Vector4 MODEL_FRONT_SIDE;
	static const Vector4 MODEL_REAR_SIDE;
	static const Vector4 MODEL_ANA_SIDE;
	static const Vector4 MODEL_KATA_SIDE;

	static const Vector4 CARDINAL_EAST;
	static const Vector4 CARDINAL_WEST;
	static const Vector4 CARDINAL_ZENITH;
	static const Vector4 CARDINAL_NADIR;
	static const Vector4 CARDINAL_SOUTH;
	static const Vector4 CARDINAL_NORTH;
	static const Vector4 CARDINAL_ANTH;
	static const Vector4 CARDINAL_KENTH;
#else
	static constexpr Vector4 ZERO = Vector4(0, 0, 0, 0);
	static constexpr Vector4 ONE = Vector4(1, 1, 1, 1);

	static constexpr Vector4 DIR_RIGHT = Vector4(1, 0, 0, 0);
	static constexpr Vector4 DIR_LEFT = Vector4(-1, 0, 0, 0);
	static constexpr Vector4 DIR_UP = Vector4(0, 1, 0, 0);
	static constexpr Vector4 DIR_DOWN = Vector4(0, -1, 0, 0);
	static constexpr Vector4 DIR_BACK = Vector4(0, 0, 1, 0);
	static constexpr Vector4 DIR_FORWARD = Vector4(0, 0, -1, 0);
	static constexpr Vector4 DIR_ANA = Vector4(0, 0, 0, 1);
	static constexpr Vector4 DIR_KATA = Vector4(0, 0, 0, -1);

	static constexpr Vector4 MODEL_LEFT_SIDE = Vector4(1, 0, 0, 0);
	static constexpr Vector4 MODEL_RIGHT_SIDE = Vector4(-1, 0, 0, 0);
	static constexpr Vector4 MODEL_TOP_SIDE = Vector4(0, 1, 0, 0);
	static constexpr Vector4 MODEL_BOTTOM_SIDE = Vector4(0, -1, 0, 0);
	static constexpr Vector4 MODEL_FRONT_SIDE = Vector4(0, 0, 1, 0);
	static constexpr Vector4 MODEL_REAR_SIDE = Vector4(0, 0, -1, 0);
	static constexpr Vector4 MODEL_ANA_SIDE = Vector4(0, 0, 0, 1);
	static constexpr Vector4 MODEL_KATA_SIDE = Vector4(0, 0, 0, -1);

	static constexpr Vector4 CARDINAL_EAST = Vector4(1, 0, 0, 0);
	static constexpr Vector4 CARDINAL_WEST = Vector4(-1, 0, 0, 0);
	static constexpr Vector4 CARDINAL_ZENITH = Vector4(0, 1, 0, 0);
	static constexpr Vector4 CARDINAL_NADIR = Vector4(0, -1, 0, 0);
	static constexpr Vector4 CARDINAL_SOUTH = Vector4(0, 0, 1, 0);
	static constexpr Vector4 CARDINAL_NORTH = Vector4(0, 0, -1, 0);
	static constexpr Vector4 CARDINAL_ANTH = Vector4(0, 0, 0, 1);
	static constexpr Vector4 CARDINAL_KENTH = Vector4(0, 0, 0, -1);
#endif

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
