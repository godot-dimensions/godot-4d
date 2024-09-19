#include "multivector_4d.h"

// Geometric algebra functions.

// Conjugate flips the sign of the vector and bivector components.
// 1 - x - y - z - w - xy - xz - xw - yz - yw - zw + xyz + xyw + xzw + yzw + xyzw
Multivector4D Multivector4D::conjugate() const {
	Multivector4D result;
	result.s = +s;
	result.x = -x;
	result.y = -y;
	result.z = -z;
	result.w = -w;
	result.xy = -xy;
	result.xz = -xz;
	result.xw = -xw;
	result.yz = -yz;
	result.yw = -yw;
	result.zw = -zw;
	result.xyz = +xyz;
	result.xyw = +xyw;
	result.xzw = +xzw;
	result.yzw = +yzw;
	result.xyzw = +xyzw;
	return result;
}

// The dot product measures how aligned two multivectors are.
real_t Multivector4D::dot(const Multivector4D &p_b) const {
	return s * p_b.s + x * p_b.x + y * p_b.y + z * p_b.z + w * p_b.w + xy * p_b.xy + xz * p_b.xz + xw * p_b.xw + yz * p_b.yz + yw * p_b.yw + zw * p_b.zw + xyz * p_b.xyz + xyw * p_b.xyw + xzw * p_b.xzw + yzw * p_b.yzw + xyzw * p_b.xyzw;
}

// Dual multiplies each component by the unit pseudoscalar (xyzw) from the right.
// For example, xy * xyzw = xyxyzw = -xxyyzw = -zw, and xz * xyzw = xzxyzw = +xxyzzw = +yw.
// This is symmetric front to back, result[i] is set to +/- elements[15 - i].
// Note that the dual is more complicated in some spaces, but for vanilla geometric algebra it's fairly simple.
// Reference: https://www.youtube.com/watch?v=2AKt6adG_OI&t=1610s
Multivector4D Multivector4D::dual() const {
	Multivector4D result;
	result.s = +xyzw;
	result.x = +yzw;
	result.y = -xzw;
	result.z = +xyw;
	result.w = -xyz;
	result.xy = -zw;
	result.xz = +yw;
	result.xw = -yz;
	result.yz = -xw;
	result.yw = +xz;
	result.zw = -xy;
	result.xyz = -w;
	result.xyw = +z;
	result.xzw = -y;
	result.yzw = +x;
	result.xyzw = +s;
	return result;
}

// The full geometric algebra inner product with all components. See scalar_product if you only want the scalar part.
// This represents a projection and then a contraction. It is bilinear, not symmetric, and may be negative.
// The inner product two basis multivectors is their geometric product if they share all factors.
// Reference: https://www.youtube.com/watch?v=2AKt6adG_OI&t=1850s
Multivector4D Multivector4D::inner_product(const Multivector4D &p_b) const {
	Multivector4D result;
	result.s = s * p_b.s + x * p_b.x + y * p_b.y + z * p_b.z + w * p_b.w - xy * p_b.xy - xz * p_b.xz - xw * p_b.xw - yz * p_b.yz - yw * p_b.yw - zw * p_b.zw - xyz * p_b.xyz - xyw * p_b.xyw - xzw * p_b.xzw - yzw * p_b.yzw + xyzw * p_b.xyzw;
	result.x = s * p_b.x + x * p_b.s - y * p_b.xy - z * p_b.xz - w * p_b.xw + xy * p_b.y + xz * p_b.z + xw * p_b.w - yz * p_b.xyz - yw * p_b.xyw - zw * p_b.xzw - xyz * p_b.yz - xyw * p_b.yw - xzw * p_b.zw + yzw * p_b.xyzw - xyzw * p_b.yzw;
	result.y = s * p_b.y + y * p_b.s + x * p_b.xy - z * p_b.yz - w * p_b.yw - xy * p_b.x + yz * p_b.z + yw * p_b.w + xz * p_b.xyz + xw * p_b.xyw - zw * p_b.yzw + xyz * p_b.xz + xyw * p_b.xw - yzw * p_b.zw - xzw * p_b.xyzw + xyzw * p_b.xzw;
	result.z = s * p_b.z + z * p_b.s + x * p_b.xz + y * p_b.yz - w * p_b.zw - xz * p_b.x - yz * p_b.y + zw * p_b.w - xy * p_b.xyz + xw * p_b.xzw + yw * p_b.yzw - xyz * p_b.xy + xzw * p_b.xw + yzw * p_b.yw + xyw * p_b.xyzw - xyzw * p_b.xyw;
	result.w = s * p_b.w + w * p_b.s + x * p_b.xw + y * p_b.yw + z * p_b.zw - xw * p_b.x - yw * p_b.y - zw * p_b.z - xy * p_b.xyw - xz * p_b.xzw - yz * p_b.yzw - xyw * p_b.xy - xzw * p_b.xz - yzw * p_b.yz - xyz * p_b.xyzw + xyzw * p_b.xyz;
	result.xy = s * p_b.xy + xy * p_b.s + z * p_b.xyz + w * p_b.xyw - zw * p_b.xyzw + xyz * p_b.z + xyw * p_b.w - xyzw * p_b.zw;
	result.xz = s * p_b.xz + xz * p_b.s - y * p_b.xyz + w * p_b.xzw + yw * p_b.xyzw - xyz * p_b.y + xzw * p_b.w + xyzw * p_b.yw;
	result.xw = s * p_b.xw + xw * p_b.s - y * p_b.xyw - z * p_b.xzw - yz * p_b.xyzw - xyw * p_b.y - xzw * p_b.z - xyzw * p_b.yz;
	result.yz = s * p_b.yz + yz * p_b.s + x * p_b.xyz + w * p_b.yzw - xw * p_b.xyzw + xyz * p_b.x + yzw * p_b.w - xyzw * p_b.xw;
	result.yw = s * p_b.yw + yw * p_b.s + x * p_b.xyw - z * p_b.yzw + xz * p_b.xyzw + xyw * p_b.x - yzw * p_b.z + xyzw * p_b.xz;
	result.zw = s * p_b.zw + zw * p_b.s + x * p_b.xzw + y * p_b.yzw - xy * p_b.xyzw + xzw * p_b.x + yzw * p_b.y - xyzw * p_b.xy;
	result.xyz = s * p_b.xyz - w * p_b.xyzw + xyz * p_b.s + xyzw * p_b.w;
	result.xyw = s * p_b.xyw + z * p_b.xyzw + xyw * p_b.s - xyzw * p_b.z;
	result.xzw = s * p_b.xzw - y * p_b.xyzw + xzw * p_b.s + xyzw * p_b.y;
	result.yzw = s * p_b.yzw + x * p_b.xyzw + yzw * p_b.s - xyzw * p_b.x;
	result.xyzw = s * p_b.xyzw + xyzw * p_b.s;
	return result;
}

Multivector4D Multivector4D::inverse() const {
	Multivector4D rev = reverse();
	Multivector4D first = (*this) * rev;
	Multivector4D negate_s = first;
	negate_s.s = -negate_s.s;
	real_t scalar = first.scalar_product(negate_s);
	Multivector4D inv = rev * negate_s / scalar;
	return inv;
}

// Grade involution flips the sign of odd-numbered grades. For 4D this means the vector and trivector components.
// 1 - x - y - z - w + xy + xz + xw + yz + yw + zw - xyz - xyw - xzw - yzw + xyzw
// Reference: https://www.youtube.com/watch?v=2AKt6adG_OI&t=860s
Multivector4D Multivector4D::involute() const {
	Multivector4D result;
	result.s = +s;
	result.x = -x;
	result.y = -y;
	result.z = -z;
	result.w = -w;
	result.xy = +xy;
	result.xz = +xz;
	result.xw = +xw;
	result.yz = +yz;
	result.yw = +yw;
	result.zw = +zw;
	result.xyz = -xyz;
	result.xyw = -xyw;
	result.xzw = -xzw;
	result.yzw = -yzw;
	result.xyzw = +xyzw;
	return result;
}

// The regressive product is the dual of the wedge/exterior/outer product.
// It represents the largest subspace contained by the input multivectors, so long as the duals of the inputs are linearly independent.
// Reference: https://www.youtube.com/watch?v=2AKt6adG_OI&t=1380s
Multivector4D Multivector4D::regressive_product(const Multivector4D &p_b) const {
	Multivector4D result;
	result.s = s * p_b.xyzw - x * p_b.yzw + y * p_b.xzw - z * p_b.xyw + w * p_b.xyz + xy * p_b.zw - xz * p_b.yw + xw * p_b.yz + yz * p_b.xw - yw * p_b.xz + zw * p_b.xy - xyz * p_b.w + xyw * p_b.z - xzw * p_b.y + yzw * p_b.x + xyzw * p_b.s;
	result.x = x * p_b.xyzw + xy * p_b.xzw - xz * p_b.xyw + xw * p_b.xyz + xyz * p_b.xw - xyw * p_b.xz + xzw * p_b.xy + xyzw * p_b.x;
	result.y = y * p_b.xyzw + xy * p_b.yzw - yz * p_b.xyw + yw * p_b.xyz + xyz * p_b.yw - xyw * p_b.yz + yzw * p_b.xy + xyzw * p_b.y;
	result.z = z * p_b.xyzw + xz * p_b.yzw - yz * p_b.xzw + zw * p_b.xyz + xyz * p_b.zw - xzw * p_b.yz + yzw * p_b.xz + xyzw * p_b.z;
	result.w = w * p_b.xyzw + xw * p_b.yzw - yw * p_b.xzw + zw * p_b.xyw + xyw * p_b.zw - xzw * p_b.yw + yzw * p_b.xw + xyzw * p_b.w;
	result.xy = xy * p_b.xyzw - xyz * p_b.xyw + xyw * p_b.xyz + xyzw * p_b.xy;
	result.xz = xz * p_b.xyzw - xyz * p_b.xzw + xzw * p_b.xyz + xyzw * p_b.xz;
	result.xw = xw * p_b.xyzw - xyw * p_b.xzw + xzw * p_b.xyw + xyzw * p_b.xw;
	result.yz = yz * p_b.xyzw - xyz * p_b.yzw + yzw * p_b.xyz + xyzw * p_b.yz;
	result.yw = yw * p_b.xyzw - xyw * p_b.yzw + yzw * p_b.xyw + xyzw * p_b.yw;
	result.zw = zw * p_b.xyzw - xzw * p_b.yzw + yzw * p_b.xzw + xyzw * p_b.zw;
	result.xyz = xyz * p_b.xyzw + xyzw * p_b.xyz;
	result.xyw = xyw * p_b.xyzw + xyzw * p_b.xyw;
	result.xzw = xzw * p_b.xyzw + xyzw * p_b.xzw;
	result.yzw = yzw * p_b.xyzw + xyzw * p_b.yzw;
	result.xyzw = xyzw * p_b.xyzw;
	return result;
}

// The reverse function flips the sign of pairs of grades (when grade/2 is odd). For 4D this means the bivector and trivector components.
// 1 + x + y + z + w - xy - xz - xw - yz - yw - zw - xyz - xyw - xzw - yzw + xyzw
// Reference: https://www.youtube.com/watch?v=2AKt6adG_OI&t=720s
Multivector4D Multivector4D::reverse() const {
	Multivector4D result;
	result.s = +s;
	result.x = +x;
	result.y = +y;
	result.z = +z;
	result.w = +w;
	result.xy = -xy;
	result.xz = -xz;
	result.xw = -xw;
	result.yz = -yz;
	result.yw = -yw;
	result.zw = -zw;
	result.xyz = -xyz;
	result.xyw = -xyw;
	result.xzw = -xzw;
	result.yzw = -yzw;
	result.xyzw = +xyzw;
	return result;
}

