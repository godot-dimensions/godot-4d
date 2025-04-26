#pragma once

#include "../godot_4d_defines.h"

struct _NO_DISCARD_ Mat6 {
	union {
		struct {
			real_t i11;
			real_t i12;
			real_t i13;
			real_t i14;
			real_t i15;
			real_t i16;

			real_t i21;
			real_t i22;
			real_t i23;
			real_t i24;
			real_t i25;
			real_t i26;

			real_t i31;
			real_t i32;
			real_t i33;
			real_t i34;
			real_t i35;
			real_t i36;

			real_t i41;
			real_t i42;
			real_t i43;
			real_t i44;
			real_t i45;
			real_t i46;

			real_t i51;
			real_t i52;
			real_t i53; 
			real_t i54;
			real_t i55;
			real_t i56;

			real_t i61;
			real_t i62;
			real_t i63;
			real_t i64;
			real_t i65;
			real_t i66;
		};

		real_t elements[36] = { 0 };
	};

	// Mat6 inverse() const;

	// // Operators.
	// bool operator==(const Mat6 &p_b) const;
	// bool operator!=(const Mat6 &p_b) const;
	// Mat6 operator-() const;

	Mat6 &operator+=(const Mat6 &p_b);
	Mat6 operator+(const Mat6 &p_b) const;
	Mat6 &operator-=(const Mat6 &p_b);
	Mat6 operator-(const Mat6 &p_b) const;

	// Mat6 &operator*=(const Mat6 &p_b);
	Mat6 operator*(const Mat6 &p_b) const;
	// Mat6 &operator/=(const Mat6 &p_b);
	// Mat6 operator/(const Mat6 &p_b) const;

	Mat6 &operator*=(const real_t p_scalar);
	Mat6 operator*(const real_t p_scalar) const;
	Mat6 &operator/=(const real_t p_scalar);
	Mat6 operator/(const real_t p_scalar) const;

	// Conversion.
	static Mat6 from_array(const PackedRealArray &p_from_array);
	PackedRealArray to_array() const;

	// Constructors.
	Mat6() {}
	Mat6(const real_t p_i11, const real_t p_i22, const real_t p_i33, const real_t p_i44, const real_t p_i55, const real_t p_i66);
	Mat6(
		const real_t p_i11, const real_t p_i12, const real_t p_i13, const real_t p_i14, const real_t p_i15, const real_t p_i16,
		const real_t p_i21, const real_t p_i22, const real_t p_i23, const real_t p_i24, const real_t p_i25, const real_t p_i26,
		const real_t p_i31, const real_t p_i32, const real_t p_i33, const real_t p_i34, const real_t p_i35, const real_t p_i36,
		const real_t p_i41, const real_t p_i42, const real_t p_i43, const real_t p_i44, const real_t p_i45, const real_t p_i46,
		const real_t p_i51, const real_t p_i52, const real_t p_i53, const real_t p_i54, const real_t p_i55, const real_t p_i56,
		const real_t p_i61, const real_t p_i62, const real_t p_i63, const real_t p_i64, const real_t p_i65, const real_t p_i66
	);
};

Mat6 operator*(const real_t p_scalar, const Mat6 &p_tensor);

static_assert(sizeof(Mat6) == sizeof(real_t) * 36);
