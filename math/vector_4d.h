#pragma once

#include "../godot_4d_defines.h"

// Stateless helper class to bind methods for Vector4.
class Vector4D : public Object {
	GDCLASS(Vector4D, Object);

protected:
	static Vector4D *singleton;
	static void _bind_methods();

public:
	static real_t angle_to(const Vector4 &p_from, const Vector4 &p_to);
	static Vector4 bounce(const Vector4 &p_vector, const Vector4 &p_normal);
	static real_t cross(const Vector4 &p_a, const Vector4 &p_b);
	static Vector4 limit_length(const Vector4 &p_vector, const real_t p_len = 1.0);
	static Vector4 limit_length_taxicab(const Vector4 &p_vector, const real_t p_len = 1.0);
	static Vector4 move_toward(const Vector4 &p_from, const Vector4 &p_to, const real_t p_delta);
	static Vector4 project(const Vector4 &p_vector, const Vector4 &p_on_normal);
	static Vector4 reflect(const Vector4 &p_vector, const Vector4 &p_normal);
	static Vector4 slide(const Vector4 &p_vector, const Vector4 &p_normal);

	// Conversion.
	static Vector4 from_3d(const Vector3 &p_vector);
	static Vector3 to_3d(const Vector4 &p_vector);

	Vector4D() { singleton = this; }
	~Vector4D() { singleton = nullptr; }
};