// The scalar part of the geometric algebra inner product only. This is also the scalar part of multiplication.
real_t Multivector4D::scalar_product(const Multivector4D &p_b) const {
	return s * p_b.s + x * p_b.x + y * p_b.y + z * p_b.z + w * p_b.w - xy * p_b.xy - xz * p_b.xz - xw * p_b.xw - yz * p_b.yz - yw * p_b.yw - zw * p_b.zw - xyz * p_b.xyz - xyw * p_b.xyw - xzw * p_b.xzw - yzw * p_b.yzw + xyzw * p_b.xyzw;
}

// The wedge/exterior/outer product is the dual of the regressive product.
// It represents the smallest subspace containing the input multivectors.
// The outer product of two basis multivectors is their geometric product if they share no factors.
Multivector4D Multivector4D::wedge_product(const Multivector4D &p_b) const {
	Multivector4D result;
	result.s = s * p_b.s;
	result.x = s * p_b.x + x * p_b.s;
	result.y = s * p_b.y + y * p_b.s;
	result.z = s * p_b.z + z * p_b.s;
	result.w = s * p_b.w + w * p_b.s;
	result.xy = s * p_b.xy + x * p_b.y - y * p_b.x + xy * p_b.s;
	result.xz = s * p_b.xz + x * p_b.z - z * p_b.x + xz * p_b.s;
	result.xw = s * p_b.xw + x * p_b.w - w * p_b.x + xw * p_b.s;
	result.yz = s * p_b.yz + y * p_b.z - z * p_b.y + yz * p_b.s;
	result.yw = s * p_b.yw + y * p_b.w - w * p_b.y + yw * p_b.s;
	result.zw = s * p_b.zw + z * p_b.w - w * p_b.z + zw * p_b.s;
	result.xyz = s * p_b.xyz + x * p_b.yz - y * p_b.xz + z * p_b.xy + xy * p_b.z - xz * p_b.y + yz * p_b.x + xyz * p_b.s;
	result.xyw = s * p_b.xyw + x * p_b.yw - y * p_b.xw + w * p_b.xy + xy * p_b.w - xw * p_b.y + yw * p_b.x + xyw * p_b.s;
	result.xzw = s * p_b.xzw + x * p_b.zw - z * p_b.xw + w * p_b.xz + xz * p_b.w - xw * p_b.z + zw * p_b.x + xzw * p_b.s;
	result.yzw = s * p_b.yzw + y * p_b.zw - z * p_b.yw + w * p_b.yz + yz * p_b.w - yw * p_b.z + zw * p_b.y + yzw * p_b.s;
	result.xyzw = s * p_b.xyzw + x * p_b.yzw - y * p_b.xzw + z * p_b.xyw - w * p_b.xyz + xy * p_b.zw - xz * p_b.yw + xw * p_b.yz + yz * p_b.xw - yw * p_b.xz + zw * p_b.xy + xyz * p_b.w - xyw * p_b.z + xzw * p_b.y - yzw * p_b.x + xyzw * p_b.s;
	return result;
}

// Rotation functions.

Basis4D Multivector4D::rotate_basis(const Basis4D &p_basis) const {
#ifdef MATH_CHECKS
	ERR_FAIL_COND_V_MSG(!is_rotor(), p_basis, "The Multivector4D must be a valid rotor in order to rotate a vector, but was " + operator String() + ".");
#endif
	const Multivector4D rev = reverse();
	return Basis4D(
			(rev * p_basis.x * *this).vector,
			(rev * p_basis.y * *this).vector,
			(rev * p_basis.z * *this).vector,
			(rev * p_basis.w * *this).vector);
}

Vector4 Multivector4D::rotate_vector(const Vector4 &p_vec) const {
#ifdef MATH_CHECKS
	ERR_FAIL_COND_V_MSG(!is_rotor(), p_vec, "The Multivector4D must be a valid rotor in order to rotate a vector, but was " + operator String() + ".");
#endif
	const Multivector4D rev = reverse();
	// Online resources seem to indicate that it should be r * v * r^-1, but that doesn't work...?
	// Instead, r^-1 * v * r is what correctly rotates the vector in the expected direction.
	// I've double-checked the constructors, products, and multiplication operator and all they seem correct.
	// This seems really weird, since Quaternions use q * v * q^-1 to do this. - aaronfranke
	const Multivector4D multivec = rev * p_vec * *this;
	return multivec.vector;
}

// Length functions.

real_t Multivector4D::length() const {
	return Math::sqrt(length_squared());
}

// The length squared is the magnitude squared, which is the square of the length or magnitude.
// The formula for length squared of a multivector is actually the scalar part of `v.reverse() * v`.
// However, for vanilla geometric algebra, this simplifies to the same trivial formula you'd expect for vectors and scalars.
// Reference: https://www.youtube.com/watch?v=2AKt6adG_OI&t=940s
real_t Multivector4D::length_squared() const {
	return scalar * scalar + vector.length_squared() + bivector.length_squared() + trivector.length_squared() + pseudoscalar * pseudoscalar;
}

Multivector4D Multivector4D::normalized() const {
	const real_t len_sq = length_squared();
	if (len_sq == 0) {
		return Multivector4D();
	}
	return *this / Math::sqrt(len_sq);
}

bool Multivector4D::is_normalized() const {
	return Math::is_equal_approx(length_squared(), (real_t)1.0);
}

bool Multivector4D::is_rotor(const bool p_pure) const {
	if (!Math::is_equal_approx(vector.length_squared(), (real_t)0.0)) {
		return false;
	}
	if (!Math::is_equal_approx(trivector.length_squared(), (real_t)0.0)) {
		return false;
	}
	if (p_pure && !Math::is_equal_approx(pseudoscalar, (real_t)0.0)) {
		return false;
	}
	return Math::is_equal_approx(length_squared(), (real_t)1.0);
}

// Static functions for doing math on non-Multivector4D types and returning a Multivector4D.

// The vector product, also known as the geometric vector product or the Holmér product, is the full multiplication of two vectors.
// The geometric vector product combines the dot product (scalar) and the wedge/cross product (bivector).
// The interpretation of the result is a Rotor4D that, when applied as the sandwich product to a vector,
// double-rotates the vector by the angle between the two input vectors. Therefore, for rotating by
// some angle, the input vectors should have an angle between them of half the desired rotation angle.
Multivector4D Multivector4D::vector_product(const Vector4 &p_a, const Vector4 &p_b) {
	Multivector4D result;
	result.scalar = p_a.dot(p_b);
	result.xy = p_a.x * p_b.y - p_a.y * p_b.x;
	result.xz = p_a.x * p_b.z - p_a.z * p_b.x;
	result.xw = p_a.x * p_b.w - p_a.w * p_b.x;
	result.yz = p_a.y * p_b.z - p_a.z * p_b.y;
	result.yw = p_a.y * p_b.w - p_a.w * p_b.y;
	result.zw = p_a.z * p_b.w - p_a.w * p_b.z;
	return result;
}

// Like the vector product, but does not include the scalar part, only the bivector parts.
// Consider using the Bivector4D class if you do not need a Multivector4D.
Multivector4D Multivector4D::vector_wedge_product(const Vector4 &p_a, const Vector4 &p_b) {
	Multivector4D result;
	result.xy = p_a.x * p_b.y - p_a.y * p_b.x;
	result.xz = p_a.x * p_b.z - p_a.z * p_b.x;
	result.xw = p_a.x * p_b.w - p_a.w * p_b.x;
	result.yz = p_a.y * p_b.z - p_a.z * p_b.y;
	result.yw = p_a.y * p_b.w - p_a.w * p_b.y;
	result.zw = p_a.z * p_b.w - p_a.w * p_b.z;
	return result;
}

// Trivial getters and setters.

real_t Multivector4D::get_scalar() const {
	return scalar;
}

void Multivector4D::set_scalar(const real_t p_scalar) {
	scalar = p_scalar;
}

Vector4 Multivector4D::get_vector() const {
	return vector;
}

void Multivector4D::set_vector(const Vector4 &p_vector) {
	vector = p_vector;
}

Bivector4D Multivector4D::get_bivector() const {
	return bivector;
}

void Multivector4D::set_bivector(const Bivector4D &p_bivector) {
	bivector = p_bivector;
}

Trivector4D Multivector4D::get_trivector() const {
	return trivector;
}

void Multivector4D::set_trivector(const Trivector4D &p_trivector) {
	trivector = p_trivector;
}

real_t Multivector4D::get_pseudoscalar() const {
	return pseudoscalar;
}

void Multivector4D::set_pseudoscalar(const real_t p_pseudoscalar) {
	pseudoscalar = p_pseudoscalar;
}

Rotor4D Multivector4D::get_rotor() const {
	return Rotor4D(scalar, bivector, pseudoscalar);
}

void Multivector4D::set_rotor(const Rotor4D &p_rotor) {
	scalar = p_rotor.scalar;
	bivector = p_rotor.bivector;
	pseudoscalar = p_rotor.pseudoscalar;
}

// Operators.

bool Multivector4D::is_equal_approx(const Multivector4D &p_other) const {
	return Math::is_equal_approx(scalar, p_other.scalar) && vector.is_equal_approx(p_other.vector) && bivector.is_equal_approx(p_other.bivector) && trivector.is_equal_approx(p_other.trivector) && Math::is_equal_approx(pseudoscalar, p_other.pseudoscalar);
}

bool Multivector4D::operator==(const Multivector4D &p_v) const {
	return (scalar == p_v.scalar && vector == p_v.vector && bivector == p_v.bivector && trivector == p_v.trivector && pseudoscalar == p_v.pseudoscalar);
}

bool Multivector4D::operator!=(const Multivector4D &p_v) const {
	return (scalar != p_v.scalar || vector != p_v.vector || bivector != p_v.bivector || trivector != p_v.trivector || pseudoscalar != p_v.pseudoscalar);
}

Multivector4D Multivector4D::operator-() const {
	return Multivector4D(-scalar, -vector, -bivector, -trivector, -pseudoscalar);
}

Multivector4D &Multivector4D::operator+=(const Multivector4D &p_v) {
	scalar += p_v.scalar;
	vector += p_v.vector;
	bivector += p_v.bivector;
	trivector += p_v.trivector;
	pseudoscalar += p_v.pseudoscalar;
	return *this;
}

Multivector4D Multivector4D::operator+(const Multivector4D &p_v) const {
	return Multivector4D(scalar + p_v.scalar, vector + p_v.vector, bivector + p_v.bivector, trivector + p_v.trivector, pseudoscalar + p_v.pseudoscalar);
}

Multivector4D &Multivector4D::operator-=(const Multivector4D &p_v) {
	scalar -= p_v.scalar;
	vector -= p_v.vector;
	bivector -= p_v.bivector;
	trivector -= p_v.trivector;
	pseudoscalar -= p_v.pseudoscalar;
	return *this;
}

Multivector4D Multivector4D::operator-(const Multivector4D &p_v) const {
	return Multivector4D(scalar - p_v.scalar, vector - p_v.vector, bivector - p_v.bivector, trivector - p_v.trivector, pseudoscalar - p_v.pseudoscalar);
}

Multivector4D &Multivector4D::operator*=(const Multivector4D &p_child) {
	// Can't do an in-place multiply because the operation requires the original values throughout the calculation.
	Multivector4D result = *this * p_child;
	scalar = result.scalar;
	vector = result.vector;
	bivector = result.bivector;
	trivector = result.trivector;
	pseudoscalar = result.pseudoscalar;
	return *this;
}

