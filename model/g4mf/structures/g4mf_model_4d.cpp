#include "g4mf_model_4d.h"

#include "../../mesh/mesh_instance_4d.h"
#include "../g4mf_document_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/node3d.hpp>
#endif

Variant G4MFModel4D::get_model_g4mf_document() const {
	return _model_g4mf_document;
}

void G4MFModel4D::set_model_g4mf_document(const Variant p_model_g4mf_document) {
	// Only allow the user to set a valid document or null.
	Ref<G4MFDocument4D> g4mf_document = p_model_g4mf_document;
	_model_g4mf_document = g4mf_document;
}

String G4MFModel4D::_get_model_path_stem(const String &p_path) {
	String path = p_path;
	// Treat a trailing .json as a wrapper suffix before removing the model's extension.
	if (path.get_extension().to_lower() == "json") {
		path = path.get_basename();
	}
	return path.get_basename();
}

Error G4MFModel4D::import_parse_file_data(const Ref<G4MFState4D> &p_g4mf_state) {
	Error err = OK;
	ERR_FAIL_COND_V(!p_g4mf_state.is_valid(), ERR_INVALID_PARAMETER);
	const String mime_type = get_mime_type();
	ERR_FAIL_COND_V(mime_type.is_empty(), ERR_INVALID_DATA);
	// Try to read the file data bytes first, even if ultimately unused.
	const PackedByteArray file_data = import_read_file_data(p_g4mf_state);
	ERR_FAIL_COND_V(file_data.is_empty(), ERR_CANT_OPEN);
	const String main_base_path = p_g4mf_state->get_g4mf_base_path();
	const String file_uri_only = get_file_uri();
	if (!file_uri_only.is_empty()) {
		_model_file_uri_path = resolve_file_uri(main_base_path);
	}
	// Handle based on MIME type, which is a required property.
	if (mime_type == "model/g4mf-binary" || mime_type == "model/g4mf+json") {
		// TODO: Check dimension before importing to try and pick the best importer. For now assume 4D.
		Ref<G4MFDocument4D> model_g4mf_document = _model_g4mf_document;
		if (model_g4mf_document.is_null()) {
			model_g4mf_document.instantiate();
			_model_g4mf_document = model_g4mf_document;
		}
		_model_g4mf_state.instantiate();
		_model_g4mf_state->set_external_data_mode(p_g4mf_state->get_external_data_mode());
		if (file_uri_only.is_empty()) {
			// Embedded models resolve any of their own relative external
			// references beside the containing G4MF file, unless overridden.
			_model_g4mf_state->set_g4mf_file_path(p_g4mf_state->get_g4mf_file_path());
			err = model_g4mf_document->import_read_from_byte_array(_model_g4mf_state, file_data);
		} else {
			err = model_g4mf_document->import_read_from_file(_model_g4mf_state, _model_file_uri_path);
		}
		ERR_FAIL_COND_V_MSG(err != OK, err, "G4MF import: G4MFModel4D: Failed to read G4MF model.");
	} else if (mime_type == "model/off") {
		_model_off_document = OFFDocument4D::import_read_from_byte_array(file_data);
		ERR_FAIL_COND_V_MSG(_model_off_document.is_null(), ERR_PARSE_ERROR, "G4MF import: G4MFModel4D: Failed to import OFF model.");
#ifdef MODULE_GLTF_ENABLED
	} else if (mime_type == "model/gltf-binary" || mime_type == "model/gltf+json") {
		_model_gltf_document.instantiate();
		_model_gltf_state.instantiate();
		if (file_uri_only.is_empty()) {
			err = _model_gltf_document->append_from_buffer(file_data, main_base_path, _model_gltf_state);
		} else {
			err = _model_gltf_document->append_from_file(_model_file_uri_path, _model_gltf_state);
		}
		ERR_FAIL_COND_V_MSG(err != OK, err, "G4MF import: G4MFModel4D: Failed to import glTF model.");
#endif // MODULE_GLTF_ENABLED
	} else {
		ERR_FAIL_V_MSG(ERR_UNAVAILABLE, "G4MF import: G4MFModel4D: Unsupported model MIME type: " + mime_type);
	}
	_export_preferred_path_stem = _get_model_path_stem(file_uri_only);
	_export_last_file_uri = file_uri_only;
	return err;
}

