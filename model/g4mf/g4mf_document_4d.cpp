#include "g4mf_document_4d.h"

#include "../mesh/mesh_instance_4d.h"
#include "../off/off_document_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/classes/marshalls.hpp>
#include <godot_cpp/core/version.hpp>
#elif GODOT_MODULE
#include "core/core_bind.h"
#include "core/io/compression.h"
#include "core/io/json.h"
#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR > 4
// In Godot 4.5 and later, namespaces were capitalized: core_bind -> CoreBind.
using namespace CoreBind;
#else
using namespace core_bind;
#endif
#endif

uint32_t G4MFDocument4D::_compression_format_to_indicator(const CompressionFormat p_compression_format) {
	switch (p_compression_format) {
		case COMPRESSION_FORMAT_NONE:
			return 0x00000000;
		case COMPRESSION_FORMAT_ZSTD:
			// "Zstd" in ASCII. Note that this does not need to match the magic number in Zstd itself (0xFD2FB528).
			return 0x6474735A;
		case COMPRESSION_FORMAT_UNKNOWN:
			return 0xFFFFFFFF; // Unknown compression format.
	}
	return 0xFFFFFFFF; // Unknown compression format.
}

G4MFDocument4D::CompressionFormat G4MFDocument4D::_compression_indicator_to_format(const uint32_t p_indicator) {
	switch (p_indicator) {
		case 0x00000000:
			return COMPRESSION_FORMAT_NONE;
		case 0x6474735A:
			// "Zstd" in ASCII. Note that this does not need to match the magic number in Zstd itself (0xFD2FB528).
			return COMPRESSION_FORMAT_ZSTD;
	}
	return COMPRESSION_FORMAT_UNKNOWN;
}

// Export process.

Error G4MFDocument4D::_export_convert_scene_node(Ref<G4MFState4D> p_g4mf_state, Node *p_current_node, const int p_parent_index) {
	ERR_FAIL_NULL_V(p_current_node, ERR_INVALID_PARAMETER);
	Ref<G4MFNode4D> g4mf_node = G4MFNode4D::from_godot_node_basic(p_g4mf_state, p_current_node);
	g4mf_node->set_parent_index(p_parent_index);
	TypedArray<Node4D> state_godot_nodes = p_g4mf_state->get_godot_nodes();
	const int new_node_index = state_godot_nodes.size();
	ERR_FAIL_COND_V_MSG(p_g4mf_state->get_g4mf_nodes().size() != new_node_index, ERR_BUG, "G4MFState4D's g4mf_nodes and godot_nodes arrays MUST always be the same size.");
	// First check if this node should be packed into a model.
	if (_max_nested_scene_depth != 0 && p_parent_index != -1 && !p_current_node->get_scene_file_path().is_empty()) {
		const int model_index = G4MFModel4D::export_pack_nodes_into_model(this, p_g4mf_state, p_current_node, true);
		if (model_index >= 0) {
			g4mf_node->set_model_index(model_index);
			// Append this one node representing the model instance, then return.
			// Since the model packs the entire sub-scene, do not recurse into children.
			p_g4mf_state->append_g4mf_node(g4mf_node);
			state_godot_nodes.resize(new_node_index + 1);
			state_godot_nodes[new_node_index] = p_current_node;
			return OK;
		}
	}
	// If it's not a model, convert the specific data from its type, like mesh, camera, physics, extensions, etc.
	g4mf_node->from_godot_node_components(p_g4mf_state, p_current_node);
	// Allow excluding a node from export by setting the parent index to -2.
	if (g4mf_node->get_parent_index() < -1) {
		return ERR_SKIP;
	}
	if (p_parent_index == -1) {
		// The root node MUST NOT have a transform as required by the G4MF specification.
		g4mf_node->set_transform(Transform4D());
	}
	// Append the node and recurse into children.
	p_g4mf_state->append_g4mf_node(g4mf_node);
	state_godot_nodes.resize(new_node_index + 1);
	state_godot_nodes[new_node_index] = p_current_node;
	PackedInt32Array children_indices;
	for (int i = 0; i < p_current_node->get_child_count(); i++) {
		Node *child = p_current_node->get_child(i);
		const int32_t child_index_if_appended = state_godot_nodes.size();
		const Error err = _export_convert_scene_node(p_g4mf_state, child, new_node_index);
		if (err == ERR_SKIP) {
			continue;
		}
		if (err != OK) {
			return err;
		}
		children_indices.append(child_index_if_appended);
	}
	g4mf_node->set_children_indices(children_indices);
	return OK;
}

Error G4MFDocument4D::_export_serialize_json_data(Ref<G4MFState4D> p_g4mf_state) {
	Dictionary g4mf_json;
	p_g4mf_state->set_g4mf_json(g4mf_json);
	_export_serialize_asset_header(p_g4mf_state, g4mf_json);
	_export_serialize_textures(p_g4mf_state, g4mf_json);
	_export_serialize_materials(p_g4mf_state, g4mf_json);
	_export_serialize_meshes(p_g4mf_state, g4mf_json);
	_export_serialize_models(p_g4mf_state, g4mf_json);
	_export_serialize_shapes(p_g4mf_state, g4mf_json);
	_export_serialize_nodes(p_g4mf_state, g4mf_json);
	// Serialize buffers, buffer views, and accessors last, in case any of the above added new ones.
	_export_serialize_buffers_accessors(p_g4mf_state, g4mf_json);
	return OK;
}

void G4MFDocument4D::_export_serialize_asset_header(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json) {
	Dictionary asset_header;
	asset_header["dimension"] = 4;
	const String godot_version = itos(GODOT_VERSION_MAJOR) + "." + itos(GODOT_VERSION_MINOR) + "." + itos(GODOT_VERSION_PATCH);
#if GDEXTENSION
	const String mod_or_ext = "GDExtension";
#elif GODOT_MODULE
	const String mod_or_ext = "module";
#endif
	asset_header["generator"] = "Godot Engine " + godot_version + " with Godot 4D " + mod_or_ext;
	asset_header["specification"] = "https://github.com/godot-dimensions/g4mf";
	p_g4mf_json["asset"] = asset_header;
}

Error G4MFDocument4D::_export_serialize_buffers_accessors(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json) {
	// Serialize buffers.
	TypedArray<PackedByteArray> state_buffers = p_g4mf_state->get_g4mf_buffers();
	const int buffer_count = state_buffers.size();
	if (buffer_count == 0) {
		return OK; // No buffers to serialize (implies no buffer views or accessors).
	}
	Array serialized_buffers;
	serialized_buffers.resize(buffer_count);
	for (int i = 0; i < buffer_count; i++) {
		const PackedByteArray state_buffer = state_buffers[i];
		Dictionary buffer_dict;
		buffer_dict["byteLength"] = state_buffer.size();
		if (_compression_format != COMPRESSION_FORMAT_NONE) {
			buffer_dict["compression"] = _uint32_to_string(_compression_format_to_indicator(_compression_format), true);
		}
		serialized_buffers[i] = buffer_dict;
	}
	if (!serialized_buffers.is_empty()) {
		p_g4mf_json["buffers"] = serialized_buffers;
	}
	// Serialize buffer views (only relevant if buffers exist).
	TypedArray<G4MFBufferView4D> state_buffer_views = p_g4mf_state->get_g4mf_buffer_views();
	const int buffer_view_count = state_buffer_views.size();
	if (buffer_view_count == 0) {
		return OK; // No buffer views to serialize (implies no accessors).
	}
	Array serialized_buffer_views;
	serialized_buffer_views.resize(buffer_view_count);
	for (int i = 0; i < buffer_view_count; i++) {
		Ref<G4MFBufferView4D> state_buffer_view = state_buffer_views[i];
		ERR_FAIL_COND_V(state_buffer_view.is_null(), ERR_INVALID_DATA);
		Dictionary buffer_view_dict = state_buffer_view->to_dictionary();
		serialized_buffer_views[i] = buffer_view_dict;
	}
	if (!serialized_buffer_views.is_empty()) {
		p_g4mf_json["bufferViews"] = serialized_buffer_views;
	}
	// Serialize accessors (only relevant if buffer views exist).
	TypedArray<G4MFAccessor4D> state_accessors = p_g4mf_state->get_g4mf_accessors();
	const int accessor_count = state_accessors.size();
	if (accessor_count == 0) {
		return OK; // No accessors to serialize.
	}
	Array serialized_accessors;
	serialized_accessors.resize(accessor_count);
	for (int i = 0; i < accessor_count; i++) {
		Ref<G4MFAccessor4D> state_accessor = state_accessors[i];
		ERR_FAIL_COND_V(state_accessor.is_null(), ERR_INVALID_DATA);
		Dictionary accessor_dict = state_accessor->to_dictionary();
		serialized_accessors[i] = accessor_dict;
	}
	if (!serialized_accessors.is_empty()) {
		p_g4mf_json["accessors"] = serialized_accessors;
	}
	return OK;
}

