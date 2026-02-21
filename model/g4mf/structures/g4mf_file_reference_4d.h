#pragma once

#include "g4mf_item_4d.h"

class G4MFState4D;

class G4MFFileReference4D : public G4MFItem4D {
	GDCLASS(G4MFFileReference4D, G4MFItem4D);

	int _buffer_view_index = -1;
	String _mime_type = "";
	String _file_uri = "";

protected:
	static void _bind_methods();

public:
	int get_buffer_view_index() const { return _buffer_view_index; }
	void set_buffer_view_index(const int p_buffer_view_index) { _buffer_view_index = p_buffer_view_index; }

	String get_mime_type() const { return _mime_type; }
	void set_mime_type(const String &p_mime_type) { _mime_type = p_mime_type; }

	String get_file_uri() const { return _file_uri; }
	void set_file_uri(const String &p_file_uri) { _file_uri = p_file_uri; }

	PackedByteArray read_file_data(const Ref<G4MFState4D> &p_g4mf_state) const;
	Error create_missing_directories_if_needed(const Ref<G4MFState4D> &p_g4mf_state) const;
	Error write_file_data(const Ref<G4MFState4D> &p_g4mf_state, const PackedByteArray &p_data, const int64_t p_alignment = 4, const bool p_deduplicate = true, const int p_buffer_index = 0);

	void read_file_reference_entries_from_dictionary(const Dictionary &p_dict);
	Dictionary write_file_reference_entries_to_dictionary() const;
};
