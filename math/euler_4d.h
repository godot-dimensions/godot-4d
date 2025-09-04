#pragma once

#include "basis_4d.h"

#if GDEXTENSION
#include <godot_cpp/variant/aabb.hpp>
#elif GODOT_MODULE
#include "core/math/aabb.h"
#endif

// The conventions for which rotations are Yaw/Pitch/Roll assumes you are
// using X=left/right, Y=up/down, Z=forward/back, and W=wint/zant.
// Whether +Z or -Z is forward does not matter for the rotation names.
struct _NO_DISCARD_ Euler4D {
	union {
		struct {
			// YZ, ZX, and XY are analogous to 3D (X, Y, and Z resp), but
			// XW, WY, and ZW are not (they are perpendicular to YZ, ZX, XY).
			// We'll put the 3D-like ones first, on top, in YZ-ZX-ZY order.
			// For UX reasons, we want the 3D-like ones to be first.
			// Yes I'm aware of the irony of trying to have good UX
			// for 4D, which is inherently complex regardless.
			real_t yz; // 4th rotation, Pitch,  Yaw3-perpendicular
			real_t zx; // 1st rotation, Yaw 1, Roll2-perpendicular
			real_t xy; // 5th rotation, Roll 1, Yaw2-perpendicular
			real_t xw; // 3rd rotation, Yaw 3, Pitch-perpendicular
			real_t wy; // 6th rotation, Roll 2, Yaw1-perpendicular
			real_t zw; // 2nd rotation, Yaw 2, Roll1-perpendicular
		};

		real_t elements[6] = { 0 };
	};

	// Misc methods.
	Euler4D compose(const Euler4D &p_child) const;
	bool is_equal_approx(const Euler4D &p_other) const;
	Basis4D rotate_basis(const Basis4D &p_basis) const;
	Vector4 rotate_point(const Vector4 &p_point) const;
	Euler4D rotation_to(const Euler4D &p_to) const;
	Euler4D snapped(const double p_step) const;
	Euler4D wrapped() const;

	// Radians/degrees.
	Euler4D deg_to_rad() const;
	Euler4D rad_to_deg() const;

	// Component with-style setters.
	Euler4D with_yz(const real_t p_yz) const {
		Euler4D e = *this;
		e.yz = p_yz;
		return e;
	}
	Euler4D with_zx(const real_t p_zx) const {
		Euler4D e = *this;
		e.zx = p_zx;
		return e;
	}
	Euler4D with_xy(const real_t p_xy) const {
		Euler4D e = *this;
		e.xy = p_xy;
		return e;
	}
	Euler4D with_xw(const real_t p_xw) const {
		Euler4D e = *this;
		e.xw = p_xw;
		return e;
	}
	Euler4D with_wy(const real_t p_wy) const {
		Euler4D e = *this;
		e.wy = p_wy;
		return e;
	}
	Euler4D with_zw(const real_t p_zw) const {
		Euler4D e = *this;
		e.zw = p_zw;
		return e;
	}

	// Operators.
	_FORCE_INLINE_ const real_t &operator[](int p_axis) const {
		DEV_ASSERT((unsigned int)p_axis < 6);
		return elements[p_axis];
	}

	_FORCE_INLINE_ real_t &operator[](int p_axis) {
		DEV_ASSERT((unsigned int)p_axis < 6);
		return elements[p_axis];
	}

	bool operator==(const Euler4D &p_v) const;
	bool operator!=(const Euler4D &p_v) const;
	Euler4D operator-() const;

	Euler4D &operator+=(const Euler4D &p_v);
	Euler4D operator+(const Euler4D &p_v) const;
	Euler4D &operator-=(const Euler4D &p_v);
	Euler4D operator-(const Euler4D &p_v) const;

	Euler4D &operator*=(const Euler4D &p_child);
	Euler4D operator*(const Euler4D &p_child) const;
	Euler4D &operator/=(const Euler4D &p_child);
	Euler4D operator/(const Euler4D &p_child) const;

	Euler4D &operator*=(const real_t p_scalar);
	Euler4D operator*(const real_t p_scalar) const;
	Euler4D &operator/=(const real_t p_scalar);
	Euler4D operator/(const real_t p_scalar) const;

	operator String() const;

	// Conversion.
	static Euler4D from_3d(const Vector3 &p_from_3d);
	Vector3 to_3d() const;
	static Euler4D from_array(const PackedRealArray &p_from_array);
	PackedRealArray to_array() const;
	static Euler4D from_basis(const Basis4D &p_basis, const bool p_use_special_cases = true);
	Basis4D to_basis() const;
	// For binding to Variant, pick an existing Variant data type.
	// AABB has 6 real_t fields so it will work fine.
	operator AABB() const;

	// Constructors.
	Euler4D() {}
	Euler4D(const AABB &p_from);
	Euler4D(const Vector3 &p_yz_zx_xy, const Vector3 p_xw_wy_zw = Vector3()) {
		yz = p_yz_zx_xy.x;
		zx = p_yz_zx_xy.y;
		xy = p_yz_zx_xy.z;
		xw = p_xw_wy_zw.x;
		wy = p_xw_wy_zw.y;
		zw = p_xw_wy_zw.z;
	}
	Euler4D(const real_t p_yz, const real_t p_zx, const real_t p_xy, const real_t p_xw = 0, const real_t p_wy = 0, const real_t p_zw = 0) {
		yz = p_yz;
		zx = p_zx;
		xy = p_xy;
		xw = p_xw;
		wy = p_wy;
		zw = p_zw;
	}
};

static_assert(sizeof(Euler4D) == sizeof(real_t) * 6);