Error G4MFDocument4D::_export_serialize_buffer_data(Ref<G4MFState4D> p_g4mf_state, const bool p_should_separate_buffers_into_files) {
	Dictionary g4mf_json = p_g4mf_state->get_g4mf_json();
	if (!g4mf_json.has("buffers")) {
		return OK; // No buffers to serialize.
	}
	TypedArray<PackedByteArray> state_buffers = p_g4mf_state->get_g4mf_buffers();
	Array json_buffers = g4mf_json["buffers"];
	ERR_FAIL_COND_V(state_buffers.size() != json_buffers.size(), ERR_INVALID_DATA);
	const String file_prefix = p_g4mf_state->get_g4mf_filename().get_basename();
	for (int buffer_index = 0; buffer_index < state_buffers.size(); buffer_index++) {
		Dictionary json_buffer_dict = json_buffers[buffer_index];
		PackedByteArray compressed_buffer = _export_compress_buffer_data(p_g4mf_state, state_buffers[buffer_index]);
		if (p_should_separate_buffers_into_files) {
			const String buffer_rel_path = file_prefix + String("_buffer") + String::num_int64(buffer_index) + String(".bin");
			Ref<FileAccess> file = FileAccess::open(p_g4mf_state->get_g4mf_base_path().path_join(buffer_rel_path), FileAccess::WRITE);
			if (file.is_valid()) {
				file->store_buffer(compressed_buffer);
				file->close();
				json_buffer_dict["uri"] = buffer_rel_path;
				continue;
			} else {
				WARN_PRINT("G4MF export: Failed to write buffer data to file: " + buffer_rel_path + ". Writing as base64 instead as fallback.");
			}
		}
		const String base64_buffer = Marshalls::get_singleton()->raw_to_base64(compressed_buffer);
		json_buffer_dict["uri"] = String("data:application/octet-stream;base64,") + base64_buffer;
	}
	return OK;
}

Error G4MFDocument4D::_export_serialize_textures(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json) {
	TypedArray<G4MFTexture4D> state_g4mf_textures = p_g4mf_state->get_g4mf_textures();
	const int texture_count = state_g4mf_textures.size();
	if (texture_count == 0) {
		return OK; // No textures to serialize.
	}
	Array serialized_textures;
	serialized_textures.resize(texture_count);
	for (int i = 0; i < texture_count; i++) {
		Ref<G4MFTexture4D> g4mf_texture = state_g4mf_textures[i];
		ERR_FAIL_COND_V(g4mf_texture.is_null(), ERR_INVALID_DATA);
		Dictionary serialized_texture = g4mf_texture->to_dictionary();
		serialized_textures[i] = serialized_texture;
	}
	if (!serialized_textures.is_empty()) {
		p_g4mf_json["textures"] = serialized_textures;
		p_g4mf_state->set_g4mf_json(p_g4mf_json);
	}
	return OK;
}

Error G4MFDocument4D::_export_serialize_materials(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json) {
	TypedArray<G4MFMaterial4D> state_g4mf_materials = p_g4mf_state->get_g4mf_materials();
	const int material_count = state_g4mf_materials.size();
	if (material_count == 0) {
		return OK; // No materials to serialize.
	}
	Array serialized_materials;
	serialized_materials.resize(material_count);
	for (int i = 0; i < material_count; i++) {
		Ref<G4MFMaterial4D> g4mf_material = state_g4mf_materials[i];
		ERR_FAIL_COND_V(g4mf_material.is_null(), ERR_INVALID_DATA);
		Dictionary serialized_material = g4mf_material->to_dictionary();
		serialized_materials[i] = serialized_material;
	}
	if (!serialized_materials.is_empty()) {
		p_g4mf_json["materials"] = serialized_materials;
		p_g4mf_state->set_g4mf_json(p_g4mf_json);
	}
	return OK;
}

Error G4MFDocument4D::_export_serialize_meshes(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json) {
	TypedArray<G4MFMesh4D> state_g4mf_meshes = p_g4mf_state->get_g4mf_meshes();
	const int mesh_count = state_g4mf_meshes.size();
	if (mesh_count == 0) {
		return OK; // No meshes to serialize.
	}
	Array serialized_meshes;
	serialized_meshes.resize(mesh_count);
	for (int i = 0; i < mesh_count; i++) {
		Ref<G4MFMesh4D> g4mf_mesh = state_g4mf_meshes[i];
		ERR_FAIL_COND_V(g4mf_mesh.is_null(), ERR_INVALID_DATA);
		Dictionary serialized_mesh = g4mf_mesh->to_dictionary();
		serialized_meshes[i] = serialized_mesh;
	}
	if (!serialized_meshes.is_empty()) {
		p_g4mf_json["meshes"] = serialized_meshes;
		p_g4mf_state->set_g4mf_json(p_g4mf_json);
	}
	return OK;
}

Error G4MFDocument4D::_export_serialize_models(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json) {
	TypedArray<G4MFModel4D> state_g4mf_models = p_g4mf_state->get_g4mf_models();
	const int model_count = state_g4mf_models.size();
	if (model_count == 0) {
		return OK; // No models to serialize.
	}
	Array serialized_models;
	serialized_models.resize(model_count);
	for (int i = 0; i < model_count; i++) {
		Ref<G4MFModel4D> g4mf_model = state_g4mf_models[i];
		ERR_FAIL_COND_V(g4mf_model.is_null(), ERR_INVALID_DATA);
		g4mf_model->export_write_model_data(p_g4mf_state);
		Dictionary serialized_model = g4mf_model->to_dictionary();
		serialized_models[i] = serialized_model;
	}
	if (!serialized_models.is_empty()) {
		p_g4mf_json["models"] = serialized_models;
		p_g4mf_state->set_g4mf_json(p_g4mf_json);
	}
	return OK;
}

Error G4MFDocument4D::_export_serialize_shapes(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json) {
	TypedArray<G4MFShape4D> state_g4mf_shapes = p_g4mf_state->get_g4mf_shapes();
	const int shape_count = state_g4mf_shapes.size();
	if (shape_count == 0) {
		return OK; // No shapes to serialize.
	}
	Array serialized_shapes;
	serialized_shapes.resize(shape_count);
	for (int i = 0; i < shape_count; i++) {
		Ref<G4MFShape4D> g4mf_shape = state_g4mf_shapes[i];
		ERR_FAIL_COND_V(g4mf_shape.is_null(), ERR_INVALID_DATA);
		Dictionary serialized_shape = g4mf_shape->to_dictionary();
		serialized_shapes[i] = serialized_shape;
	}
	if (!serialized_shapes.is_empty()) {
		p_g4mf_json["shapes"] = serialized_shapes;
		p_g4mf_state->set_g4mf_json(p_g4mf_json);
	}
	return OK;
}

Error G4MFDocument4D::_export_serialize_nodes(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json) {
	TypedArray<G4MFNode4D> state_g4mf_nodes = p_g4mf_state->get_g4mf_nodes();
	const int node_count = state_g4mf_nodes.size();
	Array serialized_nodes;
	serialized_nodes.resize(node_count);
	for (int i = 0; i < node_count; i++) {
		Ref<G4MFNode4D> g4mf_node = state_g4mf_nodes[i];
		if (i == 0) {
			g4mf_node->set_transform(Transform4D());
		}
		Dictionary serialized_node = g4mf_node->to_dictionary();
		serialized_nodes[i] = serialized_node;
	}
	Dictionary g4mf_json = p_g4mf_state->get_g4mf_json();
	if (!serialized_nodes.is_empty()) {
		g4mf_json["nodes"] = serialized_nodes;
		p_g4mf_state->set_g4mf_json(g4mf_json);
	}
	return OK;
}

