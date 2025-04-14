#include "g4mf_buffer_view_4d.h"

#include "../g4mf_state_4d.h"

void G4MFBufferView4D::set_byte_offset(const int64_t p_byte_offset) {
	ERR_FAIL_COND_MSG(p_byte_offset < 0, "Byte offset must not be negative.");
	_byte_offset = p_byte_offset;
}

void G4MFBufferView4D::set_byte_length(const int64_t p_byte_length) {
	ERR_FAIL_COND_MSG(p_byte_length < 0, "Byte length must not be negative.");
	_byte_length = p_byte_length;
}

PackedByteArray G4MFBufferView4D::load_buffer_view_data(const Ref<G4MFState4D> &p_g4mf_state) const {
	PackedByteArray ret;
	const TypedArray<PackedByteArray> state_buffers = p_g4mf_state->get_buffers();
	ERR_FAIL_INDEX_V(_buffer_index, state_buffers.size(), ret);
	const PackedByteArray buffer = state_buffers[_buffer_index];
	const int64_t buffer_size = buffer.size();
	ERR_FAIL_INDEX_V(_byte_offset, buffer_size, ret);
	const int64_t end = _byte_offset + _byte_length;
	ERR_FAIL_INDEX_V(end, buffer_size, ret);
	return buffer.slice(_byte_offset, end);
}

Ref<G4MFBufferView4D> G4MFBufferView4D::from_dictionary(const Dictionary &p_dict) {
	Ref<G4MFBufferView4D> buffer_view;
	buffer_view.instantiate();
	buffer_view->read_item_entries_from_dictionary(p_dict);
	if (p_dict.has("buffer")) {
		buffer_view->set_buffer_index(p_dict["buffer"]);
	}
	if (p_dict.has("byteLength")) {
		buffer_view->set_byte_length(p_dict["byteLength"]);
	}
	if (p_dict.has("byteOffset")) {
		buffer_view->set_byte_offset(p_dict["byteOffset"]);
	}
	return buffer_view;
}

Dictionary G4MFBufferView4D::to_dictionary() const {
	Dictionary dict = write_item_entries_to_dictionary();
	ERR_FAIL_COND_V_MSG(_buffer_index == -1, dict, "Buffer index must be set to a valid buffer before converting to Dictionary.");
	dict["buffer"] = _buffer_index;
	dict["byteLength"] = _byte_length;
	if (_byte_offset != 0) {
		dict["byteOffset"] = _byte_offset;
	}
	return dict;
}

void G4MFBufferView4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_buffer_index", "buffer_index"), &G4MFBufferView4D::set_buffer_index);
	ClassDB::bind_method(D_METHOD("get_buffer_index"), &G4MFBufferView4D::get_buffer_index);
	ClassDB::bind_method(D_METHOD("set_byte_offset", "byte_offset"), &G4MFBufferView4D::set_byte_offset);
	ClassDB::bind_method(D_METHOD("get_byte_offset"), &G4MFBufferView4D::get_byte_offset);
	ClassDB::bind_method(D_METHOD("set_byte_length", "byte_length"), &G4MFBufferView4D::set_byte_length);
	ClassDB::bind_method(D_METHOD("get_byte_length"), &G4MFBufferView4D::get_byte_length);

	ClassDB::bind_method(D_METHOD("load_buffer_view_data", "g4mf_state"), &G4MFBufferView4D::load_buffer_view_data);

	ClassDB::bind_static_method("G4MFBufferView4D", D_METHOD("from_dictionary", "dict"), &G4MFBufferView4D::from_dictionary);
	ClassDB::bind_method(D_METHOD("to_dictionary"), &G4MFBufferView4D::to_dictionary);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "buffer_index"), "set_buffer_index", "get_buffer_index");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "byte_offset"), "set_byte_offset", "get_byte_offset");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "byte_length"), "set_byte_length", "get_byte_length");
}
