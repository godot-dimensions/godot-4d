#include "g4mf_accessor_4d.h"

#include "../g4mf_state_4d.h"

// Private general helper functions.

double G4MFAccessor4D::_float8_to_double(const uint8_t p_float8) {
	const uint8_t f8_sign = (p_float8 >> 7) & 0x1;
	const uint8_t f8_exponent = (p_float8 >> 3) & 0xF;
	const uint8_t f8_mantissa = p_float8 & 0x7;
	uint64_t f64_sign = uint64_t(f8_sign) << 63;
	uint64_t f64_exponent = 0;
	uint64_t f64_mantissa = 0;
	if (f8_exponent == 0) {
		if (f8_mantissa != 0) {
			// Subnormal: real exponent is -6 so multiply 3-bit mantissa by 2^-9.
			const double value = f8_mantissa * 0.001953125;
			return f8_sign ? -value : value;
		}
	} else if (f8_exponent == 0xF) {
		// Infinity or NaN: Exponent is maxed out.
		f64_exponent = uint64_t(0x7FF) << 52;
		f64_mantissa = uint64_t(f8_mantissa) << (52 - 3);
	} else {
		// Normalized number.
		const uint64_t f8_real_exp = uint64_t(f8_exponent) - 7; // float8 bias is 7.
		const uint64_t f64_biased_exp = uint64_t(f8_real_exp + 1023); // double bias is 1023.
		f64_exponent = f64_biased_exp << 52;
		f64_mantissa = uint64_t(f8_mantissa) << (52 - 3); // Convert 3-bit mantissa to 52-bit.
	}
	union {
		double d;
		uint64_t bits;
	} u;
	u.bits = f64_sign | f64_exponent | f64_mantissa;
	return u.d;
}

double G4MFAccessor4D::_float16_to_double(const uint16_t p_float16) {
	const uint16_t f16_sign = (p_float16 >> 15) & 0x1;
	const uint16_t f16_exponent = (p_float16 >> 10) & 0x1F;
	const uint16_t f16_mantissa = p_float16 & 0x3FF;
	uint64_t f64_sign = uint64_t(f16_sign) << 63;
	uint64_t f64_exponent = 0;
	uint64_t f64_mantissa = 0;
	if (f16_exponent == 0) {
		if (f16_mantissa != 0) {
			// Subnormal: real exponent is -14 so multiply 10-bit mantissa by 2^-24.
			const double value = f16_mantissa * 0.000000059604644775390625;
			return f16_sign ? -value : value;
		}
	} else if (f16_exponent == 0x1F) {
		// Infinity or NaN.
		f64_exponent = uint64_t(0x7FF) << 52;
		f64_mantissa = uint64_t(f16_mantissa) << (52 - 10);
	} else {
		// Normalized number.
		const uint64_t f16_real_exp = uint64_t(f16_exponent) - 15; // float16 bias is 15.
		const uint64_t f64_biased_exp = f16_real_exp + 1023; // double bias is 1023.
		f64_exponent = f64_biased_exp << 52;
		f64_mantissa = uint64_t(f16_mantissa) << (52 - 10); // Convert 10-bit mantissa to 52-bit.
	}
	union {
		double d;
		uint64_t bits;
	} u;
	u.bits = f64_sign | f64_exponent | f64_mantissa;
	return u.d;
}

uint8_t G4MFAccessor4D::_double_to_float8(const double p_double) {
	union {
		double d;
		uint64_t bits;
	} u;
	u.d = p_double;
	const uint64_t f64_sign = (u.bits >> 63) & 0x1;
	const uint64_t f64_exponent = (u.bits >> 52) & 0x7FF;
	const uint64_t f64_mantissa = u.bits & ((uint64_t(1) << 52) - 1);
	const uint8_t f8_sign = f64_sign << 7;
	if (f64_exponent == 0x7FF) {
		// Infinity or NaN.
		const uint8_t mantissa = f64_mantissa ? 0x1 : 0;
		return f8_sign | uint8_t(0xF << 3) | mantissa;
	}
	const int32_t f64_real_exp = int32_t(f64_exponent) - 1023;
	if (f64_real_exp > 8) {
		// Overflow: too large, becomes Infinity.
		return f8_sign | uint8_t(0xF << 3);
	}
	if (f64_real_exp < -6) {
		// Subnormal or underflow to zero (exponent becomes zero).
		if (f64_real_exp < -9) {
			// Too small, flush to zero (or negative zero).
			return f8_sign;
		}
		// Convert to subnormal.
		const int32_t shift = (-6 - f64_real_exp);
		const uint64_t f64_mantissa_bits = (uint64_t(1) << 52) | f64_mantissa;
		const uint64_t round_bit = (f64_mantissa_bits >> (52 + shift - 4)) & 1;
		uint64_t f8_mantissa_bits = f64_mantissa_bits >> (52 + shift - 3); // 3-bit mantissa.
		// Round and clamp.
		f8_mantissa_bits += round_bit;
		if (f8_mantissa_bits > 0x7) {
			f8_mantissa_bits = 0x7;
		}
		return f8_sign | uint8_t(f8_mantissa_bits);
	}
	// Normal case.
	const uint64_t round_bit = (f64_mantissa >> (52 - 4)) & 1;
	uint8_t f8_exponent = uint8_t(f64_real_exp + 7); // Re-bias to float8.
	uint64_t f8_mantissa_bits = f64_mantissa >> (52 - 3);
	f8_mantissa_bits += round_bit;
	// If the mantissa overflows, we need to increment the exponent.
	if (f8_mantissa_bits > 0x7) {
		f8_mantissa_bits = 0;
		f8_exponent += 1;
		// Overflow may go to Infinity.
		if (f8_exponent >= 0xF) {
			return f8_sign | uint8_t(0xF << 3);
		}
	}
	return f8_sign | uint8_t(f8_exponent << 3) | uint8_t(f8_mantissa_bits);
}

uint16_t G4MFAccessor4D::_double_to_float16(const double p_double) {
	union {
		double d;
		uint64_t bits;
	} u;
	u.d = p_double;
	const uint64_t f64_sign = (u.bits >> 63) & 0x1;
	const uint64_t f64_exponent = (u.bits >> 52) & 0x7FF;
	const uint64_t f64_mantissa = u.bits & ((uint64_t(1) << 52) - 1);
	const uint16_t f16_sign = f64_sign << 15;
	if (f64_exponent == 0x7FF) {
		// Infinity or NaN.
		const uint16_t mantissa = f64_mantissa ? 0x1 : 0;
		return f16_sign | uint16_t(0x1F << 10) | mantissa;
	}
	const int32_t f64_real_exp = int32_t(f64_exponent) - 1023;
	if (f64_real_exp > 15) {
		// Overflow: too large, becomes Infinity.
		return f16_sign | uint16_t(0x1F << 10);
	}
	if (f64_real_exp < -14) {
		// Subnormal or underflow to zero.
		if (f64_real_exp < -25) {
			// Too small, flush to zero.
			return f16_sign;
		}
		// Convert to subnormal.
		const int32_t shift = (-14 - f64_real_exp);
		const uint64_t f64_mantissa_bits = (uint64_t(1) << 52) | f64_mantissa;
		const uint64_t round_bit = (f64_mantissa_bits >> (52 + shift - 11)) & 1;
		uint64_t f16_mantissa_bits = f64_mantissa_bits >> (52 + shift - 10); // 10-bit mantissa.
		// Round and clamp.
		f16_mantissa_bits += round_bit;
		if (f16_mantissa_bits > 0x3FF) {
			f16_mantissa_bits = 0x3FF;
		}
		return f16_sign | uint16_t(f16_mantissa_bits);
	}
	// Normal case.
	const uint64_t round_bit = (f64_mantissa >> (52 - 11)) & 1;
	uint16_t f16_exponent = uint16_t(f64_real_exp + 15); // Re-bias to float16.
	uint64_t f16_mantissa_bits = f64_mantissa >> (52 - 10);
	f16_mantissa_bits += round_bit;
	// If the mantissa overflows, increment exponent.
	if (f16_mantissa_bits > 0x3FF) {
		f16_mantissa_bits = 0;
		f16_exponent += 1;
		if (f16_exponent >= 0x1F) {
			return f16_sign | uint16_t(0x1F << 10);
		}
	}
	return f16_sign | uint16_t(f16_exponent << 10) | uint16_t(f16_mantissa_bits);
}

