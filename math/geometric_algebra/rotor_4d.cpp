#include "rotor_4d.h"

#include "../euler_4d.h"

// Geometric algebra functions.

// Conjugate flips the sign of the bivector components. For 4D rotors, this is identical to the reverse, but both are included for completeness and clarity.
// 1 - xy - xz - xw - yz - yw - zw + xyzw
Rotor4D Rotor4D::conjugate() const {
	Rotor4D result;
	result.s = +s;
	result.xy = -xy;
	result.xz = -xz;
	result.xw = -xw;
	result.yz = -yz;
	result.yw = -yw;
	result.zw = -zw;
	result.xyzw = +xyzw;
	return result;
}

// The dot product measures how aligned two multivectors are.
real_t Rotor4D::dot(const Rotor4D &p_b) const {
	return s * p_b.s + xy * p_b.xy + xz * p_b.xz + xw * p_b.xw + yz * p_b.yz + yw * p_b.yw + zw * p_b.zw + xyzw * p_b.xyzw;
}

// Dual multiplies each component by the unit pseudoscalar (xyzw) from the right.
// For example, xy * xyzw = xyxyzw = -xxyyzw = -zw, and xz * xyzw = xzxyzw = +xxyzzw = +yw.
// This is symmetric front to back, result[i] is set to +/- elements[15 - i].
Rotor4D Rotor4D::dual() const {
	Rotor4D result;
	result.s = +xyzw;
	result.xy = -zw;
	result.xz = +yw;
	result.xw = -yz;
	result.yz = -xw;
	result.yw = +xz;
	result.zw = -xy;
	result.xyzw = +s;
	return result;
}

// The full geometric algebra inner product with all components. See scalar_product if you only want the scalar part.
// This represents a projection and then a contraction. It is bilinear, not symmetric, and may be negative.
// The inner product two basis rotors is their geometric product if they share all factors.
// Reference: https://www.youtube.com/watch?v=2AKt6adG_OI&t=1850s
Rotor4D Rotor4D::inner_product(const Rotor4D &p_b) const {
	Rotor4D result;
	result.s = s * p_b.s - xy * p_b.xy - xz * p_b.xz - xw * p_b.xw - yz * p_b.yz - yw * p_b.yw - zw * p_b.zw + xyzw * p_b.xyzw;
	result.xy = s * p_b.xy + xy * p_b.s - zw * p_b.xyzw - xyzw * p_b.zw;
	result.xz = s * p_b.xz + xz * p_b.s + yw * p_b.xyzw + xyzw * p_b.yw;
	result.xw = s * p_b.xw + xw * p_b.s - yz * p_b.xyzw - xyzw * p_b.yz;
	result.yz = s * p_b.yz + yz * p_b.s - xw * p_b.xyzw - xyzw * p_b.xw;
	result.yw = s * p_b.yw + yw * p_b.s + xz * p_b.xyzw + xyzw * p_b.xz;
	result.zw = s * p_b.zw + zw * p_b.s - xy * p_b.xyzw - xyzw * p_b.xy;
	result.xyzw = s * p_b.xyzw + xyzw * p_b.s;
	return result;
}

Rotor4D Rotor4D::inverse() const {
	Rotor4D rev = reverse();
	Rotor4D first = (*this) * rev;
	Rotor4D negate_s = first;
	negate_s.s = -negate_s.s;
	real_t scalar_prod = first.scalar_product(negate_s);
	Rotor4D inv = rev * negate_s / scalar_prod;
	return inv;
}

// Grade involution flips the sign of odd-numbered grades. This is a no-op for 4D rotors. It is included for completeness and clarity.
Rotor4D Rotor4D::involute() const {
	return *this;
}

// The regressive product is the dual of the wedge/exterior/outer product.
// It represents the largest subspace contained by the input multivectors, so long as the duals of the inputs are linearly independent.
Rotor4D Rotor4D::regressive_product(const Rotor4D &p_b) const {
	Rotor4D result;
	result.s = s * p_b.xyzw + xy * p_b.zw - xz * p_b.yw + xw * p_b.yz + yz * p_b.xw - yw * p_b.xz + zw * p_b.xy + xyzw * p_b.s;
	result.xy = xy * p_b.xyzw + xyzw * p_b.xy;
	result.xz = xz * p_b.xyzw + xyzw * p_b.xz;
	result.xw = xw * p_b.xyzw + xyzw * p_b.xw;
	result.yz = yz * p_b.xyzw + xyzw * p_b.yz;
	result.yw = yw * p_b.xyzw + xyzw * p_b.yw;
	result.zw = zw * p_b.xyzw + xyzw * p_b.zw;
	result.xyzw = xyzw * p_b.xyzw;
	return result;
}

// The reverse function flips the sign of the bivector components. It is denoted in math notation by the dagger symbol.
// For 4D rotors, this is identical to the conjugate, but both are included for completeness and clarity.
// 1 - xy - xz - xw - yz - yw - zw + xyzw
Rotor4D Rotor4D::reverse() const {
	Rotor4D result;
	result.s = +s;
	result.xy = -xy;
	result.xz = -xz;
	result.xw = -xw;
	result.yz = -yz;
	result.yw = -yw;
	result.zw = -zw;
	result.xyzw = +xyzw;
	return result;
}

// The scalar part of the geometric algebra inner product only. This is also the scalar part of multiplication.
real_t Rotor4D::scalar_product(const Rotor4D &p_b) const {
	return s * p_b.s - xy * p_b.xy - xz * p_b.xz - xw * p_b.xw - yz * p_b.yz - yw * p_b.yw - zw * p_b.zw + xyzw * p_b.xyzw;
}

