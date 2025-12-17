#include "math_4d.h"

// Math functions for the 4D module that don't fit in any other file.
// Prefer using Geometry4D, Vector4D, etc if those are appropriate.

double Math4D::float4_to_double(const uint4_t p_float4) {
	const uint4_t f4_sign = (p_float4 >> 3) & 0x1;
	const uint4_t f4_main = p_float4 & 0x7;
	// Since float4 is so small, it's faster to just use a switch statement than fiddle with bits.
	double value;
	switch (f4_main) {
		case uint4_t(0b000): {
			value = 0.0;
		} break;
		case uint4_t(0b001): {
			value = 0.5;
		} break;
		case uint4_t(0b010): {
			value = 1.0;
		} break;
		case uint4_t(0b011): {
			value = 1.5;
		} break;
		case uint4_t(0b100): {
			value = 2.0;
		} break;
		case uint4_t(0b101): {
			value = 3.0;
		} break;
		case uint4_t(0b110): {
			value = Math_INF;
		} break;
		case uint4_t(0b111): {
			value = Math_NAN;
		} break;
		default: {
			value = Math_NAN; // Impossible, but MSVC complains without this.
		} break;
	}
	// This works correctly for all values including negative zero and negative NaN.
	return (f4_sign == 0) ? value : -value;
}

double Math4D::float8_to_double(const uint8_t p_float8) {
	const uint8_t f8_exponent = (p_float8 >> 3) & 0xF;
	const uint8_t f8_mantissa = p_float8 & 0x7;
	uint64_t f64_sign = uint64_t(p_float8 & 0x80) << 56;
	uint64_t f64_exponent = 0;
	uint64_t f64_mantissa = 0;
	if (f8_exponent == 0) {
		if (f8_mantissa != 0) {
			// Subnormal: real exponent is -6 so multiply 3-bit mantissa by 2^-9.
			const double value = f8_mantissa * 0.001953125;
			return (f64_sign == 0) ? value : -value;
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

double Math4D::float16_to_double(const uint16_t p_float16) {
	const uint16_t f16_exponent = (p_float16 >> 10) & 0x1F;
	const uint16_t f16_mantissa = p_float16 & 0x3FF;
	uint64_t f64_sign = uint64_t(p_float16 & 0x8000) << 48;
	uint64_t f64_exponent = 0;
	uint64_t f64_mantissa = 0;
	if (f16_exponent == 0) {
		if (f16_mantissa != 0) {
			// Subnormal: real exponent is -14 so multiply 10-bit mantissa by 2^-24.
			const double value = f16_mantissa * 0.000000059604644775390625;
			return (f64_sign == 0) ? value : -value;
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

uint4_t Math4D::double_to_float4(const double p_double) {
	union {
		double d;
		uint64_t bits;
	} u;
	u.d = p_double;
	const uint64_t f64_exponent = (u.bits >> 52) & 0x7FF;
	const uint64_t f64_mantissa = u.bits & ((uint64_t(1) << 52) - 1);
	const uint4_t f4_sign = (u.bits >> 60) & 0x8;
	if (f64_exponent == 0x7FF) {
		// Infinity or NaN.
		const uint4_t mantissa = f64_mantissa ? 0x1 : 0x0;
		return f4_sign | uint4_t(0x6) | mantissa;
	}
	// Handle finite values. First get the absolute value.
	u.bits &= 0x7FFFFFFFFFFFFFFF;
	// Since float4 is so small, it's faster to just use if statements than fiddle with bits.
	if (u.d < 1.75) {
		if (u.d < 0.75) {
			// Subnormal numbers.
			if (u.d <= 0.25) {
				return f4_sign | uint4_t(0b000); // 0.0
			} else {
				return f4_sign | uint4_t(0b001); // 0.5
			}
		} else {
			if (u.d <= 1.25) {
				return f4_sign | uint4_t(0b010); // 1.0
			} else {
				return f4_sign | uint4_t(0b011); // 1.5
			}
		}
	} else {
		if (u.d < 3.5) {
			if (u.d <= 2.5) {
				return f4_sign | uint4_t(0b100); // 2.0
			} else {
				return f4_sign | uint4_t(0b101); // 3.0
			}
		}
	}
	return f4_sign | uint4_t(0b110); // Infinity
}

uint8_t Math4D::double_to_float8(const double p_double) {
	union {
		double d;
		uint64_t bits;
	} u;
	u.d = p_double;
	const uint64_t f64_exponent = (u.bits >> 52) & 0x7FF;
	const uint64_t f64_mantissa = u.bits & ((uint64_t(1) << 52) - 1);
	const uint8_t f8_sign = (u.bits >> 56) & 0x80;
	if (f64_exponent == 0x7FF) {
		// Infinity or NaN.
		uint8_t f8_mantissa = f64_mantissa >> 49; // Get the top 3 bits of the mantissa.
		// Special case: NaN values must be preserved as NaN.
		if (unlikely(f8_mantissa == 0 && f64_mantissa != 0)) {
			f8_mantissa = 0x1;
		}
		return f8_sign | uint8_t(0xF << 3) | f8_mantissa;
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

uint16_t Math4D::double_to_float16(const double p_double) {
	union {
		double d;
		uint64_t bits;
	} u;
	u.d = p_double;
	const uint64_t f64_exponent = (u.bits >> 52) & 0x7FF;
	const uint64_t f64_mantissa = u.bits & ((uint64_t(1) << 52) - 1);
	const uint16_t f16_sign = (u.bits >> 48) & 0x8000;
	if (f64_exponent == 0x7FF) {
		// Infinity or NaN.
		uint16_t f16_mantissa = f64_mantissa >> 42; // Get the top 10 bits of the mantissa.
		// Special case: NaN values must be preserved as NaN.
		if (unlikely(f16_mantissa == 0 && f64_mantissa != 0)) {
			f16_mantissa = 0x1;
		}
		return f16_sign | uint16_t(0x1F << 10) | f16_mantissa;
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

int32_t Math4D::find_common_int32(const PackedInt32Array &p_a, const PackedInt32Array &p_b, int64_t &r_a_index, int64_t &r_b_index) {
	for (int64_t a_index = 0; a_index < p_a.size(); a_index++) {
		const int32_t a_item = p_a[a_index];
		for (int64_t b_index = 0; b_index < p_b.size(); b_index++) {
			const int32_t b_item = p_b[b_index];
			if (a_item == b_item) {
				r_a_index = a_index;
				r_b_index = b_index;
				return a_item;
			}
		}
	}
	return INT32_MIN;
}

Math4D *Math4D::singleton = nullptr;

void Math4D::_bind_methods() {
	ClassDB::bind_static_method("Math4D", D_METHOD("float4_to_double", "float4"), &Math4D::float4_to_double);
	ClassDB::bind_static_method("Math4D", D_METHOD("float8_to_double", "float8"), &Math4D::float8_to_double);
	ClassDB::bind_static_method("Math4D", D_METHOD("float16_to_double", "float16"), &Math4D::float16_to_double);
	ClassDB::bind_static_method("Math4D", D_METHOD("double_to_float4", "double"), &Math4D::double_to_float4);
	ClassDB::bind_static_method("Math4D", D_METHOD("double_to_float8", "double"), &Math4D::double_to_float8);
	ClassDB::bind_static_method("Math4D", D_METHOD("double_to_float16", "double"), &Math4D::double_to_float16);
}