// Private functions for determining the minimal primitive type.

bool G4MFAccessor4D::_double_bits_equal(const double p_a, const double p_b) {
	union {
		double d;
		uint64_t bits;
	} a{ p_a }, b{ p_b };
	return a.bits == b.bits;
}

void G4MFAccessor4D::_minimal_primitive_bits_for_int64(const int64_t p_value, uint32_t &r_int_bits, uint32_t &r_uint_bits) {
	if (r_int_bits == 8 && !(p_value >= INT8_MIN && p_value <= INT8_MAX)) {
		r_int_bits = 16;
	}
	if (r_int_bits == 16 && !(p_value >= INT16_MIN && p_value <= INT16_MAX)) {
		r_int_bits = 32;
	}
	if (r_int_bits == 32 && !(p_value >= INT32_MIN && p_value <= INT32_MAX)) {
		r_int_bits = 64;
	}
	if (p_value < 0) {
		// Can't be represented as an unsigned int, so just set this to a very big number.
		r_uint_bits = CANT_USE_PRIM_TYPE;
		return;
	}
	if (r_uint_bits == 8 && p_value > UINT8_MAX) {
		r_uint_bits = 16;
	}
	if (r_uint_bits == 16 && p_value > UINT16_MAX) {
		r_uint_bits = 32;
	}
	if (r_uint_bits == 32 && p_value > UINT32_MAX) {
		r_uint_bits = 64;
	}
}

void G4MFAccessor4D::_minimal_primitive_bits_for_double(const double p_value, uint32_t &r_float_bits, uint32_t &r_int_bits, uint32_t &r_uint_bits) {
	if (r_float_bits == 8 && !_double_bits_equal(_float8_to_double(_double_to_float8(p_value)), p_value)) {
		r_float_bits = 16;
	}
	if (r_float_bits == 16 && !_double_bits_equal(_float16_to_double(_double_to_float16(p_value)), p_value)) {
		r_float_bits = 32;
	}
	if (r_float_bits == 32 && !_double_bits_equal((double)(float)p_value, p_value)) {
		r_float_bits = 64;
	}
	const int64_t as_int = (int64_t)p_value;
	if (p_value != (double)as_int) {
		// Can't be represented as an int, so just set these to a very big number.
		r_int_bits = CANT_USE_PRIM_TYPE;
		r_uint_bits = CANT_USE_PRIM_TYPE;
		return;
	}
	_minimal_primitive_bits_for_int64(as_int, r_int_bits, r_uint_bits);
}

String G4MFAccessor4D::_minimal_primitive_type_given_bits(const uint32_t p_float_bits, const uint32_t p_int_bits, const uint32_t p_uint_bits) {
	// Only use floats if they are more efficient than integers.
	if (p_float_bits < p_int_bits && p_float_bits < p_uint_bits) {
		return String("float") + String::num_uint64(p_float_bits);
	}
	if (p_int_bits < p_uint_bits) {
		return String("int") + String::num_uint64(p_int_bits);
	}
	// Prefer unsigned ints if all are equally efficient.
	return String("uint") + String::num_uint64(p_int_bits);
}

// Private decode functions. Use `decode_accessor_as_variants` publicly.

