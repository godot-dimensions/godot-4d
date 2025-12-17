#include "g4mf_model_4d.h"

#include "../../mesh/mesh_instance_4d.h"
#include "../g4mf_document_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#elif GODOT_MODULE
#include "core/config/project_settings.h"
#include "core/io/dir_access.h"
#endif

Variant G4MFModel4D::get_model_g4mf_document() const {
	return _model_g4mf_document;
}

void G4MFModel4D::set_model_g4mf_document(const Variant p_model_g4mf_document) {
	// Only allow the user to set a valid document or null.
	Ref<G4MFDocument4D> g4mf_document = p_model_g4mf_document;
	_model_g4mf_document = g4mf_document;
}

// Godot actually has `String::path_to()` built-in, but unfortunately, it's not exposed,
// so we can't use it here. Instead, just re-implement a bespoke function only for our needs.
String G4MFModel4D::_get_relative_path_without_updir_dots(const String &p_original_path, const String &p_relative_to) {
	// Ensure the paths use a consistent style.
	const bool res_xor = p_original_path.begins_with("res://") != p_relative_to.begins_with("res://");
	const ProjectSettings *ps = ProjectSettings::get_singleton();
	const String original_path = (res_xor ? ps->globalize_path(p_original_path) : p_original_path).replace("\\", "/");
	const String relative_to = (res_xor ? ps->globalize_path(p_relative_to) : p_relative_to).replace("\\", "/");
	// Check for overlap.
	int last_separator_index = -1;
	for (int overlap = 0; overlap < original_path.length() && overlap < relative_to.length(); overlap++) {
		if (original_path[overlap] != relative_to[overlap]) {
			break;
		}
		if (original_path[overlap] == '/') {
			last_separator_index = overlap;
		}
	}
	last_separator_index++;
	return original_path.substr(last_separator_index, original_path.length() - last_separator_index);
}

