#include "mat6.h"

// #if GDEXTENSION
// #include <godot_cpp/variant/basis.hpp>
// #elif GODOT_MODULE
// #include "core/math/basis.h"
// #endif

// Mat6 Mat6::inverse() const { // Algorithm from https://en.wikipedia.org/wiki/Invertible_matrix#Blockwise_inversion
//     Basis Ai = Basis(
//         i11, i12, i13,
//         i12, i22, i23,
//         i13, i23, i33
//     ).inverse();
//     Basis Ct = Basis(
//         i14, i15, i16,
//         i24, i25, i26,
//         i34, i35, i36
//     );
//     Basis D = Basis(
//         i44, i45, i46,
//         i45, i55, i56,
//         i46, i56, i66
//     );

//     Basis CAi = Ct.transposed() * Ai;
//     Basis Si = (D - CAi * Ct).inverse();
//     Basis SiCAi = Si * CAi;
//     Basis AiCtSiCAi = CAi.transposed() * SiCAi;
    
//     Basis a11 = Ai + (AiCtSiCAi);
//     Basis a12 = -1 * SiCAi.transposed();
//     Basis a22 = Si;

//     Mat6 inv = Mat6(
//         a11.rows[0].x, a11.rows[0].y, a11.rows[0].z, a12.rows[0].x, a12.rows[0].y, a12.rows[0].z,
//                        a11.rows[1].y, a11.rows[1].z, a12.rows[1].x, a12.rows[1].y, a12.rows[1].z,
//                                       a11.rows[2].z, a12.rows[2].x, a12.rows[2].y, a12.rows[2].z,
//                                                      a22.rows[0].x, a22.rows[0].y, a22.rows[0].z,
//                                                                     a22.rows[1].y, a22.rows[1].z,
//                                                                                    a22.rows[2].z
//     );

//     inv = (2 * inv) - (inv * (*this) * inv); // One iteration of Newtons method

// 	return inv;
// }

Mat6 &Mat6::operator+=(const Mat6 &p_b) {
    i11 += p_b.i11;
    i12 += p_b.i12;
    i13 += p_b.i13;
    i14 += p_b.i14;
    i15 += p_b.i15;
    i16 += p_b.i16;
    
    i21 += p_b.i21;
    i22 += p_b.i22;
    i23 += p_b.i23;
    i24 += p_b.i24;
    i25 += p_b.i25;
    i26 += p_b.i26;

    i31 += p_b.i31;
    i32 += p_b.i32;
    i33 += p_b.i33;
    i34 += p_b.i34;
    i35 += p_b.i35;
    i36 += p_b.i36;
    
    i41 += p_b.i41;
    i42 += p_b.i42;
    i43 += p_b.i43;
    i44 += p_b.i44;
    i45 += p_b.i45;
    i46 += p_b.i46;
    
    i51 += p_b.i51;
    i52 += p_b.i52;
    i53 += p_b.i53;
    i54 += p_b.i54;
    i55 += p_b.i55;
    i56 += p_b.i56;
    
    i61 += p_b.i61;
    i62 += p_b.i62;
    i63 += p_b.i63;
    i64 += p_b.i64;
    i65 += p_b.i65;
    i66 += p_b.i66;
    return *this;
}

Mat6 Mat6::operator+(const Mat6 &p_b) const {
    return Mat6(
        i11+p_b.i11, i12+p_b.i12, i13+p_b.i13, i14+p_b.i14, i15+p_b.i15, i16+p_b.i16,
        i21+p_b.i21, i22+p_b.i22, i23+p_b.i23, i24+p_b.i24, i25+p_b.i25, i26+p_b.i26,
        i31+p_b.i31, i32+p_b.i32, i33+p_b.i33, i34+p_b.i34, i35+p_b.i35, i36+p_b.i36,
        i41+p_b.i41, i42+p_b.i42, i43+p_b.i43, i44+p_b.i44, i45+p_b.i45, i46+p_b.i46,
        i51+p_b.i51, i52+p_b.i52, i53+p_b.i53, i54+p_b.i54, i55+p_b.i55, i56+p_b.i56,
        i61+p_b.i61, i62+p_b.i62, i63+p_b.i63, i64+p_b.i64, i65+p_b.i65, i66+p_b.i66,
    );
}