// Includes spaces after braces and commas, and avoids unnecessary digits for floats.
String G4MFDocument4D::_export_pretty_print_inline(const Variant &p_variant) {
	if (p_variant.get_type() == Variant::DICTIONARY) {
		Dictionary dict = p_variant;
		if (dict.is_empty()) {
			return "{}";
		}
		Array keys = dict.keys();
		keys.sort();
		String compact = "{ ";
		for (int i = 0; i < keys.size(); i++) {
			String key = keys[i];
			Variant value = dict[key];
			if (i > 0) {
				compact += ", ";
			}
			compact += "\"" + key + "\": " + _export_pretty_print_inline(value);
		}
		compact += " }";
		return compact;
	} else if (p_variant.get_type() == Variant::ARRAY) {
		Array arr = p_variant;
		String compact = "[";
		for (int i = 0; i < arr.size(); i++) {
			Variant value = arr[i];
			if (i > 0) {
				compact += ", ";
			}
			compact += _export_pretty_print_inline(value);
		}
		compact += "]";
		return compact;
	} else if (p_variant.get_type() == Variant::FLOAT) {
		const double num = (double)p_variant;
		const String num_short = String::num(num, 6);
		if (num_short.contains(".")) {
			const double back = num_short.to_float();
			if (num == back || float(num) == float(back)) {
				return num_short;
			}
		}
	}
	return JSON::stringify(p_variant, "", true);
}

// Only indents the first three levels of the JSON for increased readability.
String G4MFDocument4D::_export_pretty_print_json(const Dictionary &p_g4mf_json) {
	Dictionary g4mf_json = p_g4mf_json;
#if GODOT_MODULE && (GODOT_VERSION_MAJOR > 4 || (GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR >= 4))
	g4mf_json.sort();
#else
	// HACK: `.sort()` is too new of an API to use here, so sort using an ugly workaround...
	g4mf_json = JSON::parse_string(JSON::stringify(g4mf_json, "", true));
#endif
	String pretty = "{\n";
	Array top_keys = g4mf_json.keys();
	const int top_key_count = top_keys.size();
	for (int top_index = 0; top_index < top_key_count; top_index++) {
		String top_key = top_keys[top_index];
		Variant top_value = g4mf_json[top_key];
		if (top_value.get_type() == Variant::DICTIONARY) {
			Dictionary top_dict = top_value;
#if GODOT_MODULE && (GODOT_VERSION_MAJOR > 4 || (GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR >= 4))
			top_dict.sort();
#endif
			pretty += "\t\"" + top_key + "\": {\n";
			Array sub_keys = top_dict.keys();
			for (int sub_index = 0; sub_index < sub_keys.size(); sub_index++) {
				String sub_key = sub_keys[sub_index];
				Variant sub_value = top_dict[sub_key];
				pretty += "\t\t\"" + sub_key + "\": " + _export_pretty_print_inline(sub_value);
				if (sub_index < sub_keys.size() - 1) {
					pretty += ",\n";
				} else {
					pretty += "\n";
				}
			}
			if (top_index < top_key_count - 1) {
				pretty += "\t},\n";
			} else {
				pretty += "\t}\n";
			}
		} else if (top_value.get_type() == Variant::ARRAY) {
			pretty += "\t\"" + top_key + "\": [\n";
			Array top_array = top_value;
			for (int top_array_index = 0; top_array_index < top_array.size(); top_array_index++) {
				Variant top_array_value = top_array[top_array_index];
				pretty += "\t\t";
				if (top_array_value.get_type() == Variant::DICTIONARY) {
					Dictionary sub_dict = top_array_value;
#if GODOT_MODULE && (GODOT_VERSION_MAJOR > 4 || (GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR >= 4))
					sub_dict.sort();
#endif
					pretty += "{\n";
					Array sub_keys = sub_dict.keys();
					for (int sub_dict_index = 0; sub_dict_index < sub_keys.size(); sub_dict_index++) {
						String sub_dict_key = sub_keys[sub_dict_index];
						Variant sub_dict_value = sub_dict[sub_dict_key];
						pretty += "\t\t\t\"" + sub_dict_key + "\": " + _export_pretty_print_inline(sub_dict_value);
						if (sub_dict_index < sub_keys.size() - 1) {
							pretty += ",\n";
						} else {
							pretty += "\n";
						}
					}
					pretty += "\t\t}";
				} else {
					pretty += JSON::stringify(top_array_value, "", true);
				}
				if (top_array_index < top_array.size() - 1) {
					pretty += ",\n";
				} else {
					pretty += "\n";
				}
			}
			if (top_index < top_key_count - 1) {
				pretty += "\t],\n";
			} else {
				pretty += "\t]\n";
			}
		} else {
			pretty += "\t\"" + top_key + "\": " + _export_pretty_print_inline(top_value);
			if (top_index < top_key_count - 1) {
				pretty += ",\n";
			} else {
				pretty += "\n";
			}
		}
	}
	pretty += "}\n";
	return pretty;
}

PackedByteArray G4MFDocument4D::_export_compress_buffer_data(Ref<G4MFState4D> p_g4mf_state, const PackedByteArray &p_buffer_data) {
	const int64_t buffer_size = p_buffer_data.size();
	switch (_compression_format) {
		case G4MFDocument4D::COMPRESSION_FORMAT_NONE: {
			return p_buffer_data;
		};
		case G4MFDocument4D::COMPRESSION_FORMAT_ZSTD: {
			// Compress the buffer data using Zstd.
#if GDEXTENSION
			return p_buffer_data.compress(FileAccess::CompressionMode::COMPRESSION_ZSTD);
#elif GODOT_MODULE
			PackedByteArray compressed;
			if (buffer_size > 0) {
				constexpr Compression::Mode mode = Compression::Mode::MODE_ZSTD;
				compressed.resize(Compression::get_max_compressed_buffer_size(buffer_size, mode));
				int result = Compression::compress(compressed.ptrw(), p_buffer_data.ptr(), buffer_size, mode);
				ERR_FAIL_COND_V(result < 0, PackedByteArray());
				compressed.resize(result);
			}
			return compressed;
#endif
		}
		case G4MFDocument4D::COMPRESSION_FORMAT_UNKNOWN: {
			ERR_FAIL_V_MSG(PackedByteArray(), "G4MF export: Compression format is not set.");
		};
	}
	CRASH_NOW_MSG("G4MF export: Unknown compression format. This should never happen (the switch should handle all cases the enum is allowed to be set to).");
	return PackedByteArray(); // Unreachable, but MSVC complains about not all control paths returning a value.
}

PackedByteArray G4MFDocument4D::_export_encode_as_byte_array(const Ref<G4MFState4D> &p_g4mf_state) {
	const uint32_t compression_format_indicator = _compression_format_to_indicator(_compression_format);
	CRASH_COND(compression_format_indicator == 0xFFFFFFFF);
	const Dictionary g4mf_json = p_g4mf_state->get_g4mf_json();
	ERR_FAIL_COND_V(!g4mf_json.has("asset"), PackedByteArray());
	const String json_string = JSON::stringify(g4mf_json, "", true);
	ERR_FAIL_COND_V(json_string.length() < 25, PackedByteArray()); // Minimum possible valid JSON is `{"asset":{"dimension":4}}` (25 bytes).
	const PackedByteArray json_compressed = _export_compress_buffer_data(p_g4mf_state, json_string.to_utf8_buffer());
	const uint64_t json_compressed_size = json_compressed.size();
	uint64_t total_file_size = 32 + json_compressed_size;
	// Add binary buffer chunks to the file size.
	const TypedArray<PackedByteArray> state_buffers = p_g4mf_state->get_g4mf_buffers();
	Vector<PackedByteArray> buffers_compressed;
	// "Automatic" is always embedded in this case anyway, so pass `-1` for `p_blob_size` which returns false (flipped to true here).
	const bool should_embed_buffers = !p_g4mf_state->should_separate_binary_blobs(-1);
	if (should_embed_buffers && !state_buffers.is_empty()) {
		ERR_FAIL_COND_V(!g4mf_json.has("buffers"), PackedByteArray());
		Array json_buffers = g4mf_json["buffers"];
		const int state_buffers_size = state_buffers.size();
		ERR_FAIL_COND_V(state_buffers_size != json_buffers.size(), PackedByteArray());
		buffers_compressed.resize(state_buffers_size);
		for (int i = 0; i < state_buffers_size; i++) {
			// The start of each chunk needs to be padded to 16 bytes.
			total_file_size = _ceiling_division(total_file_size, 16) * 16;
			const PackedByteArray buffer_compressed = _export_compress_buffer_data(p_g4mf_state, state_buffers[i]);
			total_file_size += 16 + buffer_compressed.size();
			buffers_compressed.set(i, buffer_compressed);
		}
	}
	// Done gathering information, now create the byte array.
	PackedByteArray file_bytes;
	file_bytes.resize(total_file_size);
	// Write the file header.
	uint8_t *file_bytes_ptrw = file_bytes.ptrw();
	*(uint32_t *)file_bytes_ptrw = (uint32_t)0x464D3447; // "G4MF"
	size_t write_offset = 4;
	*(uint32_t *)(file_bytes_ptrw + write_offset) = (uint32_t)0x00000000; // Version 0
	write_offset += 4;
	*(uint64_t *)(file_bytes_ptrw + write_offset) = total_file_size; // File size
	write_offset += 8;
	// Write the JSON chunk.
	*(uint32_t *)(file_bytes_ptrw + write_offset) = (uint32_t)0x4E4F534A; // "JSON"
	write_offset += 4;
	*(uint32_t *)(file_bytes_ptrw + write_offset) = compression_format_indicator;
	write_offset += 4;
	*(uint64_t *)(file_bytes_ptrw + write_offset) = json_compressed_size; // JSON size after compression.
	write_offset += 8;
	memcpy(file_bytes_ptrw + write_offset, json_compressed.ptr(), json_compressed_size);
	write_offset += json_compressed_size;
	// Write buffer chunks.
	if (should_embed_buffers) {
		for (int buffer_index = 0; buffer_index < buffers_compressed.size(); buffer_index++) {
			// Pad the previous chunk to 16 bytes if needed.
			const uint64_t write_offset_padded = _ceiling_division(write_offset, 16) * 16;
			for (uint64_t pad_index = write_offset; pad_index < write_offset_padded; pad_index++) {
				const bool use_spaces = (_compression_format == COMPRESSION_FORMAT_NONE && buffer_index == 0);
				*(file_bytes_ptrw + write_offset) = use_spaces ? (uint8_t)0x20 : (uint8_t)0x00; // Pad with zero bytes or spaces.
				write_offset++;
			}
			const PackedByteArray buffer_compressed = buffers_compressed[buffer_index];
			const uint64_t buffer_compressed_size = buffer_compressed.size();
			*(uint32_t *)(file_bytes_ptrw + write_offset) = (uint32_t)0x424F4C42; // "BLOB"
			write_offset += 4;
			*(uint32_t *)(file_bytes_ptrw + write_offset) = compression_format_indicator;
			write_offset += 4;
			*(uint64_t *)(file_bytes_ptrw + write_offset) = buffer_compressed_size; // Buffer size after compression.
			write_offset += 8;
			memcpy(file_bytes_ptrw + write_offset, buffer_compressed.ptr(), buffer_compressed_size);
			write_offset += buffer_compressed_size;
		}
	}
	ERR_FAIL_COND_V(write_offset != total_file_size, PackedByteArray());
	return file_bytes;
}