Error G4MFModel4D::import_preload_model_data(const Ref<G4MFState4D> &p_g4mf_state) {
	Error err = OK;
	ERR_FAIL_COND_V(!p_g4mf_state.is_valid(), ERR_INVALID_PARAMETER);
	const String mime_type = get_mime_type();
	ERR_FAIL_COND_V(mime_type.is_empty(), ERR_INVALID_DATA);
	// Try to read the file data bytes first, even if ultimately unused.
	const PackedByteArray file_data = read_file_data(p_g4mf_state);
	ERR_FAIL_COND_V(file_data.is_empty(), ERR_CANT_OPEN);
	const String main_base_path = p_g4mf_state->get_g4mf_base_path();
	const String file_uri_only = get_uri();
	if (!file_uri_only.is_empty()) {
		_model_file_uri_path = main_base_path.path_join(file_uri_only);
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
			_model_g4mf_state->set_g4mf_base_path(main_base_path);
			err = model_g4mf_document->import_read_from_byte_array(_model_g4mf_state, file_data);
		} else {
			err = model_g4mf_document->import_read_from_file(_model_g4mf_state, _model_file_uri_path);
		}
		ERR_FAIL_COND_V_MSG(err != OK, err, "G4MF import: G4MFModel4D: Failed to read G4MF model.");
	} else if (mime_type == "model/off") {
		_model_off_document = OFFDocument4D::import_load_from_byte_array(file_data);
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
		if (model_g4mf_document.is_valid() && model_g4mf_document->get_force_wireframe()) {
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
	const String target_path = _get_relative_path_without_updir_dots(scene_file_path, p_g4mf_state->get_original_path());
	const String target_uri_without_extension = (target_path.is_empty() ? String(p_node->get_name()) : target_path.get_basename()).to_snake_case();
	TypedArray<G4MFModel4D> g4mf_models = p_g4mf_state->get_g4mf_models();
	if (p_deduplicate && !target_uri_without_extension.is_empty()) {
		for (int i = 0; i < g4mf_models.size(); i++) {
			Ref<G4MFModel4D> g4mf_model = g4mf_models[i];
			if (g4mf_model.is_valid() && g4mf_model->get_uri() == target_uri_without_extension) {
				return i;
			}
		}
	}
	const int max_nested_scene_depth = model_g4mf_document->get_max_nested_scene_depth();
	if (max_nested_scene_depth > 0) {
		model_g4mf_document = model_g4mf_document->duplicate();
		model_g4mf_document->set_max_nested_scene_depth(max_nested_scene_depth - 1);
	}
	// Create a new G4MFModel4D to hold a reference to the model data.
	Ref<G4MFModel4D> g4mf_model;
	g4mf_model.instantiate();
	g4mf_model->set_model_g4mf_document(model_g4mf_document);
	g4mf_model->set_uri(target_uri_without_extension);
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
	// Append the model to the state.
	const int model_index = g4mf_models.size();
	g4mf_models.append(g4mf_model);
	p_g4mf_state->set_g4mf_models(g4mf_models);
	return model_index;
}

Error G4MFModel4D::export_write_model_data(const Ref<G4MFState4D> &p_g4mf_state, const bool p_deduplicate, int p_buffer_index) {
	ERR_FAIL_COND_V(p_g4mf_state.is_null(), ERR_INVALID_PARAMETER);
	const Ref<G4MFDocument4D> model_g4mf_document = _model_g4mf_document;
	ERR_FAIL_COND_V(model_g4mf_document.is_null(), ERR_INVALID_DATA);
	const bool is_text_file = p_g4mf_state->is_text_file();
	const bool should_separate_model_files = p_g4mf_state->should_separate_model_files();
	// Determine the file extension and MIME type to use based on what data is available.
	String file_extension;
	String mime_type;
	if (_model_g4mf_state.is_valid()) {
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
	const String target_uri = get_uri() + file_extension;
	const String file_path = p_g4mf_state->get_g4mf_base_path().path_join(target_uri);
	if (!should_separate_model_files || target_uri.contains("/")) {
		set_item_name(p_g4mf_state->reserve_unique_name(target_uri.get_file()));
	}
	// If we are writing to a separate model file, set the URI and write to that location.
	if (should_separate_model_files) {
		set_uri(target_uri);
		create_missing_directories_if_needed(p_g4mf_state);
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
	set_uri("");
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
	const int buffer_view_index = G4MFBufferView4D::write_new_buffer_view_into_state(p_g4mf_state, model_bytes, 16, p_deduplicate, p_buffer_index);
	ERR_FAIL_COND_V(buffer_view_index < 0, ERR_CANT_CREATE);
	set_buffer_view(buffer_view_index);
	return OK;
}

Ref<G4MFModel4D> G4MFModel4D::from_dictionary(const Dictionary &p_dict) {
	Ref<G4MFModel4D> texture;
	texture.instantiate();
	texture->read_file_reference_entries_from_dictionary(p_dict);
	return texture;
}

Dictionary G4MFModel4D::to_dictionary() const {
	Dictionary dict = write_file_reference_entries_to_dictionary();
	return dict;
}

void G4MFModel4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_model_g4mf_document"), &G4MFModel4D::get_model_g4mf_document);
	ClassDB::bind_method(D_METHOD("set_model_g4mf_document", "model_g4mf_document"), &G4MFModel4D::set_model_g4mf_document);

	ClassDB::bind_method(D_METHOD("import_preload_model_data", "g4mf_state"), &G4MFModel4D::import_preload_model_data);
	ClassDB::bind_method(D_METHOD("import_instantiate_model", "g4mf_state"), &G4MFModel4D::import_instantiate_model);
	ClassDB::bind_static_method("G4MFModel4D", D_METHOD("export_pack_nodes_into_model", "g4mf_document", "g4mf_state", "node", "deduplicate"), &G4MFModel4D::export_pack_nodes_into_model, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("export_write_model_data", "g4mf_state", "deduplicate", "buffer_index"), &G4MFModel4D::export_write_model_data, DEFVAL(true), DEFVAL(0));

	ClassDB::bind_static_method("G4MFModel4D", D_METHOD("from_dictionary", "dict"), &G4MFModel4D::from_dictionary);
	ClassDB::bind_method(D_METHOD("to_dictionary"), &G4MFModel4D::to_dictionary);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "model_g4mf_document", PROPERTY_HINT_RESOURCE_TYPE, "G4MFDocument4D"), "set_model_g4mf_document", "get_model_g4mf_document");
}
