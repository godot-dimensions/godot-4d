#pragma once

#include "../godot_4d_defines.h"

#include "euler_4d_bind.h"
#include "geometric_algebra/rotor_4d_bind.h"

#if GDEXTENSION
#include <godot_cpp/variant/projection.hpp>
#elif GODOT_MODULE
#include "core/math/projection.h"
#endif

namespace godot_4d_bind {
// Stateless helper class to bind methods for Basis4D.
class Basis4D : public Object {
	GDCLASS(Basis4D, Object);

protected:
	static Basis4D *singleton;
	static void _bind_methods();

public:
	// Misc methods.
	static real_t determinant(const Projection &p_basis);
	static bool is_equal_approx(const Projection &p_a, const Projection &p_b);

	// Interpolation.
	static Projection lerp(const Projection &p_from, const Projection &p_to, const real_t p_weight);
	static Projection slerp(const Projection &p_from, const Projection &p_to, const real_t p_weight);
	static Projection slerp_rotation(const Projection &p_from, const Projection &p_to, const real_t p_weight);

	// Transformation methods.
	static Projection compose(const Projection &p_parent, const Projection &p_child);
	static Projection transform_to(const Projection &p_from, const Projection &p_to);

	static Vector4 xform(const Projection &p_basis, const Vector4 &p_vector);
	static Vector4 xform_inv(const Projection &p_basis, const Vector4 &p_vector);
	static Vector4 xform_transposed(const Projection &p_basis, const Vector4 &p_vector);
	static AABB rotate_bivector(const Projection &p_basis, const AABB &p_bivector);

	// Inversion methods.
	static Projection inverse(const Projection &p_basis);
	static Projection transposed(const Projection &p_basis);

	// Scale methods.
	static Projection scaled_global(const Projection &p_basis, const Vector4 &p_scale);
	static Projection scaled_local(const Projection &p_basis, const Vector4 &p_scale);
	static Projection scaled_uniform(const Projection &p_basis, const real_t p_scale);
	static Vector4 get_scale(const Projection &p_basis);
	static Vector4 get_scale_abs(const Projection &p_basis);
	static real_t get_uniform_scale(const Projection &p_basis);

	// Validation methods.
	static Projection orthonormalized(const Projection &p_basis);
	static Projection orthogonalized(const Projection &p_basis);
	static bool is_orthogonal(const Projection &p_basis);
	static bool is_orthonormal(const Projection &p_basis);
	static bool is_conformal(const Projection &p_basis);
	static bool is_diagonal(const Projection &p_basis);
	static bool is_rotation(const Projection &p_basis);

	// Helper setters/getters.
	static Vector4 get_column(const Projection &p_basis, int p_index);
	static Vector4 get_row(const Projection &p_basis, int p_index);
	static Vector4 get_main_diagonal(const Projection &p_basis);

	// Helper Euler4D and Rotor4D functions. Can't be done internally
	// because of dependency order but exposed for convenience.
	static Projection from_euler(const Ref<Euler4D> &p_euler);
	static Projection from_euler_aabb(const AABB &p_euler);
	static Ref<Euler4D> to_euler(const Projection &p_basis, const bool p_use_special_cases = true);
	static AABB to_euler_aabb(const Projection &p_basis, const bool p_use_special_cases = true);
	static Projection from_rotor(const Ref<Rotor4D> &p_rotor);
	static Ref<Rotor4D> to_rotor(const Projection &p_basis);

	// Conversion.
	static Projection from_3d(const Basis &p_from_3d);
	static Basis to_3d(const Projection &p_from_4d, const bool p_orthonormalized = false);
	static Projection from_array(const PackedRealArray &p_from_array);
	static PackedRealArray to_array(const Projection &p_from_basis);

	// Constructors.
	static Projection from_scale(const Vector4 &p_scale);
	static Projection from_scale_uniform(const real_t p_scale);
	static Projection from_yz(const real_t p_yz);
	static Projection from_zx(const real_t p_zx);
	static Projection from_xy(const real_t p_xy);
	static Projection from_xw(const real_t p_xw);
	static Projection from_wy(const real_t p_wy);
	static Projection from_zw(const real_t p_zw);

	static Basis4D *get_singleton() { return singleton; }
	Basis4D() { singleton = this; }
	~Basis4D() { singleton = nullptr; }
};
} // namespace godot_4d_bind
