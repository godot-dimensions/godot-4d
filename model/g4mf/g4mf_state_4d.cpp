#include "g4mf_state_4d.h"

TypedArray<G4MFItem4D> G4MFState4D::get_child_items() const {
	TypedArray<G4MFItem4D> child_items = G4MFItem4D::get_child_items();
	// The order here is used to determine name assignment precedence, so start with "high-level" items first.
	const Array item_arrays[] = {
		_g4mf_nodes,
		_g4mf_meshes,
		_g4mf_materials,
		_g4mf_textures,
		_g4mf_shapes,
		_g4mf_file_references,
		_g4mf_accessors,
		_g4mf_buffer_views,
	};
	for (const Array &items : item_arrays) {
		for (int i = 0; i < items.size(); i++) {
			const Ref<G4MFItem4D> item = items[i];
			if (item.is_valid()) {
				child_items.append(item);
			}
		}
	}
	return child_items;
}

int G4MFState4D::get_node_index(const Node *p_node) {
	if (p_node == nullptr) {
		return -1;
	}
	for (int i = 0; i < _godot_nodes.size(); i++) {
		Node *node = Object::cast_to<Node>(_godot_nodes[i]);
		if (node == p_node) {
			return i;
		}
	}
	return -1;
}

int G4MFState4D::append_g4mf_node(Ref<G4MFNode4D> p_node) {
	const String requested_name = p_node->get_name();
	if (!requested_name.is_empty()) {
		assign_unique_item_name(p_node, requested_name);
	}
	const int node_index = _g4mf_nodes.size();
	_g4mf_nodes.push_back(p_node);
	return node_index;
}

void G4MFState4D::assign_unique_item_name(const Ref<G4MFItem4D> &p_item, const String &p_requested_name, const bool p_print_warning) {
	ERR_FAIL_COND(p_item.is_null());
	// Find the previous reservation even if the item was renamed directly.
	const uint64_t item_id = p_item->get_instance_id();
	const String *previous_name = _item_inst_ids_to_unique_names.getptr(item_id);
	if (previous_name != nullptr) {
		_unique_names_to_items.erase(*previous_name);
		_item_inst_ids_to_unique_names.erase(item_id);
	}
	const String unique_name = reserve_unique_name(p_requested_name, p_print_warning);
	if (!unique_name.is_empty()) {
		_unique_names_to_items[unique_name] = p_item;
		_item_inst_ids_to_unique_names[item_id] = unique_name;
	}
	// The reservation already sanitized the name.
	p_item->set_name(unique_name);
}

String G4MFState4D::reserve_unique_name(const String &p_requested_name, const bool p_print_warning) {
	// Empty names are the only case where duplicates are allowed.
	if (p_requested_name.is_empty()) {
		return String();
	}
	const String sanitized_name = G4MFItem4D::sanitize_item_name(p_requested_name, p_print_warning);
	String unique_name = sanitized_name;
	uint64_t discriminator = 2;
	bool found_any_conflict = false;
#if G4MF_USE_CASE_INSENSITIVE_UNIQUE_NAMES
	String lower_unique_name = unique_name.to_lower();
	while (true) {
		bool case_insensitive_conflict = false;
		for (const KeyValue<String, Ref<G4MFItem4D>> &existing_pair : _unique_names_to_items) {
			if (existing_pair.key.to_lower() == lower_unique_name) {
				// Attempt a new unique name by appending a discriminator.
				unique_name = sanitized_name + String::num_uint64(discriminator);
				lower_unique_name = unique_name.to_lower();
				discriminator++;
				case_insensitive_conflict = true;
				found_any_conflict = true;
			}
		}
		if (!case_insensitive_conflict) {
			break;
		}
	}
#else
	while (_unique_names_to_items.has(unique_name)) {
		// Attempt a new unique name by appending a discriminator.
		unique_name = sanitized_name + String::num_uint64(discriminator);
		discriminator++;
		found_any_conflict = true;
	}
#endif // G4MF_USE_CASE_INSENSITIVE_UNIQUE_NAMES
	if (p_print_warning && found_any_conflict) {
		WARN_PRINT("G4MFState4D: The requested name " + p_requested_name + " (sanitized to " + sanitized_name + ") is already in use. The name " + unique_name + " will be used instead.");
	}
	_unique_names_to_items.insert(unique_name, Ref<G4MFItem4D>());
	return unique_name;
}