Mat6 &Mat6::operator-=(const Mat6 &p_b) {
    i11 -= p_b.i11;
    i12 -= p_b.i12;
    i13 -= p_b.i13;
    i14 -= p_b.i14;
    i15 -= p_b.i15;
    i16 -= p_b.i16;
    
    i21 -= p_b.i21;
    i22 -= p_b.i22;
    i23 -= p_b.i23;
    i24 -= p_b.i24;
    i25 -= p_b.i25;
    i26 -= p_b.i26;

    i31 -= p_b.i31;
    i32 -= p_b.i32;
    i33 -= p_b.i33;
    i34 -= p_b.i34;
    i35 -= p_b.i35;
    i36 -= p_b.i36;
    
    i41 -= p_b.i41;
    i42 -= p_b.i42;
    i43 -= p_b.i43;
    i44 -= p_b.i44;
    i45 -= p_b.i45;
    i46 -= p_b.i46;
    
    i51 -= p_b.i51;
    i52 -= p_b.i52;
    i53 -= p_b.i53;
    i54 -= p_b.i54;
    i55 -= p_b.i55;
    i56 -= p_b.i56;
    
    i61 -= p_b.i61;
    i62 -= p_b.i62;
    i63 -= p_b.i63;
    i64 -= p_b.i64;
    i65 -= p_b.i65;
    i66 -= p_b.i66;
    return *this;
}

Mat6 Mat6::operator-(const Mat6 &p_b) const {
    return Mat6(
        i11-p_b.i11, i12-p_b.i12, i13-p_b.i13, i14-p_b.i14, i15-p_b.i15, i16-p_b.i16,
        i21-p_b.i21, i22-p_b.i22, i23-p_b.i23, i24-p_b.i24, i25-p_b.i25, i26-p_b.i26,
        i31-p_b.i31, i32-p_b.i32, i33-p_b.i33, i34-p_b.i34, i35-p_b.i35, i36-p_b.i36,
        i41-p_b.i41, i42-p_b.i42, i43-p_b.i43, i44-p_b.i44, i45-p_b.i45, i46-p_b.i46,
        i51-p_b.i51, i52-p_b.i52, i53-p_b.i53, i54-p_b.i54, i55-p_b.i55, i56-p_b.i56,
        i61-p_b.i61, i62-p_b.i62, i63-p_b.i63, i64-p_b.i64, i65-p_b.i65, i66-p_b.i66,
    );
}