Multivector4D Multivector4D::operator*(const Multivector4D &p_b) const {
	Multivector4D result;
	result[0x0] = elements[0x0] * p_b[0x0] + elements[0x1] * p_b[0x1] + elements[0x2] * p_b[0x2] + elements[0x3] * p_b[0x3] + elements[0x4] * p_b[0x4] - elements[0x5] * p_b[0x5] - elements[0x6] * p_b[0x6] - elements[0x7] * p_b[0x7] - elements[0x8] * p_b[0x8] - elements[0x9] * p_b[0x9] - elements[0xA] * p_b[0xA] - elements[0xB] * p_b[0xB] - elements[0xC] * p_b[0xC] - elements[0xD] * p_b[0xD] - elements[0xE] * p_b[0xE] + elements[0xF] * p_b[0xF]; // Scalar
	result[0x1] = elements[0x0] * p_b[0x1] + elements[0x1] * p_b[0x0] - elements[0x2] * p_b[0x5] - elements[0x3] * p_b[0x6] - elements[0x4] * p_b[0x7] + elements[0x5] * p_b[0x2] + elements[0x6] * p_b[0x3] + elements[0x7] * p_b[0x4] - elements[0x8] * p_b[0xB] - elements[0x9] * p_b[0xC] - elements[0xA] * p_b[0xD] - elements[0xB] * p_b[0x8] - elements[0xC] * p_b[0x9] - elements[0xD] * p_b[0xA] + elements[0xE] * p_b[0xF] - elements[0xF] * p_b[0xE]; // X
	result[0x2] = elements[0x0] * p_b[0x2] + elements[0x1] * p_b[0x5] + elements[0x2] * p_b[0x0] - elements[0x3] * p_b[0x8] - elements[0x4] * p_b[0x9] - elements[0x5] * p_b[0x1] + elements[0x6] * p_b[0xB] + elements[0x7] * p_b[0xC] + elements[0x8] * p_b[0x3] + elements[0x9] * p_b[0x4] - elements[0xA] * p_b[0xE] + elements[0xB] * p_b[0x6] + elements[0xC] * p_b[0x7] - elements[0xD] * p_b[0xF] - elements[0xE] * p_b[0xA] + elements[0xF] * p_b[0xD]; // Y
	result[0x3] = elements[0x0] * p_b[0x3] + elements[0x1] * p_b[0x6] + elements[0x2] * p_b[0x8] + elements[0x3] * p_b[0x0] - elements[0x4] * p_b[0xA] - elements[0x5] * p_b[0xB] - elements[0x6] * p_b[0x1] + elements[0x7] * p_b[0xD] - elements[0x8] * p_b[0x2] + elements[0x9] * p_b[0xE] + elements[0xA] * p_b[0x4] - elements[0xB] * p_b[0x5] + elements[0xC] * p_b[0xF] + elements[0xD] * p_b[0x7] + elements[0xE] * p_b[0x9] - elements[0xF] * p_b[0xC]; // Z
	result[0x4] = elements[0x0] * p_b[0x4] + elements[0x1] * p_b[0x7] + elements[0x2] * p_b[0x9] + elements[0x3] * p_b[0xA] + elements[0x4] * p_b[0x0] - elements[0x5] * p_b[0xC] - elements[0x6] * p_b[0xD] - elements[0x7] * p_b[0x1] - elements[0x8] * p_b[0xE] - elements[0x9] * p_b[0x2] - elements[0xA] * p_b[0x3] - elements[0xB] * p_b[0xF] - elements[0xC] * p_b[0x5] - elements[0xD] * p_b[0x6] - elements[0xE] * p_b[0x8] + elements[0xF] * p_b[0xB]; // W
	result[0x5] = elements[0x0] * p_b[0x5] + elements[0x1] * p_b[0x2] - elements[0x2] * p_b[0x1] + elements[0x3] * p_b[0xB] + elements[0x4] * p_b[0xC] + elements[0x5] * p_b[0x0] - elements[0x6] * p_b[0x8] - elements[0x7] * p_b[0x9] + elements[0x8] * p_b[0x6] + elements[0x9] * p_b[0x7] - elements[0xA] * p_b[0xF] + elements[0xB] * p_b[0x3] + elements[0xC] * p_b[0x4] - elements[0xD] * p_b[0xE] + elements[0xE] * p_b[0xD] - elements[0xF] * p_b[0xA]; // XY
	result[0x6] = elements[0x0] * p_b[0x6] + elements[0x1] * p_b[0x3] - elements[0x2] * p_b[0xB] - elements[0x3] * p_b[0x1] + elements[0x4] * p_b[0xD] + elements[0x5] * p_b[0x8] + elements[0x6] * p_b[0x0] - elements[0x7] * p_b[0xA] - elements[0x8] * p_b[0x5] + elements[0x9] * p_b[0xF] + elements[0xA] * p_b[0x7] - elements[0xB] * p_b[0x2] + elements[0xC] * p_b[0xE] + elements[0xD] * p_b[0x4] - elements[0xE] * p_b[0xC] + elements[0xF] * p_b[0x9]; // XZ
	result[0x7] = elements[0x0] * p_b[0x7] + elements[0x1] * p_b[0x4] - elements[0x2] * p_b[0xC] - elements[0x3] * p_b[0xD] - elements[0x4] * p_b[0x1] + elements[0x5] * p_b[0x9] + elements[0x6] * p_b[0xA] + elements[0x7] * p_b[0x0] - elements[0x8] * p_b[0xF] - elements[0x9] * p_b[0x5] - elements[0xA] * p_b[0x6] - elements[0xB] * p_b[0xE] - elements[0xC] * p_b[0x2] - elements[0xD] * p_b[0x3] + elements[0xE] * p_b[0xB] - elements[0xF] * p_b[0x8]; // XW
	result[0x8] = elements[0x0] * p_b[0x8] + elements[0x1] * p_b[0xB] + elements[0x2] * p_b[0x3] - elements[0x3] * p_b[0x2] + elements[0x4] * p_b[0xE] - elements[0x5] * p_b[0x6] + elements[0x6] * p_b[0x5] - elements[0x7] * p_b[0xF] + elements[0x8] * p_b[0x0] - elements[0x9] * p_b[0xA] + elements[0xA] * p_b[0x9] + elements[0xB] * p_b[0x1] - elements[0xC] * p_b[0xD] + elements[0xD] * p_b[0xC] + elements[0xE] * p_b[0x4] - elements[0xF] * p_b[0x7]; // YZ
	result[0x9] = elements[0x0] * p_b[0x9] + elements[0x1] * p_b[0xC] + elements[0x2] * p_b[0x4] - elements[0x3] * p_b[0xE] - elements[0x4] * p_b[0x2] - elements[0x5] * p_b[0x7] + elements[0x6] * p_b[0xF] + elements[0x7] * p_b[0x5] + elements[0x8] * p_b[0xA] + elements[0x9] * p_b[0x0] - elements[0xA] * p_b[0x8] + elements[0xB] * p_b[0xD] + elements[0xC] * p_b[0x1] - elements[0xD] * p_b[0xB] - elements[0xE] * p_b[0x3] + elements[0xF] * p_b[0x6]; // YW
	result[0xA] = elements[0x0] * p_b[0xA] + elements[0x1] * p_b[0xD] + elements[0x2] * p_b[0xE] + elements[0x3] * p_b[0x4] - elements[0x4] * p_b[0x3] - elements[0x5] * p_b[0xF] - elements[0x6] * p_b[0x7] + elements[0x7] * p_b[0x6] - elements[0x8] * p_b[0x9] + elements[0x9] * p_b[0x8] + elements[0xA] * p_b[0x0] - elements[0xB] * p_b[0xC] + elements[0xC] * p_b[0xB] + elements[0xD] * p_b[0x1] + elements[0xE] * p_b[0x2] - elements[0xF] * p_b[0x5]; // ZW
	result[0xB] = elements[0x0] * p_b[0xB] + elements[0x1] * p_b[0x8] - elements[0x2] * p_b[0x6] + elements[0x3] * p_b[0x5] - elements[0x4] * p_b[0xF] + elements[0x5] * p_b[0x3] - elements[0x6] * p_b[0x2] + elements[0x7] * p_b[0xE] + elements[0x8] * p_b[0x1] - elements[0x9] * p_b[0xD] + elements[0xA] * p_b[0xC] + elements[0xB] * p_b[0x0] - elements[0xC] * p_b[0xA] + elements[0xD] * p_b[0x9] - elements[0xE] * p_b[0x7] + elements[0xF] * p_b[0x4]; // XYZ
	result[0xC] = elements[0x0] * p_b[0xC] + elements[0x1] * p_b[0x9] - elements[0x2] * p_b[0x7] + elements[0x3] * p_b[0xF] + elements[0x4] * p_b[0x5] + elements[0x5] * p_b[0x4] - elements[0x6] * p_b[0xE] - elements[0x7] * p_b[0x2] + elements[0x8] * p_b[0xD] + elements[0x9] * p_b[0x1] - elements[0xA] * p_b[0xB] + elements[0xB] * p_b[0xA] + elements[0xC] * p_b[0x0] - elements[0xD] * p_b[0x8] + elements[0xE] * p_b[0x6] - elements[0xF] * p_b[0x3]; // XYW
	result[0xD] = elements[0x0] * p_b[0xD] + elements[0x1] * p_b[0xA] - elements[0x2] * p_b[0xF] - elements[0x3] * p_b[0x7] + elements[0x4] * p_b[0x6] + elements[0x5] * p_b[0xE] + elements[0x6] * p_b[0x4] - elements[0x7] * p_b[0x3] - elements[0x8] * p_b[0xC] + elements[0x9] * p_b[0xB] + elements[0xA] * p_b[0x1] - elements[0xB] * p_b[0x9] + elements[0xC] * p_b[0x8] + elements[0xD] * p_b[0x0] - elements[0xE] * p_b[0x5] + elements[0xF] * p_b[0x2]; // XZW
	result[0xE] = elements[0x0] * p_b[0xE] + elements[0x1] * p_b[0xF] + elements[0x2] * p_b[0xA] - elements[0x3] * p_b[0x9] + elements[0x4] * p_b[0x8] - elements[0x5] * p_b[0xD] + elements[0x6] * p_b[0xC] - elements[0x7] * p_b[0xB] + elements[0x8] * p_b[0x4] - elements[0x9] * p_b[0x3] + elements[0xA] * p_b[0x2] + elements[0xB] * p_b[0x7] - elements[0xC] * p_b[0x6] + elements[0xD] * p_b[0x5] + elements[0xE] * p_b[0x0] - elements[0xF] * p_b[0x1]; // YZW
	result[0xF] = elements[0x0] * p_b[0xF] + elements[0x1] * p_b[0xE] - elements[0x2] * p_b[0xD] + elements[0x3] * p_b[0xC] - elements[0x4] * p_b[0xB] + elements[0x5] * p_b[0xA] - elements[0x6] * p_b[0x9] + elements[0x7] * p_b[0x8] + elements[0x8] * p_b[0x7] - elements[0x9] * p_b[0x6] + elements[0xA] * p_b[0x5] + elements[0xB] * p_b[0x4] - elements[0xC] * p_b[0x3] + elements[0xD] * p_b[0x2] - elements[0xE] * p_b[0x1] + elements[0xF] * p_b[0x0]; // XYZW
	return result;
}

// Operators with Multivector4D on the left and other things on the right.

Multivector4D &Multivector4D::operator+=(const real_t p_scalar) {
	scalar += p_scalar;
	return *this;
}

Multivector4D Multivector4D::operator+(const real_t p_scalar) const {
	Multivector4D result = *this;
	result.scalar += p_scalar;
	return result;
}

Multivector4D &Multivector4D::operator-=(const real_t p_scalar) {
	scalar -= p_scalar;
	return *this;
}

Multivector4D Multivector4D::operator-(const real_t p_scalar) const {
	Multivector4D result = *this;
	result.scalar -= p_scalar;
	return result;
}

Multivector4D &Multivector4D::operator*=(const real_t p_scalar) {
	scalar *= p_scalar;
	vector *= p_scalar;
	bivector *= p_scalar;
	trivector *= p_scalar;
	pseudoscalar *= p_scalar;
	return *this;
}