// The wedge/exterior/outer product is the dual of the regressive product.
// It represents the smallest subspace containing the input multivectors.
// The outer product of two basis multivectors is their geometric product if they share no factors.
Rotor4D Rotor4D::wedge_product(const Rotor4D &p_b) const {
	Rotor4D result;
	result.s = s * p_b.s;
	result.xy = s * p_b.xy + xy * p_b.s;
	result.xz = s * p_b.xz + xz * p_b.s;
	result.xw = s * p_b.xw + xw * p_b.s;
	result.yz = s * p_b.yz + yz * p_b.s;
	result.yw = s * p_b.yw + yw * p_b.s;
	result.zw = s * p_b.zw + zw * p_b.s;
	result.xyzw = s * p_b.xyzw + xy * p_b.zw - xz * p_b.yw + xw * p_b.yz + yz * p_b.xw - yw * p_b.xz + zw * p_b.xy + xyzw * p_b.s;
	return result;
}

// Rotation functions.

Basis4D Rotor4D::to_basis() const {
	Basis4D basis;
	basis.x = Vector4(
			s * s - xy * xy - xz * xz - xw * xw + yz * yz + yw * yw + zw * zw - xyzw * xyzw, // X.X
			s * xy + xy * s - xz * yz - xw * yw - yz * xz - yw * xw + zw * xyzw + xyzw * zw, // X.Y
			s * xz + xy * yz + xz * s - xw * zw + yz * xy - yw * xyzw - zw * xw - xyzw * yw, // X.Z
			s * xw + xy * yw + xz * zw + xw * s + yz * xyzw + yw * xy + zw * xz + xyzw * yz // X.W
	);
	basis.y = Vector4(
			-xy * s - s * xy - yz * xz - yw * xw - xz * yz - xw * yw - xyzw * zw - zw * xyzw, // Y.X
			-xy * xy + s * s - yz * yz - yw * yw + xz * xz + xw * xw - xyzw * xyzw + zw * zw, // Y.Y
			-xy * xz + s * yz + yz * s - yw * zw - xz * xy + xw * xyzw + xyzw * xw - zw * yw, // Y.Z
			-xy * xw + s * yw + yz * zw + yw * s - xz * xyzw - xw * xy - xyzw * xz + zw * yz // Y.W
	);
	basis.z = Vector4(
			-xz * s + yz * xy - s * xz - zw * xw + xy * yz + xyzw * yw - xw * zw + yw * xyzw, // Z.X
			-xz * xy - yz * s - s * yz - zw * yw - xy * xz - xyzw * xw - xw * xyzw - yw * zw, // Z.Y
			-xz * xz - yz * yz + s * s - zw * zw + xy * xy - xyzw * xyzw + xw * xw + yw * yw, // Z.Z
			-xz * xw - yz * yw + s * zw + zw * s + xy * xyzw + xyzw * xy - xw * xz - yw * yz // Z.W
	);
	basis.w = Vector4(
			-xw * s + yw * xy + zw * xz - s * xw - xyzw * yz + xy * yw + xz * zw - yz * xyzw, // W.X
			-xw * xy - yw * s + zw * yz - s * yw + xyzw * xz - xy * xw + xz * xyzw + yz * zw, // W.Y
			-xw * xz - yw * yz - zw * s - s * zw - xyzw * xy - xy * xyzw - xz * xw - yz * yw, // W.Z
			-xw * xw - yw * yw - zw * zw + s * s - xyzw * xyzw + xy * xy + xz * xz + yz * yz // W.W
	);
	return basis.orthonormalized();
}

real_t Rotor4D::get_rotation_angle() const {
	return 2.0f * Math::atan2(parts.bivector.length(), s);
}

Bivector4D Rotor4D::get_rotation_bivector_magnitude() const {
	return parts.bivector * get_rotation_angle();
}

Bivector4D Rotor4D::get_rotation_bivector_normal() const {
	return parts.bivector.normalized();
}

Basis4D Rotor4D::rotate_basis(const Basis4D &p_basis) const {
#ifdef MATH_CHECKS
	ERR_FAIL_COND_V_MSG(!is_normalized(), p_basis, "The Rotor4D must be normalized in order to rotate a vector, but was " + operator String() + ".");
#endif
	const Rotor4D rev = reverse();
	return Basis4D(
			rev.sandwich(p_basis.x, *this),
			rev.sandwich(p_basis.y, *this),
			rev.sandwich(p_basis.z, *this),
			rev.sandwich(p_basis.w, *this));
}

// Same as operator* but as a function.
Rotor4D Rotor4D::rotate_rotor(const Rotor4D &p_rotor) const {
	return *this * p_rotor;
}

Vector4 Rotor4D::rotate_vector(const Vector4 &p_vec) const {
#ifdef MATH_CHECKS
	ERR_FAIL_COND_V_MSG(!is_normalized(), p_vec, "The Rotor4D must be normalized in order to rotate a vector, but was " + operator String() + ".");
#endif
	const Rotor4D rev = reverse();
	// Online resources seem to indicate that it should be r * v * r^-1, but that doesn't work...?
	// Instead, r^-1 * v * r is what correctly rotates the vector in the expected direction.
	// I've double-checked the constructors, products, and multiplication operator and all they seem correct.
	// This seems really weird, since Quaternions use q * v * q^-1 to do this. - aaronfranke
	return rev.sandwich(p_vec, *this);
}

