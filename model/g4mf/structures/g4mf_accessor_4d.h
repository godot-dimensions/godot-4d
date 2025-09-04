#pragma once

#include "g4mf_item_4d.h"

class G4MFState4D;

class G4MFAccessor4D : public G4MFItem4D {
	GDCLASS(G4MFAccessor4D, G4MFItem4D);

	String _primitive_type = ""; // No default, must be explicitly set.
	int _buffer_view_index = -1;
	int _vector_size = 1;

	// Private functions for determining the minimal primitive type.
	static constexpr uint32_t CANT_USE_PRIM_TYPE = 1000000; // Any very big number will do.
	static bool _double_bits_equal(const double p_a, const double p_b);
	static void _minimal_primitive_bits_for_double_only(const double p_value, uint32_t &r_float_bits);
	static void _minimal_primitive_bits_for_double(const double p_value, uint32_t &r_float_bits, uint32_t &r_int_bits, uint32_t &r_uint_bits);
	static void _minimal_primitive_bits_for_int64(const int64_t p_value, uint32_t &r_int_bits, uint32_t &r_uint_bits);
	static String _minimal_primitive_type_given_bits(const uint32_t p_float_bits, const uint32_t p_int_bits, const uint32_t p_uint_bits);

	// Private decode functions. Use `decode_accessor_as_variants` publicly.
	Array _decode_floats_as_variants(const Ref<G4MFState4D> &p_g4mf_state, const Variant::Type p_variant_type) const;
	Array _decode_ints_as_variants(const Ref<G4MFState4D> &p_g4mf_state, const Variant::Type p_variant_type) const;
	Array _decode_uints_as_variants(const Ref<G4MFState4D> &p_g4mf_state, const Variant::Type p_variant_type) const;

	// Private encode functions. Use `encode_variants_as_bytes` publicly.
	PackedFloat64Array _encode_variants_as_floats(const Array &p_input_data) const;
	PackedInt64Array _encode_variants_as_ints(const Array &p_input_data) const;
	Vector<uint64_t> _encode_variants_as_uints(const Array &p_input_data) const;

protected:
	static void _bind_methods();

public:
	int get_buffer_view_index() const { return _buffer_view_index; }
	void set_buffer_view_index(const int p_buffer_view_index) { _buffer_view_index = p_buffer_view_index; }

	String get_primitive_type() const { return _primitive_type; }
	void set_primitive_type(const String &p_primitive_type) { _primitive_type = p_primitive_type; }

	int get_vector_size() const { return _vector_size; }
	void set_vector_size(const int p_vector_size);

	// General helper functions.
	bool is_equal_exact(const Ref<G4MFAccessor4D> &p_other) const;
	int64_t get_bytes_per_primitive() const;
	int64_t get_bytes_per_vector() const;
	static int64_t get_numbers_per_variant(const Variant::Type p_variant_type);

	// Determine the minimal primitive type for the given data.
	// Add more types only as needed otherwise this will be a mess.
	static String minimal_primitive_type_for_colors(const PackedColorArray &p_input_data);
	static String minimal_primitive_type_for_floats(const PackedFloat64Array &p_input_data);
	static String minimal_primitive_type_for_int32s(const PackedInt32Array &p_input_data);
	static String minimal_primitive_type_for_vector4s(const PackedVector4Array &p_input_data);

	// Decode functions.
	PackedByteArray load_bytes_from_buffer_view(const Ref<G4MFState4D> &p_g4mf_state) const;
	PackedFloat64Array decode_floats_from_bytes(const Ref<G4MFState4D> &p_g4mf_state) const;
	PackedInt32Array decode_int32s_from_bytes(const Ref<G4MFState4D> &p_g4mf_state) const;
	PackedInt64Array decode_ints_from_bytes(const Ref<G4MFState4D> &p_g4mf_state) const;
	Vector<uint64_t> decode_uints_from_bytes(const Ref<G4MFState4D> &p_g4mf_state) const;
	Array decode_variants_from_bytes(const Ref<G4MFState4D> &p_g4mf_state, const Variant::Type p_variant_type) const;

	// Low-level accessor encode functions.
	PackedByteArray encode_floats_as_bytes(const PackedFloat64Array &p_input_numbers) const;
	PackedByteArray encode_ints_as_bytes(const PackedInt64Array &p_input_numbers) const;
	PackedByteArray encode_uints_as_bytes(const Vector<uint64_t> &p_input_numbers) const;
	PackedByteArray encode_variants_as_bytes(const Array &p_input_data) const;

	int store_accessor_data_into_state(const Ref<G4MFState4D> &p_g4mf_state, const PackedByteArray &p_data_bytes, const bool p_deduplicate = true);
	static Ref<G4MFAccessor4D> make_new_accessor_without_data(const String &p_primitive_type, const int p_vector_size = 1);

	// High-level accessor encode functions.
	static int encode_new_accessor_from_variants(const Ref<G4MFState4D> &p_g4mf_state, const Array &p_input_data, const String &p_primitive_type, const int p_vector_size = 1, const bool p_deduplicate = true);
	static int encode_new_accessor_from_vector4s(const Ref<G4MFState4D> &p_g4mf_state, const PackedVector4Array &p_input_data, const bool p_deduplicate = true);

	// Dictionary conversion.
	static Ref<G4MFAccessor4D> from_dictionary(const Dictionary &p_dict);
	Dictionary to_dictionary() const;
};
