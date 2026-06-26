#pragma once

#include "g4mf_state_4d.h"

#if GDEXTENSION
namespace godot {
class FileAccess;
}
#endif

class G4MFDocument4D : public Resource {
	GDCLASS(G4MFDocument4D, Resource);

public:
	enum EncodingFormat : uint32_t {
		ENCODING_FORMAT_PLAIN = 0u,
		ENCODING_FORMAT_ZSTD = 0x6474735Au, // ASCII string "Zstd" in little-endian. Note that this does not need to match the magic number in Zstd itself (0xFD2FB528).
	};

private:
	EncodingFormat _encoding_format = ENCODING_FORMAT_PLAIN;
	int _max_nested_scene_depth = -1; // -1 means unlimited depth.

	static constexpr uint64_t CHUNK_ALIGNMENT_BITMASK = (uint64_t)15;
	inline uint64_t _ceiling_division(uint64_t a, uint64_t b) {
		return (a + b - 1) / b;
	}

	static bool _is_encoding_format_supported(const EncodingFormat p_encoding_format);
	static String _uint32_to_ascii_string(uint32_t p_value, const bool p_allow_and_escape_non_ascii);
	static uint32_t _ascii_string_to_uint32(const String &p_value);

	// Export process.
	Error _export_convert_scene_node(Ref<G4MFState4D> p_g4mf_state, Node *p_current_node, const int p_parent_index);
	Error _export_serialize_json_data(Ref<G4MFState4D> p_g4mf_state);
	Error _export_serialize_nodes(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json);
	Error _export_serialize_shapes(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json);
	Error _export_serialize_meshes(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json);
	Error _export_serialize_materials(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json);
	Error _export_serialize_textures(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json);
	Error _export_serialize_files(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json);
	Error _export_serialize_buffers_accessors(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json);
	Error _export_serialize_buffer_data_to_uri(Ref<G4MFState4D> p_g4mf_state, const bool p_should_separate_buffers_into_files);
	void _export_serialize_asset_header(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json);
	static String _export_pretty_print_inline(const Variant &p_variant);
	static String _export_pretty_print_json(const Dictionary &p_g4mf_json);
	PackedByteArray _export_encode_chunk_data(Ref<G4MFState4D> p_g4mf_state, const PackedByteArray &p_buffer_data);
	PackedByteArray _export_encode_as_byte_array(const Ref<G4MFState4D> &p_g4mf_state);

	// Import process.
	Error _import_read_from_binary_file(Ref<G4MFState4D> p_g4mf_state, const Ref<FileAccess> &p_file);
	PackedByteArray _import_decode_chunk_data(const PackedByteArray &p_file_or_chunk_data, const int64_t p_chunk_data_offset, const int64_t p_chunk_data_raw_size, const EncodingFormat p_chunk_encoding_format);
	Error _import_parse_buffers(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json, PackedInt64Array *r_chunk_indices, PackedInt64Array *r_decoded_byte_lengths);
	Error _import_parse_json_data(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json);
	Error _import_parse_asset_header(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json);
	Error _import_parse_buffer_views(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json);
	Error _import_parse_accessors(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json);
	Error _import_parse_files(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json);
	Error _import_parse_textures(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json);
	Error _import_parse_materials(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json);
	Error _import_parse_meshes(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json);
	Error _import_parse_shapes(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json);
	Error _import_parse_nodes(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json);
	Error _import_infer_buffer_view_alignment(Ref<G4MFState4D> p_g4mf_state);
	Node *_import_generate_scene_node(Ref<G4MFState4D> p_g4mf_state, const int p_node_index, Node *p_scene_parent, Node *p_scene_root);
	Ref<Mesh4D> _import_generate_combined_mesh(const Ref<G4MFState4D> p_g4mf_state, const bool p_include_invisible = false);

protected:
	static void _bind_methods();

public:
	// Main import and export functions.
	Error export_append_from_godot_scene(Ref<G4MFState4D> p_g4mf_state, Node *p_scene_root);
	Error export_append_from_godot_mesh(Ref<G4MFState4D> p_g4mf_state, const Ref<Mesh4D> &p_mesh);
	Error export_repack_buffer_data(Ref<G4MFState4D> p_g4mf_state, const bool p_allow_reordering = true);
	PackedByteArray export_write_to_byte_array(Ref<G4MFState4D> p_g4mf_state);
	Error export_write_to_file(Ref<G4MFState4D> p_g4mf_state, const String &p_path);

	Error import_read_from_byte_array(Ref<G4MFState4D> p_g4mf_state, const PackedByteArray &p_byte_array);
	Error import_read_from_file(Ref<G4MFState4D> p_g4mf_state, const String &p_path);
	Node *import_generate_godot_scene(Ref<G4MFState4D> p_g4mf_state);
	Ref<Mesh4D> import_generate_godot_mesh(Ref<G4MFState4D> p_g4mf_state, const int p_which_mesh_index = -1, const bool p_include_invisible = false);

	// Settings for both import and export.
	int get_max_nested_scene_depth() const { return _max_nested_scene_depth; }
	void set_max_nested_scene_depth(const int p_max_nested_scene_depth) { _max_nested_scene_depth = p_max_nested_scene_depth; }

	// Settings for the export process.
	EncodingFormat get_encoding_format() const { return _encoding_format; }
	void set_encoding_format(const EncodingFormat p_encoding_format);
};

VARIANT_ENUM_CAST(G4MFDocument4D::EncodingFormat);