// Import process.

// Internal helper function. Expects the read offset to be at the start of a chunk, will crash otherwise.
PackedByteArray G4MFDocument4D::_import_next_chunk_bytes_uncompressed(Ref<G4MFState4D> p_g4mf_state, const uint8_t *p_file_bytes, const uint64_t p_file_size, size_t &p_read_offset) {
	CRASH_COND(p_read_offset % 16 != 0);
	PackedByteArray chunk_raw_data;
	if (unlikely(p_read_offset + 16 > p_file_size)) {
		p_read_offset = p_file_size;
		ERR_FAIL_V_MSG(chunk_raw_data, "G4MF import: Not enough bytes are left to form a chunk header. File is corrupted.");
	}
	p_read_offset += 4; // Skip chunk type. The caller can check this if needed.
	const uint32_t chunk_compression_indicator = *(uint32_t *)(p_file_bytes + p_read_offset);
	p_read_offset += 4;
	const uint64_t chunk_raw_size = *(uint64_t *)(p_file_bytes + p_read_offset);
	p_read_offset += 8;
	ERR_FAIL_COND_V_MSG(p_read_offset + chunk_raw_size > p_file_size, chunk_raw_data, "G4MF import: Chunk size goes past end of file. File is corrupted.");
	chunk_raw_data.resize(chunk_raw_size);
	memcpy(chunk_raw_data.ptrw(), p_file_bytes + p_read_offset, chunk_raw_size);
	p_read_offset += chunk_raw_size;
	p_read_offset = _ceiling_division(p_read_offset, 16) * 16; // Chunks start at 16-byte boundaries.
	// Done reading the chunk header and raw data. Now to uncompress it, if needed.
	return _import_decompress_bytes(chunk_raw_data, chunk_compression_indicator);
};

PackedByteArray G4MFDocument4D::_import_decompress_bytes(const PackedByteArray &p_raw_compressed_data, const uint32_t p_compression_indicator) {
	const CompressionFormat chunk_compression_format = _compression_indicator_to_format(p_compression_indicator);
	const int64_t chunk_raw_size = p_raw_compressed_data.size();
	switch (chunk_compression_format) {
		case COMPRESSION_FORMAT_UNKNOWN:
			break;
		case COMPRESSION_FORMAT_NONE:
			return p_raw_compressed_data;
		case COMPRESSION_FORMAT_ZSTD: {
			PackedByteArray decompressed;
			if (chunk_raw_size > 0) {
#if GDEXTENSION
				// We have to guess the decompressed size. Zstd usually has a ratio of 3 or 4, so 8 is very likely to succeed. Else, try bigger ratios.
				for (uint64_t ratio = 8; ratio < uint64_t(1 << 20); ratio *= 2) {
					const uint64_t decompressed_size_guess = chunk_raw_size * ratio;
					decompressed.resize(decompressed_size_guess);
					constexpr FileAccess::CompressionMode mode = FileAccess::CompressionMode::COMPRESSION_ZSTD;
					decompressed = p_raw_compressed_data.decompress(decompressed_size_guess, mode);
					if (decompressed.size() != 0) {
						break;
					}
				}
				ERR_FAIL_COND_V(decompressed.size() == 0, decompressed);
#elif GODOT_MODULE
				int result = 0;
				// We have to guess the decompressed size. Zstd usually has a ratio of 3 or 4, so 8 is very likely to succeed. Else, try bigger ratios.
				for (uint64_t ratio = 8; ratio < uint64_t(1 << 20); ratio *= 2) {
					const uint64_t decompressed_size_guess = chunk_raw_size * ratio;
					decompressed.resize(decompressed_size_guess);
					constexpr Compression::Mode mode = Compression::Mode::MODE_ZSTD;
					result = Compression::decompress(decompressed.ptrw(), decompressed_size_guess, p_raw_compressed_data.ptr(), chunk_raw_size, mode);
					if (result >= 0) {
						break;
					}
				}
				ERR_FAIL_COND_V(result < 0, PackedByteArray());
				decompressed.resize(result);
#endif
			}
			return decompressed;
		}
	}
	const String friendly = _uint32_to_string(p_compression_indicator, false);
	const String number = String::num_uint64(p_compression_indicator, 16, true);
	ERR_FAIL_V_MSG(PackedByteArray(), "G4MF import: Support for reading \"" + friendly + "\" (0x" + number + ") compressed data is not implemented.");
}

String G4MFDocument4D::_uint32_to_string(uint32_t p_value, const bool p_allow_and_escape_non_ascii) {
	String str = "";
	for (int i = 0; i < 4; i++) {
		const uint8_t low_byte = (uint8_t)p_value;
		if (low_byte > 0x1F && low_byte < 0x7F) {
			str += (char32_t)low_byte;
		} else if (p_allow_and_escape_non_ascii) {
			str += "\\u00" + String::num_uint64(low_byte, 16, true).pad_zeros(2);
		} else {
			str += (char32_t)'?';
		}
		p_value >>= 8;
	}
	return str;
}

uint32_t G4MFDocument4D::_string_to_uint32(const String &p_value) {
	ERR_FAIL_COND_V_MSG(p_value.length() != 4, 0, "G4MF import: String to uint32 conversion expects a 4-character string.");
	uint32_t value = 0;
	for (int i = 0; i < 4; i++) {
		const uint8_t low_byte = (uint8_t)p_value[i];
		value |= low_byte << (i * 8);
	}
	return value;
}

Error G4MFDocument4D::_import_parse_json_data(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json) {
	Error err = _import_parse_asset_header(p_g4mf_state, p_g4mf_json);
	ERR_FAIL_COND_V_MSG(err != OK, err, "G4MF import: Failed to parse asset header. Aborting file import.");
	err = _import_parse_buffers_accessors(p_g4mf_state, p_g4mf_json);
	ERR_FAIL_COND_V_MSG(err != OK, err, "G4MF import: Failed to parse buffers and/or accessors. Aborting file import.");
	err = _import_parse_textures(p_g4mf_state, p_g4mf_json);
	ERR_FAIL_COND_V_MSG(err != OK, err, "G4MF import: Failed to parse textures. Aborting file import.");
	err = _import_parse_materials(p_g4mf_state, p_g4mf_json);
	ERR_FAIL_COND_V_MSG(err != OK, err, "G4MF import: Failed to parse materials. Aborting file import.");
	err = _import_parse_meshes(p_g4mf_state, p_g4mf_json);
	ERR_FAIL_COND_V_MSG(err != OK, err, "G4MF import: Failed to parse meshes. Aborting file import.");
	err = _import_parse_models(p_g4mf_state, p_g4mf_json);
	ERR_FAIL_COND_V_MSG(err != OK, err, "G4MF import: Failed to parse models. Aborting file import.");
	err = _import_parse_shapes(p_g4mf_state, p_g4mf_json);
	ERR_FAIL_COND_V_MSG(err != OK, err, "G4MF import: Failed to parse shapes. Aborting file import.");
	err = _import_parse_nodes(p_g4mf_state, p_g4mf_json);
	return err;
}