#define G4MF_ACCESSOR_4D_DECODE_PRIMITIVES_AS_VARIANTS(m_primitives, m_values)                                                                                                                    \
	ERR_FAIL_COND_V_MSG(m_primitives.size() % _vector_size != 0, m_values, "G4MF import: The primitive size was not a multiple of the vector size. Returning an empty array.");                   \
	const int values_size = m_primitives.size() / _vector_size;                                                                                                                                   \
	const int prims_per_variant = primitives_per_variant(p_variant_type);                                                                                                                         \
	ERR_FAIL_COND_V_MSG(prims_per_variant < 1, m_values, "G4MF import: The Variant type '" + Variant::get_type_name(p_variant_type) + "' is not supported. Returning an empty array.");           \
	const int prims_to_read = MIN(_vector_size, prims_per_variant);                                                                                                                               \
	m_values.resize(values_size);                                                                                                                                                                 \
	for (int i = 0; i < values_size; i++) {                                                                                                                                                       \
		const int prim_offset = i * _vector_size;                                                                                                                                                 \
		ERR_FAIL_COND_V_MSG(prim_offset + prims_to_read > m_primitives.size(), m_values, "G4MF import: The primitive size was not a multiple of the vector size. Returning an empty array.");     \
		switch (p_variant_type) {                                                                                                                                                                 \
			case Variant::BOOL: {                                                                                                                                                                 \
				m_values[i] = m_primitives[prim_offset] != 0.0;                                                                                                                                   \
			} break;                                                                                                                                                                              \
			case Variant::INT: {                                                                                                                                                                  \
				m_values[i] = m_primitives[prim_offset];                                                                                                                                          \
			} break;                                                                                                                                                                              \
			case Variant::FLOAT: {                                                                                                                                                                \
				m_values[i] = m_primitives[prim_offset];                                                                                                                                          \
			} break;                                                                                                                                                                              \
			case Variant::VECTOR2:                                                                                                                                                                \
			case Variant::RECT2:                                                                                                                                                                  \
			case Variant::VECTOR3:                                                                                                                                                                \
			case Variant::VECTOR4:                                                                                                                                                                \
			case Variant::PLANE:                                                                                                                                                                  \
			case Variant::QUATERNION: {                                                                                                                                                           \
				/* General-purpose code for importing G4MF accessor data with any primitive count into structs up to 4 `real_t`s in size. */                                                      \
				Variant v;                                                                                                                                                                        \
				switch (prims_to_read) {                                                                                                                                                          \
					case 1: {                                                                                                                                                                     \
						v = Vector4(m_primitives[prim_offset], 0.0f, 0.0f, 0.0f);                                                                                                                 \
					} break;                                                                                                                                                                      \
					case 2: {                                                                                                                                                                     \
						v = Vector4(m_primitives[prim_offset], m_primitives[prim_offset + 1], 0.0f, 0.0f);                                                                                        \
					} break;                                                                                                                                                                      \
					case 3: {                                                                                                                                                                     \
						v = Vector4(m_primitives[prim_offset], m_primitives[prim_offset + 1], m_primitives[prim_offset + 2], 0.0f);                                                               \
					} break;                                                                                                                                                                      \
					default: {                                                                                                                                                                    \
						v = Vector4(m_primitives[prim_offset], m_primitives[prim_offset + 1], m_primitives[prim_offset + 2], m_primitives[prim_offset + 3]);                                      \
					} break;                                                                                                                                                                      \
				}                                                                                                                                                                                 \
				/* Evil hack that relies on the structure of Variant, but it's the */                                                                                                             \
				/* only way to accomplish this without a ton of code duplication. */                                                                                                              \
				*(Variant::Type *)&v = p_variant_type;                                                                                                                                            \
				m_values[i] = v;                                                                                                                                                                  \
			} break;                                                                                                                                                                              \
			case Variant::VECTOR2I:                                                                                                                                                               \
			case Variant::RECT2I:                                                                                                                                                                 \
			case Variant::VECTOR3I:                                                                                                                                                               \
			case Variant::VECTOR4I: {                                                                                                                                                             \
				/* General-purpose code for importing G4MF accessor data with any primitive count into structs up to 4 `int32_t`s in size. */                                                     \
				Variant v;                                                                                                                                                                        \
				switch (prims_to_read) {                                                                                                                                                          \
					case 1: {                                                                                                                                                                     \
						v = Vector4i((int32_t)m_primitives[prim_offset], 0, 0, 0);                                                                                                                \
					} break;                                                                                                                                                                      \
					case 2: {                                                                                                                                                                     \
						v = Vector4i((int32_t)m_primitives[prim_offset], (int32_t)m_primitives[prim_offset + 1], 0, 0);                                                                           \
					} break;                                                                                                                                                                      \
					case 3: {                                                                                                                                                                     \
						v = Vector4i((int32_t)m_primitives[prim_offset], (int32_t)m_primitives[prim_offset + 1], (int32_t)m_primitives[prim_offset + 2], 0);                                      \
					} break;                                                                                                                                                                      \
					default: {                                                                                                                                                                    \
						v = Vector4i((int32_t)m_primitives[prim_offset], (int32_t)m_primitives[prim_offset + 1], (int32_t)m_primitives[prim_offset + 2], (int32_t)m_primitives[prim_offset + 3]); \
					} break;                                                                                                                                                                      \
				}                                                                                                                                                                                 \
				/* Evil hack that relies on the structure of Variant, but it's the */                                                                                                             \
				/* only way to accomplish this without a ton of code duplication. */                                                                                                              \
				*(Variant::Type *)&v = p_variant_type;                                                                                                                                            \
				m_values[i] = v;                                                                                                                                                                  \
			} break;                                                                                                                                                                              \
			/* No more generalized hacks, each of the below types needs a lot of repetitive code. */                                                                                              \
			case Variant::COLOR: {                                                                                                                                                                \
				Variant v;                                                                                                                                                                        \
				switch (prims_to_read) {                                                                                                                                                          \
					case 1: {                                                                                                                                                                     \
						v = Color(m_primitives[prim_offset], 0.0f, 0.0f, 1.0f);                                                                                                                   \
					} break;                                                                                                                                                                      \
					case 2: {                                                                                                                                                                     \
						v = Color(m_primitives[prim_offset], m_primitives[prim_offset + 1], 0.0f, 1.0f);                                                                                          \
					} break;                                                                                                                                                                      \
					case 3: {                                                                                                                                                                     \
						v = Color(m_primitives[prim_offset], m_primitives[prim_offset + 1], m_primitives[prim_offset + 2], 1.0f);                                                                 \
					} break;                                                                                                                                                                      \
					default: {                                                                                                                                                                    \
						v = Color(m_primitives[prim_offset], m_primitives[prim_offset + 1], m_primitives[prim_offset + 2], m_primitives[prim_offset + 3]);                                        \
					} break;                                                                                                                                                                      \
				}                                                                                                                                                                                 \
				m_values[i] = v;                                                                                                                                                                  \
			} break;                                                                                                                                                                              \
			case Variant::TRANSFORM2D: {                                                                                                                                                          \
				Transform2D t;                                                                                                                                                                    \
				switch (prims_to_read) {                                                                                                                                                          \
					case 4: {                                                                                                                                                                     \
						t.columns[0] = Vector2(m_primitives[prim_offset + 0], m_primitives[prim_offset + 1]);                                                                                     \
						t.columns[1] = Vector2(m_primitives[prim_offset + 2], m_primitives[prim_offset + 3]);                                                                                     \
					} break;                                                                                                                                                                      \
					case 9: {                                                                                                                                                                     \
						t.columns[0] = Vector2(m_primitives[prim_offset + 0], m_primitives[prim_offset + 1]);                                                                                     \
						t.columns[1] = Vector2(m_primitives[prim_offset + 3], m_primitives[prim_offset + 4]);                                                                                     \
						t.columns[2] = Vector2(m_primitives[prim_offset + 6], m_primitives[prim_offset + 7]);                                                                                     \
					} break;                                                                                                                                                                      \
					case 16: {                                                                                                                                                                    \
						t.columns[0] = Vector2(m_primitives[prim_offset + 0], m_primitives[prim_offset + 1]);                                                                                     \
						t.columns[1] = Vector2(m_primitives[prim_offset + 4], m_primitives[prim_offset + 5]);                                                                                     \
						t.columns[2] = Vector2(m_primitives[prim_offset + 12], m_primitives[prim_offset + 13]);                                                                                   \
					} break;                                                                                                                                                                      \
				}                                                                                                                                                                                 \
				m_values[i] = t;                                                                                                                                                                  \
			} break;                                                                                                                                                                              \
			case Variant::BASIS: {                                                                                                                                                                \
				Basis b;                                                                                                                                                                          \
				switch (prims_to_read) {                                                                                                                                                          \
					case 4: {                                                                                                                                                                     \
						b.rows[0] = Vector3(m_primitives[prim_offset + 0], m_primitives[prim_offset + 2], 0.0f);                                                                                  \
						b.rows[1] = Vector3(m_primitives[prim_offset + 1], m_primitives[prim_offset + 3], 0.0f);                                                                                  \
					} break;                                                                                                                                                                      \
					case 9: {                                                                                                                                                                     \
						b.rows[0] = Vector3(m_primitives[prim_offset + 0], m_primitives[prim_offset + 3], m_primitives[prim_offset + 6]);                                                         \
						b.rows[1] = Vector3(m_primitives[prim_offset + 1], m_primitives[prim_offset + 4], m_primitives[prim_offset + 7]);                                                         \
						b.rows[2] = Vector3(m_primitives[prim_offset + 2], m_primitives[prim_offset + 5], m_primitives[prim_offset + 8]);                                                         \
					} break;                                                                                                                                                                      \
					case 16: {                                                                                                                                                                    \
						b.rows[0] = Vector3(m_primitives[prim_offset + 0], m_primitives[prim_offset + 4], m_primitives[prim_offset + 8]);                                                         \
						b.rows[1] = Vector3(m_primitives[prim_offset + 1], m_primitives[prim_offset + 5], m_primitives[prim_offset + 9]);                                                         \
						b.rows[2] = Vector3(m_primitives[prim_offset + 2], m_primitives[prim_offset + 6], m_primitives[prim_offset + 10]);                                                        \
					} break;                                                                                                                                                                      \
				}                                                                                                                                                                                 \
				m_values[i] = b;                                                                                                                                                                  \
			} break;                                                                                                                                                                              \
			case Variant::TRANSFORM3D: {                                                                                                                                                          \
				Transform3D t;                                                                                                                                                                    \
				switch (prims_to_read) {                                                                                                                                                          \
					case 4: {                                                                                                                                                                     \
						t.basis.rows[0] = Vector3(m_primitives[prim_offset + 0], m_primitives[prim_offset + 2], 0.0f);                                                                            \
						t.basis.rows[1] = Vector3(m_primitives[prim_offset + 1], m_primitives[prim_offset + 3], 0.0f);                                                                            \
					} break;                                                                                                                                                                      \
					case 9: {                                                                                                                                                                     \
						t.basis.rows[0] = Vector3(m_primitives[prim_offset + 0], m_primitives[prim_offset + 3], m_primitives[prim_offset + 6]);                                                   \
						t.basis.rows[1] = Vector3(m_primitives[prim_offset + 1], m_primitives[prim_offset + 4], m_primitives[prim_offset + 7]);                                                   \
						t.basis.rows[2] = Vector3(m_primitives[prim_offset + 2], m_primitives[prim_offset + 5], m_primitives[prim_offset + 8]);                                                   \
					} break;                                                                                                                                                                      \
					case 16: {                                                                                                                                                                    \
						t.basis.rows[0] = Vector3(m_primitives[prim_offset + 0], m_primitives[prim_offset + 4], m_primitives[prim_offset + 8]);                                                   \
						t.basis.rows[1] = Vector3(m_primitives[prim_offset + 1], m_primitives[prim_offset + 5], m_primitives[prim_offset + 9]);                                                   \
						t.basis.rows[2] = Vector3(m_primitives[prim_offset + 2], m_primitives[prim_offset + 6], m_primitives[prim_offset + 10]);                                                  \
						t.origin = Vector3(m_primitives[prim_offset + 12], m_primitives[prim_offset + 13], m_primitives[prim_offset + 14]);                                                       \
					} break;                                                                                                                                                                      \
				}                                                                                                                                                                                 \
				m_values[i] = t;                                                                                                                                                                  \
			} break;                                                                                                                                                                              \
			case Variant::PROJECTION: {                                                                                                                                                           \
				Projection p;                                                                                                                                                                     \
				switch (prims_to_read) {                                                                                                                                                          \
					case 4: {                                                                                                                                                                     \
						p.columns[0] = Vector4(m_primitives[prim_offset + 0], m_primitives[prim_offset + 1], 0.0f, 0.0f);                                                                         \
						p.columns[1] = Vector4(m_primitives[prim_offset + 4], m_primitives[prim_offset + 5], 0.0f, 0.0f);                                                                         \
					} break;                                                                                                                                                                      \
					case 9: {                                                                                                                                                                     \
						p.columns[0] = Vector4(m_primitives[prim_offset + 0], m_primitives[prim_offset + 1], m_primitives[prim_offset + 2], 0.0f);                                                \
						p.columns[1] = Vector4(m_primitives[prim_offset + 4], m_primitives[prim_offset + 5], m_primitives[prim_offset + 6], 0.0f);                                                \
						p.columns[2] = Vector4(m_primitives[prim_offset + 8], m_primitives[prim_offset + 9], m_primitives[prim_offset + 10], 0.0f);                                               \
					} break;                                                                                                                                                                      \
					case 16: {                                                                                                                                                                    \
						p.columns[0] = Vector4(m_primitives[prim_offset + 0], m_primitives[prim_offset + 1], m_primitives[prim_offset + 2], m_primitives[prim_offset + 3]);                       \
						p.columns[1] = Vector4(m_primitives[prim_offset + 4], m_primitives[prim_offset + 5], m_primitives[prim_offset + 6], m_primitives[prim_offset + 7]);                       \
						p.columns[2] = Vector4(m_primitives[prim_offset + 8], m_primitives[prim_offset + 9], m_primitives[prim_offset + 10], m_primitives[prim_offset + 11]);                     \
						p.columns[3] = Vector4(m_primitives[prim_offset + 12], m_primitives[prim_offset + 13], m_primitives[prim_offset + 14], m_primitives[prim_offset + 15]);                   \
					} break;                                                                                                                                                                      \
				}                                                                                                                                                                                 \
				m_values[i] = p;                                                                                                                                                                  \
			} break;                                                                                                                                                                              \
			case Variant::PACKED_BYTE_ARRAY: {                                                                                                                                                    \
				PackedByteArray packed_array;                                                                                                                                                     \
				packed_array.resize(prims_per_variant);                                                                                                                                           \
				for (int j = 0; j < prims_per_variant; j++) {                                                                                                                                     \
					packed_array.set(i, m_primitives[prim_offset + j]);                                                                                                                           \
				}                                                                                                                                                                                 \
			} break;                                                                                                                                                                              \
			case Variant::PACKED_INT32_ARRAY: {                                                                                                                                                   \
				PackedInt32Array packed_array;                                                                                                                                                    \
				packed_array.resize(prims_per_variant);                                                                                                                                           \
				for (int j = 0; j < prims_per_variant; j++) {                                                                                                                                     \
					packed_array.set(i, m_primitives[prim_offset + j]);                                                                                                                           \
				}                                                                                                                                                                                 \
			} break;                                                                                                                                                                              \
			case Variant::PACKED_INT64_ARRAY: {                                                                                                                                                   \
				PackedInt64Array packed_array;                                                                                                                                                    \
				packed_array.resize(prims_per_variant);                                                                                                                                           \
				for (int j = 0; j < prims_per_variant; j++) {                                                                                                                                     \
					packed_array.set(i, m_primitives[prim_offset + j]);                                                                                                                           \
				}                                                                                                                                                                                 \
			} break;                                                                                                                                                                              \
			case Variant::PACKED_FLOAT32_ARRAY: {                                                                                                                                                 \
				PackedFloat32Array packed_array;                                                                                                                                                  \
				packed_array.resize(prims_per_variant);                                                                                                                                           \
				for (int j = 0; j < prims_per_variant; j++) {                                                                                                                                     \
					packed_array.set(i, m_primitives[prim_offset + j]);                                                                                                                           \
				}                                                                                                                                                                                 \
			} break;                                                                                                                                                                              \
			case Variant::PACKED_FLOAT64_ARRAY: {                                                                                                                                                 \
				PackedFloat64Array packed_array;                                                                                                                                                  \
				packed_array.resize(prims_per_variant);                                                                                                                                           \
				for (int j = 0; j < prims_per_variant; j++) {                                                                                                                                     \
					packed_array.set(i, m_primitives[prim_offset + j]);                                                                                                                           \
				}                                                                                                                                                                                 \
			} break;                                                                                                                                                                              \
			default: {                                                                                                                                                                            \
				ERR_FAIL_V_MSG(m_values, "G4MF import: Cannot decode accessor as Variant of type " + Variant::get_type_name(p_variant_type) + ". Returning an empty array.");                     \
			}                                                                                                                                                                                     \
		}                                                                                                                                                                                         \
	}

