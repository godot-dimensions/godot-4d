#pragma once

#include "../godot_4d_defines.h"

#include "geometric_algebra/bivector_4d.h"

#if GDEXTENSION
#include <godot_cpp/variant/projection.hpp>
#elif GODOT_MODULE
#include "core/math/projection.h"
#endif

struct _NO_DISCARD_ Basis4D {
	// IMPORTANT NOTE: Godot stores 2D basis vectors with columns,
	// but stores 3D basis vectors with rows. Columns are much more
	// intuitive work with, the only reason Godot stores rows in 3D
	// is for low-level optimization reasons. For Basis4D, we store
	// columns, NOT rows, for improved code readability.
	Vector4 x = Vector4(1, 0, 0, 0);
	Vector4 y = Vector4(0, 1, 0, 0);
	Vector4 z = Vector4(0, 0, 1, 0);
	Vector4 w = Vector4(0, 0, 0, 1);

	// Misc methods.
	real_t determinant() const;
	bool is_equal_approx(const Basis4D &p_basis) const;
	Basis4D lerp(const Basis4D &p_to, const real_t &p_weight) const;

	// Transformation methods.
	Basis4D compose(const Basis4D &p_child) const;
	Vector4 xform(const Vector4 &p_vector) const;
	Vector4 xform_inv(const Vector4 &p_vector) const;
	Vector4 xform_transposed(const Vector4 &p_vector) const;
	Bivector4D rotate_bivector(const Bivector4D &p_bivector) const;

	// Inversion methods.
	void transpose();
	Basis4D inverse() const;
	Basis4D transposed() const;
	Basis4D adjugate() const;

	// Scale methods.
	void scale_global(const Vector4 &p_scale);
	Basis4D scaled_global(const Vector4 &p_scale) const;
	void scale_local(const Vector4 &p_scale);
	Basis4D scaled_local(const Vector4 &p_scale) const;
	void scale_uniform(const real_t p_scale);
	Basis4D scaled_uniform(const real_t p_scale) const;
	Vector4 get_scale() const;
	Vector4 get_scale_abs() const;
	void set_scale(const Vector4 &p_scale);
	real_t get_uniform_scale() const;
	void set_uniform_scale(const real_t p_uniform_scale);

	// Validation methods.
	void orthonormalize();
	Basis4D orthonormalized() const;
	void orthogonalize();
	Basis4D orthogonalized() const;
	bool is_orthogonal() const;
	bool is_orthonormal() const;
	bool is_conformal() const;
	bool is_diagonal() const;
	bool is_rotation() const;

	// Helper setters/getters.
	_FORCE_INLINE_ Vector4 get_column(int p_index) const {
		return this->operator[](p_index);
	}

	_FORCE_INLINE_ void set_column(int p_index, const Vector4 &p_value) {
		this->operator[](p_index) = p_value;
	}

	_FORCE_INLINE_ Vector4 get_row(int i) const {
		return Vector4(x[i], y[i], z[i], w[i]);
	}

	_FORCE_INLINE_ void set_row(int i, const Vector4 &p_row) {
		x[i] = p_row.x;
		y[i] = p_row.y;
		z[i] = p_row.z;
		w[i] = p_row.w;
	}

	_FORCE_INLINE_ Vector4 get_main_diagonal() const {
		return Vector4(x.x, y.y, z.z, w.w);
	}

	// Operators.
	_FORCE_INLINE_ const Vector4 &operator[](unsigned int p_index) const {
		DEV_ASSERT((unsigned int)p_index < 4);
		if (p_index == 0) {
			return x;
		} else if (p_index == 1) {
			return y;
		} else if (p_index == 2) {
			return z;
		}
		return w;
	}

	_FORCE_INLINE_ Vector4 &operator[](unsigned int p_index) {
		DEV_ASSERT((unsigned int)p_index < 4);
		if (p_index == 0) {
			return x;
		} else if (p_index == 1) {
			return y;
		} else if (p_index == 2) {
			return z;
		}
		return w;
	}

	bool operator==(const Basis4D &p_matrix) const;
	bool operator!=(const Basis4D &p_matrix) const;
	Basis4D operator-() const;

	void operator*=(const Basis4D &p_matrix);
	Basis4D operator*(const Basis4D &p_matrix) const;
	Vector4 operator*(const Vector4 &p_vector) const;
	void operator+=(const Basis4D &p_matrix);
	Basis4D operator+(const Basis4D &p_matrix) const;
	void operator-=(const Basis4D &p_matrix);
	Basis4D operator-(const Basis4D &p_matrix) const;
	void operator*=(const real_t p_val);
	Basis4D operator*(const real_t p_val) const;
	void operator/=(const real_t p_val);
	Basis4D operator/(const real_t p_val) const;
	operator String() const;

	// Conversion.
	static Basis4D from_3d(const Basis &p_from_3d);
	Basis to_3d(const bool p_orthonormalized = false) const;
	static Basis4D from_array(const PackedRealArray &p_from_array);
	PackedRealArray to_array() const;
	// For binding to Variant, pick an existing Variant data type.
	// Projection has 16 real_t fields so it will work fine.
	operator Projection() const;
	Basis4D(const Projection &p_from);

	// Constructors.
	static Basis4D from_scale(const Vector4 &p_scale);
	static Basis4D from_scale(const real_t p_x, const real_t p_y, const real_t p_z, const real_t p_w);
	static Basis4D from_scale_uniform(const real_t p_scale);
	static Basis4D from_yz(const real_t p_yz);
	static Basis4D from_zx(const real_t p_zx);
	static Basis4D from_xy(const real_t p_xy);
	static Basis4D from_xw(const real_t p_xw);
	static Basis4D from_wy(const real_t p_wy);
	static Basis4D from_zw(const real_t p_zw);

	_FORCE_INLINE_ Basis4D() {}
	_FORCE_INLINE_ Basis4D(const Vector4 &p_x, const Vector4 &p_y, const Vector4 &p_z, const Vector4 &p_w) {
		x = p_x;
		y = p_y;
		z = p_z;
		w = p_w;
	}
	_FORCE_INLINE_ Basis4D(const real_t p_xx, const real_t p_xy, const real_t p_xz, const real_t p_xw, const real_t p_yx, const real_t p_yy, const real_t p_yz, const real_t p_yw, const real_t p_zx, const real_t p_zy, const real_t p_zz, const real_t p_zw, const real_t p_wx, const real_t p_wy, const real_t p_wz, const real_t p_ww) {
		x = Vector4(p_xx, p_xy, p_xz, p_xw);
		y = Vector4(p_yx, p_yy, p_yz, p_yw);
		z = Vector4(p_zx, p_zy, p_zz, p_zw);
		w = Vector4(p_wx, p_wy, p_wz, p_ww);
	}
};