Multivector4D Multivector4D::operator*(const real_t p_scalar) const {
	return Multivector4D(scalar * p_scalar, vector * p_scalar, bivector * p_scalar, trivector * p_scalar, pseudoscalar * p_scalar);
}

Multivector4D &Multivector4D::operator/=(const real_t p_scalar) {
	scalar /= p_scalar;
	vector /= p_scalar;
	bivector /= p_scalar;
	trivector /= p_scalar;
	pseudoscalar /= p_scalar;
	return *this;
}

Multivector4D Multivector4D::operator/(const real_t p_scalar) const {
	return Multivector4D(scalar / p_scalar, vector / p_scalar, bivector / p_scalar, trivector / p_scalar, pseudoscalar / p_scalar);
}

Multivector4D Multivector4D::operator+(const Vector4 &p_vector) const {
	Multivector4D result = *this;
	result.vector += p_vector;
	return result;
}

Multivector4D Multivector4D::operator+(const Bivector4D &p_b) const {
	Multivector4D result = *this;
	result.bivector += p_b;
	return result;
}

Multivector4D Multivector4D::operator+(const Trivector4D &p_b) const {
	Multivector4D result = *this;
	result.trivector += p_b;
	return result;
}

Multivector4D Multivector4D::operator+(const Rotor4D &p_b) const {
	Multivector4D result = *this;
	result.scalar += p_b.scalar;
	result.bivector += p_b.bivector;
	result.pseudoscalar += p_b.pseudoscalar;
	return result;
}

Multivector4D Multivector4D::operator*(const Vector4 &p_b) const {
	Multivector4D result;
	result[0x0] = elements[0x1] * p_b.x + elements[0x2] * p_b.y + elements[0x3] * p_b.z + elements[0x4] * p_b.w; // Scalar
	result[0x1] = elements[0x0] * p_b.x + elements[0x5] * p_b.y + elements[0x6] * p_b.z + elements[0x7] * p_b.w; // X
	result[0x2] = elements[0x0] * p_b.y - elements[0x5] * p_b.x + elements[0x8] * p_b.z + elements[0x9] * p_b.w; // Y
	result[0x3] = elements[0x0] * p_b.z - elements[0x6] * p_b.x - elements[0x8] * p_b.y + elements[0xA] * p_b.w; // Z
	result[0x4] = elements[0x0] * p_b.w - elements[0x7] * p_b.x - elements[0x9] * p_b.y - elements[0xA] * p_b.z; // W
	result[0x5] = elements[0x1] * p_b.y - elements[0x2] * p_b.x + elements[0xB] * p_b.z + elements[0xC] * p_b.w; // XY
	result[0x6] = elements[0x1] * p_b.z - elements[0x3] * p_b.x - elements[0xB] * p_b.y + elements[0xD] * p_b.w; // XZ
	result[0x7] = elements[0x1] * p_b.w - elements[0x4] * p_b.x - elements[0xC] * p_b.y - elements[0xD] * p_b.z; // XW
	result[0x8] = elements[0x2] * p_b.z - elements[0x3] * p_b.y + elements[0xB] * p_b.x + elements[0xE] * p_b.w; // YZ
	result[0x9] = elements[0x2] * p_b.w - elements[0x4] * p_b.y + elements[0xC] * p_b.x - elements[0xE] * p_b.z; // YW
	result[0xA] = elements[0x3] * p_b.w - elements[0x4] * p_b.z + elements[0xD] * p_b.x + elements[0xE] * p_b.y; // ZW
	result[0xB] = elements[0x5] * p_b.z - elements[0x6] * p_b.y + elements[0x8] * p_b.x + elements[0xF] * p_b.w; // XYZ
	result[0xC] = elements[0x5] * p_b.w - elements[0x7] * p_b.y + elements[0x9] * p_b.x - elements[0xF] * p_b.z; // XYW
	result[0xD] = elements[0x6] * p_b.w - elements[0x7] * p_b.z + elements[0xA] * p_b.x + elements[0xF] * p_b.y; // XZW
	result[0xE] = elements[0x8] * p_b.w - elements[0x9] * p_b.z + elements[0xA] * p_b.y - elements[0xF] * p_b.x; // YZW
	result[0xF] = elements[0xB] * p_b.w - elements[0xC] * p_b.z + elements[0xD] * p_b.y - elements[0xE] * p_b.x; // XYZW
	return result;
}

Multivector4D Multivector4D::operator*(const Bivector4D &p_b) const {
	Multivector4D result;
	result[0x0] = -elements[0x5] * p_b.xy - elements[0x6] * p_b.xz - elements[0x7] * p_b.xw - elements[0x8] * p_b.yz - elements[0x9] * p_b.yw - elements[0xA] * p_b.zw; // Scalar
	result[0x1] = -elements[0x2] * p_b.xy - elements[0x3] * p_b.xz - elements[0x4] * p_b.xw - elements[0xB] * p_b.yz - elements[0xC] * p_b.yw - elements[0xD] * p_b.zw; // X
	result[0x2] = +elements[0x1] * p_b.xy - elements[0x3] * p_b.yz - elements[0x4] * p_b.yw + elements[0xB] * p_b.xz + elements[0xC] * p_b.xw - elements[0xE] * p_b.zw; // Y
	result[0x3] = +elements[0x1] * p_b.xz + elements[0x2] * p_b.yz - elements[0x4] * p_b.zw - elements[0xB] * p_b.xy + elements[0xD] * p_b.xw + elements[0xE] * p_b.yw; // Z
	result[0x4] = +elements[0x1] * p_b.xw + elements[0x2] * p_b.yw + elements[0x3] * p_b.zw - elements[0xC] * p_b.xy - elements[0xD] * p_b.xz - elements[0xE] * p_b.yz; // W
	result[0x5] = +elements[0x0] * p_b.xy - elements[0x6] * p_b.yz - elements[0x7] * p_b.yw + elements[0x8] * p_b.xz + elements[0x9] * p_b.xw - elements[0xF] * p_b.zw; // XY
	result[0x6] = +elements[0x0] * p_b.xz + elements[0x5] * p_b.yz - elements[0x7] * p_b.zw - elements[0x8] * p_b.xy + elements[0xA] * p_b.xw + elements[0xF] * p_b.yw; // XZ
	result[0x7] = +elements[0x0] * p_b.xw + elements[0x5] * p_b.yw + elements[0x6] * p_b.zw - elements[0x9] * p_b.xy - elements[0xA] * p_b.xz - elements[0xF] * p_b.yz; // XW
	result[0x8] = +elements[0x0] * p_b.yz - elements[0x5] * p_b.xz + elements[0x6] * p_b.xy - elements[0x9] * p_b.zw + elements[0xA] * p_b.yw - elements[0xF] * p_b.xw; // YZ
	result[0x9] = +elements[0x0] * p_b.yw - elements[0x5] * p_b.xw + elements[0x7] * p_b.xy + elements[0x8] * p_b.zw - elements[0xA] * p_b.yz + elements[0xF] * p_b.xz; // YW
	result[0xA] = +elements[0x0] * p_b.zw - elements[0x6] * p_b.xw + elements[0x7] * p_b.xz - elements[0x8] * p_b.yw + elements[0x9] * p_b.yz - elements[0xF] * p_b.xy; // ZW
	result[0xB] = +elements[0x1] * p_b.yz - elements[0x2] * p_b.xz + elements[0x3] * p_b.xy - elements[0xC] * p_b.zw + elements[0xD] * p_b.yw - elements[0xE] * p_b.xw; // XYZ
	result[0xC] = +elements[0x1] * p_b.yw - elements[0x2] * p_b.xw + elements[0x4] * p_b.xy + elements[0xB] * p_b.zw - elements[0xD] * p_b.yz + elements[0xE] * p_b.xz; // XYW
	result[0xD] = +elements[0x1] * p_b.zw - elements[0x3] * p_b.xw + elements[0x4] * p_b.xz - elements[0xB] * p_b.yw + elements[0xC] * p_b.yz - elements[0xE] * p_b.xy; // XZW
	result[0xE] = +elements[0x2] * p_b.zw - elements[0x3] * p_b.yw + elements[0x4] * p_b.yz + elements[0xB] * p_b.xw - elements[0xC] * p_b.xz + elements[0xD] * p_b.xy; // YZW
	result[0xF] = +elements[0x5] * p_b.zw - elements[0x6] * p_b.yw + elements[0x7] * p_b.yz + elements[0x8] * p_b.xw - elements[0x9] * p_b.xz + elements[0xA] * p_b.xy; // XYZW
	return result;
}

Multivector4D Multivector4D::operator*(const Trivector4D &p_b) const {
	Multivector4D result;
	result[0x0] = -elements[0xB] * p_b.xyz - elements[0xC] * p_b.xyw - elements[0xD] * p_b.xzw - elements[0xE] * p_b.yzw; // Scalar
	result[0x1] = -elements[0x8] * p_b.xyz - elements[0x9] * p_b.xyw - elements[0xA] * p_b.xzw - elements[0xF] * p_b.yzw; // X
	result[0x2] = +elements[0x6] * p_b.xyz + elements[0x7] * p_b.xyw - elements[0xA] * p_b.yzw + elements[0xF] * p_b.xzw; // Y
	result[0x3] = -elements[0x5] * p_b.xyz + elements[0x7] * p_b.xzw + elements[0x9] * p_b.yzw - elements[0xF] * p_b.xyw; // Z
	result[0x4] = -elements[0x5] * p_b.xyw - elements[0x6] * p_b.xzw - elements[0x8] * p_b.yzw + elements[0xF] * p_b.xyz; // W
	result[0x5] = +elements[0x3] * p_b.xyz + elements[0x4] * p_b.xyw - elements[0xD] * p_b.yzw + elements[0xE] * p_b.xzw; // XY
	result[0x6] = -elements[0x2] * p_b.xyz + elements[0x4] * p_b.xzw + elements[0xC] * p_b.yzw - elements[0xE] * p_b.xyw; // XZ
	result[0x7] = -elements[0x2] * p_b.xyw - elements[0x3] * p_b.xzw - elements[0xB] * p_b.yzw + elements[0xE] * p_b.xyz; // XW
	result[0x8] = +elements[0x1] * p_b.xyz + elements[0x4] * p_b.yzw - elements[0xC] * p_b.xzw + elements[0xD] * p_b.xyw; // YZ
	result[0x9] = +elements[0x1] * p_b.xyw - elements[0x3] * p_b.yzw + elements[0xB] * p_b.xzw - elements[0xD] * p_b.xyz; // YW
	result[0xA] = +elements[0x1] * p_b.xzw + elements[0x2] * p_b.yzw - elements[0xB] * p_b.xyw + elements[0xC] * p_b.xyz; // ZW
	result[0xB] = +elements[0x0] * p_b.xyz + elements[0x7] * p_b.yzw - elements[0x9] * p_b.xzw + elements[0xA] * p_b.xyw; // XYZ
	result[0xC] = +elements[0x0] * p_b.xyw - elements[0x6] * p_b.yzw + elements[0x8] * p_b.xzw - elements[0xA] * p_b.xyz; // XYW
	result[0xD] = +elements[0x0] * p_b.xzw + elements[0x5] * p_b.yzw - elements[0x8] * p_b.xyw + elements[0x9] * p_b.xyz; // XZW
	result[0xE] = +elements[0x0] * p_b.yzw - elements[0x5] * p_b.xzw + elements[0x6] * p_b.xyw - elements[0x7] * p_b.xyz; // YZW
	result[0xF] = +elements[0x1] * p_b.yzw - elements[0x2] * p_b.xzw + elements[0x3] * p_b.xyw - elements[0x4] * p_b.xyz; // XYZW
	return result;
}