bool G4MFState4D::unreserve_unique_name(const String &p_name) {
#if G4MF_USE_CASE_INSENSITIVE_UNIQUE_NAMES
	const String lower_name = p_name.to_lower();
	for (HashMap<String, Ref<G4MFItem4D>>::Iterator iter = _unique_names_to_items.begin(); iter;) {
		const HashMap<String, Ref<G4MFItem4D>>::Iterator current = iter;
		++iter;
		if (current->key.to_lower() == lower_name) {
			if (current->value.is_valid()) {
				_item_ids_to_unique_names.erase(current->value->get_instance_id());
			}
			_unique_names_to_items.remove(current);
			return true;
		}
	}
	return false;
#else
	const Ref<G4MFItem4D> *item = _unique_names_to_items.getptr(p_name);
	if (item != nullptr && item->is_valid()) {
		_item_inst_ids_to_unique_names.erase((*item)->get_instance_id());
	}
	return _unique_names_to_items.erase(p_name);
#endif // G4MF_USE_CASE_INSENSITIVE_UNIQUE_NAMES
}

bool G4MFState4D::has_unique_name(const String &p_name) const {
#if G4MF_USE_CASE_INSENSITIVE_UNIQUE_NAMES
	const String lower_name = p_name.to_lower();
	for (const KeyValue<String, Ref<G4MFItem4D>> &existing_pair : _unique_names_to_items) {
		if (existing_pair.key.to_lower() == lower_name) {
			return true;
		}
	}
	return false;
#else
	return _unique_names_to_items.has(p_name);
#endif // G4MF_USE_CASE_INSENSITIVE_UNIQUE_NAMES
}

Ref<G4MFItem4D> G4MFState4D::get_item_by_name(const String &p_name) const {
	if (_unique_names_to_items.has(p_name)) {
		return _unique_names_to_items[p_name];
	}
#if G4MF_USE_CASE_INSENSITIVE_UNIQUE_NAMES
	const String lower_name = p_name.to_lower();
	for (const KeyValue<String, Ref<G4MFItem4D>> &existing_pair : _unique_names_to_items) {
		if (existing_pair.key.to_lower() == lower_name) {
			return existing_pair.value;
		}
	}
#endif // G4MF_USE_CASE_INSENSITIVE_UNIQUE_NAMES
	return Ref<G4MFItem4D>();
}

void G4MFState4D::assign_unique_path_uri(const Ref<G4MFFileReference4D> &p_file_reference, const String &p_requested_path_stem, const String &p_extension, const bool p_print_warning) {
	ERR_FAIL_COND(p_file_reference.is_null());
	// Validate before releasing ownership: an invalid path must not clear the file reference.
	// Sanitation warnings are printed by reserve_unique_path_uri() after validation succeeds.
	String sanitized_ext;
	const String sanitized_path = G4MFFileReference4D::sanitize_path_uri(p_requested_path_stem, p_extension, false, sanitized_ext);
	if ((!p_requested_path_stem.is_empty() || !p_extension.is_empty()) && sanitized_path.is_empty()) {
		return;
	}
	// Release this file reference's previous reservations before choosing its new path.
	for (HashMap<String, Ref<G4MFFileReference4D>>::Iterator iter = _unique_path_uris_to_file_references.begin(); iter;) {
		const HashMap<String, Ref<G4MFFileReference4D>>::Iterator current = iter;
		++iter;
		if (current->value == p_file_reference) {
			_unique_path_uris_to_file_references.remove(current);
		}
	}
	const String unique_path_uri = reserve_unique_path_uri(p_requested_path_stem, p_extension, p_print_warning);
	if (!unique_path_uri.is_empty()) {
		_unique_path_uris_to_file_references[unique_path_uri] = p_file_reference;
	}
	p_file_reference->set_file_uri_no_sanitize(unique_path_uri);
}

String G4MFState4D::reserve_unique_path_uri(const String &p_requested_path_stem, const String &p_extension, const bool p_print_warning) {
	String sanitized_ext;
	String unique_path_uri = G4MFFileReference4D::sanitize_path_uri(p_requested_path_stem, p_extension, p_print_warning, sanitized_ext);
	if (unique_path_uri.is_empty()) {
		return String();
	}
	const String sanitized_stem = unique_path_uri.trim_suffix(sanitized_ext);
	String lower_unique_path_uri = unique_path_uri.to_lower();
	uint64_t discriminator = 2;
	bool found_any_conflict = false;
	// Filesystem path URIs are always treated case-insensitively.
	while (true) {
		bool case_insensitive_conflict = false;
		for (const KeyValue<String, Ref<G4MFFileReference4D>> &existing_pair : _unique_path_uris_to_file_references) {
			if (existing_pair.key.to_lower() == lower_unique_path_uri) {
				// Attempt a new unique name by appending a discriminator.
				unique_path_uri = sanitized_stem + String::num_uint64(discriminator) + sanitized_ext;
				lower_unique_path_uri = unique_path_uri.to_lower();
				discriminator++;
				case_insensitive_conflict = true;
				found_any_conflict = true;
			}
		}
		if (!case_insensitive_conflict) {
			break;
		}
	}
	if (p_print_warning && found_any_conflict) {
		WARN_PRINT("G4MFState4D: The requested path URI " + p_requested_path_stem + " (sanitized to " + sanitized_stem + ") is already in use. The path URI " + unique_path_uri + " will be used instead.");
	}
	_unique_path_uris_to_file_references[unique_path_uri] = Ref<G4MFFileReference4D>();
	return unique_path_uri;
}