// Multiplies the rotor with the vector, and then multiplies the result with the right rotor, returning only the vector part.
// This is useful for things like rotation operations. Prefer using rotate_vector() and rotate_basis() instead, unless you're really hungry and want a sandwich.
Vector4 Rotor4D::sandwich(const Vector4 &p_vec, const Rotor4D &p_right) const {
	Vector4 left_vector = Vector4(
			s * p_vec.x + xy * p_vec.y + xz * p_vec.z + xw * p_vec.w, // X
			s * p_vec.y - xy * p_vec.x + yz * p_vec.z + yw * p_vec.w, // Y
			s * p_vec.z - xz * p_vec.x - yz * p_vec.y + zw * p_vec.w, // Z
			s * p_vec.w - xw * p_vec.x - yw * p_vec.y - zw * p_vec.z // W
	);
	Trivector4D left_trivector = Trivector4D(
			xy * p_vec.z - xz * p_vec.y + yz * p_vec.x + xyzw * p_vec.w, // XYZ
			xy * p_vec.w - xw * p_vec.y + yw * p_vec.x - xyzw * p_vec.z, // XYW
			xz * p_vec.w - xw * p_vec.z + zw * p_vec.x + xyzw * p_vec.y, // XZW
			yz * p_vec.w - yw * p_vec.z + zw * p_vec.y - xyzw * p_vec.x // YZW
	);
	return Vector4(
			left_vector.x * p_right.s - left_vector.y * p_right.xy - left_vector.z * p_right.xz - left_vector.w * p_right.xw - left_trivector.xyz * p_right.yz - left_trivector.xyw * p_right.yw - left_trivector.xzw * p_right.zw + left_trivector.yzw * p_right.xyzw, // X
			left_vector.x * p_right.xy + left_vector.y * p_right.s - left_vector.z * p_right.yz - left_vector.w * p_right.yw + left_trivector.xyz * p_right.xz + left_trivector.xyw * p_right.xw - left_trivector.xzw * p_right.xyzw - left_trivector.yzw * p_right.zw, // Y
			left_vector.x * p_right.xz + left_vector.y * p_right.yz + left_vector.z * p_right.s - left_vector.w * p_right.zw - left_trivector.xyz * p_right.xy + left_trivector.xyw * p_right.xyzw + left_trivector.xzw * p_right.xw + left_trivector.yzw * p_right.yw, // Z
			left_vector.x * p_right.xw + left_vector.y * p_right.yw + left_vector.z * p_right.zw + left_vector.w * p_right.s - left_trivector.xyz * p_right.xyzw - left_trivector.xyw * p_right.xy - left_trivector.xzw * p_right.xz - left_trivector.yzw * p_right.yz // W
	);
}

Rotor4D Rotor4D::slerp(Rotor4D p_to, const real_t p_weight) const {
#ifdef MATH_CHECKS
	ERR_FAIL_COND_V_MSG(!is_normalized(), Rotor4D::identity(), "The start Rotor4D " + operator String() + " must be normalized.");
	ERR_FAIL_COND_V_MSG(!p_to.is_normalized(), Rotor4D::identity(), "The end Rotor4D " + p_to.operator String() + " must be normalized.");
#endif
	double_t alignment = dot(p_to);
	if (alignment < 0.0f) {
		// Due to double cover, rotors may be rotated more than 180 degrees apart from each other.
		// Those cases correspond to a state space rotation of more than 90 degrees (dot product < 0).
		// In that case, the shortest path to that physical rotation is towards the negative of p_to.
		p_to = -p_to;
		alignment = -alignment;
	}
	if (alignment > 0.999f) {
		// For very close rotors, slerp can have numerical instability, so linearly interpolate instead.
		return (*this * (1.0f - p_weight) + p_to * p_weight).normalized();
	}
	// Spherically interpolate the rotors.
	double_t angle_between_rotors = Math::acos(alignment);
	double_t sin_angle = Math::sin(angle_between_rotors);
	double_t from_weight = Math::sin((1.0f - p_weight) * angle_between_rotors) / sin_angle;
	double_t to_weight = Math::sin(p_weight * angle_between_rotors) / sin_angle;
	return (*this * from_weight + p_to * to_weight).normalized();
}

Rotor4D Rotor4D::slerp_fraction(const real_t p_weight) const {
#ifdef MATH_CHECKS
	ERR_FAIL_COND_V_MSG(!is_normalized(), Rotor4D::identity(), "The Rotor4D " + operator String() + " must be normalized.");
#endif
	double_t angle = Math::acos(parts.scalar);
	double_t sin_angle = Math::sin(angle);
	double_t fractional_angle = p_weight * angle;
	double_t new_scalar = Math::cos(fractional_angle);
	double_t new_sin_angle = Math::sin(fractional_angle);
	Bivector4D new_bivector = parts.bivector * (new_sin_angle / sin_angle);
	return Rotor4D(new_scalar, new_bivector).normalized();
}

// Length functions.

real_t Rotor4D::length() const {
	return Math::sqrt(length_squared());
}

real_t Rotor4D::length_squared() const {
	return parts.scalar * parts.scalar + parts.bivector.length_squared() + parts.pseudoscalar * parts.pseudoscalar;
}

// The multiplication of the reverse of the rotor with the rotor itself, which only has scalar and pseudoscalar components.
SplitComplex4D Rotor4D::split_magnitude_squared() const {
	SplitComplex4D result;
	result.s = s * s + xy * xy + xz * xz + xw * xw + yz * yz + yw * yw + zw * zw + xyzw * xyzw; // Scalar
	result.xyzw = s * xyzw - xy * zw + xz * yw - xw * yz - yz * xw + yw * xz - zw * xy + xyzw * s; // XYZW
	return result;
}

Rotor4D Rotor4D::normalized() const {
	const SplitComplex4D split = split_magnitude_squared();
	ERR_FAIL_COND_V_MSG(!split.is_valid(), Rotor4D::identity(), "The Rotor4D must have a valid split square magnitude in order to be normalized. Returning the identity rotor.");
	const SplitComplex4D split_inv_sqrt = split.inverse_square_root();
	// Divide by the square root of the split magnitude squared (or, faster: multiply by the inverse square root).
	return *this * split_inv_sqrt;
}

bool Rotor4D::is_normalized() const {
	return Math::is_equal_approx(length_squared(), (real_t)1.0);
}

bool Rotor4D::is_rotation() const {
	return Math::is_zero_approx(xyzw) && is_normalized();
}

// Static functions for doing math on non-Rotor4D types and returning a Rotor4D.