Multivector4D Multivector4D::operator*(const Rotor4D &p_b) const {
	Multivector4D result;
	result[0x0] = elements[0x0] * p_b.s - elements[0x5] * p_b.xy - elements[0x6] * p_b.xz - elements[0x7] * p_b.xw - elements[0x8] * p_b.yz - elements[0x9] * p_b.yw - elements[0xA] * p_b.zw + elements[0xF] * p_b.xyzw; // Scalar
	result[0x1] = elements[0x1] * p_b.s - elements[0x2] * p_b.xy - elements[0x3] * p_b.xz - elements[0x4] * p_b.xw - elements[0xB] * p_b.yz - elements[0xC] * p_b.yw - elements[0xD] * p_b.zw + elements[0xE] * p_b.xyzw; // X
	result[0x2] = elements[0x1] * p_b.xy + elements[0x2] * p_b.s - elements[0x3] * p_b.yz - elements[0x4] * p_b.yw + elements[0xB] * p_b.xz + elements[0xC] * p_b.xw - elements[0xD] * p_b.xyzw - elements[0xE] * p_b.zw; // Y
	result[0x3] = elements[0x1] * p_b.xz + elements[0x2] * p_b.yz + elements[0x3] * p_b.s - elements[0x4] * p_b.zw - elements[0xB] * p_b.xy + elements[0xC] * p_b.xyzw + elements[0xD] * p_b.xw + elements[0xE] * p_b.yw; // Z
	result[0x4] = elements[0x1] * p_b.xw + elements[0x2] * p_b.yw + elements[0x3] * p_b.zw + elements[0x4] * p_b.s - elements[0xB] * p_b.xyzw - elements[0xC] * p_b.xy - elements[0xD] * p_b.xz - elements[0xE] * p_b.yz; // W
	result[0x5] = elements[0x0] * p_b.xy + elements[0x5] * p_b.s - elements[0x6] * p_b.yz - elements[0x7] * p_b.yw + elements[0x8] * p_b.xz + elements[0x9] * p_b.xw - elements[0xA] * p_b.xyzw - elements[0xF] * p_b.zw; // XY
	result[0x6] = elements[0x0] * p_b.xz + elements[0x5] * p_b.yz + elements[0x6] * p_b.s - elements[0x7] * p_b.zw - elements[0x8] * p_b.xy + elements[0x9] * p_b.xyzw + elements[0xA] * p_b.xw + elements[0xF] * p_b.yw; // XZ
	result[0x7] = elements[0x0] * p_b.xw + elements[0x5] * p_b.yw + elements[0x6] * p_b.zw + elements[0x7] * p_b.s - elements[0x8] * p_b.xyzw - elements[0x9] * p_b.xy - elements[0xA] * p_b.xz - elements[0xF] * p_b.yz; // XW
	result[0x8] = elements[0x0] * p_b.yz - elements[0x5] * p_b.xz + elements[0x6] * p_b.xy - elements[0x7] * p_b.xyzw + elements[0x8] * p_b.s - elements[0x9] * p_b.zw + elements[0xA] * p_b.yw - elements[0xF] * p_b.xw; // YZ
	result[0x9] = elements[0x0] * p_b.yw - elements[0x5] * p_b.xw + elements[0x6] * p_b.xyzw + elements[0x7] * p_b.xy + elements[0x8] * p_b.zw + elements[0x9] * p_b.s - elements[0xA] * p_b.yz + elements[0xF] * p_b.xz; // YW
	result[0xA] = elements[0x0] * p_b.zw - elements[0x5] * p_b.xyzw - elements[0x6] * p_b.xw + elements[0x7] * p_b.xz - elements[0x8] * p_b.yw + elements[0x9] * p_b.yz + elements[0xA] * p_b.s - elements[0xF] * p_b.xy; // ZW
	result[0xB] = elements[0x1] * p_b.yz - elements[0x2] * p_b.xz + elements[0x3] * p_b.xy - elements[0x4] * p_b.xyzw + elements[0xB] * p_b.s - elements[0xC] * p_b.zw + elements[0xD] * p_b.yw - elements[0xE] * p_b.xw; // XYZ
	result[0xC] = elements[0x1] * p_b.yw - elements[0x2] * p_b.xw + elements[0x3] * p_b.xyzw + elements[0x4] * p_b.xy + elements[0xB] * p_b.zw + elements[0xC] * p_b.s - elements[0xD] * p_b.yz + elements[0xE] * p_b.xz; // XYW
	result[0xD] = elements[0x1] * p_b.zw - elements[0x2] * p_b.xyzw - elements[0x3] * p_b.xw + elements[0x4] * p_b.xz - elements[0xB] * p_b.yw + elements[0xC] * p_b.yz + elements[0xD] * p_b.s - elements[0xE] * p_b.xy; // XZW
	result[0xE] = elements[0x1] * p_b.xyzw + elements[0x2] * p_b.zw - elements[0x3] * p_b.yw + elements[0x4] * p_b.yz + elements[0xB] * p_b.xw - elements[0xC] * p_b.xz + elements[0xD] * p_b.xy + elements[0xE] * p_b.s; // YZW
	result[0xF] = elements[0x0] * p_b.xyzw + elements[0x5] * p_b.zw - elements[0x6] * p_b.yw + elements[0x7] * p_b.yz + elements[0x8] * p_b.xw - elements[0x9] * p_b.xz + elements[0xA] * p_b.xy + elements[0xF] * p_b.s; // XYZW
	return result;
}

Multivector4D Multivector4D::multiply_vector_trivector(const Vector4 &p_vector, const Trivector4D &p_triv) const {
	Multivector4D result;
	result[0x0] = elements[0x1] * p_vector.x + elements[0x2] * p_vector.y + elements[0x3] * p_vector.z + elements[0x4] * p_vector.w - elements[0xB] * p_triv.xyz - elements[0xC] * p_triv.xyw - elements[0xD] * p_triv.xzw - elements[0xE] * p_triv.yzw; // Scalar
	result[0x1] = elements[0x0] * p_vector.x + elements[0x5] * p_vector.y + elements[0x6] * p_vector.z + elements[0x7] * p_vector.w - elements[0x8] * p_triv.xyz - elements[0x9] * p_triv.xyw - elements[0xA] * p_triv.xzw - elements[0xF] * p_triv.yzw; // X
	result[0x2] = elements[0x0] * p_vector.y - elements[0x5] * p_vector.x + elements[0x6] * p_triv.xyz + elements[0x7] * p_triv.xyw + elements[0x8] * p_vector.z + elements[0x9] * p_vector.w - elements[0xA] * p_triv.yzw + elements[0xF] * p_triv.xzw; // Y
	result[0x3] = elements[0x0] * p_vector.z - elements[0x5] * p_triv.xyz - elements[0x6] * p_vector.x + elements[0x7] * p_triv.xzw - elements[0x8] * p_vector.y + elements[0x9] * p_triv.yzw + elements[0xA] * p_vector.w - elements[0xF] * p_triv.xyw; // Z
	result[0x4] = elements[0x0] * p_vector.w - elements[0x5] * p_triv.xyw - elements[0x6] * p_triv.xzw - elements[0x7] * p_vector.x - elements[0x8] * p_triv.yzw - elements[0x9] * p_vector.y - elements[0xA] * p_vector.z + elements[0xF] * p_triv.xyz; // W
	result[0x5] = elements[0x1] * p_vector.y - elements[0x2] * p_vector.x + elements[0x3] * p_triv.xyz + elements[0x4] * p_triv.xyw + elements[0xB] * p_vector.z + elements[0xC] * p_vector.w - elements[0xD] * p_triv.yzw + elements[0xE] * p_triv.xzw; // XY
	result[0x6] = elements[0x1] * p_vector.z - elements[0x2] * p_triv.xyz - elements[0x3] * p_vector.x + elements[0x4] * p_triv.xzw - elements[0xB] * p_vector.y + elements[0xC] * p_triv.yzw + elements[0xD] * p_vector.w - elements[0xE] * p_triv.xyw; // XZ
	result[0x7] = elements[0x1] * p_vector.w - elements[0x2] * p_triv.xyw - elements[0x3] * p_triv.xzw - elements[0x4] * p_vector.x - elements[0xB] * p_triv.yzw - elements[0xC] * p_vector.y - elements[0xD] * p_vector.z + elements[0xE] * p_triv.xyz; // XW
	result[0x8] = elements[0x1] * p_triv.xyz + elements[0x2] * p_vector.z - elements[0x3] * p_vector.y + elements[0x4] * p_triv.yzw + elements[0xB] * p_vector.x - elements[0xC] * p_triv.xzw + elements[0xD] * p_triv.xyw + elements[0xE] * p_vector.w; // YZ
	result[0x9] = elements[0x1] * p_triv.xyw + elements[0x2] * p_vector.w - elements[0x3] * p_triv.yzw - elements[0x4] * p_vector.y + elements[0xB] * p_triv.xzw + elements[0xC] * p_vector.x - elements[0xD] * p_triv.xyz - elements[0xE] * p_vector.z; // YW
	result[0xA] = elements[0x1] * p_triv.xzw + elements[0x2] * p_triv.yzw + elements[0x3] * p_vector.w - elements[0x4] * p_vector.z - elements[0xB] * p_triv.xyw + elements[0xC] * p_triv.xyz + elements[0xD] * p_vector.x + elements[0xE] * p_vector.y; // ZW
	result[0xB] = elements[0x0] * p_triv.xyz + elements[0x5] * p_vector.z - elements[0x6] * p_vector.y + elements[0x7] * p_triv.yzw + elements[0x8] * p_vector.x - elements[0x9] * p_triv.xzw + elements[0xA] * p_triv.xyw + elements[0xF] * p_vector.w; // XYZ
	result[0xC] = elements[0x0] * p_triv.xyw + elements[0x5] * p_vector.w - elements[0x6] * p_triv.yzw - elements[0x7] * p_vector.y + elements[0x8] * p_triv.xzw + elements[0x9] * p_vector.x - elements[0xA] * p_triv.xyz - elements[0xF] * p_vector.z; // XYW
	result[0xD] = elements[0x0] * p_triv.xzw + elements[0x5] * p_triv.yzw + elements[0x6] * p_vector.w - elements[0x7] * p_vector.z - elements[0x8] * p_triv.xyw + elements[0x9] * p_triv.xyz + elements[0xA] * p_vector.x + elements[0xF] * p_vector.y; // XZW
	result[0xE] = elements[0x0] * p_triv.yzw - elements[0x5] * p_triv.xzw + elements[0x6] * p_triv.xyw - elements[0x7] * p_triv.xyz + elements[0x8] * p_vector.w - elements[0x9] * p_vector.z + elements[0xA] * p_vector.y - elements[0xF] * p_vector.x; // YZW
	result[0xF] = elements[0x1] * p_triv.yzw - elements[0x2] * p_triv.xzw + elements[0x3] * p_triv.xyw - elements[0x4] * p_triv.xyz + elements[0xB] * p_vector.w - elements[0xC] * p_vector.z + elements[0xD] * p_vector.y - elements[0xE] * p_vector.x; // XYZW
	return result;
}

Multivector4D Multivector4D::rotor_vector_product(const Rotor4D &p_rotor, const Vector4 &p_vector) {
	Multivector4D result;
	result.x = p_rotor.s * p_vector.x + p_rotor.xy * p_vector.y + p_rotor.xz * p_vector.z + p_rotor.xw * p_vector.w; // X
	result.y = p_rotor.s * p_vector.y - p_rotor.xy * p_vector.x + p_rotor.yz * p_vector.z + p_rotor.yw * p_vector.w; // Y
	result.z = p_rotor.s * p_vector.z - p_rotor.xz * p_vector.x - p_rotor.yz * p_vector.y + p_rotor.zw * p_vector.w; // Z
	result.w = p_rotor.s * p_vector.w - p_rotor.xw * p_vector.x - p_rotor.yw * p_vector.y - p_rotor.zw * p_vector.z; // W
	result.xyz = p_rotor.xy * p_vector.z - p_rotor.xz * p_vector.y + p_rotor.yz * p_vector.x + p_rotor.xyzw * p_vector.w; // XYZ
	result.xyw = p_rotor.xy * p_vector.w - p_rotor.xw * p_vector.y + p_rotor.yw * p_vector.x - p_rotor.xyzw * p_vector.z; // XYW
	result.xzw = p_rotor.xz * p_vector.w - p_rotor.xw * p_vector.z + p_rotor.zw * p_vector.x + p_rotor.xyzw * p_vector.y; // XZW
	result.yzw = p_rotor.yz * p_vector.w - p_rotor.yw * p_vector.z + p_rotor.zw * p_vector.y - p_rotor.xyzw * p_vector.x; // YZW
	return result;
}

