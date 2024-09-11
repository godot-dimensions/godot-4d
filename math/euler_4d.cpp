#include "euler_4d.h"

// Basis4D conversions.

Basis4D Euler4D::to_basis() const {
	Basis4D yz_basis = Basis4D::from_yz(yz);
	Basis4D zx_basis = Basis4D::from_zx(zx);
	Basis4D xy_basis = Basis4D::from_xy(xy);
	// It can be tricky to figure out if we should have ex: XW or WX.
	// From looking at the matrix, and trying to stay consistent with the
	// existing 3D rotations, we should use XW, WY, and ZW. The rules are
	// that adjacent axes have the upper-right sine negative (XY, YZ, ZW),
	// axes one apart have the lower-left sine negative (ZX, WY), and axes
	// two apart have the upper-right sine negative again (XW, not WX).
	// If you are curious, a similar pattern could be extrapolated to 5D etc.
	// 5D would be ZX, ZW, VZ, XW, VX, WV, YZ, XY, YV, WY.
	Basis4D xw_basis = Basis4D::from_xw(xw);
	Basis4D wy_basis = Basis4D::from_wy(wy);
	Basis4D zw_basis = Basis4D::from_zw(zw);

	// Here's what determines the rotation order of Euler angles.
	// For 3D, we have Yaw, Pitch, and Roll, in that order.
	//
	// Yaw acts as horizontal rotation (ZX plane, no Y so it's horizontal).
	// For the 4D equivalent of Yaw, we have three angles that rotate
	// horizontally (no Y): ZX, ZW, and XW. Therefore, these three go first.
	//
	// Next is Pitch. Pitch is vertical rotation that changes the direction
	// an object is pointing by moving its forward vector (-Z) up or down (Y),
	// so pitch only exists in the YZ plane. Therefore, for the 4D equivalent
	// of pitch, there is only one angle that does this: YZ.
	//
	// Lastly is Roll. Roll is also vertical rotation, but it brings the
	// sides of an object higher or lower. For the 4D equivalent of roll,
	// we want every angle that involves Y but not Z, so that gives us
	// two angles: XY and WY. Therefore, these two go last.
	//
	// Outside of the broad Yaw-Pitch-Roll ordering, there is a fascinating
	// symmetry that arises. If we order our rotations in the right way, we
	// can make the outer-most rotations (ZX and WY) be perpendicular,
	// the inner-most rotations (XW and YZ) be perpendicular, which makes the
	// remaining rotations (ZW and XY) also perpendicular. Pitch (YZ) must be
	// the 4th rotation so that implies XW is the 3rd rotation to obey the
	// symmetry. The 1st/2nd could be swapped if the 5th/6th are swapped,
	// so this part is arbitrary, but ZX first is consistent with 3D.
	//
	// Remember that matrix multiplication is thought of as right-to-left,
	// but also Euler angles are composed in reverse, so it cancels out.
	return zx_basis * zw_basis * xw_basis * yz_basis * xy_basis * wy_basis;
}