// The vector product, also known as the geometric vector product or the Holmér product, is the full multiplication of two vectors.
// The geometric vector product combines the dot product (scalar) and the wedge/cross product (bivector).
// The interpretation of the result is a Rotor4D that, when applied as the sandwich product to a vector,
// double-rotates the vector by the angle between the two input vectors. Therefore, for rotating by
// some angle, the input vectors should have an angle between them of half the desired rotation angle.
Rotor4D Rotor4D::vector_product(const Vector4 &p_a, const Vector4 &p_b) {
	Rotor4D result;
	result.s = p_a.dot(p_b);
	result.xy = p_a.x * p_b.y - p_a.y * p_b.x;
	result.xz = p_a.x * p_b.z - p_a.z * p_b.x;
	result.xw = p_a.x * p_b.w - p_a.w * p_b.x;
	result.yz = p_a.y * p_b.z - p_a.z * p_b.y;
	result.yw = p_a.y * p_b.w - p_a.w * p_b.y;
	result.zw = p_a.z * p_b.w - p_a.w * p_b.z;
	return result;
}

Rotor4D Rotor4D::from_basis(const Basis4D &p_basis) {
	const Basis4D ortho = p_basis.orthonormalized();
	ERR_FAIL_COND_V_MSG(!Math::is_equal_approx(ortho.determinant(), 1), Rotor4D::identity(), "The basis must be orthonormal in order to create a rotation, but was " + p_basis.operator String() + " (determinant " + rtos(p_basis.determinant()) + ").");
	// TODO: This Euler-based algorithm sucks. It should be replaced with a better one when one is discovered.
	const Euler4D euler = Euler4D::from_basis(ortho);
	return from_zx(euler.zx) * from_zw(euler.zw) * from_xw(euler.xw) * from_yz(euler.yz) * from_xy(euler.xy) * from_wy(euler.wy);
}

Rotor4D Rotor4D::from_bivector_magnitude(const Bivector4D &p_bivector) {
	const real_t length_angle = p_bivector.length();
	if (Math::is_zero_approx(length_angle)) {
		return identity();
	}
	return from_bivector_normal_angle(p_bivector / length_angle, length_angle);
}

Rotor4D Rotor4D::from_bivector_normal_angle(const Bivector4D &p_bivector_normal, const real_t p_angle) {
#ifdef MATH_CHECKS
	ERR_FAIL_COND_V_MSG(!p_bivector_normal.is_normalized(), Rotor4D::identity(), "The bivector must be normalized in order to create a rotation, but was " + p_bivector_normal.operator String() + " (length " + rtos(p_bivector_normal.length()) + ").");
#endif
	if (Math::is_zero_approx(p_angle)) {
		return identity();
	}
	const real_t half_angle = p_angle * 0.5;
	const real_t cos_half_angle = Math::cos(half_angle);
	const real_t sin_half_angle = Math::sin(half_angle);
	return Rotor4D(cos_half_angle, p_bivector_normal * sin_half_angle);
}

Rotor4D Rotor4D::from_xy(const real_t p_angle) {
	Rotor4D result;
	const real_t half_angle = p_angle * 0.5;
	result.s = Math::cos(half_angle);
	result.xy = Math::sin(half_angle);
	return result;
}

Rotor4D Rotor4D::from_xz(const real_t p_angle) {
	Rotor4D result;
	const real_t half_angle = p_angle * 0.5;
	result.s = Math::cos(half_angle);
	result.xz = Math::sin(half_angle);
	return result;
}

Rotor4D Rotor4D::from_zx(const real_t p_angle) {
	Rotor4D result;
	const real_t half_angle = p_angle * -0.5;
	result.s = Math::cos(half_angle);
	result.xz = Math::sin(half_angle);
	return result;
}

Rotor4D Rotor4D::from_xw(const real_t p_angle) {
	Rotor4D result;
	const real_t half_angle = p_angle * 0.5;
	result.s = Math::cos(half_angle);
	result.xw = Math::sin(half_angle);
	return result;
}

Rotor4D Rotor4D::from_yz(const real_t p_angle) {
	Rotor4D result;
	const real_t half_angle = p_angle * 0.5;
	result.s = Math::cos(half_angle);
	result.yz = Math::sin(half_angle);
	return result;
}

Rotor4D Rotor4D::from_yw(const real_t p_angle) {
	Rotor4D result;
	const real_t half_angle = p_angle * 0.5;
	result.s = Math::cos(half_angle);
	result.yw = Math::sin(half_angle);
	return result;
}

Rotor4D Rotor4D::from_wy(const real_t p_angle) {
	Rotor4D result;
	const real_t half_angle = p_angle * -0.5;
	result.s = Math::cos(half_angle);
	result.yw = Math::sin(half_angle);
	return result;
}

Rotor4D Rotor4D::from_zw(const real_t p_angle) {
	Rotor4D result;
	const real_t half_angle = p_angle * 0.5;
	result.s = Math::cos(half_angle);
	result.zw = Math::sin(half_angle);
	return result;
}

Rotor4D Rotor4D::identity() {
	return Rotor4D(1, 0, 0, 0, 0, 0, 0, 0);
}

// Component with-style setters.

Rotor4D Rotor4D::with_s(const real_t p_s) const {
	Rotor4D result = *this;
	result.s = p_s;
	return result;
}

Rotor4D Rotor4D::with_xy(const real_t p_xy) const {
	Rotor4D result = *this;
	result.xy = p_xy;
	return result;
}

Rotor4D Rotor4D::with_xz(const real_t p_xz) const {
	Rotor4D result = *this;
	result.xz = p_xz;
	return result;
}

Rotor4D Rotor4D::with_xw(const real_t p_xw) const {
	Rotor4D result = *this;
	result.xw = p_xw;
	return result;
}

Rotor4D Rotor4D::with_yz(const real_t p_yz) const {
	Rotor4D result = *this;
	result.yz = p_yz;
	return result;
}

Rotor4D Rotor4D::with_yw(const real_t p_yw) const {
	Rotor4D result = *this;
	result.yw = p_yw;
	return result;
}

Rotor4D Rotor4D::with_zw(const real_t p_zw) const {
	Rotor4D result = *this;
	result.zw = p_zw;
	return result;
}

