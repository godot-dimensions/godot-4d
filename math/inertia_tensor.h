#pragma once

#include "bivector_4d.h"
#include "rotor_4d.h"

// The Inertia Tensor is symetric so we only store the top half
struct _NO_DISCARD_ InertiaTensor {
	union {
		struct {
			real_t i11;
			real_t i12;
			real_t i13;
			real_t i14;
			real_t i15;
			real_t i16;

			real_t i22;
			real_t i23;
			real_t i24;
			real_t i25;
			real_t i26;

			real_t i33;
			real_t i34;
			real_t i35;
			real_t i36;

			real_t i44;
			real_t i45;
			real_t i46;

			real_t i55;
			real_t i56;

			real_t i66;
		};

		real_t elements[21] = { 0 };
	};

	InertiaTensor inverse() const;

	// // Operators.
	// bool operator==(const InertiaTensor &p_b) const;
	// bool operator!=(const InertiaTensor &p_b) const;
	// InertiaTensor operator-() const;

	InertiaTensor &operator+=(const InertiaTensor &p_b);
	InertiaTensor operator+(const InertiaTensor &p_b) const;
	InertiaTensor &operator-=(const InertiaTensor &p_b);
	InertiaTensor operator-(const InertiaTensor &p_b) const;

	// InertiaTensor &operator*=(const InertiaTensor &p_b);
	InertiaTensor operator*(const InertiaTensor &p_b) const;
	// InertiaTensor &operator/=(const InertiaTensor &p_b);
	// InertiaTensor operator/(const InertiaTensor &p_b) const;

	InertiaTensor &operator*=(const real_t p_scalar);
	InertiaTensor operator*(const real_t p_scalar) const;
	InertiaTensor &operator/=(const real_t p_scalar);
	InertiaTensor operator/(const real_t p_scalar) const;

	// Conversion.
	static InertiaTensor from_array(const PackedRealArray &p_from_array);
	PackedRealArray to_array() const;

	// Constructors.
	InertiaTensor() {}
	InertiaTensor(const real_t p_i11, const real_t p_i22, const real_t p_i33, const real_t p_i44, const real_t p_i55, const real_t p_i66);
	InertiaTensor(
		const real_t p_i11, const real_t p_i12, const real_t p_i13, const real_t p_i14, const real_t p_i15, const real_t p_i16,
		const real_t p_i22, const real_t p_i23, const real_t p_i24, const real_t p_i25, const real_t p_i26,
		const real_t p_i33, const real_t p_i34, const real_t p_i35, const real_t p_i36,
		const real_t p_i44, const real_t p_i45, const real_t p_i46,
		const real_t p_i55, const real_t p_i56,
		const real_t p_i66
	);
};

InertiaTensor operator*(const real_t p_scalar, const InertiaTensor &p_tensor);

static_assert(sizeof(InertiaTensor) == sizeof(real_t) * 21);