Array G4MFAccessor4D::_decode_floats_as_variants(const Ref<G4MFState4D> &p_g4mf_state, const Variant::Type p_variant_type) const {
	PackedFloat64Array primitives = decode_floats_from_primitives(p_g4mf_state);
	Array values;
	G4MF_ACCESSOR_4D_DECODE_PRIMITIVES_AS_VARIANTS(primitives, values);
	return values;
}

Array G4MFAccessor4D::_decode_ints_as_variants(const Ref<G4MFState4D> &p_g4mf_state, const Variant::Type p_variant_type) const {
	PackedInt64Array primitives = decode_ints_from_primitives(p_g4mf_state);
	Array values;
	G4MF_ACCESSOR_4D_DECODE_PRIMITIVES_AS_VARIANTS(primitives, values);
	return values;
}

Array G4MFAccessor4D::_decode_uints_as_variants(const Ref<G4MFState4D> &p_g4mf_state, const Variant::Type p_variant_type) const {
	Vector<uint64_t> primitives = decode_uints_from_primitives(p_g4mf_state);
	Array values;
	G4MF_ACCESSOR_4D_DECODE_PRIMITIVES_AS_VARIANTS(primitives, values);
	return values;
}

void G4MFAccessor4D::set_vector_size(const int p_vector_size) {
	ERR_FAIL_COND_MSG(p_vector_size < 1, "G4MFAccessor4D: Vector size must be a positive integer. Refusing to set.");
	_vector_size = p_vector_size;
}

// General helper functions.

bool G4MFAccessor4D::is_equal_exact(const Ref<G4MFAccessor4D> &p_other) const {
	return (_buffer_view_index == p_other->get_buffer_view_index() &&
			_vector_size == p_other->get_vector_size() &&
			_primitive_type == p_other->get_primitive_type());
}

int64_t G4MFAccessor4D::bytes_per_primitive() const {
	// The `to_int` function only looks at numeric digits, so for example, "float32" -> 32 -> 4.
	return _primitive_type.to_int() / 8;
}

int64_t G4MFAccessor4D::bytes_per_vector() const {
	return bytes_per_primitive() * _vector_size;
}

int64_t G4MFAccessor4D::primitives_per_variant(const Variant::Type p_variant_type) {
	switch (p_variant_type) {
		case Variant::NIL:
		case Variant::STRING:
		case Variant::STRING_NAME:
		case Variant::NODE_PATH:
		case Variant::RID:
		case Variant::OBJECT:
		case Variant::CALLABLE:
		case Variant::SIGNAL:
		case Variant::DICTIONARY:
		case Variant::ARRAY:
		case Variant::PACKED_STRING_ARRAY:
		case Variant::PACKED_VECTOR2_ARRAY:
		case Variant::PACKED_VECTOR3_ARRAY:
		case Variant::PACKED_COLOR_ARRAY:
		case Variant::PACKED_VECTOR4_ARRAY:
		case Variant::VARIANT_MAX:
			return 0; // Not supported.
		case Variant::BOOL:
		case Variant::INT:
		case Variant::FLOAT:
			return 1;
		case Variant::VECTOR2:
		case Variant::VECTOR2I:
			return 2;
		case Variant::VECTOR3:
		case Variant::VECTOR3I:
			return 3;
		case Variant::RECT2:
		case Variant::RECT2I:
		case Variant::VECTOR4:
		case Variant::VECTOR4I:
		case Variant::PLANE:
		case Variant::QUATERNION:
		case Variant::COLOR:
			return 4;
		case Variant::TRANSFORM2D:
		case Variant::AABB:
			return 6;
		case Variant::BASIS:
			return 9;
		case Variant::TRANSFORM3D:
			return 12;
		case Variant::PROJECTION:
			return 16;
		case Variant::PACKED_BYTE_ARRAY:
		case Variant::PACKED_INT32_ARRAY:
		case Variant::PACKED_INT64_ARRAY:
		case Variant::PACKED_FLOAT32_ARRAY:
		case Variant::PACKED_FLOAT64_ARRAY:
			return INT64_MAX; // Special, use `_vector_size` only to determine size.
	}
	return 0;
}

