#pragma once

#include "../godot_4d_defines.h"

#if GDEXTENSION
#include <godot_cpp/templates/hash_set.hpp>
#elif GODOT_MODULE
#include "core/templates/hash_set.h"
#endif

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

	static Variant quantize_to_float8(const Variant &p_variant);
	static Variant quantize_to_float16(const Variant &p_variant);

	static int32_t find_common_int32(const PackedInt32Array &p_a, const PackedInt32Array &p_b, int64_t &r_a_index, int64_t &r_b_index);
	static bool has_common_int32(const PackedInt32Array &p_a, const PackedInt32Array &p_b);
	static bool ensure_first_two_indices_share_common_int32(PackedInt32Array &r_indices, const Vector<PackedInt32Array> &p_indexed_data);

	// Index remapping. These treat int32 arrays as buckets of indices and never look at what the indices point to.

	// Remaps every index through an old-to-new table, where a negative entry marks a removed element.
	// An index outside `p_old_to_new` becomes -1, so the positions of the other indices are kept.
	static PackedInt32Array remap_int32_array(const PackedInt32Array &p_indices, const PackedInt32Array &p_old_to_new);
	// Remaps every array in place, as above. With `p_remove_duplicates`, indices that map to the same new index
	// are kept once, in first-seen order, which is what member lists want when their members were merged.
	static void remap_int32_arrays(Vector<PackedInt32Array> &r_arrays, const PackedInt32Array &p_old_to_new, const bool p_remove_duplicates);
	// Remaps a set of indices, dropping removed elements.
	static HashSet<int32_t> remap_int32_set(const HashSet<int32_t> &p_indices, const PackedInt32Array &p_old_to_new);

	static Math4D *get_singleton() { return singleton; }
	Math4D() { singleton = this; }
	~Math4D() { singleton = nullptr; }
};