Multivector4D Multivector4D::identity() {
	return Multivector4D(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

// Operators with other things on the left and Multivector4D on the right.

Multivector4D operator+(const real_t p_scalar, const Multivector4D &p_multivec) {
	Multivector4D result = p_multivec;
	result.scalar += p_scalar;
	return result;
}

Multivector4D operator+(const Vector4 &p_vector, const Multivector4D &p_multivec) {
	Multivector4D result = p_multivec;
	result.vector += p_vector;
	return result;
}

Multivector4D operator+(const Bivector4D &p_bivector, const Multivector4D &p_multivec) {
	// Addition is commutative, so b += a is the same as a = a + b.
	Multivector4D result = p_multivec;
	result.bivector += p_bivector;
	return result;
}

Multivector4D operator+(const Trivector4D &p_trivector, const Multivector4D &p_multivec) {
	Multivector4D result = p_multivec;
	result.trivector += p_trivector;
	return result;
}

Multivector4D operator+(const Rotor4D &p_rotor, const Multivector4D &p_multivec) {
	Multivector4D result = p_multivec;
	result.scalar += p_rotor.scalar;
	result.bivector += p_rotor.bivector;
	result.pseudoscalar += p_rotor.pseudoscalar;
	return result;
}

Multivector4D operator*(const real_t p_scalar, const Multivector4D &p_multivec) {
	return Multivector4D(p_scalar * p_multivec.scalar, p_scalar * p_multivec.vector, p_scalar * p_multivec.bivector, p_scalar * p_multivec.trivector, p_scalar * p_multivec.pseudoscalar);
}

Multivector4D operator*(const Vector4 &p_vector, const Multivector4D &p_multivec) {
	Multivector4D result;
	result[0x0] = p_vector.x * p_multivec[0x1] + p_vector.y * p_multivec[0x2] + p_vector.z * p_multivec[0x3] + p_vector.w * p_multivec[0x4]; // Scalar
	result[0x1] = p_vector.x * p_multivec[0x0] - p_vector.y * p_multivec[0x5] - p_vector.z * p_multivec[0x6] - p_vector.w * p_multivec[0x7]; // X
	result[0x2] = p_vector.x * p_multivec[0x5] + p_vector.y * p_multivec[0x0] - p_vector.z * p_multivec[0x8] - p_vector.w * p_multivec[0x9]; // Y
	result[0x3] = p_vector.x * p_multivec[0x6] + p_vector.y * p_multivec[0x8] + p_vector.z * p_multivec[0x0] - p_vector.w * p_multivec[0xA]; // Z
	result[0x4] = p_vector.x * p_multivec[0x7] + p_vector.y * p_multivec[0x9] + p_vector.z * p_multivec[0xA] + p_vector.w * p_multivec[0x0]; // W
	result[0x5] = p_vector.x * p_multivec[0x2] - p_vector.y * p_multivec[0x1] + p_vector.z * p_multivec[0xB] + p_vector.w * p_multivec[0xC]; // XY
	result[0x6] = p_vector.x * p_multivec[0x3] - p_vector.y * p_multivec[0xB] - p_vector.z * p_multivec[0x1] + p_vector.w * p_multivec[0xD]; // XZ
	result[0x7] = p_vector.x * p_multivec[0x4] - p_vector.y * p_multivec[0xC] - p_vector.z * p_multivec[0xD] - p_vector.w * p_multivec[0x1]; // XW
	result[0x8] = p_vector.x * p_multivec[0xB] + p_vector.y * p_multivec[0x3] - p_vector.z * p_multivec[0x2] + p_vector.w * p_multivec[0xE]; // YZ
	result[0x9] = p_vector.x * p_multivec[0xC] + p_vector.y * p_multivec[0x4] - p_vector.z * p_multivec[0xE] - p_vector.w * p_multivec[0x2]; // YW
	result[0xA] = p_vector.x * p_multivec[0xD] + p_vector.y * p_multivec[0xE] + p_vector.z * p_multivec[0x4] - p_vector.w * p_multivec[0x3]; // ZW
	result[0xB] = p_vector.x * p_multivec[0x8] - p_vector.y * p_multivec[0x6] + p_vector.z * p_multivec[0x5] - p_vector.w * p_multivec[0xF]; // XYZ
	result[0xC] = p_vector.x * p_multivec[0x9] - p_vector.y * p_multivec[0x7] + p_vector.z * p_multivec[0xF] + p_vector.w * p_multivec[0x5]; // XYW
	result[0xD] = p_vector.x * p_multivec[0xA] - p_vector.y * p_multivec[0xF] - p_vector.z * p_multivec[0x7] + p_vector.w * p_multivec[0x6]; // XZW
	result[0xE] = p_vector.x * p_multivec[0xF] + p_vector.y * p_multivec[0xA] - p_vector.z * p_multivec[0x9] + p_vector.w * p_multivec[0x8]; // YZW
	result[0xF] = p_vector.x * p_multivec[0xE] - p_vector.y * p_multivec[0xD] + p_vector.z * p_multivec[0xC] - p_vector.w * p_multivec[0xB]; // XYZW
	return result;
}

Multivector4D operator*(const Bivector4D &p_bivector, const Multivector4D &p_multivec) {
	Multivector4D result;
	result[0x0] = -p_bivector.xy * p_multivec[0x5] - p_bivector.xz * p_multivec[0x6] - p_bivector.xw * p_multivec[0x7] - p_bivector.yz * p_multivec[0x8] - p_bivector.yw * p_multivec[0x9] - p_bivector.zw * p_multivec[0xA]; // Scalar
	result[0x1] = +p_bivector.xy * p_multivec[0x2] + p_bivector.xz * p_multivec[0x3] + p_bivector.xw * p_multivec[0x4] - p_bivector.yz * p_multivec[0xB] - p_bivector.yw * p_multivec[0xC] - p_bivector.zw * p_multivec[0xD]; // X
	result[0x2] = -p_bivector.xy * p_multivec[0x1] + p_bivector.xz * p_multivec[0xB] + p_bivector.xw * p_multivec[0xC] + p_bivector.yz * p_multivec[0x3] + p_bivector.yw * p_multivec[0x4] - p_bivector.zw * p_multivec[0xE]; // Y
	result[0x3] = -p_bivector.xy * p_multivec[0xB] - p_bivector.xz * p_multivec[0x1] + p_bivector.xw * p_multivec[0xD] - p_bivector.yz * p_multivec[0x2] + p_bivector.yw * p_multivec[0xE] + p_bivector.zw * p_multivec[0x4]; // Z
	result[0x4] = -p_bivector.xy * p_multivec[0xC] - p_bivector.xz * p_multivec[0xD] - p_bivector.xw * p_multivec[0x1] - p_bivector.yz * p_multivec[0xE] - p_bivector.yw * p_multivec[0x2] - p_bivector.zw * p_multivec[0x3]; // W
	result[0x5] = +p_bivector.xy * p_multivec[0x0] - p_bivector.xz * p_multivec[0x8] - p_bivector.xw * p_multivec[0x9] + p_bivector.yz * p_multivec[0x6] + p_bivector.yw * p_multivec[0x7] - p_bivector.zw * p_multivec[0xF]; // XY
	result[0x6] = +p_bivector.xy * p_multivec[0x8] + p_bivector.xz * p_multivec[0x0] - p_bivector.xw * p_multivec[0xA] - p_bivector.yz * p_multivec[0x5] + p_bivector.yw * p_multivec[0xF] + p_bivector.zw * p_multivec[0x7]; // XZ
	result[0x7] = +p_bivector.xy * p_multivec[0x9] + p_bivector.xz * p_multivec[0xA] + p_bivector.xw * p_multivec[0x0] - p_bivector.yz * p_multivec[0xF] - p_bivector.yw * p_multivec[0x5] - p_bivector.zw * p_multivec[0x6]; // XW
	result[0x8] = -p_bivector.xy * p_multivec[0x6] + p_bivector.xz * p_multivec[0x5] - p_bivector.xw * p_multivec[0xF] + p_bivector.yz * p_multivec[0x0] - p_bivector.yw * p_multivec[0xA] + p_bivector.zw * p_multivec[0x9]; // YZ
	result[0x9] = -p_bivector.xy * p_multivec[0x7] + p_bivector.xz * p_multivec[0xF] + p_bivector.xw * p_multivec[0x5] + p_bivector.yz * p_multivec[0xA] + p_bivector.yw * p_multivec[0x0] - p_bivector.zw * p_multivec[0x8]; // YW
	result[0xA] = -p_bivector.xy * p_multivec[0xF] - p_bivector.xz * p_multivec[0x7] + p_bivector.xw * p_multivec[0x6] - p_bivector.yz * p_multivec[0x9] + p_bivector.yw * p_multivec[0x8] + p_bivector.zw * p_multivec[0x0]; // ZW
	result[0xB] = +p_bivector.xy * p_multivec[0x3] - p_bivector.xz * p_multivec[0x2] + p_bivector.xw * p_multivec[0xE] + p_bivector.yz * p_multivec[0x1] - p_bivector.yw * p_multivec[0xD] + p_bivector.zw * p_multivec[0xC]; // XYZ
	result[0xC] = +p_bivector.xy * p_multivec[0x4] - p_bivector.xz * p_multivec[0xE] - p_bivector.xw * p_multivec[0x2] + p_bivector.yz * p_multivec[0xD] + p_bivector.yw * p_multivec[0x1] - p_bivector.zw * p_multivec[0xB]; // XYW
	result[0xD] = +p_bivector.xy * p_multivec[0xE] + p_bivector.xz * p_multivec[0x4] - p_bivector.xw * p_multivec[0x3] - p_bivector.yz * p_multivec[0xC] + p_bivector.yw * p_multivec[0xB] + p_bivector.zw * p_multivec[0x1]; // XZW
	result[0xE] = -p_bivector.xy * p_multivec[0xD] + p_bivector.xz * p_multivec[0xC] - p_bivector.xw * p_multivec[0xB] + p_bivector.yz * p_multivec[0x4] - p_bivector.yw * p_multivec[0x3] + p_bivector.zw * p_multivec[0x2]; // YZW
	result[0xF] = +p_bivector.xy * p_multivec[0xA] - p_bivector.xz * p_multivec[0x9] + p_bivector.xw * p_multivec[0x8] + p_bivector.yz * p_multivec[0x7] - p_bivector.yw * p_multivec[0x6] + p_bivector.zw * p_multivec[0x5]; // XYZW
	return result;
}

Multivector4D operator*(const Trivector4D &p_trivector, const Multivector4D &p_multivec) {
	Multivector4D result;
	result[0x0] = -p_trivector.xyz * p_multivec[0xB] - p_trivector.xyw * p_multivec[0xC] - p_trivector.xzw * p_multivec[0xD] - p_trivector.yzw * p_multivec[0xE]; // Scalar
	result[0x1] = -p_trivector.xyz * p_multivec[0x8] - p_trivector.xyw * p_multivec[0x9] - p_trivector.xzw * p_multivec[0xA] + p_trivector.yzw * p_multivec[0xF]; // X
	result[0x2] = +p_trivector.xyz * p_multivec[0x6] + p_trivector.xyw * p_multivec[0x7] - p_trivector.xzw * p_multivec[0xF] - p_trivector.yzw * p_multivec[0xA]; // Y
	result[0x3] = -p_trivector.xyz * p_multivec[0x5] + p_trivector.xyw * p_multivec[0xF] + p_trivector.xzw * p_multivec[0x7] + p_trivector.yzw * p_multivec[0x9]; // Z
	result[0x4] = -p_trivector.xyz * p_multivec[0xF] - p_trivector.xyw * p_multivec[0x5] - p_trivector.xzw * p_multivec[0x6] - p_trivector.yzw * p_multivec[0x8]; // W
	result[0x5] = +p_trivector.xyz * p_multivec[0x3] + p_trivector.xyw * p_multivec[0x4] - p_trivector.xzw * p_multivec[0xE] + p_trivector.yzw * p_multivec[0xD]; // XY
	result[0x6] = -p_trivector.xyz * p_multivec[0x2] + p_trivector.xyw * p_multivec[0xE] + p_trivector.xzw * p_multivec[0x4] - p_trivector.yzw * p_multivec[0xC]; // XZ
	result[0x7] = -p_trivector.xyz * p_multivec[0xE] - p_trivector.xyw * p_multivec[0x2] - p_trivector.xzw * p_multivec[0x3] + p_trivector.yzw * p_multivec[0xB]; // XW
	result[0x8] = +p_trivector.xyz * p_multivec[0x1] - p_trivector.xyw * p_multivec[0xD] + p_trivector.xzw * p_multivec[0xC] + p_trivector.yzw * p_multivec[0x4]; // YZ
	result[0x9] = +p_trivector.xyz * p_multivec[0xD] + p_trivector.xyw * p_multivec[0x1] - p_trivector.xzw * p_multivec[0xB] - p_trivector.yzw * p_multivec[0x3]; // YW
	result[0xA] = -p_trivector.xyz * p_multivec[0xC] + p_trivector.xyw * p_multivec[0xB] + p_trivector.xzw * p_multivec[0x1] + p_trivector.yzw * p_multivec[0x2]; // ZW
	result[0xB] = +p_trivector.xyz * p_multivec[0x0] - p_trivector.xyw * p_multivec[0xA] + p_trivector.xzw * p_multivec[0x9] - p_trivector.yzw * p_multivec[0x7]; // XYZ
	result[0xC] = +p_trivector.xyz * p_multivec[0xA] + p_trivector.xyw * p_multivec[0x0] - p_trivector.xzw * p_multivec[0x8] + p_trivector.yzw * p_multivec[0x6]; // XYW
	result[0xD] = -p_trivector.xyz * p_multivec[0x9] + p_trivector.xyw * p_multivec[0x8] + p_trivector.xzw * p_multivec[0x0] - p_trivector.yzw * p_multivec[0x5]; // XZW
	result[0xE] = +p_trivector.xyz * p_multivec[0x7] - p_trivector.xyw * p_multivec[0x6] + p_trivector.xzw * p_multivec[0x5] + p_trivector.yzw * p_multivec[0x0]; // YZW
	result[0xF] = +p_trivector.xyz * p_multivec[0x4] - p_trivector.xyw * p_multivec[0x3] + p_trivector.xzw * p_multivec[0x2] - p_trivector.yzw * p_multivec[0x1]; // XYZW
	return result;
}

Multivector4D operator*(const Rotor4D &p_rotor, const Multivector4D &p_multivec) {
	Multivector4D result;
	result[0x0] = p_rotor.s * p_multivec[0x0] - p_rotor.xy * p_multivec[0x5] - p_rotor.xz * p_multivec[0x6] - p_rotor.xw * p_multivec[0x7] - p_rotor.yz * p_multivec[0x8] - p_rotor.yw * p_multivec[0x9] - p_rotor.zw * p_multivec[0xA] + p_rotor.xyzw * p_multivec[0xF]; // Scalar
	result[0x1] = p_rotor.s * p_multivec[0x1] + p_rotor.xy * p_multivec[0x2] + p_rotor.xz * p_multivec[0x3] + p_rotor.xw * p_multivec[0x4] - p_rotor.yz * p_multivec[0xB] - p_rotor.yw * p_multivec[0xC] - p_rotor.zw * p_multivec[0xD] - p_rotor.xyzw * p_multivec[0xE]; // X
	result[0x2] = p_rotor.s * p_multivec[0x2] - p_rotor.xy * p_multivec[0x1] + p_rotor.xz * p_multivec[0xB] + p_rotor.xw * p_multivec[0xC] + p_rotor.yz * p_multivec[0x3] + p_rotor.yw * p_multivec[0x4] - p_rotor.zw * p_multivec[0xE] + p_rotor.xyzw * p_multivec[0xD]; // Y
	result[0x3] = p_rotor.s * p_multivec[0x3] - p_rotor.xy * p_multivec[0xB] - p_rotor.xz * p_multivec[0x1] + p_rotor.xw * p_multivec[0xD] - p_rotor.yz * p_multivec[0x2] + p_rotor.yw * p_multivec[0xE] + p_rotor.zw * p_multivec[0x4] - p_rotor.xyzw * p_multivec[0xC]; // Z
	result[0x4] = p_rotor.s * p_multivec[0x4] - p_rotor.xy * p_multivec[0xC] - p_rotor.xz * p_multivec[0xD] - p_rotor.xw * p_multivec[0x1] - p_rotor.yz * p_multivec[0xE] - p_rotor.yw * p_multivec[0x2] - p_rotor.zw * p_multivec[0x3] + p_rotor.xyzw * p_multivec[0xB]; // W
	result[0x5] = p_rotor.s * p_multivec[0x5] + p_rotor.xy * p_multivec[0x0] - p_rotor.xz * p_multivec[0x8] - p_rotor.xw * p_multivec[0x9] + p_rotor.yz * p_multivec[0x6] + p_rotor.yw * p_multivec[0x7] - p_rotor.zw * p_multivec[0xF] - p_rotor.xyzw * p_multivec[0xA]; // XY
	result[0x6] = p_rotor.s * p_multivec[0x6] + p_rotor.xy * p_multivec[0x8] + p_rotor.xz * p_multivec[0x0] - p_rotor.xw * p_multivec[0xA] - p_rotor.yz * p_multivec[0x5] + p_rotor.yw * p_multivec[0xF] + p_rotor.zw * p_multivec[0x7] + p_rotor.xyzw * p_multivec[0x9]; // XZ
	result[0x7] = p_rotor.s * p_multivec[0x7] + p_rotor.xy * p_multivec[0x9] + p_rotor.xz * p_multivec[0xA] + p_rotor.xw * p_multivec[0x0] - p_rotor.yz * p_multivec[0xF] - p_rotor.yw * p_multivec[0x5] - p_rotor.zw * p_multivec[0x6] - p_rotor.xyzw * p_multivec[0x8]; // XW
	result[0x8] = p_rotor.s * p_multivec[0x8] - p_rotor.xy * p_multivec[0x6] + p_rotor.xz * p_multivec[0x5] - p_rotor.xw * p_multivec[0xF] + p_rotor.yz * p_multivec[0x0] - p_rotor.yw * p_multivec[0xA] + p_rotor.zw * p_multivec[0x9] - p_rotor.xyzw * p_multivec[0x7]; // YZ
	result[0x9] = p_rotor.s * p_multivec[0x9] - p_rotor.xy * p_multivec[0x7] + p_rotor.xz * p_multivec[0xF] + p_rotor.xw * p_multivec[0x5] + p_rotor.yz * p_multivec[0xA] + p_rotor.yw * p_multivec[0x0] - p_rotor.zw * p_multivec[0x8] + p_rotor.xyzw * p_multivec[0x6]; // YW
	result[0xA] = p_rotor.s * p_multivec[0xA] - p_rotor.xy * p_multivec[0xF] - p_rotor.xz * p_multivec[0x7] + p_rotor.xw * p_multivec[0x6] - p_rotor.yz * p_multivec[0x9] + p_rotor.yw * p_multivec[0x8] + p_rotor.zw * p_multivec[0x0] - p_rotor.xyzw * p_multivec[0x5]; // ZW
	result[0xB] = p_rotor.s * p_multivec[0xB] + p_rotor.xy * p_multivec[0x3] - p_rotor.xz * p_multivec[0x2] + p_rotor.xw * p_multivec[0xE] + p_rotor.yz * p_multivec[0x1] - p_rotor.yw * p_multivec[0xD] + p_rotor.zw * p_multivec[0xC] + p_rotor.xyzw * p_multivec[0x4]; // XYZ
	result[0xC] = p_rotor.s * p_multivec[0xC] + p_rotor.xy * p_multivec[0x4] - p_rotor.xz * p_multivec[0xE] - p_rotor.xw * p_multivec[0x2] + p_rotor.yz * p_multivec[0xD] + p_rotor.yw * p_multivec[0x1] - p_rotor.zw * p_multivec[0xB] - p_rotor.xyzw * p_multivec[0x3]; // XYW
	result[0xD] = p_rotor.s * p_multivec[0xD] + p_rotor.xy * p_multivec[0xE] + p_rotor.xz * p_multivec[0x4] - p_rotor.xw * p_multivec[0x3] - p_rotor.yz * p_multivec[0xC] + p_rotor.yw * p_multivec[0xB] + p_rotor.zw * p_multivec[0x1] + p_rotor.xyzw * p_multivec[0x2]; // XZW
	result[0xE] = p_rotor.s * p_multivec[0xE] - p_rotor.xy * p_multivec[0xD] + p_rotor.xz * p_multivec[0xC] - p_rotor.xw * p_multivec[0xB] + p_rotor.yz * p_multivec[0x4] - p_rotor.yw * p_multivec[0x3] + p_rotor.zw * p_multivec[0x2] - p_rotor.xyzw * p_multivec[0x1]; // YZW
	result[0xF] = p_rotor.s * p_multivec[0xF] + p_rotor.xy * p_multivec[0xA] - p_rotor.xz * p_multivec[0x9] + p_rotor.xw * p_multivec[0x8] + p_rotor.yz * p_multivec[0x7] - p_rotor.yw * p_multivec[0x6] + p_rotor.zw * p_multivec[0x5] + p_rotor.xyzw * p_multivec[0x0]; // XYZW
	return result;
}

Multivector4D Multivector4D::premultiply_vector_trivector(const Vector4 &p_vector, const Trivector4D &p_trivec) const {
	Multivector4D result;
	result[0x0] = p_vector.x * elements[0x1] + p_vector.y * elements[0x2] + p_vector.z * elements[0x3] + p_vector.w * elements[0x4] - p_trivec.xyz * elements[0xB] - p_trivec.xyw * elements[0xC] - p_trivec.xzw * elements[0xD] - p_trivec.yzw * elements[0xE]; // Scalar
	result[0x1] = p_vector.x * elements[0x0] - p_vector.y * elements[0x5] - p_vector.z * elements[0x6] - p_vector.w * elements[0x7] - p_trivec.xyz * elements[0x8] - p_trivec.xyw * elements[0x9] - p_trivec.xzw * elements[0xA] + p_trivec.yzw * elements[0xF]; // X
	result[0x2] = p_vector.x * elements[0x5] + p_vector.y * elements[0x0] - p_vector.z * elements[0x8] - p_vector.w * elements[0x9] + p_trivec.xyz * elements[0x6] + p_trivec.xyw * elements[0x7] - p_trivec.xzw * elements[0xF] - p_trivec.yzw * elements[0xA]; // Y
	result[0x3] = p_vector.x * elements[0x6] + p_vector.y * elements[0x8] + p_vector.z * elements[0x0] - p_vector.w * elements[0xA] - p_trivec.xyz * elements[0x5] + p_trivec.xyw * elements[0xF] + p_trivec.xzw * elements[0x7] + p_trivec.yzw * elements[0x9]; // Z
	result[0x4] = p_vector.x * elements[0x7] + p_vector.y * elements[0x9] + p_vector.z * elements[0xA] + p_vector.w * elements[0x0] - p_trivec.xyz * elements[0xF] - p_trivec.xyw * elements[0x5] - p_trivec.xzw * elements[0x6] - p_trivec.yzw * elements[0x8]; // W
	result[0x5] = p_vector.x * elements[0x2] - p_vector.y * elements[0x1] + p_vector.z * elements[0xB] + p_vector.w * elements[0xC] + p_trivec.xyz * elements[0x3] + p_trivec.xyw * elements[0x4] - p_trivec.xzw * elements[0xE] + p_trivec.yzw * elements[0xD]; // XY
	result[0x6] = p_vector.x * elements[0x3] - p_vector.y * elements[0xB] - p_vector.z * elements[0x1] + p_vector.w * elements[0xD] - p_trivec.xyz * elements[0x2] + p_trivec.xyw * elements[0xE] + p_trivec.xzw * elements[0x4] - p_trivec.yzw * elements[0xC]; // XZ
	result[0x7] = p_vector.x * elements[0x4] - p_vector.y * elements[0xC] - p_vector.z * elements[0xD] - p_vector.w * elements[0x1] - p_trivec.xyz * elements[0xE] - p_trivec.xyw * elements[0x2] - p_trivec.xzw * elements[0x3] + p_trivec.yzw * elements[0xB]; // XW
	result[0x8] = p_vector.x * elements[0xB] + p_vector.y * elements[0x3] - p_vector.z * elements[0x2] + p_vector.w * elements[0xE] + p_trivec.xyz * elements[0x1] - p_trivec.xyw * elements[0xD] + p_trivec.xzw * elements[0xC] + p_trivec.yzw * elements[0x4]; // YZ
	result[0x9] = p_vector.x * elements[0xC] + p_vector.y * elements[0x4] - p_vector.z * elements[0xE] - p_vector.w * elements[0x2] + p_trivec.xyz * elements[0xD] + p_trivec.xyw * elements[0x1] - p_trivec.xzw * elements[0xB] - p_trivec.yzw * elements[0x3]; // YW
	result[0xA] = p_vector.x * elements[0xD] + p_vector.y * elements[0xE] + p_vector.z * elements[0x4] - p_vector.w * elements[0x3] - p_trivec.xyz * elements[0xC] + p_trivec.xyw * elements[0xB] + p_trivec.xzw * elements[0x1] + p_trivec.yzw * elements[0x2]; // ZW
	result[0xB] = p_vector.x * elements[0x8] - p_vector.y * elements[0x6] + p_vector.z * elements[0x5] - p_vector.w * elements[0xF] + p_trivec.xyz * elements[0x0] - p_trivec.xyw * elements[0xA] + p_trivec.xzw * elements[0x9] - p_trivec.yzw * elements[0x7]; // XYZ
	result[0xC] = p_vector.x * elements[0x9] - p_vector.y * elements[0x7] + p_vector.z * elements[0xF] + p_vector.w * elements[0x5] + p_trivec.xyz * elements[0xA] + p_trivec.xyw * elements[0x0] - p_trivec.xzw * elements[0x8] + p_trivec.yzw * elements[0x6]; // XYW
	result[0xD] = p_vector.x * elements[0xA] - p_vector.y * elements[0xF] - p_vector.z * elements[0x7] + p_vector.w * elements[0x6] - p_trivec.xyz * elements[0x9] + p_trivec.xyw * elements[0x8] + p_trivec.xzw * elements[0x0] - p_trivec.yzw * elements[0x5]; // XZW
	result[0xE] = p_vector.x * elements[0xF] + p_vector.y * elements[0xA] - p_vector.z * elements[0x9] + p_vector.w * elements[0x8] + p_trivec.xyz * elements[0x7] - p_trivec.xyw * elements[0x6] + p_trivec.xzw * elements[0x5] + p_trivec.yzw * elements[0x0]; // YZW
	result[0xF] = p_vector.x * elements[0xE] - p_vector.y * elements[0xD] + p_vector.z * elements[0xC] - p_vector.w * elements[0xB] + p_trivec.xyz * elements[0x4] - p_trivec.xyw * elements[0x3] + p_trivec.xzw * elements[0x2] - p_trivec.yzw * elements[0x1]; // XYZW
	return result;
}

Multivector4D Multivector4D::from_array(const PackedRealArray &p_from_array) {
	const int stop_index = MIN(p_from_array.size(), 16);
	Multivector4D multivec;
	for (int i = 0; i < stop_index; i++) {
		multivec.elements[i] = p_from_array[i];
	}
	return multivec;
}

PackedRealArray Multivector4D::to_array() const {
	PackedRealArray arr;
	arr.resize(16);
	real_t *ptr = arr.ptrw();
	ptr[0] = scalar;
	ptr[1] = x;
	ptr[2] = y;
	ptr[3] = z;
	ptr[4] = w;
	ptr[5] = xy;
	ptr[6] = xz;
	ptr[7] = xw;
	ptr[8] = yz;
	ptr[9] = yw;
	ptr[10] = zw;
	ptr[11] = xyz;
	ptr[12] = xyw;
	ptr[13] = xzw;
	ptr[14] = yzw;
	ptr[15] = xyzw;
	return arr;
}

Multivector4D Multivector4D::from_rotor(const Rotor4D &p_rotor) {
	Multivector4D result;
	result.scalar = p_rotor.scalar;
	result.bivector = p_rotor.bivector;
	result.pseudoscalar = p_rotor.pseudoscalar;
	return result;
}

Multivector4D Multivector4D::from_rotor_vector(const Rotor4D &p_rotor, const Vector4 &p_vector) {
	Multivector4D result;
	result.scalar = p_rotor.scalar;
	result.vector = p_vector;
	result.bivector = p_rotor.bivector;
	result.pseudoscalar = p_rotor.pseudoscalar;
	return result;
}

Multivector4D Multivector4D::from_scalar(const real_t p_scalar) {
	Multivector4D result;
	result.scalar = p_scalar;
	return result;
}

Multivector4D Multivector4D::from_vector(const Vector4 &p_vector) {
	Multivector4D result;
	result.vector = p_vector;
	return result;
}

Multivector4D Multivector4D::from_bivector(const Bivector4D &p_bivector) {
	Multivector4D result;
	result.bivector = p_bivector;
	return result;
}

Multivector4D Multivector4D::from_trivector(const Trivector4D &p_trivector) {
	Multivector4D result;
	result.trivector = p_trivector;
	return result;
}

Multivector4D Multivector4D::from_pseudoscalar(const real_t p_pseudoscalar) {
	Multivector4D result;
	result.pseudoscalar = p_pseudoscalar;
	return result;
}

Multivector4D Multivector4D::from_scalar_vector(const real_t p_scalar, const Vector4 &p_vector) {
	Multivector4D result;
	result.scalar = p_scalar;
	result.vector = p_vector;
	return result;
}

Multivector4D Multivector4D::from_scalar_bivector(const real_t p_scalar, const Bivector4D &p_bivector) {
	Multivector4D result;
	result.scalar = p_scalar;
	result.bivector = p_bivector;
	return result;
}

Multivector4D Multivector4D::from_scalar_trivector(const real_t p_scalar, const Trivector4D &p_trivector) {
	Multivector4D result;
	result.scalar = p_scalar;
	result.trivector = p_trivector;
	return result;
}

Multivector4D Multivector4D::from_scalar_pseudoscalar(const real_t p_scalar, const real_t p_pseudoscalar) {
	Multivector4D result;
	result.scalar = p_scalar;
	result.pseudoscalar = p_pseudoscalar;
	return result;
}

Multivector4D Multivector4D::from_vector_bivector(const Vector4 &p_vector, const Bivector4D &p_bivector) {
	Multivector4D result;
	result.vector = p_vector;
	result.bivector = p_bivector;
	return result;
}

Multivector4D Multivector4D::from_vector_trivector(const Vector4 &p_vector, const Trivector4D &p_trivector) {
	Multivector4D result;
	result.vector = p_vector;
	result.trivector = p_trivector;
	return result;
}

Multivector4D Multivector4D::from_vector_pseudoscalar(const Vector4 &p_vector, const real_t p_pseudoscalar) {
	Multivector4D result;
	result.vector = p_vector;
	result.pseudoscalar = p_pseudoscalar;
	return result;
}

Multivector4D Multivector4D::from_bivector_trivector(const Bivector4D &p_bivector, const Trivector4D &p_trivector) {
	Multivector4D result;
	result.bivector = p_bivector;
	result.trivector = p_trivector;
	return result;
}

Multivector4D Multivector4D::from_bivector_pseudoscalar(const Bivector4D &p_bivector, const real_t p_pseudoscalar) {
	Multivector4D result;
	result.bivector = p_bivector;
	result.pseudoscalar = p_pseudoscalar;
	return result;
}

Multivector4D Multivector4D::from_trivector_pseudoscalar(const Trivector4D &p_trivector, const real_t p_pseudoscalar) {
	Multivector4D result;
	result.trivector = p_trivector;
	result.pseudoscalar = p_pseudoscalar;
	return result;
}

Multivector4D::Multivector4D(const Multivector4D &p_multivector) {
	scalar = p_multivector.scalar;
	vector = p_multivector.vector;
	bivector = p_multivector.bivector;
	trivector = p_multivector.trivector;
	pseudoscalar = p_multivector.pseudoscalar;
}

Multivector4D::Multivector4D(const real_t p_scalar, const Vector4 &p_vector, const Bivector4D &p_bivector, const Trivector4D &p_trivector, const real_t p_pseudoscalar) {
	scalar = p_scalar;
	vector = p_vector;
	bivector = p_bivector;
	trivector = p_trivector;
	pseudoscalar = p_pseudoscalar;
}

Multivector4D::Multivector4D(const real_t p_s, const real_t p_x, const real_t p_y, const real_t p_z, const real_t p_w, const real_t p_xy, const real_t p_xz, const real_t p_xw, const real_t p_yz, const real_t p_yw, const real_t p_zw, const real_t p_xyz, const real_t p_xyw, const real_t p_xzw, const real_t p_yzw, const real_t p_xyzw) {
	s = p_s;
	x = p_x;
	y = p_y;
	z = p_z;
	w = p_w;
	xy = p_xy;
	xz = p_xz;
	xw = p_xw;
	yz = p_yz;
	yw = p_yw;
	zw = p_zw;
	xyz = p_xyz;
	xyw = p_xyw;
	xzw = p_xzw;
	yzw = p_yzw;
	xyzw = p_xyzw;
}

#include "component_to_string.inc"

Multivector4D::operator String() const {
	String ret;
	if (!Math::is_zero_approx(s)) {
		ret += String::num(s);
	}
	APPEND_COMPONENT_TO_STRING(ret, x);
	APPEND_COMPONENT_TO_STRING(ret, y);
	APPEND_COMPONENT_TO_STRING(ret, z);
	APPEND_COMPONENT_TO_STRING(ret, w);
	APPEND_COMPONENT_TO_STRING(ret, xy);
	APPEND_COMPONENT_TO_STRING(ret, xz);
	APPEND_COMPONENT_TO_STRING(ret, xw);
	APPEND_COMPONENT_TO_STRING(ret, yz);
	APPEND_COMPONENT_TO_STRING(ret, yw);
	APPEND_COMPONENT_TO_STRING(ret, zw);
	APPEND_COMPONENT_TO_STRING(ret, xyz);
	APPEND_COMPONENT_TO_STRING(ret, xyw);
	APPEND_COMPONENT_TO_STRING(ret, xzw);
	APPEND_COMPONENT_TO_STRING(ret, yzw);
	APPEND_COMPONENT_TO_STRING(ret, xyzw);
	return ret;
}

static_assert(sizeof(Multivector4D) == 16 * sizeof(real_t));
