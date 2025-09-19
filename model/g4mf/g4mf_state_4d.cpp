#include "g4mf_state_4d.h"

int G4MFState4D::append_g4mf_node(Ref<G4MFNode4D> p_node) {
	const String requested_name = p_node->get_name();
	if (!requested_name.is_empty()) {
		p_node->set_name(reserve_unique_name(requested_name));
	}
	const int node_index = _g4mf_nodes.size();
	_g4mf_nodes.push_back(p_node);
	return node_index;
}

int G4MFState4D::get_node_index(const Node4D *p_node) {
	if (p_node == nullptr) {
		return -1;
	}
	for (int i = 0; i < _godot_nodes.size(); i++) {
		Node4D *node = Object::cast_to<Node4D>(_godot_nodes[i]);
		if (node == p_node) {
			return i;
		}
	}
	return -1;
}

String G4MFState4D::reserve_unique_name(const String &p_requested_name) {
	String unique_name = p_requested_name;
	if (_unique_names.has(unique_name)) {
		uint64_t discriminator = 2;
		while (_unique_names.has(unique_name)) {
			unique_name = p_requested_name + String::num_uint64(discriminator);
			discriminator++;
		}
		WARN_PRINT("G4MFState4D: The requested name " + p_requested_name + " is already in use. The name " + unique_name + " will be used instead.");
	}
	_unique_names.insert(unique_name);
	return unique_name;
}

bool G4MFState4D::unreserve_unique_name(const String &p_name) {
	return _unique_names.erase(p_name);
}

bool G4MFState4D::is_text_file() const {
	// Checking `length > 3` handles "g4tf", "g4mf", "json", "g4tf.json", "g4mf.json", etc.
	return _g4mf_filename.get_extension().length() > 3;
}

bool G4MFState4D::should_separate_binary_blobs(const int64_t p_blob_size) const {
	if (_external_data_mode == EXTERNAL_DATA_MODE_SEPARATE_ALL_FILES || _external_data_mode == EXTERNAL_DATA_MODE_SEPARATE_BINARY_BLOBS) {
		ERR_FAIL_COND_V_MSG(_g4mf_base_path.is_empty(), false, "G4MF: No base path is set, cannot separate binary blob files.");
		ERR_FAIL_COND_V_MSG(_g4mf_filename.get_basename().is_empty(), false, "G4MF: No filename is set, cannot separate binary blob files.");
		return true;
	}
	if (_external_data_mode == EXTERNAL_DATA_MODE_EMBED_EVERYTHING || _external_data_mode == EXTERNAL_DATA_MODE_SEPARATE_RESOURCE_FILES) {
		return false;
	}
	// EXTERNAL_DATA_MODE_AUTOMATIC embeds everything for binary files (.g4b),
	// byte arrays in memory (no extension), when there is no base path set,
	// and when the size is below an arbitrary threshold (we'll use 2 KiB),
	// but separates for text files (.g4tf) when the base path and filename are valid.
	if (p_blob_size < 2048) {
		return false;
	}
	return is_text_file() && !_g4mf_base_path.is_empty() && !_g4mf_filename.get_basename().is_empty();
}

bool G4MFState4D::should_separate_resource_files() const {
	if (_external_data_mode == EXTERNAL_DATA_MODE_SEPARATE_ALL_FILES || _external_data_mode == EXTERNAL_DATA_MODE_SEPARATE_RESOURCE_FILES) {
		ERR_FAIL_COND_V_MSG(_g4mf_base_path.is_empty(), false, "G4MF: No base path is set, cannot separate resource files.");
		ERR_FAIL_COND_V_MSG(_g4mf_filename.get_basename().is_empty(), false, "G4MF: No filename is set, cannot separate resource files.");
		return true;
	}
	if (_external_data_mode == EXTERNAL_DATA_MODE_EMBED_EVERYTHING || _external_data_mode == EXTERNAL_DATA_MODE_SEPARATE_BINARY_BLOBS) {
		return false;
	}
	return is_text_file() && !_g4mf_base_path.is_empty() && !_g4mf_filename.get_basename().is_empty();
}

bool G4MFState4D::should_separate_model_files() const {
	// For model files, we separate them in all modes except when embedding everything or if
	// we are forced to embed everything because the base path is empty (nowhere to put files).
	if (_external_data_mode == EXTERNAL_DATA_MODE_EMBED_EVERYTHING) {
		return false;
	}
	return !_g4mf_base_path.is_empty();
}

