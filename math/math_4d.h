#pragma once

#include "../godot_4d_defines.h"

// There is no real uint4_t type, but this semantically means we only use the 4 least significant bits.
using uint4_t = uint8_t;

// Math functions for the 4D module that don't fit in any other file.
// Prefer using Geometry4D, Vector4D, etc if those are appropriate.
class Math4D : public Object {
	GDCLASS(Math4D, Object);

protected:
	static Math4D *singleton;
	static void _bind_methods();

public:
	static double float4_to_double(const uint4_t p_float4);
	static double float8_to_double(const uint8_t p_float8);
	static double float16_to_double(const uint16_t p_float16);
	static uint4_t double_to_float4(const double p_double);
	static uint8_t double_to_float8(const double p_double);
	static uint16_t double_to_float16(const double p_double);

	static int32_t find_common_int32(const PackedInt32Array &p_a, const PackedInt32Array &p_b, int64_t &r_a_index, int64_t &r_b_index);

	static Math4D *get_singleton() { return singleton; }
	Math4D() { singleton = this; }
	~Math4D() { singleton = nullptr; }
};