Error G4MFDocument4D::_import_parse_asset_header(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json) {
	ERR_FAIL_COND_V_MSG(!p_g4mf_json.has("asset"), ERR_INVALID_DATA, "G4MF import: Missing required 'asset' header. Aborting file import.");
	const Dictionary asset_header = p_g4mf_json["asset"];
	ERR_FAIL_COND_V_MSG(!asset_header.has("dimension"), ERR_INVALID_DATA, "G4MF import: Missing required asset header field 'dimension'. Aborting file import.");
	int dimension = asset_header["dimension"];
	p_g4mf_state->set_declared_dimension(dimension);
	if (dimension != 4) {
#if GDEXTENSION
		const bool can_import_nd = ClassDBSingleton::get_singleton()->class_exists("NodeND");
#elif GODOT_MODULE
		const bool can_import_nd = ClassDB::class_exists("NodeND");
#endif
		if (can_import_nd) {
			WARN_PRINT("G4MF import: Trying to import a non-4D G4MF as 4D. This is allowed but may not work correctly. Consider changing the import type to 'ND Scene' in the Import dock.");
		} else {
			WARN_PRINT("G4MF import: Trying to import a non-4D G4MF as 4D. This is allowed but may not work correctly.");
		}
	}
	return OK;
}

Error G4MFDocument4D::_import_parse_buffers_accessors(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json) {
	p_g4mf_state->set_external_data_mode(G4MFState4D::EXTERNAL_DATA_MODE_EMBED_EVERYTHING);
	if (p_g4mf_json.has("buffers")) {
		const Array json_buffers = p_g4mf_json["buffers"];
		const int buffer_count = json_buffers.size();
		if (buffer_count == 0) {
			return OK; // No buffers to parse.
		}
		TypedArray<PackedByteArray> buffers = p_g4mf_state->get_g4mf_buffers();
		if (buffers.size() < buffer_count) {
			buffers.resize(buffer_count);
		}
		for (int i = 0; i < buffer_count; i++) {
			const Dictionary json_buffer = json_buffers[i];
			ERR_FAIL_COND_V_MSG(!json_buffer.has("byteLength"), ERR_INVALID_DATA, "G4MF import: Buffer is missing required field 'byteLength'. Aborting file import.");
			const int64_t byte_length = json_buffer["byteLength"];
			uint32_t compression_indicator = 0;
			if (json_buffer.has("compression")) {
				const String compression_str = json_buffer["compression"];
				compression_indicator = _string_to_uint32(compression_str);
			}
			PackedByteArray buffer = buffers[i];
			if (json_buffer.has("uri")) {
				const String uri = json_buffer["uri"];
				if (uri.begins_with("data:")) {
					PackedStringArray split = uri.split(";base64,", true, 1);
					ERR_FAIL_COND_V_MSG(split.size() != 2, ERR_INVALID_DATA, "G4MF import: Buffer URI is malformed. Expected 'data:application/octet-stream;base64,<base64 data>'. Aborting file import.");
					buffer = _import_decompress_bytes(Marshalls::get_singleton()->base64_to_raw(split[1]), compression_indicator);
				} else {
					// Infer the external data mode on import in case the user wishes to round-trip the G4MF file back out of Godot later.
					p_g4mf_state->set_external_data_mode(G4MFState4D::EXTERNAL_DATA_MODE_SEPARATE_BINARY_BLOBS);
					const String buffer_path = p_g4mf_state->get_g4mf_base_path().path_join(uri);
					Ref<FileAccess> file = FileAccess::open(buffer_path, FileAccess::READ);
					if (file.is_valid()) {
						buffer = _import_decompress_bytes(file->get_buffer(byte_length), compression_indicator);
						file->close();
					} else {
						// The file is not valid, but only fail if the buffer is not empty. It may have been filled by a chunk.
						ERR_FAIL_COND_V_MSG(buffer.is_empty(), ERR_INVALID_DATA, "G4MF import: Failed to open buffer file: " + buffer_path + ". Aborting file import.");
					}
				}
			}
			if (buffer.size() < byte_length) {
				ERR_FAIL_COND_V_MSG(buffer.is_empty(), ERR_INVALID_DATA, "G4MF import: Failed to read buffer data. Aborting file import.");
				ERR_FAIL_V_MSG(ERR_INVALID_DATA, "G4MF import: Buffer size is not at least the declared size. Aborting file import.");
			}
			buffer.resize(byte_length);
			buffers[i] = buffer;
		}
		p_g4mf_state->set_g4mf_buffers(buffers);
	}
	if (p_g4mf_json.has("bufferViews")) {
		Array json_buffer_views = p_g4mf_json["bufferViews"];
		const int buffer_view_count = json_buffer_views.size();
		if (buffer_view_count == 0) {
			return OK; // No buffer views to parse.
		}
		TypedArray<G4MFBufferView4D> buffer_views;
		buffer_views.resize(buffer_view_count);
		for (int i = 0; i < buffer_view_count; i++) {
			const Dictionary json_buffer_view = json_buffer_views[i];
			Ref<G4MFBufferView4D> buffer_view = G4MFBufferView4D::from_dictionary(json_buffer_view);
			ERR_FAIL_COND_V_MSG(buffer_view.is_null(), ERR_INVALID_DATA, "G4MF import: Failed to parse buffer view. Aborting file import.");
			buffer_views[i] = buffer_view;
		}
		p_g4mf_state->set_g4mf_buffer_views(buffer_views);
	}
	if (p_g4mf_json.has("accessors")) {
		Array json_accessors = p_g4mf_json["accessors"];
		const int accessor_count = json_accessors.size();
		if (accessor_count == 0) {
			return OK; // No accessors to parse.
		}
		TypedArray<G4MFAccessor4D> accessors;
		accessors.resize(accessor_count);
		for (int i = 0; i < accessor_count; i++) {
			const Dictionary json_accessor = json_accessors[i];
			Ref<G4MFAccessor4D> accessor = G4MFAccessor4D::from_dictionary(json_accessor);
			ERR_FAIL_COND_V_MSG(accessor.is_null(), ERR_INVALID_DATA, "G4MF import: Failed to parse accessor. Aborting file import.");
			accessors[i] = accessor;
		}
		p_g4mf_state->set_g4mf_accessors(accessors);
	}
	return OK;
}

Error G4MFDocument4D::_import_parse_textures(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json) {
	if (!p_g4mf_json.has("textures")) {
		return OK; // No textures to parse.
	}
	Array json_textures = p_g4mf_json["textures"];
	const int texture_count = json_textures.size();
	if (texture_count == 0) {
		return OK; // No textures to parse.
	}
	TypedArray<G4MFTexture4D> g4mf_textures = p_g4mf_state->get_g4mf_textures();
	g4mf_textures.resize(texture_count);
	for (int i = 0; i < texture_count; i++) {
		const Dictionary file_texture = json_textures[i];
		Ref<G4MFTexture4D> g4mf_texture = G4MFTexture4D::from_dictionary(file_texture);
		ERR_FAIL_COND_V_MSG(g4mf_texture.is_null(), ERR_INVALID_DATA, "G4MF import: Failed to parse texture. Aborting file import.");
		g4mf_textures[i] = g4mf_texture;
	}
	p_g4mf_state->set_g4mf_textures(g4mf_textures);
	return OK;
}

Error G4MFDocument4D::_import_parse_materials(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json) {
	if (!p_g4mf_json.has("materials")) {
		return OK; // No materials to parse.
	}
	Array json_materials = p_g4mf_json["materials"];
	const int material_count = json_materials.size();
	if (material_count == 0) {
		return OK; // No materials to parse.
	}
	TypedArray<G4MFMaterial4D> g4mf_materials = p_g4mf_state->get_g4mf_materials();
	g4mf_materials.resize(material_count);
	for (int i = 0; i < material_count; i++) {
		const Dictionary file_material = json_materials[i];
		Ref<G4MFMaterial4D> g4mf_material = G4MFMaterial4D::from_dictionary(file_material);
		ERR_FAIL_COND_V_MSG(g4mf_material.is_null(), ERR_INVALID_DATA, "G4MF import: Failed to parse material. Aborting file import.");
		g4mf_materials[i] = g4mf_material;
	}
	p_g4mf_state->set_g4mf_materials(g4mf_materials);
	return OK;
}

