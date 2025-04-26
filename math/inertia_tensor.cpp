#include "inertia_tensor.h"

#if GDEXTENSION
#include <godot_cpp/variant/basis.hpp>
#elif GODOT_MODULE
#include "core/math/basis.h"
#endif

InertiaTensor InertiaTensor::inverse() const { // Algorithm from https://en.wikipedia.org/wiki/Invertible_matrix#Blockwise_inversion
    Basis Ai = Basis(
        i11, i12, i13,
        i12, i22, i23,
        i13, i23, i33
    ).inverse();
    Basis Ct = Basis(
        i14, i15, i16,
        i24, i25, i26,
        i34, i35, i36
    );
    Basis D = Basis(
        i44, i45, i46,
        i45, i55, i56,
        i46, i56, i66
    );

    Basis CAi = Ct.transposed() * Ai;
    Basis Si = (D - CAi * Ct).inverse();
    Basis SiCAi = Si * CAi;
    Basis AiCtSiCAi = CAi.transposed() * SiCAi;
    
    Basis a11 = Ai + (AiCtSiCAi);
    Basis a12 = -1 * SiCAi.transposed();
    Basis a22 = Si;

    InertiaTensor inv = InertiaTensor(
        a11.rows[0].x, a11.rows[0].y, a11.rows[0].z, a12.rows[0].x, a12.rows[0].y, a12.rows[0].z,
                       a11.rows[1].y, a11.rows[1].z, a12.rows[1].x, a12.rows[1].y, a12.rows[1].z,
                                      a11.rows[2].z, a12.rows[2].x, a12.rows[2].y, a12.rows[2].z,
                                                     a22.rows[0].x, a22.rows[0].y, a22.rows[0].z,
                                                                    a22.rows[1].y, a22.rows[1].z,
                                                                                   a22.rows[2].z
    );

    // inv = (2 * inv) - (inv * (*this) * inv); // One iteration of Newtons method

	return inv;
}

InertiaTensor &InertiaTensor::operator+=(const InertiaTensor &p_b) {
    i11 += p_b.i11;
    i12 += p_b.i12;
    i13 += p_b.i13;
    i14 += p_b.i14;
    i15 += p_b.i15;
    i16 += p_b.i16;
    i22 += p_b.i22;
    i23 += p_b.i23;
    i24 += p_b.i24;
    i25 += p_b.i25;
    i26 += p_b.i26;
    i33 += p_b.i33;
    i34 += p_b.i34;
    i35 += p_b.i35;
    i36 += p_b.i36;
    i44 += p_b.i44;
    i45 += p_b.i45;
    i46 += p_b.i46;
    i55 += p_b.i55;
    i56 += p_b.i56;
    i66 += p_b.i66;
    return *this;
}

InertiaTensor InertiaTensor::operator+(const InertiaTensor &p_b) const {
    return InertiaTensor(
        i11+p_b.i11, i12+p_b.i12, i13+p_b.i13, i14+p_b.i14, i15+p_b.i15, i16+p_b.i16,
        i22+p_b.i22, i23+p_b.i23, i24+p_b.i24, i25+p_b.i25, i26+p_b.i26,
        i33+p_b.i33, i34+p_b.i34, i35+p_b.i35, i36+p_b.i36,
        i44+p_b.i44, i45+p_b.i45, i46+p_b.i46,
        i55+p_b.i55, i56+p_b.i56,
        i66+p_b.i66
    );
}

InertiaTensor &InertiaTensor::operator-=(const InertiaTensor &p_b) {
    i11 -= p_b.i11;
    i12 -= p_b.i12;
    i13 -= p_b.i13;
    i14 -= p_b.i14;
    i15 -= p_b.i15;
    i16 -= p_b.i16;
    i22 -= p_b.i22;
    i23 -= p_b.i23;
    i24 -= p_b.i24;
    i25 -= p_b.i25;
    i26 -= p_b.i26;
    i33 -= p_b.i33;
    i34 -= p_b.i34;
    i35 -= p_b.i35;
    i36 -= p_b.i36;
    i44 -= p_b.i44;
    i45 -= p_b.i45;
    i46 -= p_b.i46;
    i55 -= p_b.i55;
    i56 -= p_b.i56;
    i66 -= p_b.i66;
    return *this;
}