// Determine the minimal primitive type for encoding the given data.
// Add more types only as needed for export, otherwise this will be a mess.

String G4MFAccessor4D::minimal_primitive_type_for_int32s(const PackedInt32Array &p_input_data) {
	uint32_t min_int_bits = 8;
	uint32_t min_uint_bits = 8;
	for (const int32_t i : p_input_data) {
		_minimal_primitive_bits_for_int64(i, min_int_bits, min_uint_bits);
	}
	return _minimal_primitive_type_given_bits(CANT_USE_PRIM_TYPE, min_int_bits, min_uint_bits);
}

String G4MFAccessor4D::minimal_primitive_type_for_vector4s(const PackedVector4Array &p_input_data) {
	uint32_t min_float_bits = 8;
	uint32_t min_int_bits = 8;
	uint32_t min_uint_bits = 8;
	for (const Vector4 &vec : p_input_data) {
		_minimal_primitive_bits_for_double(vec.x, min_float_bits, min_int_bits, min_uint_bits);
		_minimal_primitive_bits_for_double(vec.y, min_float_bits, min_int_bits, min_uint_bits);
		_minimal_primitive_bits_for_double(vec.z, min_float_bits, min_int_bits, min_uint_bits);
		_minimal_primitive_bits_for_double(vec.w, min_float_bits, min_int_bits, min_uint_bits);
	}
	return _minimal_primitive_type_given_bits(min_float_bits, min_int_bits, min_uint_bits);
}

// Decode functions.

PackedByteArray G4MFAccessor4D::load_bytes_from_buffer_view(const Ref<G4MFState4D> &p_g4mf_state) const {
	const TypedArray<G4MFBufferView4D> state_buffer_views = p_g4mf_state->get_buffer_views();
	ERR_FAIL_INDEX_V_MSG(_buffer_view_index, state_buffer_views.size(), PackedByteArray(), "G4MF import: The buffer view index is out of bounds. Returning an empty byte array.");
	const Ref<G4MFBufferView4D> buffer_view = state_buffer_views[_buffer_view_index];
	const PackedByteArray raw_bytes = buffer_view->load_buffer_view_data(p_g4mf_state);
	ERR_FAIL_COND_V_MSG(raw_bytes.size() % bytes_per_vector() != 0, PackedByteArray(), "G4MF import: The buffer view size was not a multiple of the vector size. Returning an empty byte array.");
	return raw_bytes;
}

#define G4MF_ACCESSOR_4D_DECODE_NUMBERS_FROM_PRIMITIVES(m_numbers, m_prim_type)                                                                                                         \
	const PackedByteArray raw_bytes = load_bytes_from_buffer_view(p_g4mf_state);                                                                                                        \
	const int64_t raw_byte_size = raw_bytes.size();                                                                                                                                     \
	const int64_t bytes_per_prim = bytes_per_primitive();                                                                                                                               \
	const int64_t prim_size = raw_byte_size / bytes_per_prim;                                                                                                                           \
	m_numbers.resize(prim_size);                                                                                                                                                        \
	if (_primitive_type.begins_with("uint")) {                                                                                                                                          \
		for (int i = 0; i < prim_size; i++) {                                                                                                                                           \
			const int byte_offset = i * bytes_per_prim;                                                                                                                                 \
			switch (bytes_per_prim) {                                                                                                                                                   \
				case 1: {                                                                                                                                                               \
					m_numbers.set(i, m_prim_type(*(const uint8_t *)&raw_bytes[byte_offset]));                                                                                           \
				} break;                                                                                                                                                                \
				case 2: {                                                                                                                                                               \
					m_numbers.set(i, m_prim_type(*(const uint16_t *)&raw_bytes[byte_offset]));                                                                                          \
				} break;                                                                                                                                                                \
				case 4: {                                                                                                                                                               \
					m_numbers.set(i, m_prim_type(*(const uint32_t *)&raw_bytes[byte_offset]));                                                                                          \
				} break;                                                                                                                                                                \
				case 8: {                                                                                                                                                               \
					m_numbers.set(i, m_prim_type(*(const uint64_t *)&raw_bytes[byte_offset]));                                                                                          \
				} break;                                                                                                                                                                \
				default: {                                                                                                                                                              \
					ERR_FAIL_V_MSG(m_numbers, "G4MF import: Godot does not support reading G4MF accessor uint primitives of type '" + _primitive_type + "'. Returning an zero array."); \
				}                                                                                                                                                                       \
			}                                                                                                                                                                           \
		}                                                                                                                                                                               \
	} else if (_primitive_type.begins_with("int")) {                                                                                                                                    \
		for (int i = 0; i < prim_size; i++) {                                                                                                                                           \
			const int byte_offset = i * bytes_per_prim;                                                                                                                                 \
			switch (bytes_per_prim) {                                                                                                                                                   \
				case 1: {                                                                                                                                                               \
					m_numbers.set(i, m_prim_type(*(const int8_t *)&raw_bytes[byte_offset]));                                                                                            \
				} break;                                                                                                                                                                \
				case 2: {                                                                                                                                                               \
					m_numbers.set(i, m_prim_type(*(const int16_t *)&raw_bytes[byte_offset]));                                                                                           \
				} break;                                                                                                                                                                \
				case 4: {                                                                                                                                                               \
					m_numbers.set(i, m_prim_type(*(const int32_t *)&raw_bytes[byte_offset]));                                                                                           \
				} break;                                                                                                                                                                \
				case 8: {                                                                                                                                                               \
					m_numbers.set(i, m_prim_type(*(const int64_t *)&raw_bytes[byte_offset]));                                                                                           \
				} break;                                                                                                                                                                \
				default: {                                                                                                                                                              \
					ERR_FAIL_V_MSG(m_numbers, "G4MF import: Godot does not support reading G4MF accessor int primitives of type '" + _primitive_type + "'. Returning an zero array.");  \
				}                                                                                                                                                                       \
			}                                                                                                                                                                           \
		}                                                                                                                                                                               \
	} else if (_primitive_type.begins_with("float")) {                                                                                                                                  \
		for (int i = 0; i < prim_size; i++) {                                                                                                                                           \
			const int byte_offset = i * bytes_per_prim;                                                                                                                                 \
			switch (bytes_per_prim) {                                                                                                                                                   \
				case 1: {                                                                                                                                                               \
					const uint8_t quarter_float_bits = *(const uint8_t *)&raw_bytes[byte_offset];                                                                                       \
					m_numbers.set(i, m_prim_type(_float8_to_double(quarter_float_bits)));                                                                                               \
				} break;                                                                                                                                                                \
				case 2: {                                                                                                                                                               \
					const uint16_t half_float_bits = *(const uint16_t *)&raw_bytes[byte_offset];                                                                                        \
					m_numbers.set(i, m_prim_type(_float16_to_double(half_float_bits)));                                                                                                 \
				} break;                                                                                                                                                                \
				case 4: {                                                                                                                                                               \
					m_numbers.set(i, m_prim_type(*(const float *)&raw_bytes[byte_offset]));                                                                                             \
				} break;                                                                                                                                                                \
				case 8: {                                                                                                                                                               \
					m_numbers.set(i, m_prim_type(*(const double *)&raw_bytes[byte_offset]));                                                                                            \
				} break;                                                                                                                                                                \
				default: {                                                                                                                                                              \
					ERR_FAIL_V_MSG(m_numbers, "G4MF import: Godot does not support reading G4MF accessor primitives of type '" + _primitive_type + "'. Returning an zero array.");      \
				}                                                                                                                                                                       \
			}                                                                                                                                                                           \
		}                                                                                                                                                                               \
	} else {                                                                                                                                                                            \
		ERR_FAIL_V_MSG(m_numbers, "G4MF import: Godot does not support reading G4MF accessor primitives of type '" + _primitive_type + "'. Returning an zero array.");                  \
	}

