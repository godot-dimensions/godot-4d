#pragma once

#include "euler_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/projection.hpp>
#elif GODOT_MODULE
#include "core/math/projection.h"
#include "core/object/ref_counted.h"
#endif

namespace godot_4d_bind {
class Transform4D;
// Stateless helper class to bind methods for Euler4D.
class Euler4D : public RefCounted {
	GDCLASS(Euler4D, RefCounted);

	::Euler4D euler;

protected:
	static void _bind_methods();

public:
	// Instance methods, providing a friendly readable Euler4D object, which unfortunately uses lots of memory and is passed by reference.
	// Misc methods.
	Ref<Euler4D> add(const Ref<Euler4D> &p_other) const;
	Ref<Euler4D> compose(const Ref<Euler4D> &p_child) const;
	bool is_equal_approx(const Ref<Euler4D> &p_other) const;
	bool is_equal_exact(const Ref<Euler4D> &p_other) const;
	Ref<Euler4D> rotation_to(const Ref<Euler4D> &p_to) const;
	Ref<Euler4D> wrapped() const;

	// Radians/degrees.
	Ref<Euler4D> deg_to_rad() const;
	Ref<Euler4D> rad_to_deg() const;

	// Individual component getters/setters.
	real_t get_yz() const { return euler.yz; }
	real_t get_zx() const { return euler.zx; }
	real_t get_xy() const { return euler.xy; }
	real_t get_xw() const { return euler.xw; }
	real_t get_wy() const { return euler.wy; }
	real_t get_zw() const { return euler.zw; }
	void set_yz(const real_t p_yz) { euler.yz = p_yz; }
	void set_zx(const real_t p_zx) { euler.zx = p_zx; }
	void set_xy(const real_t p_xy) { euler.xy = p_xy; }
	void set_xw(const real_t p_xw) { euler.xw = p_xw; }
	void set_wy(const real_t p_wy) { euler.wy = p_wy; }
	void set_zw(const real_t p_zw) { euler.zw = p_zw; }
	Ref<Euler4D> with_yz(const real_t p_yz) const;
	Ref<Euler4D> with_zx(const real_t p_zx) const;
	Ref<Euler4D> with_xy(const real_t p_xy) const;
	Ref<Euler4D> with_xw(const real_t p_xw) const;
	Ref<Euler4D> with_wy(const real_t p_wy) const;
	Ref<Euler4D> with_zw(const real_t p_zw) const;

	// Helper getters/setters.
	::Euler4D get_euler() const { return euler; }
	void set_euler(const ::Euler4D &p_euler) { euler = p_euler; }

	// Conversion.
	static Ref<Euler4D> from_3d(const Vector3 &p_from_euler_3d);
	Vector3 to_3d() const;
	static Ref<Euler4D> from_array(const PackedRealArray &p_from_array);
	PackedRealArray to_array() const;
	static Ref<Euler4D> from_basis(const Projection &p_basis, const bool p_use_special_cases = true);
	Projection to_basis() const;
	static Ref<Euler4D> from_transform(const Ref<Transform4D> &p_transform, const bool p_use_special_cases = true);
	Ref<Transform4D> to_transform() const;
	static Ref<Euler4D> from_numbers(const real_t p_yz, const real_t p_zx, const real_t p_xy, const real_t p_xw = 0.0f, const real_t p_wy = 0.0f, const real_t p_zw = 0.0f);
	static Ref<Euler4D> from_vectors(const Vector3 &p_yz_zx_xy, const Vector3 &p_xw_wy_zw = Vector3());
	Ref<Euler4D> copy() const;
	virtual String to_string() override;

	// Constructors.
	Euler4D() {}
	Euler4D(const AABB &p_euler_aabb) { euler = ::Euler4D(p_euler_aabb); }
	Euler4D(const Vector3 &p_yz_zx_xy, const Vector3 p_xw_wy_zw = Vector3()) {
		euler.yz = p_yz_zx_xy.x;
		euler.zx = p_yz_zx_xy.y;
		euler.xy = p_yz_zx_xy.z;
		euler.xw = p_xw_wy_zw.x;
		euler.wy = p_xw_wy_zw.y;
		euler.zw = p_xw_wy_zw.z;
	}
	Euler4D(const real_t p_yz, const real_t p_zx, const real_t p_xy, const real_t p_xw = 0, const real_t p_wy = 0, const real_t p_zw = 0) {
		euler.yz = p_yz;
		euler.zx = p_zx;
		euler.xy = p_xy;
		euler.xw = p_xw;
		euler.wy = p_wy;
		euler.zw = p_zw;
	}

	// Static methods, allowing the user to use AABB to represent an Euler4D, which saves memory and is passed by value.
	// Misc methods.
	static AABB aabb_compose(const AABB &p_parent, const AABB &p_child);
	static bool aabb_is_equal_approx(const AABB &p_a, const AABB &p_b);
	static AABB aabb_rotation_to(const AABB &p_from, const AABB &p_to);
	static AABB aabb_wrapped(const AABB &p_euler_4d);

	// Radians/degrees.
	static AABB aabb_deg_to_rad(const AABB &p_euler_4d);
	static AABB aabb_rad_to_deg(const AABB &p_euler_4d);

	// Individual component getters.
	static real_t aabb_get_yz(const AABB &p_euler_4d);
	static real_t aabb_get_zx(const AABB &p_euler_4d);
	static real_t aabb_get_xy(const AABB &p_euler_4d);
	static real_t aabb_get_xw(const AABB &p_euler_4d);
	static real_t aabb_get_wy(const AABB &p_euler_4d);
	static real_t aabb_get_zw(const AABB &p_euler_4d);

	// Individual component with-style setters.
	static AABB aabb_with_yz(const AABB &p_euler_4d, const real_t p_yz);
	static AABB aabb_with_zx(const AABB &p_euler_4d, const real_t p_zx);
	static AABB aabb_with_xy(const AABB &p_euler_4d, const real_t p_xy);
	static AABB aabb_with_xw(const AABB &p_euler_4d, const real_t p_xw);
	static AABB aabb_with_wy(const AABB &p_euler_4d, const real_t p_wy);
	static AABB aabb_with_zw(const AABB &p_euler_4d, const real_t p_zw);

	// Conversion.
	static AABB aabb_from_3d(const Vector3 &p_from_euler_3d);
	static Vector3 aabb_to_3d(const AABB &p_from_euler_4d);
	static AABB aabb_from_array(const PackedRealArray &p_from_array);
	static PackedRealArray aabb_to_array(const AABB &p_from_euler_4d);
	static AABB aabb_from_basis(const Projection &p_from_basis_4d, const bool p_use_special_cases = true);
	static Projection aabb_to_basis(const AABB &p_from_euler_4d);

	// Constructors.
	static AABB aabb_from_numbers(const real_t p_yz, const real_t p_zx, const real_t p_xy, const real_t p_xw = 0.0f, const real_t p_wy = 0.0f, const real_t p_zw = 0.0f);
	static AABB aabb_from_vectors(const Vector3 &p_yz_zx_xy, const Vector3 &p_xw_wy_zw = Vector3());
};
} // namespace godot_4d_bind