Node *G4MFModel4D::import_instantiate_model(const Ref<G4MFState4D> &p_g4mf_state) const {
	Node *ret = nullptr;
	ERR_FAIL_COND_V(p_g4mf_state.is_null(), ret);
	const String mime_type = get_mime_type();
	ERR_FAIL_COND_V(mime_type.is_empty(), ret);
	Ref<G4MFDocument4D> model_g4mf_document = _model_g4mf_document;
	int max_nested_scene_depth = -1;
	// Handle based on MIME type, which is a required property.
	if (mime_type == "model/g4mf-binary" || mime_type == "model/g4mf+json") {
		// TODO: Check dimension before importing to try and pick the best importer. For now assume 4D.
		ERR_FAIL_COND_V_MSG(model_g4mf_document.is_null() || _model_g4mf_state.is_null(), ret, "G4MF import: G4MFModel4D: Failed to instantiate G4MF model without preloaded data.");
		max_nested_scene_depth = model_g4mf_document->get_max_nested_scene_depth();
		if (max_nested_scene_depth > 0) {
			model_g4mf_document = model_g4mf_document->duplicate();
			model_g4mf_document->set_max_nested_scene_depth(max_nested_scene_depth - 1);
		}
		const int _model_node_count = _model_g4mf_state->get_g4mf_nodes().size();
		_model_g4mf_state->set_external_data_mode(p_g4mf_state->get_external_data_mode());
		if (_model_node_count == 0) {
			const int _model_mesh_count = _model_g4mf_state->get_g4mf_meshes().size();
			ERR_FAIL_COND_V_MSG(_model_mesh_count == 0, ret, "G4MF import: G4MFModel4D: Referenced G4MF file contains no nodes or meshes.");
			MeshInstance4D *mesh_instance = memnew(MeshInstance4D);
			mesh_instance->set_mesh(model_g4mf_document->import_generate_godot_mesh(_model_g4mf_state));
			ret = mesh_instance;
		} else {
			ret = model_g4mf_document->import_generate_godot_scene(_model_g4mf_state);
		}
	} else if (mime_type == "model/off") {
		// OFF files are simple meshes, so we need to create a mesh instance node to hold the mesh.
		ERR_FAIL_COND_V_MSG(_model_off_document.is_null(), ret, "G4MF import: G4MFModel4D: Failed to instantiate OFF model without preloaded data.");
		if (_model_g4mf_state.is_valid() && _model_g4mf_state->get_preferred_mesh_surface_format() == G4MFMeshSurface4D::MESH_SURFACE_FORMAT_WIREFRAME) {
			MeshInstance4D *mesh_instance_4d = memnew(MeshInstance4D);
			Ref<ArrayWireMesh4D> wire_mesh = _model_off_document->import_generate_wire_mesh_4d(true);
			mesh_instance_4d->set_mesh(wire_mesh);
			ret = mesh_instance_4d;
		} else {
			ret = _model_off_document->import_generate_node();
		}
#ifdef MODULE_GLTF_ENABLED
	} else if (mime_type == "model/gltf-binary" || mime_type == "model/gltf+json") {
		ERR_FAIL_COND_V_MSG(_model_gltf_document.is_null() || _model_gltf_state.is_null(), ret, "G4MF import: G4MFModel4D: Failed to instantiate glTF model without preloaded data.");
		ret = _model_gltf_document->generate_scene(_model_gltf_state);
#endif // MODULE_GLTF_ENABLED
	} else {
		ERR_FAIL_V_MSG(ret, "G4MF import: G4MFModel4D: Unsupported model MIME type: " + mime_type);
	}
	ERR_FAIL_NULL_V_MSG(ret, ret, "G4MF import: G4MFModel4D: Failed to instantiate model.");
#ifdef TOOLS_ENABLED
	if (max_nested_scene_depth != 0 && ret != nullptr && _model_file_uri_path.begins_with("res://")) {
		ret->set_scene_file_path(_model_file_uri_path);
	}
#endif // TOOLS_ENABLED
	return ret;
}