Euler4D Euler4D::from_basis(const Basis4D &p_basis, const bool p_use_special_cases) {
	// This process produces a LOT of floating-point error.
	// It's also likely that this could be optimized a lot.
	// With 32-bit floats, this has a margin of error of 0.1%.

	Basis4D basis = p_basis.orthonormalized();

	// Stage 0: Determine if this is a special case. This is optional, for UX.
	// This ensures that single rotations in the inspector don't transform into
	// other rotations. Godot does the same thing for 3D euler angles.
	if (p_use_special_cases) {
		if (Math::is_zero_approx(basis.y.z) && Math::is_zero_approx(basis.z.y) && Math::is_zero_approx(basis.z.x) && Math::is_zero_approx(basis.x.z) && Math::is_zero_approx(basis.x.w) && Math::is_zero_approx(basis.w.x) && Math::is_zero_approx(basis.w.y) && Math::is_zero_approx(basis.y.w)) {
			// Special case: No YZ, ZX, XW, or WY rotations, so the only rotations may be in XY or ZW.
			return Euler4D(0.0, 0.0, Math::atan2(basis.x.y, basis.x.x), 0.0, 0.0, Math::atan2(basis.z.w, basis.z.z));
		}
		if (Math::is_zero_approx(basis.z.x) && Math::is_zero_approx(basis.x.z) && Math::is_zero_approx(basis.x.y) && Math::is_zero_approx(basis.y.x) && Math::is_zero_approx(basis.w.y) && Math::is_zero_approx(basis.y.w) && Math::is_zero_approx(basis.z.w) && Math::is_zero_approx(basis.w.z)) {
			// Special case: No ZX, XY, WY, or ZW rotations, so the only rotations may be in YZ or XW.
			return Euler4D(Math::atan2(basis.y.z, basis.y.y), 0.0, 0.0, Math::atan2(basis.x.w, basis.x.x), 0.0, 0.0);
		}
		if (Math::is_zero_approx(basis.y.z) && Math::is_zero_approx(basis.z.y) && Math::is_zero_approx(basis.x.y) && Math::is_zero_approx(basis.y.x) && Math::is_zero_approx(basis.x.w) && Math::is_zero_approx(basis.w.x) && Math::is_zero_approx(basis.z.w) && Math::is_zero_approx(basis.w.z)) {
			// Special case: No YZ, XY, XW, or ZW rotations, so the only rotations may be in ZX or WY.
			return Euler4D(0.0, Math::atan2(basis.z.x, basis.z.z), 0.0, 0.0, Math::atan2(basis.w.y, basis.w.w), 0.0);
		}
	}

	// Stage 1: The outer rotations (and yz is special).
	real_t yz = -Math::asin(basis.z.y);
	real_t zx = Math::atan2(basis.z.x, basis.z.z);
	real_t wy = Math::atan2(basis.w.y, basis.y.y);

	// Un-rotate by perpendicular ZX and WY, the outermost rotations.
	basis = Basis4D::from_zx(-zx) * basis * Basis4D::from_wy(-wy);

	// Stage 2: The inner rotations (and xw is special).
	real_t xw = -Math::asin(basis.w.x);
	real_t zw = Math::atan2(basis.z.w, basis.z.z);
	real_t xy = Math::atan2(basis.x.y, basis.y.y);

	return Euler4D(yz, zx, xy, xw, wy, zw);
}

// Misc methods.

Euler4D Euler4D::compose(const Euler4D &p_child) const {
	return from_basis(to_basis() * p_child.to_basis());
}

bool Euler4D::is_equal_approx(const Euler4D &p_other) const {
	return Math::is_equal_approx(yz, p_other.yz) && Math::is_equal_approx(zx, p_other.zx) && Math::is_equal_approx(xy, p_other.xy) && Math::is_equal_approx(xw, p_other.xw) && Math::is_equal_approx(wy, p_other.wy) && Math::is_equal_approx(zw, p_other.zw);
}

Euler4D Euler4D::rotation_to(const Euler4D &p_to) const {
	return from_basis(to_basis().inverse() * p_to.to_basis());
}

Euler4D Euler4D::wrapped() const {
	return Euler4D(
			Math::wrapf(yz, (real_t)-Math_PI, (real_t)Math_PI),
			Math::wrapf(zx, (real_t)-Math_PI, (real_t)Math_PI),
			Math::wrapf(xy, (real_t)-Math_PI, (real_t)Math_PI),
			Math::wrapf(xw, (real_t)-Math_PI, (real_t)Math_PI),
			Math::wrapf(wy, (real_t)-Math_PI, (real_t)Math_PI),
			Math::wrapf(zw, (real_t)-Math_PI, (real_t)Math_PI));
}

// Radians/degrees.

Euler4D Euler4D::deg_to_rad() const {
	return *this * real_t(Math_PI / 180.0);
}

Euler4D Euler4D::rad_to_deg() const {
	return *this * real_t(180.0 / Math_PI);
}

// Operators.

Euler4D &Euler4D::operator+=(const Euler4D &p_v) {
	yz += p_v.yz;
	zx += p_v.zx;
	xy += p_v.xy;
	xw += p_v.xw;
	wy += p_v.wy;
	zw += p_v.zw;
	return *this;
}

Euler4D Euler4D::operator+(const Euler4D &p_v) const {
	return Euler4D(yz + p_v.yz, zx + p_v.zx, xy + p_v.xy, xw + p_v.xw, wy + p_v.wy, zw + p_v.zw);
}

Euler4D &Euler4D::operator-=(const Euler4D &p_v) {
	yz -= p_v.yz;
	zx -= p_v.zx;
	xy -= p_v.xy;
	xw -= p_v.xw;
	wy -= p_v.wy;
	zw -= p_v.zw;
	return *this;
}