PackedFloat64Array G4MFAccessor4D::decode_floats_from_primitives(const Ref<G4MFState4D> &p_g4mf_state) const {
	PackedFloat64Array numbers;
	G4MF_ACCESSOR_4D_DECODE_NUMBERS_FROM_PRIMITIVES(numbers, double);
	return numbers;
}

// The 64-bit version alone works but since int32 is very common
// and needed internally let's provide a dedicated function for it.
PackedInt32Array G4MFAccessor4D::decode_int32s_from_primitives(const Ref<G4MFState4D> &p_g4mf_state) const {
	PackedInt32Array numbers;
	G4MF_ACCESSOR_4D_DECODE_NUMBERS_FROM_PRIMITIVES(numbers, int32_t);
	return numbers;
}

PackedInt64Array G4MFAccessor4D::decode_ints_from_primitives(const Ref<G4MFState4D> &p_g4mf_state) const {
	PackedInt64Array numbers;
	G4MF_ACCESSOR_4D_DECODE_NUMBERS_FROM_PRIMITIVES(numbers, int64_t);
	return numbers;
}

Vector<uint64_t> G4MFAccessor4D::decode_uints_from_primitives(const Ref<G4MFState4D> &p_g4mf_state) const {
	Vector<uint64_t> numbers;
	G4MF_ACCESSOR_4D_DECODE_NUMBERS_FROM_PRIMITIVES(numbers, uint64_t);
	return numbers;
}

Array G4MFAccessor4D::decode_accessor_as_variants(const Ref<G4MFState4D> &p_g4mf_state, const Variant::Type p_variant_type) const {
	if (_primitive_type.begins_with("float")) {
		return _decode_floats_as_variants(p_g4mf_state, p_variant_type);
	} else if (_primitive_type.begins_with("int")) {
		return _decode_ints_as_variants(p_g4mf_state, p_variant_type);
	} else if (_primitive_type.begins_with("uint")) {
		return _decode_uints_as_variants(p_g4mf_state, p_variant_type);
	} else {
		ERR_PRINT("G4MFAccessor4D: Unknown primitive type '" + _primitive_type + "', cannot decode.");
	}
	return Array();
}

// Encode functions.

PackedByteArray G4MFAccessor4D::encode_floats_as_primitives(const PackedFloat64Array &p_input_data) const {
	PackedByteArray ret;
	ERR_FAIL_COND_V_MSG(!_primitive_type.begins_with("float"), ret, "G4MF export: Cannot encode floats as primitives of type '" + _primitive_type + "'.");
	const int64_t input_size = p_input_data.size();
	const int64_t bytes_per_prim = bytes_per_primitive();
	const int64_t raw_byte_size = bytes_per_prim * input_size;
	ret.resize(raw_byte_size);
	uint8_t *ret_write = ret.ptrw();
	for (int i = 0; i < input_size; i++) {
		const int byte_offset = i * bytes_per_prim;
		switch (bytes_per_prim) {
			case 1: {
				const uint8_t quarter_float_bits = _double_to_float8(p_input_data[i]);
				*(uint8_t *)&ret_write[byte_offset] = quarter_float_bits;
			} break;
			case 2: {
				const uint16_t half_float_bits = _double_to_float16(p_input_data[i]);
				*(uint16_t *)&ret_write[byte_offset] = half_float_bits;
			} break;
			case 4: {
				*(float *)&ret_write[byte_offset] = p_input_data[i];
			} break;
			case 8: {
				*(double *)&ret_write[byte_offset] = p_input_data[i];
			} break;
			default: {
				ERR_FAIL_V_MSG(ret, "G4MF export: Godot does not support writing G4MF accessor primitives of type '" + _primitive_type + "'.");
			}
		}
	}
	return ret;
}

PackedByteArray G4MFAccessor4D::encode_ints_as_primitives(const PackedInt64Array &p_input_data) const {
	PackedByteArray ret;
	ERR_FAIL_COND_V_MSG(!_primitive_type.begins_with("int"), ret, "G4MF export: Cannot encode ints as primitives of type '" + _primitive_type + "'.");
	const int64_t input_size = p_input_data.size();
	const int64_t bytes_per_prim = bytes_per_primitive();
	const int64_t raw_byte_size = bytes_per_prim * input_size;
	ret.resize(raw_byte_size);
	uint8_t *ret_write = ret.ptrw();
	for (int i = 0; i < input_size; i++) {
		const int byte_offset = i * bytes_per_prim;
		switch (bytes_per_prim) {
			case 1: {
				*(int8_t *)&ret_write[byte_offset] = p_input_data[i];
			} break;
			case 2: {
				*(int16_t *)&ret_write[byte_offset] = p_input_data[i];
			} break;
			case 4: {
				*(int32_t *)&ret_write[byte_offset] = p_input_data[i];
			} break;
			case 8: {
				*(int64_t *)&ret_write[byte_offset] = p_input_data[i];
			} break;
			default: {
				ERR_FAIL_V_MSG(ret, "G4MF export: Godot does not support writing G4MF accessor primitives of type '" + _primitive_type + "'.");
			}
		}
	}
	return ret;
}

PackedByteArray G4MFAccessor4D::encode_uints_as_primitives(const Vector<uint64_t> &p_input_data) const {
	PackedByteArray ret;
	ERR_FAIL_COND_V_MSG(!_primitive_type.begins_with("uint"), ret, "G4MF export: Cannot encode uints as primitives of type '" + _primitive_type + "'.");
	const int64_t input_size = p_input_data.size();
	const int64_t bytes_per_prim = bytes_per_primitive();
	const int64_t raw_byte_size = bytes_per_prim * input_size;
	ret.resize(raw_byte_size);
	uint8_t *ret_write = ret.ptrw();
	for (int i = 0; i < input_size; i++) {
		const int byte_offset = i * bytes_per_prim;
		switch (bytes_per_prim) {
			case 1: {
				*(uint8_t *)&ret_write[byte_offset] = p_input_data[i];
			} break;
			case 2: {
				*(uint16_t *)&ret_write[byte_offset] = p_input_data[i];
			} break;
			case 4: {
				*(uint32_t *)&ret_write[byte_offset] = p_input_data[i];
			} break;
			case 8: {
				*(uint64_t *)&ret_write[byte_offset] = p_input_data[i];
			} break;
			default: {
				ERR_FAIL_V_MSG(ret, "G4MF export: Godot does not support writing G4MF accessor primitives of type '" + _primitive_type + "'.");
			}
		}
	}
	return ret;
}