Error G4MFDocument4D::_import_parse_meshes(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json) {
	if (!p_g4mf_json.has("meshes")) {
		return OK; // No meshes to parse.
	}
	Array json_meshes = p_g4mf_json["meshes"];
	const int json_mesh_count = json_meshes.size();
	if (json_mesh_count == 0) {
		return OK; // No meshes to parse.
	}
	TypedArray<G4MFMesh4D> g4mf_meshes = p_g4mf_state->get_g4mf_meshes();
	g4mf_meshes.resize(json_mesh_count);
	for (int i = 0; i < json_mesh_count; i++) {
		const Dictionary file_mesh = json_meshes[i];
		Ref<G4MFMesh4D> g4mf_mesh = G4MFMesh4D::from_dictionary(file_mesh);
		ERR_FAIL_COND_V_MSG(g4mf_mesh.is_null(), ERR_INVALID_DATA, "G4MF import: Failed to parse mesh. Aborting file import.");
		g4mf_meshes[i] = g4mf_mesh;
	}
	p_g4mf_state->set_g4mf_meshes(g4mf_meshes);
	return OK;
}

Error G4MFDocument4D::_import_parse_models(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json) {
	if (!p_g4mf_json.has("models")) {
		return OK; // No models to parse.
	}
	Array json_models = p_g4mf_json["models"];
	const int model_count = json_models.size();
	if (model_count == 0) {
		return OK; // No models to parse.
	}
	TypedArray<G4MFModel4D> g4mf_models = p_g4mf_state->get_g4mf_models();
	g4mf_models.resize(model_count);
	Error err;
	for (int i = 0; i < model_count; i++) {
		const Dictionary file_model = json_models[i];
		Ref<G4MFModel4D> g4mf_model = G4MFModel4D::from_dictionary(file_model);
		g4mf_model->set_model_g4mf_document(this);
		ERR_FAIL_COND_V_MSG(g4mf_model.is_null(), ERR_INVALID_DATA, "G4MF import: Failed to parse model JSON. Aborting file import.");
		err = g4mf_model->import_preload_model_data(p_g4mf_state);
		ERR_FAIL_COND_V_MSG(err != OK, err, "G4MF import: Failed to preload model data. Aborting file import.");
		g4mf_models[i] = g4mf_model;
	}
	p_g4mf_state->set_g4mf_models(g4mf_models);
	return OK;
}

Error G4MFDocument4D::_import_parse_shapes(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json) {
	if (!p_g4mf_json.has("shapes")) {
		return OK; // No shapes to parse.
	}
	Array json_shapes = p_g4mf_json["shapes"];
	const int shape_count = json_shapes.size();
	if (shape_count == 0) {
		return OK; // No shapes to parse.
	}
	TypedArray<G4MFShape4D> g4mf_shapes = p_g4mf_state->get_g4mf_shapes();
	g4mf_shapes.resize(shape_count);
	for (int i = 0; i < shape_count; i++) {
		const Dictionary file_shape = json_shapes[i];
		Ref<G4MFShape4D> g4mf_shape = G4MFShape4D::from_dictionary(file_shape);
		ERR_FAIL_COND_V_MSG(g4mf_shape.is_null(), ERR_INVALID_DATA, "G4MF import: Failed to parse shape. Aborting file import.");
		g4mf_shapes[i] = g4mf_shape;
	}
	p_g4mf_state->set_g4mf_shapes(g4mf_shapes);
	return OK;
}

Error G4MFDocument4D::_import_parse_nodes(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json) {
	if (!p_g4mf_json.has("nodes")) {
		return OK; // No nodes is valid, just not a scene.
	}
	Array file_nodes = p_g4mf_json["nodes"];
	const int file_node_count = file_nodes.size();
	if (file_node_count == 0) {
		return OK; // No nodes is valid, just not a scene.
	}
	// Initial parsing of nodes.
	TypedArray<G4MFNode4D> g4mf_nodes;
	g4mf_nodes.resize(file_node_count);
	for (int i = 0; i < file_node_count; i++) {
		Ref<G4MFNode4D> g4mf_node = G4MFNode4D::from_dictionary(file_nodes[i]);
		g4mf_nodes[i] = g4mf_node;
	}
	// Assign parent_index.
	for (int parent_index = 0; parent_index < file_node_count; parent_index++) {
		Ref<G4MFNode4D> g4mf_node = g4mf_nodes[parent_index];
		const PackedInt32Array children_indices = g4mf_node->get_children_indices();
		for (int32_t child_index : children_indices) {
			ERR_FAIL_INDEX_V(child_index, file_node_count, ERR_INVALID_DATA);
			Ref<G4MFNode4D> child_node = g4mf_nodes[child_index];
			child_node->set_parent_index(parent_index);
		}
	}
	// TODO: Extensions.
	Ref<G4MFNode4D> root_node = g4mf_nodes[0];
	root_node->set_transform(Transform4D());
	p_g4mf_state->set_g4mf_nodes(g4mf_nodes);
	return OK;
}

Node *G4MFDocument4D::_import_generate_scene_node(Ref<G4MFState4D> p_g4mf_state, const int p_node_index, Node *p_scene_parent, Node *p_scene_root) {
	TypedArray<G4MFNode4D> state_g4mf_nodes = p_g4mf_state->get_g4mf_nodes();
	ERR_FAIL_INDEX_V(p_node_index, state_g4mf_nodes.size(), nullptr);
	Ref<G4MFNode4D> g4mf_node = state_g4mf_nodes[p_node_index];
	Node *godot_node = nullptr;
	// First, check if any extension wants to generate a node.
	// If no extension generated a node, check the built-in types.
	if (godot_node == nullptr) {
		godot_node = g4mf_node->generate_godot_node(p_g4mf_state, p_scene_parent, _force_wireframe);
		if (godot_node == nullptr) {
			godot_node = memnew(Node4D);
		}
	}
	// Set up the generated Godot node.
	g4mf_node->apply_to_godot_node(godot_node);
	TypedArray<Node4D> state_godot_nodes = p_g4mf_state->get_godot_nodes();
	state_godot_nodes[p_node_index] = godot_node;
	// Add the Godot node to the tree and set the owner.
	if (p_scene_parent) {
		p_scene_parent->add_child(godot_node);
		godot_node->set_owner(p_scene_root);
	} else {
		Node4D *godot_node_4d = Object::cast_to<Node4D>(godot_node);
		if (godot_node_4d != nullptr) {
			// The root node MUST NOT have a transform as required by the G4MF specification.
			godot_node_4d->set_transform(Transform4D());
		}
		p_scene_parent = godot_node;
		p_scene_root = godot_node;
	}
	// If multiple nodes were generated under this node, ensure they have the owner set.
	if (unlikely(godot_node->get_child_count() > 0 && godot_node->get_scene_file_path().is_empty())) {
		Array args;
		args.append(p_scene_root);
		for (int i = 0; i < godot_node->get_child_count(); i++) {
			Node *child = godot_node->get_child(i);
			child->propagate_call(StringName("set_owner"), args);
		}
	}
	// Generate children.
	const PackedInt32Array children_indices = g4mf_node->get_children_indices();
	for (int32_t child_index : children_indices) {
		_import_generate_scene_node(p_g4mf_state, child_index, godot_node, p_scene_root);
	}
	return godot_node;
}