InertiaTensor InertiaTensor::operator-(const InertiaTensor &p_b) const {
    return InertiaTensor(
        i11-p_b.i11, i12-p_b.i12, i13-p_b.i13, i14-p_b.i14, i15-p_b.i15, i16-p_b.i16,
        i22-p_b.i22, i23-p_b.i23, i24-p_b.i24, i25-p_b.i25, i26-p_b.i26,
        i33-p_b.i33, i34-p_b.i34, i35-p_b.i35, i36-p_b.i36,
        i44-p_b.i44, i45-p_b.i45, i46-p_b.i46,
        i55-p_b.i55, i56-p_b.i56,
        i66-p_b.i66
    );
}

Mat6 InertiaTensor::operator*(const InertiaTensor &p_b) const {
    return Mat6(
        i11*p_b.i11 + i12*p_b.i12 + i13*p_b.i13 + i14*p_b.i14 + i15*p_b.i15 + i16*p_b.i16,
        i11*p_b.i12 + i12*p_b.i22 + i13*p_b.i23 + i14*p_b.i24 + i15*p_b.i25 + i16*p_b.i26,
        i11*p_b.i13 + i12*p_b.i23 + i13*p_b.i33 + i14*p_b.i34 + i15*p_b.i35 + i16*p_b.i36,
        i11*p_b.i14 + i12*p_b.i24 + i13*p_b.i34 + i14*p_b.i44 + i15*p_b.i45 + i16*p_b.i46,
        i11*p_b.i15 + i12*p_b.i25 + i13*p_b.i35 + i14*p_b.i45 + i15*p_b.i55 + i16*p_b.i56,
        i11*p_b.i16 + i12*p_b.i26 + i13*p_b.i36 + i14*p_b.i46 + i15*p_b.i56 + i16*p_b.i66,

        i12*p_b.i11 + i22*p_b.i12 + i23*p_b.i13 + i24*p_b.i14 + i25*p_b.i15 + i26*p_b.i16,
        i12*p_b.i12 + i22*p_b.i22 + i23*p_b.i23 + i24*p_b.i24 + i25*p_b.i25 + i26*p_b.i26,
        i12*p_b.i13 + i22*p_b.i23 + i23*p_b.i33 + i24*p_b.i34 + i25*p_b.i35 + i26*p_b.i36,
        i12*p_b.i14 + i22*p_b.i24 + i23*p_b.i34 + i24*p_b.i44 + i25*p_b.i45 + i26*p_b.i46,
        i12*p_b.i15 + i22*p_b.i25 + i23*p_b.i35 + i24*p_b.i45 + i25*p_b.i55 + i26*p_b.i56,
        i12*p_b.i16 + i22*p_b.i26 + i23*p_b.i36 + i24*p_b.i46 + i25*p_b.i56 + i26*p_b.i66,

        i13*p_b.i11 + i23*p_b.i12 + i33*p_b.i13 + i34*p_b.i14 + i35*p_b.i15 + i36*p_b.i16,
        i13*p_b.i12 + i23*p_b.i22 + i33*p_b.i23 + i34*p_b.i24 + i35*p_b.i25 + i36*p_b.i26,
        i13*p_b.i13 + i23*p_b.i23 + i33*p_b.i33 + i34*p_b.i34 + i35*p_b.i35 + i36*p_b.i36,
        i13*p_b.i14 + i23*p_b.i24 + i33*p_b.i34 + i34*p_b.i44 + i35*p_b.i45 + i36*p_b.i46,
        i13*p_b.i15 + i23*p_b.i25 + i33*p_b.i35 + i34*p_b.i45 + i35*p_b.i55 + i36*p_b.i56,
        i13*p_b.i16 + i23*p_b.i26 + i33*p_b.i36 + i34*p_b.i46 + i35*p_b.i56 + i36*p_b.i66,

        i14*p_b.i11 + i24*p_b.i12 + i34*p_b.i13 + i44*p_b.i14 + i45*p_b.i15 + i46*p_b.i16,
        i14*p_b.i12 + i24*p_b.i22 + i34*p_b.i23 + i44*p_b.i24 + i45*p_b.i25 + i46*p_b.i26,
        i14*p_b.i13 + i24*p_b.i23 + i34*p_b.i33 + i44*p_b.i34 + i45*p_b.i35 + i46*p_b.i36,
        i14*p_b.i14 + i24*p_b.i24 + i34*p_b.i34 + i44*p_b.i44 + i45*p_b.i45 + i46*p_b.i46,
        i14*p_b.i15 + i24*p_b.i25 + i34*p_b.i35 + i44*p_b.i45 + i45*p_b.i55 + i46*p_b.i56,
        i14*p_b.i16 + i24*p_b.i26 + i34*p_b.i36 + i44*p_b.i46 + i45*p_b.i56 + i46*p_b.i66,

        i15*p_b.i11 + i25*p_b.i12 + i35*p_b.i13 + i45*p_b.i14 + i55*p_b.i15 + i56*p_b.i16,
        i15*p_b.i12 + i25*p_b.i22 + i35*p_b.i23 + i45*p_b.i24 + i55*p_b.i25 + i56*p_b.i26,
        i15*p_b.i13 + i25*p_b.i23 + i35*p_b.i33 + i45*p_b.i34 + i55*p_b.i35 + i56*p_b.i36,
        i15*p_b.i14 + i25*p_b.i24 + i35*p_b.i34 + i45*p_b.i44 + i55*p_b.i45 + i56*p_b.i46,
        i15*p_b.i15 + i25*p_b.i25 + i35*p_b.i35 + i45*p_b.i45 + i55*p_b.i55 + i56*p_b.i56,
        i15*p_b.i16 + i25*p_b.i26 + i35*p_b.i36 + i45*p_b.i46 + i55*p_b.i56 + i56*p_b.i66,

        i16*p_b.i11 + i26*p_b.i12 + i36*p_b.i13 + i46*p_b.i14 + i56*p_b.i15 + i66*p_b.i16,
        i16*p_b.i12 + i26*p_b.i22 + i36*p_b.i23 + i46*p_b.i24 + i56*p_b.i25 + i66*p_b.i26,
        i16*p_b.i13 + i26*p_b.i23 + i36*p_b.i33 + i46*p_b.i34 + i56*p_b.i35 + i66*p_b.i36,
        i16*p_b.i14 + i26*p_b.i24 + i36*p_b.i34 + i46*p_b.i44 + i56*p_b.i45 + i66*p_b.i46,
        i16*p_b.i15 + i26*p_b.i25 + i36*p_b.i35 + i46*p_b.i45 + i56*p_b.i55 + i66*p_b.i56,
        i16*p_b.i16 + i26*p_b.i26 + i36*p_b.i36 + i46*p_b.i46 + i56*p_b.i56 + i66*p_b.i66
    );
}