int G4MFModel4D::export_pack_nodes_into_model(const Variant p_g4mf_document, const Ref<G4MFState4D> &p_g4mf_state, Node *p_node, const bool p_deduplicate) {
	ERR_FAIL_COND_V(p_g4mf_state.is_null(), ERR_INVALID_PARAMETER);
	Ref<G4MFDocument4D> model_g4mf_document = p_g4mf_document;
	if (model_g4mf_document.is_null()) {
		model_g4mf_document.instantiate();
	}
	ERR_FAIL_NULL_V(p_node, -1);
	const String scene_file_path = p_node->get_scene_file_path();
	const String target_path = G4MFPathContext4D::get_source_placement_path(scene_file_path, p_g4mf_state->get_source_path());
	const String target_stem = _get_model_path_stem(target_path);
	const String target_uri_without_extension = sanitize_file_path_uri(target_path.is_empty() ? String(p_node->get_name()) : target_stem, true);
	const int max_nested_scene_depth = model_g4mf_document->get_max_nested_scene_depth();
	if (max_nested_scene_depth > 0) {
		model_g4mf_document = model_g4mf_document->duplicate();
		model_g4mf_document->set_max_nested_scene_depth(max_nested_scene_depth - 1);
	}
	TypedArray<G4MFFileReference4D> g4mf_file_refs = p_g4mf_state->get_g4mf_file_references();
	if (p_deduplicate && !scene_file_path.is_empty()) {
		for (int i = 0; i < g4mf_file_refs.size(); i++) {
			Ref<G4MFModel4D> g4mf_model = g4mf_file_refs[i];
			if (g4mf_model.is_null()) {
				continue;
			}
			if (!g4mf_model->_export_allow_file_reuse || g4mf_model->_model_file_uri_path != scene_file_path) {
				continue;
			}
			const Ref<G4MFDocument4D> existing_document = g4mf_model->_model_g4mf_document;
			if (existing_document.is_null()) {
				continue;
			}
			if (existing_document->get_max_nested_scene_depth() != model_g4mf_document->get_max_nested_scene_depth()) {
				continue;
			}
			if (existing_document->get_encoding_format() != model_g4mf_document->get_encoding_format()) {
				continue;
			}
			if (g4mf_model->_model_g4mf_state.is_valid() && g4mf_model->_model_g4mf_state->get_external_data_mode() != p_g4mf_state->get_external_data_mode()) {
				continue;
			}
			return i;
		}
	}
	// Create a new G4MFModel4D to hold a reference to the model data.
	Ref<G4MFModel4D> g4mf_model;
	g4mf_model.instantiate();
	g4mf_model->set_model_g4mf_document(model_g4mf_document);
	g4mf_model->set_file_uri(target_uri_without_extension);
	g4mf_model->_export_preferred_path_stem = target_uri_without_extension;
	g4mf_model->_export_last_file_uri = g4mf_model->get_file_uri();
	g4mf_model->_model_file_uri_path = scene_file_path;
	g4mf_model->_export_allow_file_reuse = p_deduplicate;
	// Fill the G4MFModel4D with the model data.
#ifdef MODULE_GLTF_ENABLED
	if (Object::cast_to<Node3D>(p_node) != nullptr) {
		g4mf_model->_model_gltf_document.instantiate();
		g4mf_model->_model_gltf_state.instantiate();
		Error err = g4mf_model->_model_gltf_document->append_from_scene(p_node, g4mf_model->_model_gltf_state);
		ERR_FAIL_COND_V(err != OK, -1);
	} else
#endif // MODULE_GLTF_ENABLED
	{
		Ref<G4MFState4D> model_g4mf_state;
		model_g4mf_state.instantiate();
		model_g4mf_state->use_path_context_of(p_g4mf_state);
		model_g4mf_state->set_external_data_mode(p_g4mf_state->get_external_data_mode());
		Error err = model_g4mf_document->export_append_from_godot_scene(model_g4mf_state, p_node);
		ERR_FAIL_COND_V(err != OK, -1);
		// Ensure that the root node is untransformed and visible (if invisible, the *instance* should be the one to be invisible, not the model itself).
		{
			TypedArray<G4MFNode4D> model_g4mf_nodes = model_g4mf_state->get_g4mf_nodes();
			ERR_FAIL_COND_V(model_g4mf_nodes.is_empty(), -1);
			Ref<G4MFNode4D> root_node = model_g4mf_nodes[0];
			ERR_FAIL_COND_V(root_node.is_null(), -1);
			root_node->set_transform(Transform4D());
			root_node->set_visible(true);
		}
		g4mf_model->_model_g4mf_state = model_g4mf_state;
	}
	// Append the model file reference to the state.
	const int model_file_ref_index = g4mf_file_refs.size();
	g4mf_file_refs.append(g4mf_model);
	p_g4mf_state->set_g4mf_file_references(g4mf_file_refs);
	return model_file_ref_index;
}

