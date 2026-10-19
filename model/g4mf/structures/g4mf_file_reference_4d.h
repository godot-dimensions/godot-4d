#pragma once

#include "g4mf_item_4d.h"

class G4MFFileReference4D : public G4MFItem4D {
	GDCLASS(G4MFFileReference4D, G4MFItem4D);

	// Internal cache for file data.
	PackedByteArray _file_data_cache;

	// Contents of the JSON file reference object.
	int _buffer_view_index = -1;
	String _mime_type = "";
	String _file_uri = "";

protected:
	static void _bind_methods();

public:
	static String sanitize_file_extension(const String &p_extension, const bool p_warn);
	static String sanitize_file_path_uri(const String &p_desired_file_path, const bool p_warn);
	static String sanitize_path_uri(const String &p_requested_path_stem, const String &p_extension, const bool p_print_warning, String &r_sanitized_ext);

	// Contents of the JSON file reference object.
	int get_buffer_view_index() const { return _buffer_view_index; }
	void set_buffer_view_index(const int p_buffer_view_index) { _buffer_view_index = p_buffer_view_index; }

	String get_mime_type() const { return _mime_type; }
	void set_mime_type(const String &p_mime_type) { _mime_type = p_mime_type; }

	String get_file_uri() const { return _file_uri; }
	void set_file_uri(const String &p_file_uri);
	void set_file_uri_no_sanitize(const String &p_file_uri);
	void set_unique_file_path_uri(const Ref<G4MFState4D> &p_g4mf_state, const String &p_desired_path_stem, const String &p_file_ext, const bool p_print_warning = true);
	void set_unique_file_path_combined_uri(const Ref<G4MFState4D> &p_g4mf_state, const String &p_desired_file_uri, const bool p_print_warning = true);

	String resolve_file_uri(const String &p_base_path) const;
	PackedByteArray import_read_file_data(const Ref<G4MFState4D> &p_g4mf_state, const bool p_read_from_cache = true);
	virtual Error import_parse_file_data(const Ref<G4MFState4D> &p_g4mf_state);
	virtual Error export_serialize_file_data(const Ref<G4MFState4D> &p_g4mf_state, const bool p_deduplicate = true, const int p_buffer_index = 0);
#ifdef GDEXTENSION
	GDVIRTUAL1R(Error, _import_parse_file_data, Ref<Resource>);
	GDVIRTUAL3R(Error, _export_serialize_file_data, Ref<Resource>, bool, int);
#else // GODOT_MODULE
	GDVIRTUAL1R(Error, _import_parse_file_data, Ref<G4MFState4D>);
	GDVIRTUAL3R(Error, _export_serialize_file_data, Ref<G4MFState4D>, bool, int);
#endif

	Error export_create_missing_directories_if_needed(const Ref<G4MFState4D> &p_g4mf_state) const;
	void export_store_file_bytes(const PackedByteArray &p_data);
	Error export_write_to_buffer(const Ref<G4MFState4D> &p_g4mf_state, const int64_t p_alignment = 4, const bool p_deduplicate = true, const int p_buffer_index = 0);
	Error export_write_to_file(const Ref<G4MFState4D> &p_g4mf_state);

	static Ref<G4MFFileReference4D> file_reference_from_dictionary(const Dictionary &p_dict);
	void read_file_reference_entries_from_dictionary(const Dictionary &p_dict);
	Dictionary write_file_reference_entries_to_dictionary() const;
};