Rotor4D Rotor4D::with_xyzw(const real_t p_xyzw) const {
	Rotor4D result = *this;
	result.xyzw = p_xyzw;
	return result;
}

// Trivial getters and setters.

real_t Rotor4D::get_scalar() const {
	return parts.scalar;
}

void Rotor4D::set_scalar(const real_t p_scalar) {
	parts.scalar = p_scalar;
}

Bivector4D Rotor4D::get_bivector() const {
	return parts.bivector;
}

void Rotor4D::set_bivector(const Bivector4D &p_bivector) {
	parts.bivector = p_bivector;
}

real_t Rotor4D::get_pseudoscalar() const {
	return parts.pseudoscalar;
}

void Rotor4D::set_pseudoscalar(const real_t p_pseudoscalar) {
	parts.pseudoscalar = p_pseudoscalar;
}

SplitComplex4D Rotor4D::get_split_complex() const {
	return SplitComplex4D(parts.scalar, parts.pseudoscalar);
}

void Rotor4D::set_split_complex(const SplitComplex4D &p_split) {
	parts.scalar = p_split.s;
	parts.pseudoscalar = p_split.xyzw;
}

// Operators.

bool Rotor4D::is_equal_approx(const Rotor4D &p_other) const {
	return Math::is_equal_approx(parts.scalar, p_other.parts.scalar) && parts.bivector.is_equal_approx(p_other.parts.bivector) && Math::is_equal_approx(parts.pseudoscalar, p_other.parts.pseudoscalar);
}

bool Rotor4D::operator==(const Rotor4D &p_v) const {
	return (parts.scalar == p_v.parts.scalar && parts.bivector == p_v.parts.bivector && parts.pseudoscalar == p_v.parts.pseudoscalar);
}

bool Rotor4D::operator!=(const Rotor4D &p_v) const {
	return (parts.scalar != p_v.parts.scalar || parts.bivector != p_v.parts.bivector || parts.pseudoscalar != p_v.parts.pseudoscalar);
}

Rotor4D Rotor4D::operator-() const {
	return Rotor4D(-parts.scalar, -parts.bivector, -parts.pseudoscalar);
}

Rotor4D &Rotor4D::operator+=(const Rotor4D &p_v) {
	parts.scalar += p_v.parts.scalar;
	parts.bivector += p_v.parts.bivector;
	parts.pseudoscalar += p_v.parts.pseudoscalar;
	return *this;
}

Rotor4D Rotor4D::operator+(const Rotor4D &p_v) const {
	return Rotor4D(parts.scalar + p_v.parts.scalar, parts.bivector + p_v.parts.bivector, parts.pseudoscalar + p_v.parts.pseudoscalar);
}

Rotor4D &Rotor4D::operator-=(const Rotor4D &p_v) {
	parts.scalar -= p_v.parts.scalar;
	parts.bivector -= p_v.parts.bivector;
	parts.pseudoscalar -= p_v.parts.pseudoscalar;
	return *this;
}

Rotor4D Rotor4D::operator-(const Rotor4D &p_v) const {
	return Rotor4D(parts.scalar - p_v.parts.scalar, parts.bivector - p_v.parts.bivector, parts.pseudoscalar - p_v.parts.pseudoscalar);
}

Rotor4D &Rotor4D::operator*=(const Rotor4D &p_child) {
	// Can't do an in-place multiply because the operation requires the original values throughout the calculation.
	Rotor4D result = *this * p_child;
	parts.scalar = result.parts.scalar;
	parts.bivector = result.parts.bivector;
	parts.pseudoscalar = result.parts.pseudoscalar;
	return *this;
}

Rotor4D Rotor4D::operator*(const Rotor4D &p_b) const {
	Rotor4D result;
	result.s = s * p_b.s - xy * p_b.xy - xz * p_b.xz - xw * p_b.xw - yz * p_b.yz - yw * p_b.yw - zw * p_b.zw + xyzw * p_b.xyzw; // Scalar
	result.xy = s * p_b.xy + xy * p_b.s - xz * p_b.yz - xw * p_b.yw + yz * p_b.xz + yw * p_b.xw - zw * p_b.xyzw - xyzw * p_b.zw; // XY
	result.xz = s * p_b.xz + xy * p_b.yz + xz * p_b.s - xw * p_b.zw - yz * p_b.xy + yw * p_b.xyzw + zw * p_b.xw + xyzw * p_b.yw; // XZ
	result.xw = s * p_b.xw + xy * p_b.yw + xz * p_b.zw + xw * p_b.s - yz * p_b.xyzw - yw * p_b.xy - zw * p_b.xz - xyzw * p_b.yz; // XW
	result.yz = s * p_b.yz - xy * p_b.xz + xz * p_b.xy - xw * p_b.xyzw + yz * p_b.s - yw * p_b.zw + zw * p_b.yw - xyzw * p_b.xw; // YZ
	result.yw = s * p_b.yw - xy * p_b.xw + xz * p_b.xyzw + xw * p_b.xy + yz * p_b.zw + yw * p_b.s - zw * p_b.yz + xyzw * p_b.xz; // YW
	result.zw = s * p_b.zw - xy * p_b.xyzw - xz * p_b.xw + xw * p_b.xz - yz * p_b.yw + yw * p_b.yz + zw * p_b.s - xyzw * p_b.xy; // ZW
	result.xyzw = s * p_b.xyzw + xy * p_b.zw - xz * p_b.yw + xw * p_b.yz + yz * p_b.xw - yw * p_b.xz + zw * p_b.xy + xyzw * p_b.s; // XYZW
	return result;
}

Rotor4D &Rotor4D::operator*=(const SplitComplex4D &p_split) {
	// Can't do an in-place multiply because the operation requires the original values throughout the calculation.
	Rotor4D result = *this * p_split;
	parts.scalar = result.parts.scalar;
	parts.bivector = result.parts.bivector;
	parts.pseudoscalar = result.parts.pseudoscalar;
	return *this;
}

