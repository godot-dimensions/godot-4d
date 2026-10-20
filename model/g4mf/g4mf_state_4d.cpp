#include "g4mf_state_4d.h"

TypedArray<G4MFItem4D> G4MFState4D::get_child_items() const {
	TypedArray<G4MFItem4D> child_items = G4MFItem4D::get_child_items();
	// The order here is used to determine serialization prep order and
	// name assignment precedence, so start with "high-level" items first.
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
				_item_inst_ids_to_unique_names.erase(current->value->get_instance_id());
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

void G4MFState4D::assign_unique_path_uri_from_stem(const Ref<G4MFFileReference4D> &p_file_reference, const String &p_requested_path_stem, const String &p_extension, const bool p_print_warning) {
	ERR_FAIL_COND(p_file_reference.is_null());
	if (_path_context.is_null()) {
		_path_context.instantiate();
	}
	_path_context->assign_unique_path_uri_from_stem(p_file_reference, p_requested_path_stem, p_extension, _g4mf_base_path, p_print_warning);
}

void G4MFState4D::assign_unique_path_uri(const Ref<G4MFFileReference4D> &p_file_reference, const String &p_requested_path_uri, const bool p_print_warning) {
	ERR_FAIL_COND(p_file_reference.is_null());
	if (_path_context.is_null()) {
		_path_context.instantiate();
	}
	_path_context->assign_unique_path_uri(p_file_reference, p_requested_path_uri, _g4mf_base_path, p_print_warning);
}

String G4MFState4D::reserve_unique_path_uri_from_stem(const String &p_requested_path_stem, const String &p_extension, const bool p_print_warning) {
	if (_path_context.is_null()) {
		_path_context.instantiate();
	}
	return _path_context->reserve_unique_path_uri_from_stem(p_requested_path_stem, p_extension, _g4mf_base_path, p_print_warning);
}

String G4MFState4D::reserve_destination_path_for_source(const String &p_source_abs_path, const String &p_usage_context, const String &p_requested_path, const bool p_print_warning) {
	if (_path_context.is_null()) {
		_path_context.instantiate();
	}
	// An absolute path may be reserved at any time, though all paths should usually be reserved later in the process.
	if (p_requested_path.is_absolute_path()) {
		return _path_context->reserve_destination_abs_path_for_source(p_source_abs_path, p_usage_context, p_requested_path, p_print_warning);
	}
	// A relative path may only be reserved after the destination path is known.
	ERR_FAIL_COND_V(_g4mf_filename.is_empty(), String());
	const String referencing_destination_abs_path = get_g4mf_file_path();
	return _path_context->reserve_destination_path_for_source(p_source_abs_path, _source_path, p_usage_context, referencing_destination_abs_path, p_requested_path, p_print_warning);
}

String G4MFState4D::reserve_destination_usage_path(const String &p_usage_context, const String &p_requested_path_stem, const String &p_extension, const bool p_print_warning) {
	ERR_FAIL_COND_V(_g4mf_filename.is_empty(), String());
	if (_path_context.is_null()) {
		_path_context.instantiate();
	}
	return _path_context->reserve_destination_usage_path(get_g4mf_file_path(), p_usage_context, p_requested_path_stem, p_extension, p_print_warning);
}

void G4MFState4D::mark_path_as_reserved(const String &p_path) {
	// Mark the caller's chosen location without sanitizing it or selecting a unique replacement.
	if (_path_context.is_null()) {
		_path_context.instantiate();
	}
	if (p_path.is_absolute_path()) {
		_path_context->mark_path_as_reserved(p_path);
	} else {
		_path_context->mark_path_as_reserved(_g4mf_base_path.path_join(p_path));
	}
}

void G4MFState4D::reset_path_context() {
	// Detach this state without clearing reservations still used by other states sharing the context.
	_path_context.unref();
}

void G4MFState4D::use_path_context_of(const Ref<G4MFState4D> &p_other_state) {
	// Direction is intentional: this state adopts the other state's shared context and external data mode.
	ERR_FAIL_COND(p_other_state.is_null());
	if (p_other_state->_path_context.is_null()) {
		p_other_state->_path_context.instantiate();
	}
	_path_context = p_other_state->_path_context;
	_external_data_mode = p_other_state->_external_data_mode;
}

void G4MFState4D::set_g4mf_file_path(const String &p_g4mf_file_abs_path) {
	ERR_FAIL_COND_MSG(!p_g4mf_file_abs_path.is_empty() && !p_g4mf_file_abs_path.is_absolute_path(), "G4MF: The file path must be absolute, including res:// or user://.");
	_g4mf_base_path = p_g4mf_file_abs_path.get_base_dir();
	_g4mf_filename = p_g4mf_file_abs_path.get_file();
}

void G4MFState4D::set_g4mf_base_path(const String &p_g4mf_base_abs_path) {
	ERR_FAIL_COND_MSG(!p_g4mf_base_abs_path.is_empty() && !p_g4mf_base_abs_path.is_absolute_path(), "G4MF: The base path must be absolute, including res:// or user://.");
	_g4mf_base_path = p_g4mf_base_abs_path;
}

void G4MFState4D::set_source_path(const String &p_source_abs_path) {
	ERR_FAIL_COND_MSG(!p_source_abs_path.is_empty() && !p_source_abs_path.is_absolute_path(), "G4MF: The source path must be absolute, including res:// or user://.");
	_source_path = p_source_abs_path;
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

	ClassDB::bind_method(D_METHOD("assign_unique_path_uri_from_stem", "file_reference", "requested_path_stem", "extension", "print_warning"), &G4MFState4D::assign_unique_path_uri_from_stem, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("reserve_unique_path_uri_from_stem", "requested_path_stem", "extension", "print_warning"), &G4MFState4D::reserve_unique_path_uri_from_stem, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("reserve_destination_path_for_source", "source_abs_path", "usage_context", "requested_path", "print_warning"), &G4MFState4D::reserve_destination_path_for_source, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("reserve_destination_usage_path", "usage_context", "requested_path_stem", "extension", "print_warning"), &G4MFState4D::reserve_destination_usage_path, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("mark_path_as_reserved", "path"), &G4MFState4D::mark_path_as_reserved);
	ClassDB::bind_method(D_METHOD("reset_path_context"), &G4MFState4D::reset_path_context);
	ClassDB::bind_method(D_METHOD("use_path_context_of", "other_state"), &G4MFState4D::use_path_context_of);

	ClassDB::bind_method(D_METHOD("get_declared_dimension"), &G4MFState4D::get_declared_dimension);

	// Path data for the file.
	ClassDB::bind_method(D_METHOD("get_g4mf_file_path"), &G4MFState4D::get_g4mf_file_path);
	ClassDB::bind_method(D_METHOD("set_g4mf_file_path", "g4mf_file_abs_path"), &G4MFState4D::set_g4mf_file_path);
	ClassDB::bind_method(D_METHOD("get_g4mf_base_path"), &G4MFState4D::get_g4mf_base_path);
	ClassDB::bind_method(D_METHOD("set_g4mf_base_path", "g4mf_base_abs_path"), &G4MFState4D::set_g4mf_base_path);
	ClassDB::bind_method(D_METHOD("get_g4mf_filename"), &G4MFState4D::get_g4mf_filename);
	ClassDB::bind_method(D_METHOD("set_g4mf_filename", "g4mf_filename"), &G4MFState4D::set_g4mf_filename);
	ClassDB::bind_method(D_METHOD("get_source_path"), &G4MFState4D::get_source_path);
	ClassDB::bind_method(D_METHOD("set_source_path", "source_abs_path"), &G4MFState4D::set_source_path);

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