Ref<Mesh4D> G4MFDocument4D::_import_generate_combined_mesh(const Ref<G4MFState4D> p_g4mf_state, const bool p_include_invisible) {
	const TypedArray<G4MFMesh4D> state_g4mf_meshes = p_g4mf_state->get_g4mf_meshes();
	const int mesh_count = state_g4mf_meshes.size();
	const TypedArray<G4MFNode4D> state_g4mf_nodes = p_g4mf_state->get_g4mf_nodes();
	const int node_count = state_g4mf_nodes.size();
	bool wireframe = _force_wireframe;
	if (!_force_wireframe) {
		for (int i = 0; i < mesh_count; i++) {
			Ref<G4MFMesh4D> g4mf_mesh = state_g4mf_meshes[i];
			if (!g4mf_mesh->can_generate_tetra_meshes_for_all_surfaces()) {
				wireframe = true;
				break;
			}
		}
	}
	if (wireframe) {
		Ref<ArrayWireMesh4D> combined_wire_mesh;
		combined_wire_mesh.instantiate();
		for (int i = 0; i < node_count; i++) {
			Ref<G4MFNode4D> g4mf_node = state_g4mf_nodes[i];
			const int mesh_index = g4mf_node->get_mesh_index();
			if (mesh_index < 0) {
				continue;
			}
			if (!(p_include_invisible || g4mf_node->get_visible())) {
				continue;
			}
			ERR_FAIL_INDEX_V(mesh_index, mesh_count, Ref<Mesh4D>());
			Ref<G4MFMesh4D> g4mf_mesh = state_g4mf_meshes[mesh_index];
			Ref<ArrayWireMesh4D> mesh = g4mf_mesh->generate_mesh(p_g4mf_state, wireframe);
			if (mesh.is_valid()) {
				combined_wire_mesh->merge_with(mesh, g4mf_node->get_scene_global_transform(p_g4mf_state));
			}
		}
		return combined_wire_mesh;
	}
	Ref<ArrayTetraMesh4D> combined_tetra_mesh;
	combined_tetra_mesh.instantiate();
	for (int i = 0; i < node_count; i++) {
		Ref<G4MFNode4D> g4mf_node = state_g4mf_nodes[i];
		const int mesh_index = g4mf_node->get_mesh_index();
		if (mesh_index < 0) {
			continue;
		}
		if (!(p_include_invisible || g4mf_node->get_visible())) {
			continue;
		}
		ERR_FAIL_INDEX_V(mesh_index, mesh_count, Ref<Mesh4D>());
		Ref<G4MFMesh4D> g4mf_mesh = state_g4mf_meshes[mesh_index];
		Ref<ArrayTetraMesh4D> mesh = g4mf_mesh->generate_mesh(p_g4mf_state, wireframe);
		if (mesh.is_valid()) {
			combined_tetra_mesh->merge_with(mesh, g4mf_node->get_scene_global_transform(p_g4mf_state));
		}
	}
	return combined_tetra_mesh;
}

// Public functions.

Error G4MFDocument4D::export_append_from_godot_scene(Ref<G4MFState4D> p_g4mf_state, Node *p_scene_root) {
	ERR_FAIL_COND_V_MSG(p_scene_root == nullptr, ERR_INVALID_PARAMETER, "G4MF export: Cannot export a scene without a root node.");
	p_g4mf_state->set_original_path(p_scene_root->get_scene_file_path());
	return _export_convert_scene_node(p_g4mf_state, p_scene_root, -1);
}

Error G4MFDocument4D::export_append_from_godot_mesh(Ref<G4MFState4D> p_g4mf_state, const Ref<Mesh4D> &p_mesh) {
	ERR_FAIL_COND_V_MSG(p_mesh.is_null(), ERR_INVALID_PARAMETER, "G4MF export: Cannot export a null mesh.");
	const int mesh_index = G4MFMesh4D::convert_mesh_into_state(p_g4mf_state, p_mesh, p_mesh->get_material(), true);
	ERR_FAIL_COND_V_MSG(mesh_index < 0, ERR_INVALID_PARAMETER, "G4MF export: Failed to convert mesh into G4MF state.");
	return OK;
}

PackedByteArray G4MFDocument4D::export_write_to_byte_array(Ref<G4MFState4D> p_g4mf_state) {
	Error err = _export_serialize_json_data(p_g4mf_state);
	ERR_FAIL_COND_V_MSG(err != OK, PackedByteArray(), "G4MF export: Failed to serialize G4MF data.");
	return _export_encode_as_byte_array(p_g4mf_state);
}

Error G4MFDocument4D::export_write_to_file(Ref<G4MFState4D> p_g4mf_state, const String &p_path) {
	Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::WRITE);
	ERR_FAIL_COND_V_MSG(file.is_null(), ERR_CANT_OPEN, "G4MF export: Failed to open file for writing.");
	p_g4mf_state->set_g4mf_base_path(p_path.get_base_dir());
	p_g4mf_state->set_g4mf_filename(p_path.get_file());
	p_g4mf_state->set_original_path(p_path);
	Error err = _export_serialize_json_data(p_g4mf_state);
	ERR_FAIL_COND_V_MSG(err != OK, err, "G4MF export: Failed to serialize G4MF data.");
	const int64_t buffer0_size = p_g4mf_state->get_g4mf_buffers().size() > 0 ? PackedByteArray(p_g4mf_state->get_g4mf_buffers()[0]).size() : 0;
	const bool is_text_file = p_g4mf_state->is_text_file();
	const bool should_separate_buffers_into_files = p_g4mf_state->should_separate_binary_blobs(buffer0_size);
	if (is_text_file) {
		// Write to a G4MF text file. Export the buffers either as base64 or as separate files.
		err = _export_serialize_buffer_data(p_g4mf_state, should_separate_buffers_into_files);
		ERR_FAIL_COND_V_MSG(err != OK, err, "G4MF export: Failed to serialize G4MF buffer data.");
		const Dictionary g4mf_json = p_g4mf_state->get_g4mf_json();
		const String json_string = _export_pretty_print_json(g4mf_json);
		file->store_string(json_string);
	} else {
		// Write to a G4MF binary file. Export the buffers as binary blob chunks or as separate files.
		if (should_separate_buffers_into_files) {
			err = _export_serialize_buffer_data(p_g4mf_state, true);
		}
		const PackedByteArray json_bytes = _export_encode_as_byte_array(p_g4mf_state);
		file->store_buffer(json_bytes);
	}
	return OK;
}

Error G4MFDocument4D::import_read_from_byte_array(Ref<G4MFState4D> p_g4mf_state, const PackedByteArray &p_byte_array) {
	const uint64_t byte_array_size = p_byte_array.size();
	ERR_FAIL_COND_V_MSG(byte_array_size < (uint64_t)32, ERR_INVALID_DATA, "G4MF import: Byte array is too small to be a valid G4MF file.");
	const uint8_t *byte_array_ptr = p_byte_array.ptr();
	ERR_FAIL_COND_V_MSG(*(uint32_t *)byte_array_ptr != (uint32_t)0x464D3447, ERR_INVALID_DATA, "G4MF import: Byte array is not a valid G4MF file.");
	size_t read_offset = 4;
	ERR_FAIL_COND_V_MSG(*(uint32_t *)(byte_array_ptr + read_offset) > (uint32_t)0x00000000, ERR_UNAVAILABLE, "G4MF import: G4MF file version is not supported.");
	read_offset += 4;
	const uint64_t file_size = *(uint64_t *)(byte_array_ptr + read_offset);
	ERR_FAIL_COND_V_MSG(file_size != byte_array_size, ERR_INVALID_DATA, "G4MF import: Declared file size does not match actual file size. File is corrupted.");
	read_offset += 8;
	// Read the chunks.
	ERR_FAIL_COND_V_MSG(*(uint32_t *)(byte_array_ptr + read_offset) != (uint32_t)0x4E4F534A, ERR_INVALID_DATA, "G4MF import: First chunk is not JSON. File is corrupted.");
	const PackedByteArray json_chunk = _import_next_chunk_bytes_uncompressed(p_g4mf_state, byte_array_ptr, byte_array_size, read_offset);
	ERR_FAIL_COND_V_MSG(json_chunk.is_empty(), ERR_INVALID_DATA, "G4MF import: Failed to read JSON chunk. File may be corrupted or uses unsupported compression.");
	TypedArray<PackedByteArray> blob_chunks;
	while (read_offset < byte_array_size) {
		const uint32_t chunk_type = *(uint32_t *)(byte_array_ptr + read_offset);
		if (chunk_type != (uint32_t)0x424F4C42) {
			break; // Not a "BLOB" chunk. All "BLOB" chunks MUST come first according to the specification, so there must not be any more.
		}
		const PackedByteArray blob_chunk = _import_next_chunk_bytes_uncompressed(p_g4mf_state, byte_array_ptr, byte_array_size, read_offset);
		blob_chunks.append(blob_chunk);
	}
	// Technically, the spec allows for non-buffer chunks, but there is no harm in setting those into the buffers array here.
	p_g4mf_state->set_g4mf_buffers(blob_chunks);
	// Parse the data.
	const String json_string = String::utf8(reinterpret_cast<const char *>(json_chunk.ptr()), json_chunk.size());
	Dictionary g4mf_json = JSON::parse_string(json_string);
	return _import_parse_json_data(p_g4mf_state, g4mf_json);
}