InertiaTensor &InertiaTensor::operator*=(const real_t p_scalar) {
    i11 *= p_scalar;
    i12 *= p_scalar;
    i13 *= p_scalar;
    i14 *= p_scalar;
    i15 *= p_scalar;
    i16 *= p_scalar;
    i22 *= p_scalar;
    i23 *= p_scalar;
    i24 *= p_scalar;
    i25 *= p_scalar;
    i26 *= p_scalar;
    i33 *= p_scalar;
    i34 *= p_scalar;
    i35 *= p_scalar;
    i36 *= p_scalar;
    i44 *= p_scalar;
    i45 *= p_scalar;
    i46 *= p_scalar;
    i55 *= p_scalar;
    i56 *= p_scalar;
    i66 *= p_scalar;
    return *this;
}

InertiaTensor InertiaTensor::operator*(const real_t p_scalar) const {
    return InertiaTensor(
        i11*p_scalar, i12*p_scalar, i13*p_scalar, i14*p_scalar, i15*p_scalar, i16*p_scalar,
        i22*p_scalar, i23*p_scalar, i24*p_scalar, i25*p_scalar, i26*p_scalar,
        i33*p_scalar, i34*p_scalar, i35*p_scalar, i36*p_scalar,
        i44*p_scalar, i45*p_scalar, i46*p_scalar,
        i55*p_scalar, i56*p_scalar,
        i66*p_scalar
    );
}

InertiaTensor &InertiaTensor::operator/=(const real_t p_scalar) {
    i11 /= p_scalar;
    i12 /= p_scalar;
    i13 /= p_scalar;
    i14 /= p_scalar;
    i15 /= p_scalar;
    i16 /= p_scalar;
    i22 /= p_scalar;
    i23 /= p_scalar;
    i24 /= p_scalar;
    i25 /= p_scalar;
    i26 /= p_scalar;
    i33 /= p_scalar;
    i34 /= p_scalar;
    i35 /= p_scalar;
    i36 /= p_scalar;
    i44 /= p_scalar;
    i45 /= p_scalar;
    i46 /= p_scalar;
    i55 /= p_scalar;
    i56 /= p_scalar;
    i66 /= p_scalar;
    return *this;
}