Euler4D Euler4D::operator-(const Euler4D &p_v) const {
	return Euler4D(yz - p_v.yz, zx - p_v.zx, xy - p_v.xy, xw - p_v.xw, wy - p_v.wy, zw - p_v.zw);
}

Euler4D &Euler4D::operator*=(const Euler4D &p_child) {
	*this = from_basis(to_basis() * p_child.to_basis());
	return *this;
}

Euler4D Euler4D::operator*(const Euler4D &p_child) const {
	return from_basis(to_basis() * p_child.to_basis());
}

Euler4D &Euler4D::operator/=(const Euler4D &p_child) {
	*this = from_basis(to_basis() * p_child.to_basis().inverse());
	return *this;
}

Euler4D Euler4D::operator/(const Euler4D &p_child) const {
	return from_basis(to_basis() * p_child.to_basis().inverse());
}

Euler4D &Euler4D::operator*=(const real_t p_scalar) {
	yz *= p_scalar;
	zx *= p_scalar;
	xy *= p_scalar;
	xw *= p_scalar;
	wy *= p_scalar;
	zw *= p_scalar;
	return *this;
}

Euler4D Euler4D::operator*(const real_t p_scalar) const {
	return Euler4D(yz * p_scalar, zx * p_scalar, xy * p_scalar, xw * p_scalar, wy * p_scalar, zw * p_scalar);
}

Euler4D &Euler4D::operator/=(const real_t p_scalar) {
	yz /= p_scalar;
	zx /= p_scalar;
	xy /= p_scalar;
	xw /= p_scalar;
	wy /= p_scalar;
	zw /= p_scalar;
	return *this;
}

Euler4D Euler4D::operator/(const real_t p_scalar) const {
	return Euler4D(yz / p_scalar, zx / p_scalar, xy / p_scalar, xw / p_scalar, wy / p_scalar, zw / p_scalar);
}

Euler4D Euler4D::operator-() const {
	return Euler4D(-yz, -zx, -xy, -xw, -wy, -zw);
}

bool Euler4D::operator==(const Euler4D &p_v) const {
	return (xy == p_v.xy && zx == p_v.zx && xw == p_v.xw && yz == p_v.yz && wy == p_v.wy && zw == p_v.zw);
}

bool Euler4D::operator!=(const Euler4D &p_v) const {
	return (yz != p_v.yz || zx != p_v.zx || xy != p_v.xy || xw != p_v.xw || wy != p_v.wy || zw != p_v.zw);
}

Euler4D::operator String() const {
	return "(YZ: " + String::num_real(yz, false) + ", ZX: " + String::num_real(zx, false) + ", XY: " + String::num_real(xy, false) + ", XW: " + String::num_real(xw, false) + ", WY: " + String::num_real(wy, false) + ", ZW: " + String::num_real(zw, false) + ")";
}

// Conversion.

Euler4D Euler4D::from_3d(const Vector3 &p_from_3d) {
	return Euler4D(p_from_3d);
}

Vector3 Euler4D::to_3d() const {
	return Vector3(yz, zx, xy);
}

Euler4D Euler4D::from_array(const PackedRealArray &p_from_array) {
	const int stop_index = MIN(p_from_array.size(), 6);
	Euler4D euler;
	for (int i = 0; i < stop_index; i++) {
		euler.elements[i] = p_from_array[i];
	}
	return euler;
}

PackedRealArray Euler4D::to_array() const {
	PackedRealArray arr;
	arr.resize(6);
	real_t *ptr = arr.ptrw();
	ptr[0] = yz;
	ptr[1] = zx;
	ptr[2] = xy;
	ptr[3] = xw;
	ptr[4] = wy;
	ptr[5] = zw;
	return arr;
}

Euler4D::operator AABB() const {
	return AABB(Vector3(yz, zx, xy), Vector3(xw, wy, zw));
}

Euler4D::Euler4D(const AABB &p_from) {
	yz = p_from.position.x;
	zx = p_from.position.y;
	xy = p_from.position.z;
	xw = p_from.size.x;
	wy = p_from.size.y;
	zw = p_from.size.z;
}

static_assert(sizeof(Euler4D) == 6 * sizeof(real_t));