Rotor4D Rotor4D::operator*(const SplitComplex4D &p_split) const {
	Rotor4D result;
	result.s = s * p_split.s + xyzw * p_split.xyzw; // Scalar
	result.xy = xy * p_split.s - zw * p_split.xyzw; // XY
	result.xz = xz * p_split.s + yw * p_split.xyzw; // XZ
	result.xw = xw * p_split.s - yz * p_split.xyzw; // XW
	result.yz = yz * p_split.s - xw * p_split.xyzw; // YZ
	result.yw = yw * p_split.s + xz * p_split.xyzw; // YW
	result.zw = zw * p_split.s - xy * p_split.xyzw; // ZW
	result.xyzw = xyzw * p_split.s + s * p_split.xyzw; // XYZW
	return result;
}

Rotor4D &Rotor4D::operator/=(const SplitComplex4D &p_split) {
	// Can't do an in-place multiply because the operation requires the original values throughout the calculation.
	Rotor4D result = *this * p_split.inverse();
	parts.scalar = result.parts.scalar;
	parts.bivector = result.parts.bivector;
	parts.pseudoscalar = result.parts.pseudoscalar;
	return *this;
}

Rotor4D Rotor4D::operator/(const SplitComplex4D &p_split) const {
	return *this * p_split.inverse();
}

Rotor4D &Rotor4D::operator+=(const real_t p_scalar) {
	parts.scalar += p_scalar;
	return *this;
}

Rotor4D Rotor4D::operator+(const real_t p_scalar) const {
	Rotor4D result = *this;
	result.parts.scalar += p_scalar;
	return result;
}

Rotor4D &Rotor4D::operator-=(const real_t p_scalar) {
	parts.scalar -= p_scalar;
	return *this;
}

Rotor4D Rotor4D::operator-(const real_t p_scalar) const {
	Rotor4D result = *this;
	result.parts.scalar -= p_scalar;
	return result;
}

Rotor4D &Rotor4D::operator*=(const real_t p_scalar) {
	parts.scalar *= p_scalar;
	parts.bivector *= p_scalar;
	parts.pseudoscalar *= p_scalar;
	return *this;
}

Rotor4D Rotor4D::operator*(const real_t p_scalar) const {
	return Rotor4D(parts.scalar * p_scalar, parts.bivector * p_scalar, parts.pseudoscalar * p_scalar);
}

Rotor4D &Rotor4D::operator/=(const real_t p_scalar) {
	parts.scalar /= p_scalar;
	parts.bivector /= p_scalar;
	parts.pseudoscalar /= p_scalar;
	return *this;
}

Rotor4D Rotor4D::operator/(const real_t p_scalar) const {
	return Rotor4D(parts.scalar / p_scalar, parts.bivector / p_scalar, parts.pseudoscalar / p_scalar);
}

void Rotor4D::multiply_vector(const Vector4 &p_in_vec, Vector4 &r_out_vector, Trivector4D &r_out_trivec) const {
	r_out_vector.x = s * p_in_vec.x + xy * p_in_vec.y + xz * p_in_vec.z + xw * p_in_vec.w; // X
	r_out_vector.y = s * p_in_vec.y - xy * p_in_vec.x + yz * p_in_vec.z + yw * p_in_vec.w; // Y
	r_out_vector.z = s * p_in_vec.z - xz * p_in_vec.x - yz * p_in_vec.y + zw * p_in_vec.w; // Z
	r_out_vector.w = s * p_in_vec.w - xw * p_in_vec.x - yw * p_in_vec.y - zw * p_in_vec.z; // W
	r_out_trivec.xyz = xy * p_in_vec.z - xz * p_in_vec.y + yz * p_in_vec.x + xyzw * p_in_vec.w; // XYZ
	r_out_trivec.xyw = xy * p_in_vec.w - xw * p_in_vec.y + yw * p_in_vec.x - xyzw * p_in_vec.z; // XYW
	r_out_trivec.xzw = xz * p_in_vec.w - xw * p_in_vec.z + zw * p_in_vec.x + xyzw * p_in_vec.y; // XZW
	r_out_trivec.yzw = yz * p_in_vec.w - yw * p_in_vec.z + zw * p_in_vec.y - xyzw * p_in_vec.x; // YZW
}

void Rotor4D::premultiply_vector(const Vector4 &p_in_vector, Vector4 &r_out_vector, Trivector4D &r_out_triv) const {
	r_out_vector.x = p_in_vector.x * s - p_in_vector.y * xy - p_in_vector.z * xz - p_in_vector.w * xw; // X
	r_out_vector.y = p_in_vector.x * xy + p_in_vector.y * s - p_in_vector.z * yz - p_in_vector.w * yw; // Y
	r_out_vector.z = p_in_vector.x * xz + p_in_vector.y * yz + p_in_vector.z * s - p_in_vector.w * zw; // Z
	r_out_vector.w = p_in_vector.x * xw + p_in_vector.y * yw + p_in_vector.z * zw + p_in_vector.w * s; // W
	r_out_triv.xyz = p_in_vector.x * yz - p_in_vector.y * xz + p_in_vector.z * xy - p_in_vector.w * xyzw; // XYZ
	r_out_triv.xyw = p_in_vector.x * yw - p_in_vector.y * xw + p_in_vector.z * xyzw + p_in_vector.w * xy; // XYW
	r_out_triv.xzw = p_in_vector.x * zw - p_in_vector.y * xyzw - p_in_vector.z * xw + p_in_vector.w * xz; // XZW
	r_out_triv.yzw = p_in_vector.x * xyzw + p_in_vector.y * zw - p_in_vector.z * yw + p_in_vector.w * yz; // YZW
}