#define G4MF_ACCESSOR_4D_ENCODE_VARIANTS_AS_PRIMITIVES(m_input_data, m_numbers, m_error_type)                                                                                                     \
	const Variant &first_element = m_input_data[0];                                                                                                                                               \
	const Variant::Type first_type = first_element.get_type();                                                                                                                                    \
	const int64_t prim_per_variant = primitives_per_variant(first_type);                                                                                                                          \
	/* For the most part, use the vector size, so for example, encoding Vector2s as Vector3s leaves 0 at the end. */                                                                              \
	/* However, when the inputs are scalars, it's usually expected to just write those all together. */                                                                                           \
	/* Ex: If the G4MF is supposed to contain an integer Vector2 array, but Godot doesn't have PackedVector2iArray, */                                                                            \
	/* we would use PackedInt32Array in Godot as a substitute, so the encoding needs to handle this. */                                                                                           \
	const int vector_size = prim_per_variant == 1 ? 1 : _vector_size;                                                                                                                             \
	const int input_size = m_input_data.size();                                                                                                                                                   \
	m_numbers.resize(input_size *vector_size);                                                                                                                                                    \
	for (int input_index = 0; input_index < input_size; input_index++) {                                                                                                                          \
		Variant variant = m_input_data[input_index];                                                                                                                                              \
		const int vector_offset = input_index * vector_size;                                                                                                                                      \
		const Variant::Type variant_type = variant.get_type();                                                                                                                                    \
		ERR_FAIL_COND_V_MSG(variant_type != first_type, m_error_type(), "G4MF export: Cannot encode an array of mixed types. All elements must be of the same homogeneous type.");                \
		switch (variant_type) {                                                                                                                                                                   \
			case Variant::NIL:                                                                                                                                                                    \
			case Variant::BOOL:                                                                                                                                                                   \
			case Variant::INT:                                                                                                                                                                    \
			case Variant::FLOAT: {                                                                                                                                                                \
				/* For scalar values, just set them. Variant can convert all of these to each other */                                                                                            \
				m_numbers.set(vector_offset, variant);                                                                                                                                            \
			} break;                                                                                                                                                                              \
			case Variant::PLANE:                                                                                                                                                                  \
			case Variant::QUATERNION:                                                                                                                                                             \
			case Variant::RECT2:                                                                                                                                                                  \
			case Variant::RECT2I: {                                                                                                                                                               \
				/* Evil hack that relies on the structure of Variant, but it's the */                                                                                                             \
				/* only way to accomplish this without a ton of code duplication. */                                                                                                              \
				if (variant_type == Variant::RECT2I) {                                                                                                                                            \
					*(Variant::Type *)&variant = Variant::VECTOR4I;                                                                                                                               \
				} else {                                                                                                                                                                          \
					*(Variant::Type *)&variant = Variant::VECTOR4;                                                                                                                                \
				}                                                                                                                                                                                 \
			}                                                                                                                                                                                     \
				[[fallthrough]];                                                                                                                                                                  \
			case Variant::VECTOR2:                                                                                                                                                                \
			case Variant::VECTOR2I:                                                                                                                                                               \
			case Variant::VECTOR3:                                                                                                                                                                \
			case Variant::VECTOR3I:                                                                                                                                                               \
			case Variant::VECTOR4:                                                                                                                                                                \
			case Variant::VECTOR4I: {                                                                                                                                                             \
				/* Variant can handle converting Vector2/2i/3/3i/4/4i to Vector4 for us. */                                                                                                       \
				Vector4 vec = variant;                                                                                                                                                            \
				for (int i = 0; i < vector_size; i++) {                                                                                                                                           \
					if (i < prim_per_variant) {                                                                                                                                                   \
						m_numbers.set(vector_offset + i, vec[i]);                                                                                                                                 \
					} else {                                                                                                                                                                      \
						m_numbers.set(vector_offset + i, 0);                                                                                                                                      \
					}                                                                                                                                                                             \
				}                                                                                                                                                                                 \
			} break;                                                                                                                                                                              \
			case Variant::COLOR: {                                                                                                                                                                \
				Color c = variant;                                                                                                                                                                \
				for (int i = 0; i < vector_size; i++) {                                                                                                                                           \
					if (i < prim_per_variant) {                                                                                                                                                   \
						m_numbers.set(vector_offset + i, c[i]);                                                                                                                                   \
					} else {                                                                                                                                                                      \
						m_numbers.set(vector_offset + i, 0);                                                                                                                                      \
					}                                                                                                                                                                             \
				}                                                                                                                                                                                 \
			} break;                                                                                                                                                                              \
			case Variant::TRANSFORM2D:                                                                                                                                                            \
			case Variant::BASIS:                                                                                                                                                                  \
			case Variant::TRANSFORM3D:                                                                                                                                                            \
			case Variant::PROJECTION: {                                                                                                                                                           \
				/* Variant can handle converting Transform2D/Transform3D/Basis to Projection for us. */                                                                                           \
				Projection p = variant;                                                                                                                                                           \
				real_t *proj_numbers = (real_t *)&p;                                                                                                                                              \
				for (int i = 0; i < vector_size; i++) {                                                                                                                                           \
					if (i < 16) {                                                                                                                                                                 \
						m_numbers.set(vector_offset + i, proj_numbers[i]);                                                                                                                        \
					} else {                                                                                                                                                                      \
						m_numbers.set(vector_offset + i, 0);                                                                                                                                      \
					}                                                                                                                                                                             \
				}                                                                                                                                                                                 \
			} break;                                                                                                                                                                              \
			case Variant::PACKED_BYTE_ARRAY: {                                                                                                                                                    \
				PackedByteArray packed_array = variant;                                                                                                                                           \
				for (int i = 0; i < vector_size; i++) {                                                                                                                                           \
					if (i < packed_array.size()) {                                                                                                                                                \
						m_numbers.set(vector_offset + i, packed_array[i]);                                                                                                                        \
					} else {                                                                                                                                                                      \
						m_numbers.set(vector_offset + i, 0);                                                                                                                                      \
					}                                                                                                                                                                             \
				}                                                                                                                                                                                 \
			} break;                                                                                                                                                                              \
			case Variant::PACKED_INT32_ARRAY: {                                                                                                                                                   \
				PackedInt32Array packed_array = variant;                                                                                                                                          \
				for (int i = 0; i < vector_size; i++) {                                                                                                                                           \
					if (i < packed_array.size()) {                                                                                                                                                \
						m_numbers.set(vector_offset + i, packed_array[i]);                                                                                                                        \
					} else {                                                                                                                                                                      \
						m_numbers.set(vector_offset + i, 0);                                                                                                                                      \
					}                                                                                                                                                                             \
				}                                                                                                                                                                                 \
			} break;                                                                                                                                                                              \
			case Variant::PACKED_INT64_ARRAY: {                                                                                                                                                   \
				PackedInt64Array packed_array = variant;                                                                                                                                          \
				for (int i = 0; i < vector_size; i++) {                                                                                                                                           \
					if (i < packed_array.size()) {                                                                                                                                                \
						m_numbers.set(vector_offset + i, packed_array[i]);                                                                                                                        \
					} else {                                                                                                                                                                      \
						m_numbers.set(vector_offset + i, 0);                                                                                                                                      \
					}                                                                                                                                                                             \
				}                                                                                                                                                                                 \
			} break;                                                                                                                                                                              \
			case Variant::PACKED_FLOAT32_ARRAY: {                                                                                                                                                 \
				PackedFloat32Array packed_array = variant;                                                                                                                                        \
				for (int i = 0; i < vector_size; i++) {                                                                                                                                           \
					if (i < packed_array.size()) {                                                                                                                                                \
						m_numbers.set(vector_offset + i, packed_array[i]);                                                                                                                        \
					} else {                                                                                                                                                                      \
						m_numbers.set(vector_offset + i, 0);                                                                                                                                      \
					}                                                                                                                                                                             \
				}                                                                                                                                                                                 \
			} break;                                                                                                                                                                              \
			case Variant::PACKED_FLOAT64_ARRAY: {                                                                                                                                                 \
				PackedFloat64Array packed_array = variant;                                                                                                                                        \
				for (int i = 0; i < vector_size; i++) {                                                                                                                                           \
					if (i < packed_array.size()) {                                                                                                                                                \
						m_numbers.set(vector_offset + i, packed_array[i]);                                                                                                                        \
					} else {                                                                                                                                                                      \
						m_numbers.set(vector_offset + i, 0);                                                                                                                                      \
					}                                                                                                                                                                             \
				}                                                                                                                                                                                 \
			} break;                                                                                                                                                                              \
			default: {                                                                                                                                                                            \
				ERR_FAIL_V_MSG(m_error_type(), "G4MF export: Cannot encode an array of type '" + Variant::get_type_name(variant_type) + "' as a G4MF accessor. Returning an empty float array."); \
			}                                                                                                                                                                                     \
		}                                                                                                                                                                                         \
	}

PackedFloat64Array G4MFAccessor4D::_encode_variants_as_floats(const Array &p_input_data) const {
	PackedFloat64Array numbers;
	G4MF_ACCESSOR_4D_ENCODE_VARIANTS_AS_PRIMITIVES(p_input_data, numbers, PackedFloat64Array);
	return numbers;
}

PackedInt64Array G4MFAccessor4D::_encode_variants_as_ints(const Array &p_input_data) const {
	PackedInt64Array numbers;
	G4MF_ACCESSOR_4D_ENCODE_VARIANTS_AS_PRIMITIVES(p_input_data, numbers, PackedInt64Array);
	return numbers;
}