void G4MFState4D::_bind_methods() {
	// Data for the contents of the file.
	ClassDB::bind_method(D_METHOD("get_g4mf_json"), &G4MFState4D::get_g4mf_json);
	ClassDB::bind_method(D_METHOD("set_g4mf_json", "g4mf_json"), &G4MFState4D::set_g4mf_json);
	ClassDB::bind_method(D_METHOD("get_g4mf_accessors"), &G4MFState4D::get_g4mf_accessors);
	ClassDB::bind_method(D_METHOD("set_g4mf_accessors", "g4mf_accessors"), &G4MFState4D::set_g4mf_accessors);
	ClassDB::bind_method(D_METHOD("get_g4mf_buffer_views"), &G4MFState4D::get_g4mf_buffer_views);
	ClassDB::bind_method(D_METHOD("set_g4mf_buffer_views", "g4mf_buffer_views"), &G4MFState4D::set_g4mf_buffer_views);
	ClassDB::bind_method(D_METHOD("get_g4mf_buffers"), &G4MFState4D::get_g4mf_buffers);
	ClassDB::bind_method(D_METHOD("set_g4mf_buffers", "g4mf_buffers"), &G4MFState4D::set_g4mf_buffers);
	ClassDB::bind_method(D_METHOD("get_g4mf_textures"), &G4MFState4D::get_g4mf_textures);
	ClassDB::bind_method(D_METHOD("set_g4mf_textures", "g4mf_textures"), &G4MFState4D::set_g4mf_textures);
	ClassDB::bind_method(D_METHOD("get_g4mf_materials"), &G4MFState4D::get_g4mf_materials);
	ClassDB::bind_method(D_METHOD("set_g4mf_materials", "g4mf_materials"), &G4MFState4D::set_g4mf_materials);
	ClassDB::bind_method(D_METHOD("get_g4mf_meshes"), &G4MFState4D::get_g4mf_meshes);
	ClassDB::bind_method(D_METHOD("set_g4mf_meshes", "g4mf_meshes"), &G4MFState4D::set_g4mf_meshes);
	ClassDB::bind_method(D_METHOD("get_g4mf_models"), &G4MFState4D::get_g4mf_models);
	ClassDB::bind_method(D_METHOD("set_g4mf_models", "g4mf_models"), &G4MFState4D::set_g4mf_models);
	ClassDB::bind_method(D_METHOD("get_g4mf_shapes"), &G4MFState4D::get_g4mf_shapes);
	ClassDB::bind_method(D_METHOD("set_g4mf_shapes", "g4mf_shapes"), &G4MFState4D::set_g4mf_shapes);
	ClassDB::bind_method(D_METHOD("get_g4mf_lights"), &G4MFState4D::get_g4mf_lights);
	ClassDB::bind_method(D_METHOD("set_g4mf_lights", "g4mf_lights"), &G4MFState4D::set_g4mf_lights);
	ClassDB::bind_method(D_METHOD("get_g4mf_nodes"), &G4MFState4D::get_g4mf_nodes);
	ClassDB::bind_method(D_METHOD("set_g4mf_nodes", "g4mf_nodes"), &G4MFState4D::set_g4mf_nodes);
	ClassDB::bind_method(D_METHOD("get_godot_nodes"), &G4MFState4D::get_godot_nodes);
	ClassDB::bind_method(D_METHOD("set_godot_nodes", "godot_nodes"), &G4MFState4D::set_godot_nodes);

	ClassDB::bind_method(D_METHOD("append_g4mf_node", "node"), &G4MFState4D::append_g4mf_node);
	ClassDB::bind_method(D_METHOD("get_node_index", "node"), &G4MFState4D::get_node_index);
	ClassDB::bind_method(D_METHOD("has_unique_name", "name"), &G4MFState4D::has_unique_name);
	ClassDB::bind_method(D_METHOD("reserve_unique_name", "requested_name"), &G4MFState4D::reserve_unique_name);
	ClassDB::bind_method(D_METHOD("unreserve_unique_name", "name"), &G4MFState4D::unreserve_unique_name);

	// Path data for the file.
	ClassDB::bind_method(D_METHOD("get_g4mf_base_path"), &G4MFState4D::get_g4mf_base_path);
	ClassDB::bind_method(D_METHOD("set_g4mf_base_path", "g4mf_base_path"), &G4MFState4D::set_g4mf_base_path);
	ClassDB::bind_method(D_METHOD("get_g4mf_filename"), &G4MFState4D::get_g4mf_filename);
	ClassDB::bind_method(D_METHOD("set_g4mf_filename", "g4mf_filename"), &G4MFState4D::set_g4mf_filename);
	ClassDB::bind_method(D_METHOD("get_original_path"), &G4MFState4D::get_original_path);
	ClassDB::bind_method(D_METHOD("set_original_path", "original_path"), &G4MFState4D::set_original_path);

	// Settings for handling the file.
	ClassDB::bind_method(D_METHOD("is_text_file"), &G4MFState4D::is_text_file);
	ClassDB::bind_method(D_METHOD("get_external_data_mode"), &G4MFState4D::get_external_data_mode);
	ClassDB::bind_method(D_METHOD("set_external_data_mode", "mode"), &G4MFState4D::set_external_data_mode);
	ClassDB::bind_method(D_METHOD("should_separate_resource_files"), &G4MFState4D::should_separate_resource_files);

	// Data for the contents of the file.
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "g4mf_json"), "set_g4mf_json", "get_g4mf_json");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "g4mf_accessors", PROPERTY_HINT_ARRAY_TYPE, "G4MFAccessor4D"), "set_g4mf_accessors", "get_g4mf_accessors");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "g4mf_buffer_views", PROPERTY_HINT_ARRAY_TYPE, "G4MFBufferView4D"), "set_g4mf_buffer_views", "get_g4mf_buffer_views");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "g4mf_buffers", PROPERTY_HINT_ARRAY_TYPE, "PackedByteArray"), "set_g4mf_buffers", "get_g4mf_buffers");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "g4mf_textures", PROPERTY_HINT_ARRAY_TYPE, "G4MFTexture4D"), "set_g4mf_textures", "get_g4mf_textures");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "g4mf_materials", PROPERTY_HINT_ARRAY_TYPE, "G4MFMaterial4D"), "set_g4mf_materials", "get_g4mf_materials");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "g4mf_meshes", PROPERTY_HINT_ARRAY_TYPE, "G4MFMesh4D"), "set_g4mf_meshes", "get_g4mf_meshes");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "g4mf_models", PROPERTY_HINT_ARRAY_TYPE, "G4MFModel4D"), "set_g4mf_models", "get_g4mf_models");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "g4mf_shapes", PROPERTY_HINT_ARRAY_TYPE, "G4MFShape4D"), "set_g4mf_shapes", "get_g4mf_shapes");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "g4mf_lights", PROPERTY_HINT_ARRAY_TYPE, "G4MFLight4D"), "set_g4mf_lights", "get_g4mf_lights");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "g4mf_nodes", PROPERTY_HINT_ARRAY_TYPE, "G4MFNode4D"), "set_g4mf_nodes", "get_g4mf_nodes");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "godot_nodes", PROPERTY_HINT_ARRAY_TYPE, "Node4D"), "set_godot_nodes", "get_godot_nodes");

	// Path data for the file.
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "g4mf_base_path"), "set_g4mf_base_path", "get_g4mf_base_path");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "g4mf_filename"), "set_g4mf_filename", "get_g4mf_filename");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "original_path"), "set_original_path", "get_original_path");

	// Settings for handling the file.
	ADD_PROPERTY(PropertyInfo(Variant::INT, "external_data_mode", PROPERTY_HINT_ENUM, "Automatic,Embed Everything,Separate All Files,Separate Binary Blobs,Separate Resource Files"), "set_external_data_mode", "get_external_data_mode");

	BIND_ENUM_CONSTANT(EXTERNAL_DATA_MODE_AUTOMATIC);
	BIND_ENUM_CONSTANT(EXTERNAL_DATA_MODE_EMBED_EVERYTHING);
	BIND_ENUM_CONSTANT(EXTERNAL_DATA_MODE_SEPARATE_ALL_FILES);
	BIND_ENUM_CONSTANT(EXTERNAL_DATA_MODE_SEPARATE_BINARY_BLOBS);
	BIND_ENUM_CONSTANT(EXTERNAL_DATA_MODE_SEPARATE_RESOURCE_FILES);
	BIND_ENUM_CONSTANT(EXTERNAL_DATA_MODE_SEPARATE_MODEL_FILES);
}