void Rotor4D::premultiply_vector_trivector(const Vector4 &p_in_vector, const Trivector4D &p_in_trivec, Vector4 &r_out_vector, Trivector4D &r_out_triv) const {
	r_out_vector.x = p_in_vector.x * s - p_in_vector.y * xy - p_in_vector.z * xz - p_in_vector.w * xw - p_in_trivec.xyz * yz - p_in_trivec.xyw * yw - p_in_trivec.xzw * zw + p_in_trivec.yzw * xyzw; // X
	r_out_vector.y = p_in_vector.x * xy + p_in_vector.y * s - p_in_vector.z * yz - p_in_vector.w * yw + p_in_trivec.xyz * xz + p_in_trivec.xyw * xw - p_in_trivec.xzw * xyzw - p_in_trivec.yzw * zw; // Y
	r_out_vector.z = p_in_vector.x * xz + p_in_vector.y * yz + p_in_vector.z * s - p_in_vector.w * zw - p_in_trivec.xyz * xy + p_in_trivec.xyw * xyzw + p_in_trivec.xzw * xw + p_in_trivec.yzw * yw; // Z
	r_out_vector.w = p_in_vector.x * xw + p_in_vector.y * yw + p_in_vector.z * zw + p_in_vector.w * s - p_in_trivec.xyz * xyzw - p_in_trivec.xyw * xy - p_in_trivec.xzw * xz - p_in_trivec.yzw * yz; // W
	r_out_triv.xyz = p_in_vector.x * yz - p_in_vector.y * xz + p_in_vector.z * xy - p_in_vector.w * xyzw + p_in_trivec.xyz * s - p_in_trivec.xyw * zw + p_in_trivec.xzw * yw - p_in_trivec.yzw * xw; // XYZ
	r_out_triv.xyw = p_in_vector.x * yw - p_in_vector.y * xw + p_in_vector.z * xyzw + p_in_vector.w * xy + p_in_trivec.xyz * zw + p_in_trivec.xyw * s - p_in_trivec.xzw * yz + p_in_trivec.yzw * xz; // XYW
	r_out_triv.xzw = p_in_vector.x * zw - p_in_vector.y * xyzw - p_in_vector.z * xw + p_in_vector.w * xz - p_in_trivec.xyz * yw + p_in_trivec.xyw * yz + p_in_trivec.xzw * s - p_in_trivec.yzw * xy; // XZW
	r_out_triv.yzw = p_in_vector.x * xyzw + p_in_vector.y * zw - p_in_vector.z * yw + p_in_vector.w * yz + p_in_trivec.xyz * xw - p_in_trivec.xyw * xz + p_in_trivec.xzw * xy + p_in_trivec.yzw * s; // YZW
}

Rotor4D Rotor4D::operator*(const Bivector4D &p_b) const {
	Rotor4D result;
	result.s = -xy * p_b.xy - xz * p_b.xz - xw * p_b.xw - yz * p_b.yz - yw * p_b.yw - zw * p_b.zw; // Scalar
	result.xy = s * p_b.xy - xz * p_b.yz - xw * p_b.yw + yz * p_b.xz + yw * p_b.xw - xyzw * p_b.zw; // XY
	result.xz = s * p_b.xz + xy * p_b.yz - xw * p_b.zw - yz * p_b.xy + zw * p_b.xw + xyzw * p_b.yw; // XZ
	result.xw = s * p_b.xw + xy * p_b.yw + xz * p_b.zw - yw * p_b.xy - zw * p_b.xz - xyzw * p_b.yz; // XW
	result.yz = s * p_b.yz - xy * p_b.xz + xz * p_b.xy - yw * p_b.zw + zw * p_b.yw - xyzw * p_b.xw; // YZ
	result.yw = s * p_b.yw - xy * p_b.xw + xw * p_b.xy + yz * p_b.zw - zw * p_b.yz + xyzw * p_b.xz; // YW
	result.zw = s * p_b.zw - xz * p_b.xw + xw * p_b.xz - yz * p_b.yw + yw * p_b.yz - xyzw * p_b.xy; // ZW
	result.xyzw = xy * p_b.zw - xz * p_b.yw + xw * p_b.yz + yz * p_b.xw - yw * p_b.xz + zw * p_b.xy; // XYZW
	return result;
}

Rotor4D operator+(const real_t p_scalar, const Rotor4D &p_rotor) {
	Rotor4D result = p_rotor;
	result.parts.scalar += p_scalar;
	return result;
}

Rotor4D operator+(const Bivector4D &p_bivector, const Rotor4D &p_rotor) {
	// Addition is commutative, so b += a is the same as a = a + b.
	Rotor4D result = p_rotor;
	result.parts.bivector += p_bivector;
	return result;
}

Rotor4D operator*(const real_t p_scalar, const Rotor4D &p_rotor) {
	return Rotor4D(p_scalar * p_rotor.parts.scalar, p_scalar * p_rotor.parts.bivector, p_scalar * p_rotor.parts.pseudoscalar);
}