void G4MFState4D::set_g4mf_file_path(const String &p_g4mf_file_path) {
	_g4mf_base_path = p_g4mf_file_path.get_base_dir();
	_g4mf_filename = p_g4mf_file_path.get_file();
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
	ClassDB::bind_method(D_METHOD("get_g4mf_file_references"), &G4MFState4D::get_g4mf_file_references);
	ClassDB::bind_method(D_METHOD("set_g4mf_file_references", "g4mf_file_references"), &G4MFState4D::set_g4mf_file_references);
	ClassDB::bind_method(D_METHOD("get_g4mf_textures"), &G4MFState4D::get_g4mf_textures);
	ClassDB::bind_method(D_METHOD("set_g4mf_textures", "g4mf_textures"), &G4MFState4D::set_g4mf_textures);
	ClassDB::bind_method(D_METHOD("get_g4mf_materials"), &G4MFState4D::get_g4mf_materials);
	ClassDB::bind_method(D_METHOD("set_g4mf_materials", "g4mf_materials"), &G4MFState4D::set_g4mf_materials);
	ClassDB::bind_method(D_METHOD("get_g4mf_meshes"), &G4MFState4D::get_g4mf_meshes);
	ClassDB::bind_method(D_METHOD("set_g4mf_meshes", "g4mf_meshes"), &G4MFState4D::set_g4mf_meshes);
	ClassDB::bind_method(D_METHOD("get_g4mf_shapes"), &G4MFState4D::get_g4mf_shapes);
	ClassDB::bind_method(D_METHOD("set_g4mf_shapes", "g4mf_shapes"), &G4MFState4D::set_g4mf_shapes);
	ClassDB::bind_method(D_METHOD("get_g4mf_nodes"), &G4MFState4D::get_g4mf_nodes);
	ClassDB::bind_method(D_METHOD("set_g4mf_nodes", "g4mf_nodes"), &G4MFState4D::set_g4mf_nodes);
	ClassDB::bind_method(D_METHOD("get_godot_nodes"), &G4MFState4D::get_godot_nodes);
	ClassDB::bind_method(D_METHOD("set_godot_nodes", "godot_nodes"), &G4MFState4D::set_godot_nodes);

	ClassDB::bind_method(D_METHOD("get_node_index", "node"), &G4MFState4D::get_node_index);
	ClassDB::bind_method(D_METHOD("append_g4mf_node", "node"), &G4MFState4D::append_g4mf_node);

	ClassDB::bind_method(D_METHOD("has_unique_name", "name"), &G4MFState4D::has_unique_name);
	ClassDB::bind_method(D_METHOD("assign_unique_item_name", "item", "requested_name", "print_warning"), &G4MFState4D::assign_unique_item_name, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("reserve_unique_name", "requested_name", "print_warning"), &G4MFState4D::reserve_unique_name, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("unreserve_unique_name", "name"), &G4MFState4D::unreserve_unique_name);
	ClassDB::bind_method(D_METHOD("get_item_by_name", "name"), &G4MFState4D::get_item_by_name);

	ClassDB::bind_method(D_METHOD("assign_unique_path_uri", "file_reference", "requested_path_stem", "extension", "print_warning"), &G4MFState4D::assign_unique_path_uri, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("reserve_unique_path_uri", "requested_path_stem", "extension", "print_warning"), &G4MFState4D::reserve_unique_path_uri, DEFVAL(true));

	ClassDB::bind_method(D_METHOD("get_declared_dimension"), &G4MFState4D::get_declared_dimension);

	// Path data for the file.
	ClassDB::bind_method(D_METHOD("get_g4mf_file_path"), &G4MFState4D::get_g4mf_file_path);
	ClassDB::bind_method(D_METHOD("set_g4mf_file_path", "g4mf_file_path"), &G4MFState4D::set_g4mf_file_path);
	ClassDB::bind_method(D_METHOD("get_g4mf_base_path"), &G4MFState4D::get_g4mf_base_path);
	ClassDB::bind_method(D_METHOD("set_g4mf_base_path", "g4mf_base_path"), &G4MFState4D::set_g4mf_base_path);
	ClassDB::bind_method(D_METHOD("get_g4mf_filename"), &G4MFState4D::get_g4mf_filename);
	ClassDB::bind_method(D_METHOD("set_g4mf_filename", "g4mf_filename"), &G4MFState4D::set_g4mf_filename);
	ClassDB::bind_method(D_METHOD("get_source_path"), &G4MFState4D::get_source_path);
	ClassDB::bind_method(D_METHOD("set_source_path", "source_path"), &G4MFState4D::set_source_path);

	// Settings for handling the file.
	ClassDB::bind_method(D_METHOD("is_text_file"), &G4MFState4D::is_text_file);
	ClassDB::bind_method(D_METHOD("get_external_data_mode"), &G4MFState4D::get_external_data_mode);
	ClassDB::bind_method(D_METHOD("set_external_data_mode", "mode"), &G4MFState4D::set_external_data_mode);
	ClassDB::bind_method(D_METHOD("should_separate_resource_files"), &G4MFState4D::should_separate_resource_files);

	ClassDB::bind_method(D_METHOD("get_preferred_mesh_surface_format"), &G4MFState4D::get_preferred_mesh_surface_format);
	ClassDB::bind_method(D_METHOD("set_preferred_mesh_surface_format", "preferred_format"), &G4MFState4D::set_preferred_mesh_surface_format);

	// Data for the contents of the file.
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "g4mf_json"), "set_g4mf_json", "get_g4mf_json");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "g4mf_accessors", PROPERTY_HINT_ARRAY_TYPE, "G4MFAccessor4D"), "set_g4mf_accessors", "get_g4mf_accessors");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "g4mf_buffer_views", PROPERTY_HINT_ARRAY_TYPE, "G4MFBufferView4D"), "set_g4mf_buffer_views", "get_g4mf_buffer_views");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "g4mf_buffers", PROPERTY_HINT_ARRAY_TYPE, "PackedByteArray"), "set_g4mf_buffers", "get_g4mf_buffers");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "g4mf_file_references", PROPERTY_HINT_ARRAY_TYPE, "G4MFFileReference4D"), "set_g4mf_file_references", "get_g4mf_file_references");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "g4mf_textures", PROPERTY_HINT_ARRAY_TYPE, "G4MFTexture4D"), "set_g4mf_textures", "get_g4mf_textures");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "g4mf_materials", PROPERTY_HINT_ARRAY_TYPE, "G4MFMaterial4D"), "set_g4mf_materials", "get_g4mf_materials");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "g4mf_meshes", PROPERTY_HINT_ARRAY_TYPE, "G4MFMesh4D"), "set_g4mf_meshes", "get_g4mf_meshes");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "g4mf_shapes", PROPERTY_HINT_ARRAY_TYPE, "G4MFShape4D"), "set_g4mf_shapes", "get_g4mf_shapes");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "g4mf_nodes", PROPERTY_HINT_ARRAY_TYPE, "G4MFNode4D"), "set_g4mf_nodes", "get_g4mf_nodes");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "godot_nodes", PROPERTY_HINT_ARRAY_TYPE, "Node"), "set_godot_nodes", "get_godot_nodes");

	// Path data for the file.
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "g4mf_file_path"), "set_g4mf_file_path", "get_g4mf_file_path");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "g4mf_base_path"), "set_g4mf_base_path", "get_g4mf_base_path");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "g4mf_filename"), "set_g4mf_filename", "get_g4mf_filename");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "source_path"), "set_source_path", "get_source_path");

	// Settings for handling the file.
	ADD_PROPERTY(PropertyInfo(Variant::INT, "external_data_mode", PROPERTY_HINT_ENUM, "Automatic,Embed Everything,Separate All Files,Separate Binary Blobs,Separate Resource Files"), "set_external_data_mode", "get_external_data_mode");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "preferred_mesh_surface_format", PROPERTY_HINT_ENUM, "Polytope,Tetrahedral,Wireframe"), "set_preferred_mesh_surface_format", "get_preferred_mesh_surface_format");

	BIND_ENUM_CONSTANT(EXTERNAL_DATA_MODE_AUTOMATIC);
	BIND_ENUM_CONSTANT(EXTERNAL_DATA_MODE_EMBED_EVERYTHING);
	BIND_ENUM_CONSTANT(EXTERNAL_DATA_MODE_SEPARATE_ALL_FILES);
	BIND_ENUM_CONSTANT(EXTERNAL_DATA_MODE_SEPARATE_BINARY_BLOBS);
	BIND_ENUM_CONSTANT(EXTERNAL_DATA_MODE_SEPARATE_RESOURCE_FILES);
	BIND_ENUM_CONSTANT(EXTERNAL_DATA_MODE_SEPARATE_MODEL_FILES);
}
