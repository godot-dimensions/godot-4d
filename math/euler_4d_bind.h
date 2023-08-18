#pragma once

#include "../godot_4d_defines.h"

#if GDEXTENSION
#include <godot_cpp/variant/projection.hpp>
#elif GODOT_MODULE
#include "core/math/projection.h"
#endif

namespace godot_4d_bind {
// Stateless helper class to bind methods for Euler4D.
class Euler4D : public Object {
	GDCLASS(Euler4D, Object);

protected:
	static Euler4D *singleton;
	static void _bind_methods();

public:
	// Misc methods.
	static AABB compose(const AABB &p_parent, const AABB &p_child);
	static bool is_equal_approx(const AABB &p_a, const AABB &p_b);
	static AABB rotation_to(const AABB &p_from, const AABB &p_to);
	static AABB wrapped(const AABB &p_euler_4d);

	// Radians/degrees.
	static AABB deg_to_rad(const AABB &p_euler_4d);
	static AABB rad_to_deg(const AABB &p_euler_4d);

	// Individual component getters.
	static real_t get_yz(const AABB &p_euler_4d);
	static real_t get_zx(const AABB &p_euler_4d);
	static real_t get_xy(const AABB &p_euler_4d);
	static real_t get_xw(const AABB &p_euler_4d);
	static real_t get_wy(const AABB &p_euler_4d);
	static real_t get_zw(const AABB &p_euler_4d);

	// Individual component with-style setters.
	static AABB with_yz(const AABB &p_euler_4d, const real_t p_yz);
	static AABB with_zx(const AABB &p_euler_4d, const real_t p_zx);
	static AABB with_xy(const AABB &p_euler_4d, const real_t p_xy);
	static AABB with_xw(const AABB &p_euler_4d, const real_t p_xw);
	static AABB with_wy(const AABB &p_euler_4d, const real_t p_wy);
	static AABB with_zw(const AABB &p_euler_4d, const real_t p_zw);

	// Conversion.
	static AABB from_3d(const Vector3 &p_from_euler_3d);
	static Vector3 to_3d(const AABB &p_from_euler_4d);
	static AABB from_array(const PackedRealArray &p_from_array);
	static PackedRealArray to_array(const AABB &p_from_euler_4d);
	static AABB from_basis(const Projection &p_from_basis_4d, const bool p_use_special_cases = true);
	static Projection to_basis(const AABB &p_from_euler_4d);

	// Constructors.
	static AABB from_numbers(const real_t p_yz, const real_t p_zx, const real_t p_xy, const real_t p_xw = 0.0f, const real_t p_wy = 0.0f, const real_t p_zw = 0.0f);
	static AABB from_vectors(const Vector3 &p_yz_zx_xy, const Vector3 &p_xw_wy_zw = Vector3());

	Euler4D() { singleton = this; }
	~Euler4D() { singleton = nullptr; }
};
} // namespace godot_4d_bind