Rotor4D operator*(const Bivector4D &p_bivector, const Rotor4D &p_rotor) {
	Rotor4D result;
	result.s = p_rotor.s - p_bivector.xy * p_rotor.xy - p_bivector.xz * p_rotor.xz - p_bivector.xw * p_rotor.xw - p_bivector.yz * p_rotor.yz - p_bivector.yw * p_rotor.yw - p_bivector.zw * p_rotor.zw; // Scalar
	result.xy = p_rotor.xy + p_bivector.xy * p_rotor.s - p_bivector.xz * p_rotor.yz - p_bivector.xw * p_rotor.yw + p_bivector.yz * p_rotor.xz + p_bivector.yw * p_rotor.xw - p_bivector.zw * p_rotor.xyzw; // XY
	result.xz = p_rotor.xz + p_bivector.xy * p_rotor.yz + p_bivector.xz * p_rotor.s - p_bivector.xw * p_rotor.zw - p_bivector.yz * p_rotor.xy + p_bivector.yw * p_rotor.xyzw + p_bivector.zw * p_rotor.xw; // XZ
	result.xw = p_rotor.xw + p_bivector.xy * p_rotor.yw + p_bivector.xz * p_rotor.zw + p_bivector.xw * p_rotor.s - p_bivector.yz * p_rotor.xyzw - p_bivector.yw * p_rotor.xy - p_bivector.zw * p_rotor.xz; // XW
	result.yz = p_rotor.yz - p_bivector.xy * p_rotor.xz + p_bivector.xz * p_rotor.xy - p_bivector.xw * p_rotor.xyzw + p_bivector.yz * p_rotor.s - p_bivector.yw * p_rotor.zw + p_bivector.zw * p_rotor.yw; // YZ
	result.yw = p_rotor.yw - p_bivector.xy * p_rotor.xw + p_bivector.xz * p_rotor.xyzw + p_bivector.xw * p_rotor.xy + p_bivector.yz * p_rotor.zw + p_bivector.yw * p_rotor.s - p_bivector.zw * p_rotor.yz; // YW
	result.zw = p_rotor.zw - p_bivector.xy * p_rotor.xyzw - p_bivector.xz * p_rotor.xw + p_bivector.xw * p_rotor.xz - p_bivector.yz * p_rotor.yw + p_bivector.yw * p_rotor.yz + p_bivector.zw * p_rotor.s; // ZW
	result.xyzw = p_rotor.xyzw + p_bivector.xy * p_rotor.zw - p_bivector.xz * p_rotor.yw + p_bivector.xw * p_rotor.yz + p_bivector.yz * p_rotor.xw - p_bivector.yw * p_rotor.xz + p_bivector.zw * p_rotor.xy; // XYZW
	return result;
}

Rotor4D operator*(const SplitComplex4D &p_split, const Rotor4D &p_rotor) {
	Rotor4D result;
	result.s = p_split.s * p_rotor.s + p_split.xyzw * p_rotor.xyzw; // Scalar
	result.xy = p_split.s * p_rotor.xy - p_split.xyzw * p_rotor.zw; // XY
	result.xz = p_split.s * p_rotor.xz + p_split.xyzw * p_rotor.yw; // XZ
	result.xw = p_split.s * p_rotor.xw - p_split.xyzw * p_rotor.yz; // XW
	result.yz = p_split.s * p_rotor.yz - p_split.xyzw * p_rotor.xw; // YZ
	result.yw = p_split.s * p_rotor.yw + p_split.xyzw * p_rotor.xz; // YW
	result.zw = p_split.s * p_rotor.zw - p_split.xyzw * p_rotor.xy; // ZW
	result.xyzw = p_split.s * p_rotor.xyzw + p_split.xyzw * p_rotor.s; // XYZW
	return result;
}

Rotor4D Rotor4D::from_array(const PackedRealArray &p_from_array) {
	const int stop_index = MIN(p_from_array.size(), 8);
	Rotor4D rot;
	for (int i = 0; i < stop_index; i++) {
		rot.elements[i] = p_from_array[i];
	}
	return rot;
}

PackedRealArray Rotor4D::to_array() const {
	PackedRealArray arr;
	arr.resize(8);
	real_t *ptr = arr.ptrw();
	ptr[0] = s;
	ptr[1] = xy;
	ptr[2] = xz;
	ptr[3] = xw;
	ptr[4] = yz;
	ptr[5] = yw;
	ptr[6] = zw;
	ptr[7] = xyzw;
	return arr;
}

Rotor4D Rotor4D::from_scalar(const real_t p_scalar) {
	Rotor4D result;
	result.parts.scalar = p_scalar;
	return result;
}

Rotor4D Rotor4D::from_bivector(const Bivector4D &p_bivector) {
	Rotor4D result;
	result.parts.bivector = p_bivector;
	return result;
}

Rotor4D Rotor4D::from_pseudoscalar(const real_t p_pseudoscalar) {
	Rotor4D result;
	result.parts.pseudoscalar = p_pseudoscalar;
	return result;
}

Rotor4D Rotor4D::from_scalar_bivector(const real_t p_scalar, const Bivector4D &p_bivector) {
	Rotor4D result;
	result.parts.scalar = p_scalar;
	result.parts.bivector = p_bivector;
	return result;
}

Rotor4D Rotor4D::from_scalar_pseudoscalar(const real_t p_scalar, const real_t p_pseudoscalar) {
	Rotor4D result;
	result.parts.scalar = p_scalar;
	result.parts.pseudoscalar = p_pseudoscalar;
	return result;
}

Rotor4D Rotor4D::from_bivector_pseudoscalar(const Bivector4D &p_bivector, const real_t p_pseudoscalar) {
	Rotor4D result;
	result.parts.bivector = p_bivector;
	result.parts.pseudoscalar = p_pseudoscalar;
	return result;
}

Rotor4D::Rotor4D(const real_t p_scalar, const Bivector4D &p_bivector, const real_t p_pseudoscalar) {
	parts.scalar = p_scalar;
	parts.bivector = p_bivector;
	parts.pseudoscalar = p_pseudoscalar;
}

Rotor4D::Rotor4D(const real_t p_s, const real_t p_xy, const real_t p_xz, const real_t p_xw, const real_t p_yz, const real_t p_yw, const real_t p_zw, const real_t p_xyzw) {
	s = p_s;
	xy = p_xy;
	xz = p_xz;
	xw = p_xw;
	yz = p_yz;
	yw = p_yw;
	zw = p_zw;
	xyzw = p_xyzw;
}

#include "component_to_string.inc"

Rotor4D::operator String() const {
	String ret;
	if (!Math::is_zero_approx(s)) {
		ret += String::num(s);
	}
	APPEND_COMPONENT_TO_STRING(ret, xy);
	APPEND_COMPONENT_TO_STRING(ret, xz);
	APPEND_COMPONENT_TO_STRING(ret, xw);
	APPEND_COMPONENT_TO_STRING(ret, yz);
	APPEND_COMPONENT_TO_STRING(ret, yw);
	APPEND_COMPONENT_TO_STRING(ret, zw);
	APPEND_COMPONENT_TO_STRING(ret, xyzw);
	return ret;
}

static_assert(sizeof(Rotor4D) == 8 * sizeof(real_t));