Error G4MFModel4D::_export_prepare_for_serialization_internal(const Ref<G4MFState4D> &p_g4mf_state, const bool p_deduplicate) {
	const Ref<G4MFDocument4D> model_g4mf_document = _model_g4mf_document;
	ERR_FAIL_COND_V(model_g4mf_document.is_null(), ERR_INVALID_DATA);
	const bool should_separate_model_files = p_g4mf_state->should_separate_model_files();
	// Embedded model writers always produce binary G4MF or GLB data.
	const bool is_text_file = should_separate_model_files && p_g4mf_state->is_text_file();
	// Determine the file extension and MIME type to use based on what data is available.
	String file_extension;
	String mime_type;
	if (_model_g4mf_state.is_valid()) {
		_model_g4mf_state->use_path_context_of(p_g4mf_state);
		_model_g4mf_state->set_external_data_mode(p_g4mf_state->get_external_data_mode());
		file_extension = is_text_file ? ".g4tf" : ".g4b";
		mime_type = is_text_file ? "model/g4mf+json" : "model/g4mf-binary";
	} else if (_model_off_document.is_valid()) {
		file_extension = ".off";
		mime_type = "model/off";
#ifdef MODULE_GLTF_ENABLED
	} else if (_model_gltf_state.is_valid()) {
		file_extension = is_text_file ? ".gltf" : ".glb";
		mime_type = is_text_file ? "model/gltf+json" : "model/gltf-binary";
#endif // MODULE_GLTF_ENABLED
	} else {
		ERR_FAIL_V_MSG(ERR_INVALID_DATA, "G4MF export: G4MFModel4D: No model data to write.");
	}
	// Use the MIME type determined above as-is, and use the file extension to determine the path and name.
	set_mime_type(mime_type);
	const String file_uri = get_file_uri();
	if (file_uri != _export_last_file_uri) {
		// A caller supplied a new filename. Derive its stem independently of the selected output format.
		_export_preferred_path_stem = _get_model_path_stem(file_uri);
	}
	if (_export_preferred_path_stem.is_empty()) {
		_export_preferred_path_stem = get_item_name();
		if (_export_preferred_path_stem.is_empty()) {
			int64_t index_of_self = p_g4mf_state->get_g4mf_file_references().find(this);
			_export_preferred_path_stem = "model" + String::num_int64(index_of_self);
		}
	}
	String target_uri = _export_preferred_path_stem + file_extension;
	if (should_separate_model_files) {
		String source_path;
		String usage_context;
		if (_model_g4mf_state.is_valid()) {
			if (p_deduplicate && _export_allow_file_reuse) {
				source_path = _model_g4mf_state->get_source_path();
			}
			if (!source_path.is_empty()) {
				// The source path is a separate part of the key; usage describes only the exported representation.
				usage_context = String::num_int64(model_g4mf_document->get_max_nested_scene_depth()) + ":" +
						String::num_int64(model_g4mf_document->get_encoding_format()) + ":" +
						String::num_int64(_model_g4mf_state->get_external_data_mode()) + ":" + file_extension;
			}
		}
		if (!usage_context.is_empty()) {
			target_uri = p_g4mf_state->reserve_destination_path_for_source(source_path, usage_context, target_uri);
			ERR_FAIL_COND_V(target_uri.is_empty(), ERR_CANT_CREATE);
			set_file_uri_no_sanitize(target_uri);
		} else {
			set_unique_file_path_uri_from_stem(p_g4mf_state, target_uri.trim_suffix(file_extension), file_extension);
			target_uri = get_file_uri();
		}
		_export_last_file_uri = get_file_uri();
	}
	if (get_item_name().is_empty() && (!should_separate_model_files || target_uri.contains("/"))) {
		p_g4mf_state->assign_unique_item_name(Ref<G4MFItem4D>(this), target_uri.get_file());
	}
	export_assign_name_if_empty(p_g4mf_state);
	// If we are writing to a separate model file, set the URI and write to that location.
	if (should_separate_model_files) {
		const String file_path = resolve_file_uri(p_g4mf_state->get_g4mf_base_path());
		export_create_missing_directories_if_needed(p_g4mf_state);
		if (_model_g4mf_state.is_valid()) {
			return model_g4mf_document->export_write_to_file(_model_g4mf_state, file_path);
		} else if (_model_off_document.is_valid()) {
			_model_off_document->export_save_to_file(file_path);
			return OK;
#ifdef MODULE_GLTF_ENABLED
		} else if (_model_gltf_state.is_valid()) {
			return _model_gltf_document->write_to_filesystem(_model_gltf_state, file_path);
#endif // MODULE_GLTF_ENABLED
		} else {
			CRASH_NOW_MSG("G4MF export: G4MFModel4D: No model data to write, but should've exited earlier.");
		}
	}
	// Embed the model data into a buffer view. The URI must be empty in this case.
	set_unique_file_path_uri_from_stem(p_g4mf_state, "", "");
	_export_last_file_uri = get_file_uri();
	PackedByteArray model_bytes;
	if (_model_g4mf_state.is_valid()) {
		model_bytes = model_g4mf_document->export_write_to_byte_array(_model_g4mf_state);
	} else if (_model_off_document.is_valid()) {
		model_bytes = _model_off_document->export_save_to_byte_array();
#ifdef MODULE_GLTF_ENABLED
	} else if (_model_gltf_state.is_valid()) {
		model_bytes = _model_gltf_document->generate_buffer(_model_gltf_state);
#endif // MODULE_GLTF_ENABLED
	} else {
		CRASH_NOW_MSG("G4MF export: G4MFModel4D: No model data to write, but should've exited earlier.");
	}
	// Now that we have the bytes, write them into a new buffer view in the state.
	ERR_FAIL_COND_V_MSG(model_bytes.is_empty(), ERR_CANT_CREATE, "G4MF export: G4MFModel4D: The exported model data is an empty byte array, export failed.");
	// Always use 16-byte alignment for models to allow 16-byte headers to be aligned.
	const int buffer_view_index = G4MFBufferView4D::write_new_buffer_view_into_state(p_g4mf_state, model_bytes, 16, p_deduplicate, 0);
	ERR_FAIL_COND_V(buffer_view_index < 0, ERR_CANT_CREATE);
	set_buffer_view_index(buffer_view_index);
	return OK;
}

