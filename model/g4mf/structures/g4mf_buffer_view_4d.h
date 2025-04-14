#pragma once

#include "g4mf_item_4d.h"

class G4MFState4D;

class G4MFBufferView4D : public G4MFItem4D {
	GDCLASS(G4MFBufferView4D, G4MFItem4D);

	int _buffer_index = -1;
	int64_t _byte_offset = 0;
	int64_t _byte_length = 0;

protected:
	static void _bind_methods();

public:
	int get_buffer_index() const { return _buffer_index; }
	void set_buffer_index(const int p_buffer_index) { _buffer_index = p_buffer_index; }

	int64_t get_byte_offset() const { return _byte_offset; }
	void set_byte_offset(const int64_t p_byte_offset);

	int64_t get_byte_length() const { return _byte_length; }
	void set_byte_length(const int64_t p_byte_length);

	PackedByteArray load_buffer_view_data(const Ref<G4MFState4D> &p_g4mf_state) const;

	static Ref<G4MFBufferView4D> from_dictionary(const Dictionary &p_dict);
	Dictionary to_dictionary() const;
};