InertiaTensor InertiaTensor::operator/(const real_t p_scalar) const {
    return InertiaTensor(
        i11/p_scalar, i12/p_scalar, i13/p_scalar, i14/p_scalar, i15/p_scalar, i16/p_scalar,
        i22/p_scalar, i23/p_scalar, i24/p_scalar, i25/p_scalar, i26/p_scalar,
        i33/p_scalar, i34/p_scalar, i35/p_scalar, i36/p_scalar,
        i44/p_scalar, i45/p_scalar, i46/p_scalar,
        i55/p_scalar, i56/p_scalar,
        i66/p_scalar
    );
}

InertiaTensor InertiaTensor::from_array(const PackedRealArray &p_from_array) {
	const int stop_index = MIN(p_from_array.size(), 21);
	InertiaTensor tensor;
	for (int i = 0; i < stop_index; i++) {
		tensor.elements[i] = p_from_array[i];
	}
	return tensor;
}

PackedRealArray InertiaTensor::to_array() const {
	PackedRealArray arr;
	arr.resize(21);
	real_t *ptr = arr.ptrw();

	ptr[0] = i11;
	ptr[1] = i12;
	ptr[2] = i13;
	ptr[3] = i14;
	ptr[4] = i15;
	ptr[5] = i16;
    
	ptr[6] = i22;
	ptr[7] = i23;
	ptr[8] = i24;
	ptr[9] = i25;
	ptr[10] = i26;
    
	ptr[11] = i33;
	ptr[12] = i34;
	ptr[13] = i35;
	ptr[14] = i36;
    
	ptr[15] = i44;
	ptr[16] = i45;
	ptr[17] = i46;
    
	ptr[18] = i55;
	ptr[19] = i56;

	ptr[20] = i66;

	return arr;
}

InertiaTensor::InertiaTensor(const real_t p_i11, const real_t p_i22, const real_t p_i33, const real_t p_i44, const real_t p_i55, const real_t p_i66) {
	i11 = p_i11;
    i22 = p_i22;
    i33 = p_i33;
    i44 = p_i44;
    i55 = p_i55;
    i66 = p_i66;
}

InertiaTensor::InertiaTensor(
    const real_t p_i11, const real_t p_i12, const real_t p_i13, const real_t p_i14, const real_t p_i15, const real_t p_i16,
    const real_t p_i22, const real_t p_i23, const real_t p_i24, const real_t p_i25, const real_t p_i26,
    const real_t p_i33, const real_t p_i34, const real_t p_i35, const real_t p_i36,
    const real_t p_i44, const real_t p_i45, const real_t p_i46,
    const real_t p_i55, const real_t p_i56,
    const real_t p_i66
) {
	i11 = p_i11;
    i12 = p_i12;
    i13 = p_i13;
    i14 = p_i14;
    i15 = p_i15;
    i16 = p_i16;
    i22 = p_i22;
    i23 = p_i23;
    i24 = p_i24;
    i25 = p_i25;
    i26 = p_i26;
    i33 = p_i33;
    i34 = p_i34;
    i35 = p_i35;
    i36 = p_i36;
    i44 = p_i44;
    i45 = p_i45;
    i46 = p_i46;
    i55 = p_i55;
    i56 = p_i56;
    i66 = p_i66;
}

InertiaTensor operator*(const real_t p_scalar, const InertiaTensor &p_tensor) {
    return InertiaTensor(
        p_tensor.i11*p_scalar, p_tensor.i12*p_scalar, p_tensor.i13*p_scalar, p_tensor.i14*p_scalar, p_tensor.i15*p_scalar, p_tensor.i16*p_scalar,
        p_tensor.i22*p_scalar, p_tensor.i23*p_scalar, p_tensor.i24*p_scalar, p_tensor.i25*p_scalar, p_tensor.i26*p_scalar,
        p_tensor.i33*p_scalar, p_tensor.i34*p_scalar, p_tensor.i35*p_scalar, p_tensor.i36*p_scalar,
        p_tensor.i44*p_scalar, p_tensor.i45*p_scalar, p_tensor.i46*p_scalar,
        p_tensor.i55*p_scalar, p_tensor.i56*p_scalar,
        p_tensor.i66*p_scalar
    );
}

static_assert(sizeof(InertiaTensor) == 21 * sizeof(real_t));