Vector<uint64_t> G4MFAccessor4D::_encode_variants_as_uints(const Array &p_input_data) const {
	Vector<uint64_t> numbers;
	G4MF_ACCESSOR_4D_ENCODE_VARIANTS_AS_PRIMITIVES(p_input_data, numbers, Vector<uint64_t>);
	return numbers;
}

PackedByteArray G4MFAccessor4D::encode_accessor_from_variants(const Array &p_input_data) const {
	ERR_FAIL_COND_V_MSG(p_input_data.is_empty(), PackedByteArray(), "G4MF export: Cannot encode an empty array.");
	const int64_t bytes_per_vec = bytes_per_vector();
	ERR_FAIL_COND_V_MSG(bytes_per_vec == 0, PackedByteArray(), "G4MF export: Cannot encode an accessor of type '" + _primitive_type + "'.");
	if (_primitive_type.begins_with("float")) {
		PackedFloat64Array numbers = _encode_variants_as_floats(p_input_data);
		return encode_floats_as_primitives(numbers);
	} else if (_primitive_type.begins_with("int")) {
		PackedInt64Array numbers = _encode_variants_as_ints(p_input_data);
		return encode_ints_as_primitives(numbers);
	} else if (_primitive_type.begins_with("uint")) {
		Vector<uint64_t> numbers = _encode_variants_as_uints(p_input_data);
		return encode_uints_as_primitives(numbers);
	}
	ERR_FAIL_V_MSG(PackedByteArray(), "G4MF export: Cannot encode an accessor of type '" + _primitive_type + "' as a G4MF accessor.");
}

int G4MFAccessor4D::encode_new_accessor_into_state(const Ref<G4MFState4D> &p_g4mf_state, const Array &p_input_data, const String &p_primitive_type, const int p_vector_size, const bool p_deduplicate) {
	Ref<G4MFAccessor4D> accessor;
	accessor.instantiate();
	accessor->set_primitive_type(p_primitive_type);
	accessor->set_vector_size(p_vector_size);
	// Write the data into a new buffer view.
	PackedByteArray encoded_bytes = accessor->encode_accessor_from_variants(p_input_data);
	ERR_FAIL_COND_V_MSG(encoded_bytes.is_empty(), -1, "G4MF export: Accessor failed to encode data as bytes (was the input data empty?).");
	const int buffer_view_index = G4MFBufferView4D::write_new_buffer_view_into_state(p_g4mf_state, encoded_bytes, p_deduplicate);
	ERR_FAIL_COND_V_MSG(buffer_view_index == -1, -1, "G4MF export: Accessor failed to write new buffer view into G4MF state.");
	accessor->set_buffer_view_index(buffer_view_index);
	// Add the new accessor to the state, but check for duplicates first.
	TypedArray<G4MFAccessor4D> state_accessors = p_g4mf_state->get_accessors();
	const int accessor_count = state_accessors.size();
	for (int i = 0; i < accessor_count; i++) {
		Ref<G4MFAccessor4D> existing_accessor = state_accessors[i];
		if (accessor->is_equal_exact(existing_accessor)) {
			// An identical accessor already exists in the state, so just return the index.
			return i;
		}
	}
	state_accessors.append(accessor);
	p_g4mf_state->set_accessors(state_accessors);
	return accessor_count;
}

Ref<G4MFAccessor4D> G4MFAccessor4D::from_dictionary(const Dictionary &p_dict) {
	Ref<G4MFAccessor4D> accessor;
	accessor.instantiate();
	accessor->read_item_entries_from_dictionary(p_dict);
	if (p_dict.has("bufferView")) {
		accessor->set_buffer_view_index(p_dict["bufferView"]);
	}
	if (p_dict.has("primitiveType")) {
		accessor->set_primitive_type(p_dict["primitiveType"]);
	}
	if (p_dict.has("vectorSize")) {
		accessor->set_vector_size(p_dict["vectorSize"]);
	}
	return accessor;
}

Dictionary G4MFAccessor4D::to_dictionary() const {
	Dictionary dict = write_item_entries_to_dictionary();
	if (_buffer_view_index != -1) {
		dict["bufferView"] = _buffer_view_index;
	}
	if (!_primitive_type.is_empty()) {
		dict["primitiveType"] = _primitive_type;
	}
	if (_vector_size > 1) {
		dict["vectorSize"] = _vector_size;
	}
	return dict;
}

void G4MFAccessor4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_buffer_view_index"), &G4MFAccessor4D::get_buffer_view_index);
	ClassDB::bind_method(D_METHOD("set_buffer_view_index", "buffer_view_index"), &G4MFAccessor4D::set_buffer_view_index);
	ClassDB::bind_method(D_METHOD("get_primitive_type"), &G4MFAccessor4D::get_primitive_type);
	ClassDB::bind_method(D_METHOD("set_primitive_type", "primitive_type"), &G4MFAccessor4D::set_primitive_type);
	ClassDB::bind_method(D_METHOD("get_vector_size"), &G4MFAccessor4D::get_vector_size);
	ClassDB::bind_method(D_METHOD("set_vector_size", "vector_size"), &G4MFAccessor4D::set_vector_size);

	// General helper functions.
	ClassDB::bind_method(D_METHOD("bytes_per_primitive"), &G4MFAccessor4D::bytes_per_primitive);
	ClassDB::bind_method(D_METHOD("bytes_per_vector"), &G4MFAccessor4D::bytes_per_vector);
	ClassDB::bind_static_method("G4MFAccessor4D", D_METHOD("primitives_per_variant", "variant_type"), &G4MFAccessor4D::primitives_per_variant);

	// Determine the minimal primitive type for the given data.
	ClassDB::bind_static_method("G4MFAccessor4D", D_METHOD("minimal_primitive_type_for_int32s", "input_data"), &G4MFAccessor4D::minimal_primitive_type_for_int32s);
	ClassDB::bind_static_method("G4MFAccessor4D", D_METHOD("minimal_primitive_type_for_vector4s", "input_data"), &G4MFAccessor4D::minimal_primitive_type_for_vector4s);

	// Decode functions.
	ClassDB::bind_method(D_METHOD("decode_accessor_as_variants", "g4mf_state", "variant_type"), &G4MFAccessor4D::decode_accessor_as_variants);
	ClassDB::bind_method(D_METHOD("decode_floats_from_primitives", "g4mf_state"), &G4MFAccessor4D::decode_floats_from_primitives);
	ClassDB::bind_method(D_METHOD("decode_ints_from_primitives", "g4mf_state"), &G4MFAccessor4D::decode_ints_from_primitives);
	ClassDB::bind_method(D_METHOD("load_bytes_from_buffer_view", "g4mf_state"), &G4MFAccessor4D::load_bytes_from_buffer_view);

	// Encode functions.
	ClassDB::bind_method(D_METHOD("encode_accessor_from_variants", "input_data"), &G4MFAccessor4D::encode_accessor_from_variants);
	ClassDB::bind_method(D_METHOD("encode_floats_as_primitives", "input_data"), &G4MFAccessor4D::encode_floats_as_primitives);
	ClassDB::bind_method(D_METHOD("encode_ints_as_primitives", "input_data"), &G4MFAccessor4D::encode_ints_as_primitives);
	ClassDB::bind_static_method("G4MFAccessor4D", D_METHOD("encode_new_accessor_into_state", "g4mf_state", "input_data", "primitive_type", "vector_size", "deduplicate"), &G4MFAccessor4D::encode_new_accessor_into_state, DEFVAL(true));

	ClassDB::bind_static_method("G4MFAccessor4D", D_METHOD("from_dictionary", "dict"), &G4MFAccessor4D::from_dictionary);
	ClassDB::bind_method(D_METHOD("to_dictionary"), &G4MFAccessor4D::to_dictionary);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "buffer_view_index"), "set_buffer_view_index", "get_buffer_view_index");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "primitive_type"), "set_primitive_type", "get_primitive_type");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "vector_size"), "set_vector_size", "get_vector_size");
}