Ref<G4MFModel4D> G4MFModel4D::from_dictionary(const Dictionary &p_dict) {
	Ref<G4MFModel4D> model;
	model.instantiate();
	model->read_file_reference_entries_from_dictionary(p_dict);
	return model;
}

Dictionary G4MFModel4D::to_dictionary() const {
	Dictionary dict = write_file_reference_entries_to_dictionary();
	return dict;
}

void G4MFModel4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_model_g4mf_document"), &G4MFModel4D::get_model_g4mf_document);
	ClassDB::bind_method(D_METHOD("set_model_g4mf_document", "model_g4mf_document"), &G4MFModel4D::set_model_g4mf_document);

	ClassDB::bind_method(D_METHOD("import_instantiate_model", "g4mf_state"), &G4MFModel4D::import_instantiate_model);
	ClassDB::bind_static_method("G4MFModel4D", D_METHOD("export_pack_nodes_into_model", "g4mf_document", "g4mf_state", "node", "deduplicate"), &G4MFModel4D::export_pack_nodes_into_model, DEFVAL(true));

	ClassDB::bind_static_method("G4MFModel4D", D_METHOD("from_dictionary", "dict"), &G4MFModel4D::from_dictionary);
	ClassDB::bind_method(D_METHOD("to_dictionary"), &G4MFModel4D::to_dictionary);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "model_g4mf_document", PROPERTY_HINT_RESOURCE_TYPE, "G4MFDocument4D"), "set_model_g4mf_document", "get_model_g4mf_document");
}
