#pragma once

#include "g4mf_state_4d.h"

class G4MFDocument4D : public Resource {
	GDCLASS(G4MFDocument4D, Resource);

public:
	enum CompressionFormat {
		COMPRESSION_FORMAT_UNKNOWN = -1,
		COMPRESSION_FORMAT_NONE = 0,
		COMPRESSION_FORMAT_ZSTD = 1,
	};

private:
	CompressionFormat _compression_format = COMPRESSION_FORMAT_NONE;
	bool _force_wireframe = false;

	inline uint64_t _ceiling_division(uint64_t a, uint64_t b) {
		return (a + b - 1) / b;
	}

	uint32_t _compression_format_to_indicator(const CompressionFormat p_compression_format);
	CompressionFormat _compression_indicator_to_format(const uint32_t p_magic);
	String _uint32_to_string(uint32_t p_value);

	// Export process.
	Error _export_convert_scene_node(Ref<G4MFState4D> p_g4mf_state, Node *p_current_node, const int p_parent_index);
	Error _export_serialize_json_data(Ref<G4MFState4D> p_g4mf_state);
	void _export_serialize_asset_header(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json);
	Error _export_serialize_buffers_accessors(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json);
	Error _export_serialize_textures(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json);
	Error _export_serialize_materials(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json);
	Error _export_serialize_meshes(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json);
	Error _export_serialize_shapes(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json);
	Error _export_serialize_nodes(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json);
	Error _export_serialize_buffer_data(Ref<G4MFState4D> p_g4mf_state);
	static String _export_pretty_print_compact(const Variant &p_variant);
	static String _export_pretty_print_json(const Dictionary &p_g4mf_json);
	PackedByteArray _export_compress_buffer_data(Ref<G4MFState4D> p_g4mf_state, const PackedByteArray &p_buffer_data);
	PackedByteArray _export_encode_as_byte_array(const Ref<G4MFState4D> &p_g4mf_state);

	// Import process.
	PackedByteArray _import_next_chunk_bytes_uncompressed(Ref<G4MFState4D> p_g4mf_state, const uint8_t *p_file_bytes, const uint64_t p_file_size, size_t &p_read_offset);
	Error _import_parse_json_data(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json);
	Error _import_parse_asset_header(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json);
	Error _import_parse_buffers_accessors(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json);
	Error _import_parse_textures(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json);
	Error _import_parse_materials(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json);
	Error _import_parse_meshes(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json);
	Error _import_parse_shapes(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json);
	Error _import_parse_nodes(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json);
	Node4D *_import_generate_scene_node(Ref<G4MFState4D> p_g4mf_state, const int p_node_index, Node *p_scene_parent, Node *p_scene_root);
	Ref<Mesh4D> _import_generate_combined_mesh(const Ref<G4MFState4D> p_g4mf_state, const bool p_include_invisible = false);

protected:
	static void _bind_methods();

public:
	// Main import and export functions.
	Error export_append_from_godot_scene(Ref<G4MFState4D> p_g4mf_state, Node *p_scene_root);
	Error export_append_from_godot_mesh(Ref<G4MFState4D> p_g4mf_state, const Ref<Mesh4D> &p_mesh);
	PackedByteArray export_write_to_byte_array(Ref<G4MFState4D> p_g4mf_state);
	Error export_write_to_file(Ref<G4MFState4D> p_g4mf_state, const String &p_path);
	Error import_read_from_byte_array(Ref<G4MFState4D> p_g4mf_state, const PackedByteArray &p_byte_array);
	Error import_read_from_file(Ref<G4MFState4D> p_g4mf_state, const String &p_path);
	Node4D *import_generate_godot_scene(Ref<G4MFState4D> p_g4mf_state);
	Ref<Mesh4D> import_generate_godot_mesh(Ref<G4MFState4D> p_g4mf_state, const bool p_include_invisible = false);

	// Settings for the export process.
	CompressionFormat get_compression_format() const { return _compression_format; }
	void set_compression_format(const CompressionFormat p_compression_format);

	// Settings for the import process.
	bool get_force_wireframe() const { return _force_wireframe; }
	void set_force_wireframe(const bool p_force_wireframe) { _force_wireframe = p_force_wireframe; }
};

VARIANT_ENUM_CAST(G4MFDocument4D::CompressionFormat);