Error G4MFDocument4D::import_read_from_file(Ref<G4MFState4D> p_g4mf_state, const String &p_path) {
	Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::READ);
	ERR_FAIL_COND_V_MSG(file.is_null(), ERR_CANT_OPEN, "G4MF import: Failed to open file for reading.");
	p_g4mf_state->set_g4mf_base_path(p_path.get_base_dir());
	p_g4mf_state->set_g4mf_filename(p_path.get_file());
	p_g4mf_state->set_original_path(p_path);
	ERR_FAIL_COND_V_MSG(file->get_length() < 25, ERR_INVALID_DATA, "G4MF import: File is too small to be a valid G4MF file.");
	// Check for the magic number to allow reading G4MF files regardless of file extension.
	const uint32_t magic_number_maybe = file->get_32();
	file->seek(0);
	if (magic_number_maybe == (uint32_t)0x464D3447) {
		const PackedByteArray file_bytes = file->get_buffer(file->get_length());
		return import_read_from_byte_array(p_g4mf_state, file_bytes);
	}
	// If there is no magic number, try reading the file as JSON.
	Dictionary g4mf_json = JSON::parse_string(file->get_as_text(true));
	p_g4mf_state->set_g4mf_json(g4mf_json);
	return _import_parse_json_data(p_g4mf_state, g4mf_json);
}

Node *G4MFDocument4D::import_generate_godot_scene(Ref<G4MFState4D> p_g4mf_state) {
	// Ensure all models have a reference to this document (the working document may have changed).
	TypedArray<G4MFModel4D> state_g4mf_models = p_g4mf_state->get_g4mf_models();
	for (int i = 0; i < state_g4mf_models.size(); i++) {
		Ref<G4MFModel4D> g4mf_model = state_g4mf_models[i];
		ERR_FAIL_COND_V(g4mf_model.is_null(), nullptr);
		g4mf_model->set_model_g4mf_document(this);
	}
	// Check how many nodes we have, and handle the case of zero nodes.
	TypedArray<G4MFNode4D> state_g4mf_nodes = p_g4mf_state->get_g4mf_nodes();
	if (state_g4mf_nodes.is_empty()) {
		// If there are no nodes, we can implicitly generate a MeshInstance4D for the first mesh.
		TypedArray<G4MFMesh4D> state_g4mf_meshes = p_g4mf_state->get_g4mf_meshes();
		const int mesh_count = state_g4mf_meshes.size();
		ERR_FAIL_COND_V_MSG(mesh_count == 0, nullptr, "G4MF import: This G4MF file has no nodes or meshes, so it cannot be imported as a scene.");
		WARN_PRINT("G4MF import: This G4MF file has no nodes. Try importing as a mesh instead (Import dock -> Import As: 4D Mesh).");
		MeshInstance4D *mesh_instance = memnew(MeshInstance4D);
		Ref<G4MFMesh4D> g4mf_mesh = state_g4mf_meshes[0];
		mesh_instance->set_mesh(g4mf_mesh->generate_mesh(p_g4mf_state, _force_wireframe));
		const String mesh_name = g4mf_mesh->get_name();
		if (mesh_name.is_empty()) {
			mesh_instance->set_name(p_g4mf_state->get_g4mf_filename().get_basename());
		} else {
			mesh_instance->set_name(mesh_name);
		}
		return mesh_instance;
	}
	// Prepare the Godot nodes array to be filled during scene node generation.
	TypedArray<Node4D> state_godot_nodes;
	state_godot_nodes.resize(state_g4mf_nodes.size());
	p_g4mf_state->set_godot_nodes(state_godot_nodes);
	// Start from the root node (index 0) which has no parent. This will recursively call itself for child nodes.
	return _import_generate_scene_node(p_g4mf_state, 0, nullptr, nullptr);
}

Ref<Mesh4D> G4MFDocument4D::import_generate_godot_mesh(Ref<G4MFState4D> p_g4mf_state, const int p_which_mesh_index, const bool p_include_invisible) {
	const TypedArray<G4MFMesh4D> state_g4mf_meshes = p_g4mf_state->get_g4mf_meshes();
	const int mesh_count = state_g4mf_meshes.size();
	ERR_FAIL_COND_V_MSG(mesh_count == 0, Ref<Mesh4D>(), "G4MF import: This G4MF file has no meshes, so it cannot be imported as a mesh.");
	if (p_which_mesh_index >= 0) {
		ERR_FAIL_INDEX_V_MSG(p_which_mesh_index, mesh_count, Ref<Mesh4D>(), "G4MF import: Specified mesh index is out of range.");
		Ref<G4MFMesh4D> g4mf_mesh = state_g4mf_meshes[p_which_mesh_index];
		return g4mf_mesh->generate_mesh(p_g4mf_state, _force_wireframe);
	}
	// If p_which_mesh_index is negative (default), generate a combined mesh using the nodes.
	const TypedArray<G4MFNode4D> state_g4mf_nodes = p_g4mf_state->get_g4mf_nodes();
	// If there are no nodes, generate the first mesh. We can't combine
	// in this situation because we don't know the mesh transforms.
	if (state_g4mf_nodes.is_empty()) {
		if (mesh_count > 1) {
			WARN_PRINT("G4MF import: This G4MF file has multiple meshes, but only the first mesh will be imported.");
		}
		Ref<G4MFMesh4D> g4mf_mesh = state_g4mf_meshes[0];
		return g4mf_mesh->generate_mesh(p_g4mf_state, _force_wireframe);
	}
	// If there are nodes, generate a combined mesh.
	return _import_generate_combined_mesh(p_g4mf_state, p_include_invisible);
}

void G4MFDocument4D::set_compression_format(const CompressionFormat p_compression_format) {
	ERR_FAIL_COND_MSG(_compression_format_to_indicator(p_compression_format) == (uint32_t)0xFFFFFFFF, "G4MF: Invalid compression format.");
	_compression_format = p_compression_format;
}

void G4MFDocument4D::_bind_methods() {
	// Main import and export functions.
	ClassDB::bind_method(D_METHOD("export_append_from_godot_scene", "g4mf_state", "scene_root"), &G4MFDocument4D::export_append_from_godot_scene);
	ClassDB::bind_method(D_METHOD("export_append_from_godot_mesh", "g4mf_state", "mesh"), &G4MFDocument4D::export_append_from_godot_mesh);
	ClassDB::bind_method(D_METHOD("export_write_to_byte_array", "g4mf_state"), &G4MFDocument4D::export_write_to_byte_array);
	ClassDB::bind_method(D_METHOD("export_write_to_file", "g4mf_state", "path"), &G4MFDocument4D::export_write_to_file);
	ClassDB::bind_method(D_METHOD("import_read_from_byte_array", "g4mf_state", "byte_array"), &G4MFDocument4D::import_read_from_byte_array);
	ClassDB::bind_method(D_METHOD("import_read_from_file", "g4mf_state", "path"), &G4MFDocument4D::import_read_from_file);
	ClassDB::bind_method(D_METHOD("import_generate_godot_scene", "g4mf_state"), &G4MFDocument4D::import_generate_godot_scene);
	ClassDB::bind_method(D_METHOD("import_generate_godot_mesh", "g4mf_state", "which_mesh_index", "include_invisible"), &G4MFDocument4D::import_generate_godot_mesh, DEFVAL(-1), DEFVAL(false));

	// Settings for the export process.
	ClassDB::bind_method(D_METHOD("get_compression_format"), &G4MFDocument4D::get_compression_format);
	ClassDB::bind_method(D_METHOD("set_compression_format", "compression_format"), &G4MFDocument4D::set_compression_format);
	ClassDB::bind_method(D_METHOD("get_max_nested_scene_depth"), &G4MFDocument4D::get_max_nested_scene_depth);
	ClassDB::bind_method(D_METHOD("set_max_nested_scene_depth", "max_nested_scene_depth"), &G4MFDocument4D::set_max_nested_scene_depth);

	// Settings for the import process.
	ClassDB::bind_method(D_METHOD("get_force_wireframe"), &G4MFDocument4D::get_force_wireframe);
	ClassDB::bind_method(D_METHOD("set_force_wireframe", "force_wireframe"), &G4MFDocument4D::set_force_wireframe);

	// Properties.
	ADD_PROPERTY(PropertyInfo(Variant::INT, "compression_format", PROPERTY_HINT_ENUM, "None,Zstd"), "set_compression_format", "get_compression_format");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_nested_scene_depth", PROPERTY_HINT_ENUM, "Allow Nested:-1,Merge into Single File:0,Merge into Flat Hierarchy:1"), "set_max_nested_scene_depth", "get_max_nested_scene_depth");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "force_wireframe"), "set_force_wireframe", "get_force_wireframe");

	BIND_ENUM_CONSTANT(COMPRESSION_FORMAT_NONE);
	BIND_ENUM_CONSTANT(COMPRESSION_FORMAT_ZSTD);
}
