#pragma once

#include "g4mf_item_4d.h"

class G4MFState4D;

class G4MFAccessor4D : public G4MFItem4D {
	GDCLASS(G4MFAccessor4D, G4MFItem4D);

	String _primitive_type;
	int _buffer_view_index = -1;
	int _vector_size = 1;

	// Private general helper functions.
	static double _float8_to_double(uint8_t p_float8);
	static double _float16_to_double(uint16_t p_float16);
	static uint8_t _double_to_float8(double p_double);
	static uint16_t _double_to_float16(double p_double);

	// Private decode functions. Use `decode_accessor_as_variants` publicly.
	Array _decode_floats_as_variants(const Ref<G4MFState4D> &p_g4mf_state, const Variant::Type p_variant_type) const;
	Array _decode_ints_as_variants(const Ref<G4MFState4D> &p_g4mf_state, const Variant::Type p_variant_type) const;
	Array _decode_uints_as_variants(const Ref<G4MFState4D> &p_g4mf_state, const Variant::Type p_variant_type) const;

	// Private encode functions. Use `encode_accessor` publicly.
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
	int64_t bytes_per_primitive() const;
	int64_t bytes_per_vector() const;
	static int64_t primitives_per_variant(const Variant::Type p_variant_type);

	// Decode functions.
	PackedByteArray load_bytes_from_buffer_view(const Ref<G4MFState4D> &p_g4mf_state) const;
	PackedFloat64Array decode_floats_from_primitives(const Ref<G4MFState4D> &p_g4mf_state) const;
	PackedInt64Array decode_ints_from_primitives(const Ref<G4MFState4D> &p_g4mf_state) const;
	Vector<uint64_t> decode_uints_from_primitives(const Ref<G4MFState4D> &p_g4mf_state) const;
	Array decode_accessor_as_variants(const Ref<G4MFState4D> &p_g4mf_state, const Variant::Type p_variant_type) const;

	// Encode functions.
	PackedByteArray encode_floats_as_primitives(const PackedFloat64Array &p_input_data) const;
	PackedByteArray encode_ints_as_primitives(const PackedInt64Array &p_input_data) const;
	PackedByteArray encode_uints_as_primitives(const Vector<uint64_t> &p_input_data) const;
	PackedByteArray encode_accessor(const Array &p_input_data) const;

	static Ref<G4MFAccessor4D> from_dictionary(const Dictionary &p_dict);
	Dictionary to_dictionary() const;
};