Mat6 Mat6::operator*(const Mat6 &p_b) const {
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

Mat6 &Mat6::operator*=(const real_t p_scalar) {
    i11 *= p_scalar;
    i12 *= p_scalar;
    i13 *= p_scalar;
    i14 *= p_scalar;
    i15 *= p_scalar;
    i16 *= p_scalar;

    i21 *= p_scalar;
    i22 *= p_scalar;
    i23 *= p_scalar;
    i24 *= p_scalar;
    i25 *= p_scalar;
    i26 *= p_scalar;

    i31 *= p_scalar;
    i32 *= p_scalar;
    i33 *= p_scalar;
    i34 *= p_scalar;
    i35 *= p_scalar;
    i36 *= p_scalar;

    i41 *= p_scalar;
    i42 *= p_scalar;
    i43 *= p_scalar;
    i44 *= p_scalar;
    i45 *= p_scalar;
    i46 *= p_scalar;

    i51 *= p_scalar;
    i52 *= p_scalar;
    i53 *= p_scalar;
    i54 *= p_scalar;
    i55 *= p_scalar;
    i56 *= p_scalar;

    i61 *= p_scalar;
    i62 *= p_scalar;
    i63 *= p_scalar;
    i64 *= p_scalar;
    i65 *= p_scalar;
    i66 *= p_scalar;
    return *this;
}

Mat6 Mat6::operator*(const real_t p_scalar) const {
    return Mat6(
        i11*p_scalar, i12*p_scalar, i13*p_scalar, i14*p_scalar, i15*p_scalar, i16*p_scalar,
        i21*p_scalar, i22*p_scalar, i23*p_scalar, i24*p_scalar, i25*p_scalar, i26*p_scalar,
        i31*p_scalar, i32*p_scalar, i33*p_scalar, i34*p_scalar, i35*p_scalar, i36*p_scalar,
        i41*p_scalar, i42*p_scalar, i43*p_scalar, i44*p_scalar, i45*p_scalar, i46*p_scalar,
        i51*p_scalar, i52*p_scalar, i53*p_scalar, i54*p_scalar, i55*p_scalar, i56*p_scalar,
        i61*p_scalar, i62*p_scalar, i63*p_scalar, i64*p_scalar, i65*p_scalar, i66*p_scalar
    );
}

Mat6 &Mat6::operator/=(const real_t p_scalar) {
    i11 /= p_scalar;
    i12 /= p_scalar;
    i13 /= p_scalar;
    i14 /= p_scalar;
    i15 /= p_scalar;
    i16 /= p_scalar;

    i21 /= p_scalar;
    i22 /= p_scalar;
    i23 /= p_scalar;
    i24 /= p_scalar;
    i25 /= p_scalar;
    i26 /= p_scalar;

    i31 /= p_scalar;
    i32 /= p_scalar;
    i33 /= p_scalar;
    i34 /= p_scalar;
    i35 /= p_scalar;
    i36 /= p_scalar;

    i41 /= p_scalar;
    i42 /= p_scalar;
    i43 /= p_scalar;
    i44 /= p_scalar;
    i45 /= p_scalar;
    i46 /= p_scalar;

    i51 /= p_scalar;
    i52 /= p_scalar;
    i53 /= p_scalar;
    i54 /= p_scalar;
    i55 /= p_scalar;
    i56 /= p_scalar;

    i61 /= p_scalar;
    i62 /= p_scalar;
    i63 /= p_scalar;
    i64 /= p_scalar;
    i65 /= p_scalar;
    i66 /= p_scalar;
    return *this;
}

Mat6 Mat6::operator/(const real_t p_scalar) const {
    return Mat6(
        i11/p_scalar, i12/p_scalar, i13/p_scalar, i14/p_scalar, i15/p_scalar, i16/p_scalar,
        i21/p_scalar, i22/p_scalar, i23/p_scalar, i24/p_scalar, i25/p_scalar, i26/p_scalar,
        i31/p_scalar, i32/p_scalar, i33/p_scalar, i34/p_scalar, i35/p_scalar, i36/p_scalar,
        i41/p_scalar, i42/p_scalar, i43/p_scalar, i44/p_scalar, i45/p_scalar, i46/p_scalar,
        i51/p_scalar, i52/p_scalar, i53/p_scalar, i54/p_scalar, i55/p_scalar, i56/p_scalar,
        i61/p_scalar, i62/p_scalar, i63/p_scalar, i64/p_scalar, i65/p_scalar, i66/p_scalar
    );
}

Mat6 Mat6::from_array(const PackedRealArray &p_from_array) {
	const int stop_index = MIN(p_from_array.size(), 36);
	Mat6 mat;
	for (int i = 0; i < stop_index; i++) {
		mat.elements[i] = p_from_array[i];
	}
	return mat;
}

PackedRealArray Mat6::to_array() const {
	PackedRealArray arr;
	arr.resize(36);
	real_t *ptr = arr.ptrw();

	ptr[0] = i11;
	ptr[1] = i12;
	ptr[2] = i13;
	ptr[3] = i14;
	ptr[4] = i15;
	ptr[5] = i16;

	ptr[6] = i21;
	ptr[7] = i22;
	ptr[8] = i23;
	ptr[9] = i24;
	ptr[10] = i25;
	ptr[11] = i26;

	ptr[12] = i31;
	ptr[13] = i32;
	ptr[14] = i33;
	ptr[15] = i34;
	ptr[16] = i35;
	ptr[17] = i36;

	ptr[18] = i41;
	ptr[19] = i42;
	ptr[20] = i43;
	ptr[21] = i44;
	ptr[22] = i45;
	ptr[23] = i46;

	ptr[24] = i51;
	ptr[25] = i52;
	ptr[26] = i53;
	ptr[27] = i54;
	ptr[28] = i55;
	ptr[29] = i56;

	ptr[30] = i61;
	ptr[31] = i62;
	ptr[32] = i63;
	ptr[33] = i64;
	ptr[34] = i65;
	ptr[35] = i66;

	return arr;
}

Mat6::Mat6(const real_t p_i11, const real_t p_i22, const real_t p_i33, const real_t p_i44, const real_t p_i55, const real_t p_i66) {
	i11 = p_i11;
    i22 = p_i22;
    i33 = p_i33;
    i44 = p_i44;
    i55 = p_i55;
    i66 = p_i66;
}

Mat6::Mat6(
    const real_t p_i11, const real_t p_i12, const real_t p_i13, const real_t p_i14, const real_t p_i15, const real_t p_i16,
    const real_t p_i21, const real_t p_i22, const real_t p_i23, const real_t p_i24, const real_t p_i25, const real_t p_i26,
    const real_t p_i31, const real_t p_i32, const real_t p_i33, const real_t p_i34, const real_t p_i35, const real_t p_i36,
    const real_t p_i41, const real_t p_i42, const real_t p_i43, const real_t p_i44, const real_t p_i45, const real_t p_i46,
    const real_t p_i51, const real_t p_i52, const real_t p_i53, const real_t p_i54, const real_t p_i55, const real_t p_i56,
    const real_t p_i61, const real_t p_i62, const real_t p_i63, const real_t p_i64, const real_t p_i65, const real_t p_i66
) {
	i11 = p_i11;
    i12 = p_i12;
    i13 = p_i13;
    i14 = p_i14;
    i15 = p_i15;
    i16 = p_i16;

	i21 = p_i21;
    i22 = p_i22;
    i23 = p_i23;
    i24 = p_i24;
    i25 = p_i25;
    i26 = p_i26;

	i31 = p_i31;
    i32 = p_i32;
    i33 = p_i33;
    i34 = p_i34;
    i35 = p_i35;
    i36 = p_i36;

	i41 = p_i41;
    i42 = p_i42;
    i43 = p_i43;
    i44 = p_i44;
    i45 = p_i45;
    i46 = p_i46;

	i51 = p_i51;
    i52 = p_i52;
    i53 = p_i53;
    i54 = p_i54;
    i55 = p_i55;
    i56 = p_i56;

	i61 = p_i61;
    i62 = p_i62;
    i63 = p_i63;
    i64 = p_i64;
    i65 = p_i65;
    i66 = p_i66;
}

Mat6 operator*(const real_t p_scalar, const Mat6 &p_tensor) {
    return Mat6(
        p_tensor.i11*p_scalar, p_tensor.i12*p_scalar, p_tensor.i13*p_scalar, p_tensor.i14*p_scalar, p_tensor.i15*p_scalar, p_tensor.i16*p_scalar,
        p_tensor.i21*p_scalar, p_tensor.i22*p_scalar, p_tensor.i23*p_scalar, p_tensor.i24*p_scalar, p_tensor.i25*p_scalar, p_tensor.i26*p_scalar,
        p_tensor.i31*p_scalar, p_tensor.i32*p_scalar, p_tensor.i33*p_scalar, p_tensor.i34*p_scalar, p_tensor.i35*p_scalar, p_tensor.i36*p_scalar,
        p_tensor.i41*p_scalar, p_tensor.i42*p_scalar, p_tensor.i43*p_scalar, p_tensor.i44*p_scalar, p_tensor.i45*p_scalar, p_tensor.i46*p_scalar,
        p_tensor.i51*p_scalar, p_tensor.i52*p_scalar, p_tensor.i53*p_scalar, p_tensor.i54*p_scalar, p_tensor.i55*p_scalar, p_tensor.i56*p_scalar,
        p_tensor.i61*p_scalar, p_tensor.i62*p_scalar, p_tensor.i63*p_scalar, p_tensor.i64*p_scalar, p_tensor.i65*p_scalar, p_tensor.i66*p_scalar
    );
}

static_assert(sizeof(Mat6) == 36 * sizeof(real_t));