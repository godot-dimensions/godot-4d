#include "g4mf_model_4d.h"

#include "../../mesh/mesh_instance_4d.h"
#include "../g4mf_document_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#elif GODOT_MODULE
#include "core/config/project_settings.h"
#endif

Variant G4MFModel4D::get_model_g4mf_document() const {
	return _model_g4mf_document;
}

void G4MFModel4D::set_model_g4mf_document(const Variant p_model_g4mf_document) {
	// Only allow the user to set a valid document or null.
	Ref<G4MFDocument4D> g4mf_document = p_model_g4mf_document;
	_model_g4mf_document = g4mf_document;
}

Node *G4MFModel4D::_find_node_by_name(Node *p_owner, Node *p_node, const String &p_node_name, const bool p_exclude_foreign_owned_nodes) {
	// Check descendants recursively.
	for (int i = 0; i < p_node->get_child_count(); i++) {
		Node *child = p_node->get_child(i);
		if (p_exclude_foreign_owned_nodes) {
			// Skip child nodes which have an owner, but that owner is not `p_owner`.
			// Note that this intentionally does not need to consider nodes added via
			// the additional children feature, because those are customized directly
			// in the parent file; there is nothing in the child file to override.
			Node *child_owner = child->get_owner();
			if (child_owner != nullptr && child_owner != p_owner) {
				continue;
			}
		}
		if (child->get_name() == p_node_name) {
			return child;
		}
		Node *result = _find_node_by_name(p_owner, child, p_node_name, p_exclude_foreign_owned_nodes);
		if (result != nullptr) {
			return result;
		}
	}
	return nullptr;
}

bool G4MFModel4D::_export_can_deduplicate_node(Node *p_node) {
	Node *parent = p_node->get_parent();
	// Godot requires an ancestor to query this flag, so detached roots cannot be checked safely.
	if (parent == nullptr) {
		return false;
	}
	if (parent->is_editable_instance(p_node)) {
		return false;
	}
	// Sharing an enclosing model would also discard customizations in its nested instances.
	for (int i = 0; i < p_node->get_child_count(); i++) {
		const bool child_can_deduplicate = _export_can_deduplicate_node(p_node->get_child(i));
		if (!child_can_deduplicate) {
			return false;
		}
	}
	return true;
}

String G4MFModel4D::_export_normalize_source_path(const String &p_source_abs_path) {
	if (p_source_abs_path.is_empty()) {
		return String();
	}
	String source_path = p_source_abs_path.replace("\\", "/");
	ERR_FAIL_COND_V_MSG(!source_path.is_absolute_path(), String(), "G4MF: The source path must be absolute, including res:// or user://.");
	const String global_path = ProjectSettings::get_singleton()->globalize_path(source_path).replace("\\", "/");
	// Without a loaded project, globalize_path() strips res:// and returns a relative path.
	// Keep the protocol in that case instead of confusing it with another source root.
	if (global_path.is_absolute_path()) {
		source_path = global_path;
	}
	// Keep the protocol outside simplify_path(), independently of its protocol support.
	const int protocol_separator = source_path.find("://");
	if (protocol_separator != -1) {
		const int path_start = protocol_separator + 3;
		return source_path.substr(0, path_start) + source_path.substr(path_start).simplify_path();
	}
	return source_path.simplify_path();
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
	ERR_FAIL_COND_V(p_g4mf_state.is_null(), ERR_INVALID_PARAMETER);
	const TypedArray<G4MFFileReference4D> &state_file_refs = p_g4mf_state->get_g4mf_file_references();
	ERR_FAIL_INDEX_V(_file_index, state_file_refs.size(), ERR_DOES_NOT_EXIST);
	const Ref<G4MFFileReference4D> state_file_ref = state_file_refs[_file_index];
	ERR_FAIL_COND_V(state_file_ref.is_null(), ERR_DOES_NOT_EXIST);
	const String mime_type = state_file_ref->get_mime_type();
	ERR_FAIL_COND_V(mime_type.is_empty(), ERR_INVALID_DATA);
	const String main_base_path = p_g4mf_state->get_g4mf_base_path();
	const bool embedded = state_file_ref->get_file_uri().is_empty() || state_file_ref->get_file_uri().begins_with("data:");
	const String model_file_uri_path = embedded ? String() : state_file_ref->resolve_file_uri(main_base_path);
	Error err = OK;
	// Handle based only on MIME type, which is a required property of G4MFFileReference4D.
	if (mime_type == "model/g4mf-binary" || mime_type == "model/g4mf+json") {
		// TODO: Check dimension before importing to try and pick the best importer. For now assume 4D.
		Ref<G4MFDocument4D> model_g4mf_document = _model_g4mf_document;
		if (model_g4mf_document.is_null()) {
			model_g4mf_document.instantiate();
			_model_g4mf_document = model_g4mf_document;
		}
		_model_g4mf_state.instantiate();
		_model_g4mf_state->set_external_data_mode(p_g4mf_state->get_external_data_mode());
		if (embedded) {
			const PackedByteArray g4mf_file_data = state_file_ref->import_read_file_data(p_g4mf_state);
			ERR_FAIL_COND_V(g4mf_file_data.is_empty(), ERR_CANT_OPEN);
			// Embedded models resolve any of their own relative external
			// references beside the containing G4MF file, unless overridden.
			_model_g4mf_state->set_g4mf_file_path(p_g4mf_state->get_g4mf_file_path());
			err = model_g4mf_document->import_read_from_byte_array(_model_g4mf_state, g4mf_file_data);
		} else {
			err = model_g4mf_document->import_read_from_file(_model_g4mf_state, model_file_uri_path);
		}
		ERR_FAIL_COND_V_MSG(err != OK, err, "G4MF import: G4MFModel4D: Failed to read G4MF model.");
	} else if (mime_type == "model/off") {
		const PackedByteArray file_data = state_file_ref->import_read_file_data(p_g4mf_state);
		ERR_FAIL_COND_V(file_data.is_empty(), ERR_CANT_OPEN);
		_model_off_document = OFFDocument4D::import_read_from_byte_array(file_data);
		ERR_FAIL_COND_V_MSG(_model_off_document.is_null(), ERR_PARSE_ERROR, "G4MF import: G4MFModel4D: Failed to import OFF model.");
#ifdef MODULE_GLTF_ENABLED
	} else if (mime_type == "model/gltf-binary" || mime_type == "model/gltf+json") {
		_model_gltf_document.instantiate();
		_model_gltf_state.instantiate();
		if (embedded) {
			const PackedByteArray gltf_file_data = state_file_ref->import_read_file_data(p_g4mf_state);
			ERR_FAIL_COND_V(gltf_file_data.is_empty(), ERR_CANT_OPEN);
			err = _model_gltf_document->append_from_buffer(gltf_file_data, main_base_path, _model_gltf_state);
		} else {
			err = _model_gltf_document->append_from_file(model_file_uri_path, _model_gltf_state);
		}
		ERR_FAIL_COND_V_MSG(err != OK, err, "G4MF import: G4MFModel4D: Failed to import glTF model.");
#endif // MODULE_GLTF_ENABLED
	} else {
		ERR_FAIL_V_MSG(ERR_UNAVAILABLE, "G4MF import: G4MFModel4D: Unsupported model MIME type: " + mime_type);
	}
	// Capture the source URI before any model using this file reference can serialize and change it.
	if (state_file_ref->get_export_preferred_path_stem().is_empty()) {
		const String initial_uri = state_file_ref->get_file_uri();
		if (!initial_uri.is_empty() && !initial_uri.begins_with("data:")) {
			state_file_ref->set_export_preferred_path_stem(_get_model_path_stem(initial_uri));
		}
	}
	return err;
}

Node *G4MFModel4D::_import_instantiate_g4mf(const Ref<G4MFState4D> &p_parent_g4mf_state, bool &r_has_any_override_applied) {
	// TODO: Check dimension before importing to try and pick the best importer. For now assume 4D.
	Ref<G4MFDocument4D> model_g4mf_document = _model_g4mf_document;
	ERR_FAIL_COND_V_MSG(model_g4mf_document.is_null() || _model_g4mf_state.is_null(), nullptr, "G4MF import: G4MFModel4D: Failed to instantiate G4MF model without preloaded data.");
	// Transfer settings from the parent document to the instance.
	const int max_nested_scene_depth = model_g4mf_document->get_max_nested_scene_depth();
	if (max_nested_scene_depth > 0) {
		model_g4mf_document = model_g4mf_document->duplicate();
		model_g4mf_document->set_max_nested_scene_depth(max_nested_scene_depth - 1);
	}
	_model_g4mf_state->set_external_data_mode(p_parent_g4mf_state->get_external_data_mode());
	_model_g4mf_state->set_preferred_mesh_surface_format(p_parent_g4mf_state->get_preferred_mesh_surface_format());
	// Apply any overrides to the instance before generation.
	const Array override_name_keys = _overrides.keys();
	for (int i = 0; i < override_name_keys.size(); i++) {
		const String item_name = override_name_keys[i];
		Ref<G4MFItem4D> g4mf_item = _model_g4mf_state->get_item_by_name(item_name);
		if (g4mf_item.is_null()) {
			WARN_PRINT("G4MF import: Tried to override item with name '" + item_name + "', but no item with that name was found.");
			continue;
		}
		// Track which overrides have been applied.
		r_has_any_override_applied = true;
		const Dictionary override_dict = _overrides[item_name];
		// TODO: Overrides.
		(void)override_dict; // Explicitly ignore unused variable warning by casting into the void.
	}
	// Generate the Godot scene. Create a MeshInstance4D if there is a mesh but no nodes.
	Node *godot_root = nullptr;
	const int model_node_count = _model_g4mf_state->get_g4mf_nodes().size();
	if (model_node_count == 0) {
		const int _model_mesh_count = _model_g4mf_state->get_g4mf_meshes().size();
		ERR_FAIL_COND_V_MSG(_model_mesh_count == 0, godot_root, "G4MF import: G4MFModel4D: Referenced G4MF file contains no nodes or meshes.");
		MeshInstance4D *mesh_instance = memnew(MeshInstance4D);
		mesh_instance->set_mesh(model_g4mf_document->import_generate_godot_mesh(_model_g4mf_state));
		godot_root = mesh_instance;
	} else {
		godot_root = model_g4mf_document->import_generate_godot_scene(_model_g4mf_state);
	}
	return godot_root;
}

Node *G4MFModel4D::import_instantiate_model(const Ref<G4MFState4D> &p_g4mf_state) {
	ERR_FAIL_COND_V(p_g4mf_state.is_null(), nullptr);
	const TypedArray<G4MFFileReference4D> &state_file_refs = p_g4mf_state->get_g4mf_file_references();
	ERR_FAIL_INDEX_V(_file_index, state_file_refs.size(), nullptr);
	const Ref<G4MFFileReference4D> state_file_ref = state_file_refs[_file_index];
	ERR_FAIL_COND_V(state_file_ref.is_null(), nullptr);
	const String mime_type = state_file_ref->get_mime_type();
	ERR_FAIL_COND_V(mime_type.is_empty(), nullptr);
	bool has_any_override_applied = false;
	G4MFModelImportInstanceData4D instance_data;
	Node *ret = nullptr;
	// Handle based only on MIME type, which is a required property of G4MFFileReference4D.
	if (mime_type == "model/g4mf-binary" || mime_type == "model/g4mf+json") {
		ret = _import_instantiate_g4mf(p_g4mf_state, has_any_override_applied);
	} else if (mime_type == "model/off") {
		// OFF files are simple meshes, so we need to create a mesh instance node to hold the mesh.
		ERR_FAIL_COND_V_MSG(_model_off_document.is_null(), ret, "G4MF import: G4MFModel4D: Failed to instantiate OFF model without preloaded data.");
		if (p_g4mf_state->get_preferred_mesh_surface_format() == G4MFMeshSurface4D::MESH_SURFACE_FORMAT_WIREFRAME) {
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
	// Only needed for editor imports: Set the scene file path for the root node.
	const Ref<G4MFDocument4D> model_document = _model_g4mf_document;
	const int max_nested_scene_depth = model_document.is_valid() ? model_document->get_max_nested_scene_depth() : -1;
	const String file_uri = state_file_ref->get_file_uri();
	if (max_nested_scene_depth != 0 && !file_uri.is_empty() && !file_uri.begins_with("data:")) {
		const String model_file_uri_path = state_file_ref->resolve_file_uri(p_g4mf_state->get_g4mf_base_path());
		if (model_file_uri_path.begins_with("res://")) {
			ret->set_scene_file_path(model_file_uri_path);
		}
	}
#endif // TOOLS_ENABLED
	// Build additional children before the caller can rename the model root. Its eventual scene owner
	// is still unknown while detached, so collect ownership and editable-instance work for finalization.
	if (!_node_additional_children.is_empty()) {
		Ref<G4MFDocument4D> g4mf_document = _model_g4mf_document;
		if (g4mf_document.is_null()) {
			g4mf_document.instantiate();
		}
		const Array node_names = _node_additional_children.keys();
		for (int i = 0; i < node_names.size(); i++) {
			const String node_name = node_names[i];
			Node *target = nullptr;
			if (node_name == ret->get_name()) {
				target = ret;
			} else {
				target = _find_node_by_name(ret, ret, node_name, true);
			}
			if (target == nullptr) {
				target = _find_node_by_name(ret, ret, node_name, false);
			}
			ERR_CONTINUE_MSG(target == nullptr, "G4MF import: Could not find model node '" + node_name + "' for additional children.");
			const PackedInt32Array child_indices = _node_additional_children[node_name];
			bool added_any_children = false;
			for (int32_t child_index : child_indices) {
				// Generate this node from the parent G4MF file, and add it as a child
				// to the already-generated Godot node created from the child model.
				Node *child_godot_node = g4mf_document->import_generate_godot_node_deferred(p_g4mf_state, child_index, target, instance_data.nodes_to_own, instance_data.editable_instances);
				if (child_godot_node != nullptr) {
					added_any_children = true;
				}
			}
			if (added_any_children) {
				has_any_override_applied = true;
				// Each enclosing scene instance is customized too, so none may be shared on re-export.
				for (Node *node = target; node != ret; node = node->get_parent()) {
					if (!node->get_scene_file_path().is_empty()) {
						instance_data.editable_instances.push_back(node->get_instance_id());
					}
				}
			}
		}
	}
	const uint64_t instance_id = ret->get_instance_id();
	if (has_any_override_applied) {
		instance_data.editable_instances.push_back(instance_id);
	}
	_import_instances.insert(instance_id, instance_data);
	return ret;
}

void G4MFModel4D::import_take_instance_finalization(Node *p_model_root, Vector<uint64_t> &r_nodes_to_own, Vector<uint64_t> &r_editable_instances) {
	// A model generated inside another model's additional children cannot use the outer scene root yet.
	// Transfer its pending work into the enclosing instance's record, so that instance finalizes both.
	// Consuming this record delegates the work; this nested instance must not be finalized separately.
	ERR_FAIL_NULL(p_model_root);
	const uint64_t instance_id = p_model_root->get_instance_id();
	const G4MFModelImportInstanceData4D *instance_data = _import_instances.getptr(instance_id);
	ERR_FAIL_NULL_MSG(instance_data, "G4MF import: No pending finalization exists for this model instance.");
	r_nodes_to_own.append_array(instance_data->nodes_to_own);
	r_editable_instances.append_array(instance_data->editable_instances);
	_import_instances.erase(instance_id);
}

void G4MFModel4D::import_finalize_instance(Node *p_model_root, Node *p_scene_root) {
	// Apply the pending work after parenting. This record can also contain work transferred from
	// nested models generated inside this instance's additional-child subtrees.
	ERR_FAIL_NULL(p_model_root);
	const uint64_t instance_id = p_model_root->get_instance_id();
	const G4MFModelImportInstanceData4D *pending_data = _import_instances.getptr(instance_id);
	ERR_FAIL_NULL_MSG(pending_data, "G4MF import: No pending finalization exists for this model instance. It may have already been finalized.");
	if (p_scene_root != nullptr && p_scene_root != p_model_root) {
		ERR_FAIL_COND_MSG(!p_scene_root->is_ancestor_of(p_model_root), "G4MF import: The model must be attached below its scene root before finalization.");
	}
	const G4MFModelImportInstanceData4D instance_data = *pending_data;
	// Consume before changing owners, since notifications may cause reentrant calls.
	_import_instances.erase(instance_id);
	if (p_scene_root == nullptr) {
		// Runtime callers can release pending bookkeeping while managing ownership themselves.
		return;
	}
	// The caller may have freed nodes or moved them outside this instance after generation.
	for (uint64_t node_id : instance_data.nodes_to_own) {
		Node *node = Object::cast_to<Node>(ObjectDB::get_instance(ObjectID(node_id)));
		if (node == nullptr) {
			continue;
		}
		if (!p_model_root->is_ancestor_of(node)) {
			continue;
		}
		if (node->get_owner() != p_scene_root) {
			node->set_owner(p_scene_root);
		}
	}
	// Mark customized instances as editable so later exports keep their changes, including at runtime.
	for (uint64_t editable_id : instance_data.editable_instances) {
		Node *editable_instance = Object::cast_to<Node>(ObjectDB::get_instance(ObjectID(editable_id)));
		if (editable_instance == nullptr || editable_instance == p_scene_root) {
			continue;
		}
		if (editable_instance != p_model_root && !p_model_root->is_ancestor_of(editable_instance)) {
			continue;
		}
		p_scene_root->set_editable_instance(editable_instance, true);
	}
}

int G4MFModel4D::export_pack_nodes_into_model(const Variant p_g4mf_document, const Ref<G4MFState4D> &p_g4mf_state, Node *p_node, const bool p_deduplicate) {
	ERR_FAIL_COND_V(p_g4mf_state.is_null(), -1);
	Ref<G4MFDocument4D> model_g4mf_document = p_g4mf_document;
	if (model_g4mf_document.is_null()) {
		model_g4mf_document.instantiate();
	}
	ERR_FAIL_NULL_V(p_node, -1);
	const String scene_file_path = p_node->get_scene_file_path();
	const String source_path = _export_normalize_source_path(scene_file_path);
	ERR_FAIL_COND_V(!scene_file_path.is_empty() && source_path.is_empty(), -1);
	const bool can_deduplicate = p_deduplicate && !source_path.is_empty() && _export_can_deduplicate_node(p_node);
	// Identify the target URI for the model based on the scene file path and node name.
	const String target_path = G4MFPathContext4D::get_source_placement_path(scene_file_path, p_g4mf_state->get_source_path());
	String target_uri_no_ext = G4MFFileReference4D::sanitize_file_path_uri(target_path.is_empty() ? String(p_node->get_name()) : _get_model_path_stem(target_path), true);
	// Source identity retains the full filename and is independent of the generated output URI.
	TypedArray<G4MFFileReference4D> state_file_refs = p_g4mf_state->get_g4mf_file_references();
	TypedArray<G4MFModel4D> state_models = p_g4mf_state->get_g4mf_models();
	Ref<G4MFModel4D> base_model;
	// NOTE: Models with different nesting levels will be deduplicated, so the
	// behavior will be slightly different depending on which comes first.
	if (can_deduplicate) {
		for (int existing_model_index = 0; existing_model_index < state_models.size(); existing_model_index++) {
			Ref<G4MFModel4D> existing_model = state_models[existing_model_index];
			if (existing_model.is_valid() && existing_model->_export_can_deduplicate) {
				if (existing_model->_export_source_path == source_path) {
					base_model = existing_model;
					// This call creates a plain descriptor. Customized descriptors may share
					// its base file, but their overrides and added children must not be reused.
					if (existing_model->_overrides.is_empty() && existing_model->_node_additional_children.is_empty()) {
						return existing_model_index;
					}
				}
			}
		}
	}
	if (base_model.is_valid()) {
		Ref<G4MFModel4D> model;
		model.instantiate();
		model->_export_can_deduplicate = true;
		model->_export_source_path = source_path;
		model->_file_index = base_model->_file_index;
		// Reuse conversion data and the file reference, while keeping descriptor
		// properties and pending instance finalization local to each model.
		model->_model_g4mf_document = base_model->_model_g4mf_document;
		model->_model_g4mf_state = base_model->_model_g4mf_state;
		model->_model_off_document = base_model->_model_off_document;
#ifdef MODULE_GLTF_ENABLED
		model->_model_gltf_document = base_model->_model_gltf_document;
		model->_model_gltf_state = base_model->_model_gltf_state;
#endif
		const int model_index = state_models.size();
		state_models.append(model);
		p_g4mf_state->set_g4mf_models(state_models);
		return model_index;
	}
	// Transfer the maximum nested scene depth to a duplicated document if necessary.
	const int max_nested_scene_depth = model_g4mf_document->get_max_nested_scene_depth();
	if (max_nested_scene_depth > 0) {
		model_g4mf_document = model_g4mf_document->duplicate();
		model_g4mf_document->set_max_nested_scene_depth(max_nested_scene_depth - 1);
	}
	// Create a new G4MFModel4D to hold a reference to the model data.
	Ref<G4MFModel4D> g4mf_model;
	g4mf_model.instantiate();
	g4mf_model->_export_can_deduplicate = can_deduplicate;
	g4mf_model->_export_source_path = source_path;
	g4mf_model->set_model_g4mf_document(model_g4mf_document);
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
		model_g4mf_state->set_preferred_mesh_surface_format(p_g4mf_state->get_preferred_mesh_surface_format());
		Error err = model_g4mf_document->export_append_from_godot_scene(model_g4mf_state, p_node);
		ERR_FAIL_COND_V(err != OK, -1);
		// Ensure that the root node is untransformed and visible.
		// If invisible, the *instance* should be the one to be invisible, not the model itself.
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
	// Create a file reference for this model to use. The file reference won't
	// have any data in it yet; it will be populated during serialization.
	Ref<G4MFFileReference4D> g4mf_file_ref;
	g4mf_file_ref.instantiate();
	g4mf_file_ref->set_export_preferred_path_stem(target_uri_no_ext);
	// Append the file reference to the state.
	const int file_ref_index = state_file_refs.size();
	state_file_refs.append(g4mf_file_ref);
	g4mf_model->set_file_index(file_ref_index);
	p_g4mf_state->set_g4mf_file_references(state_file_refs);
	// Append the model to the state.
	const int model_index = state_models.size();
	state_models.append(g4mf_model);
	p_g4mf_state->set_g4mf_models(state_models);
	return model_index;
}

Error G4MFModel4D::_export_prepare_for_serialization_internal(const Ref<G4MFState4D> &p_g4mf_state, const bool p_deduplicate) {
	const TypedArray<G4MFFileReference4D> state_file_refs = p_g4mf_state->get_g4mf_file_references();
	ERR_FAIL_INDEX_V(_file_index, state_file_refs.size(), ERR_DOES_NOT_EXIST);
	const Ref<G4MFFileReference4D> file_ref = state_file_refs[_file_index];
	ERR_FAIL_COND_V(file_ref.is_null(), ERR_INVALID_DATA);
	const Error file_err = _export_prepare_file(p_g4mf_state, file_ref, p_deduplicate);
	ERR_FAIL_COND_V(file_err != OK, file_err);
	file_ref->export_assign_name_if_empty(p_g4mf_state);
	const String file_name = file_ref->get_file_uri().is_empty() ? file_ref->get_item_name() : file_ref->get_file_uri();
	ERR_FAIL_COND_V(file_name.is_empty(), ERR_INVALID_DATA);
	// Provide a name when embedding, or when a relative folder makes it less trivially inferable.
	const bool should_separate_model_files = p_g4mf_state->should_separate_model_files();
	if (get_item_name().is_empty() && (!should_separate_model_files || file_name.contains("/"))) {
		p_g4mf_state->assign_unique_item_name(Ref<G4MFItem4D>(this), file_name.get_file());
	}
	return OK;
}

Error G4MFModel4D::_export_prepare_file(const Ref<G4MFState4D> &p_g4mf_state, const Ref<G4MFFileReference4D> &p_file_ref, const bool p_deduplicate) {
	const Ref<G4MFDocument4D> model_g4mf_document = _model_g4mf_document;
	ERR_FAIL_COND_V(model_g4mf_document.is_null(), ERR_INVALID_DATA);
	const bool should_separate_model_files = p_g4mf_state->should_separate_model_files();
	// Embedded model writers produce binary payloads, even in a text parent file.
	const bool is_text_file = should_separate_model_files && p_g4mf_state->is_text_file();
	// Determine the file extension and MIME type to use based on what data is available.
	String file_extension;
	String mime_type;
	if (_model_g4mf_state.is_valid()) {
		// Conversion data survives writes, but output policy follows the containing state each time.
		_model_g4mf_state->use_path_context_of(p_g4mf_state);
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
	p_file_ref->set_mime_type(mime_type);
	String source_path;
	String usage_context;
	if (should_separate_model_files && _model_g4mf_state.is_valid()) {
		// Input source identity preserves case and the extension: distinct source files
		// must stay distinct even when their generated lowercase snake_case filenames collide.
		// Different conversion depths or output settings must not reuse an incompatible representation.
		// Mesh surface preference only affects import generation, not the exported representation.
		usage_context = String::num_int64(model_g4mf_document->get_max_nested_scene_depth()) + ":" +
				String::num_int64(model_g4mf_document->get_encoding_format()) + ":" +
				String::num_int64(_model_g4mf_state->get_external_data_mode()) + ":" + file_extension;
		if (p_deduplicate && _export_can_deduplicate) {
			source_path = _export_source_path;
		} else {
			// Imported or customized models reuse their own allocation without sharing another file reference's output.
			usage_context = "file:" + String::num_uint64(p_file_ref->get_instance_id()) + ":" + usage_context;
		}
	}
	String preferred_stem = p_file_ref->get_export_preferred_path_stem();
	const String file_uri = p_file_ref->get_file_uri();
	if (preferred_stem.is_empty() && !file_uri.is_empty() && !file_uri.begins_with("data:")) {
		preferred_stem = _get_model_path_stem(file_uri);
		p_file_ref->set_export_preferred_path_stem(preferred_stem);
	}
	if (preferred_stem.is_empty()) {
		preferred_stem = get_item_name();
		if (preferred_stem.is_empty()) {
			const int64_t index_of_self = p_g4mf_state->get_g4mf_models().find(this);
			preferred_stem = "model" + String::num_int64(index_of_self);
		}
		p_file_ref->set_export_preferred_path_stem(preferred_stem);
	}
	if (should_separate_model_files) {
		if (usage_context.is_empty()) {
			p_file_ref->set_unique_file_path_uri_from_stem(p_g4mf_state, preferred_stem, file_extension);
		} else {
			const String target_uri = p_g4mf_state->reserve_destination_path_for_source(source_path, usage_context, preferred_stem + file_extension);
			ERR_FAIL_COND_V(target_uri.is_empty(), ERR_CANT_CREATE);
			p_file_ref->set_file_uri_no_sanitize(target_uri);
		}
	}
	// If we are writing to a separate model file, set the URI and write to that location.
	if (should_separate_model_files) {
		const String file_path = p_file_ref->resolve_file_uri(p_g4mf_state->get_g4mf_base_path());
		const Error err = p_file_ref->export_create_missing_directories_if_needed(p_g4mf_state);
		ERR_FAIL_COND_V(err != OK, err);
		// If you need to export both embedded and separate versions of the same G4MF file,
		// export the separate one first (such as the `.g4tf` file before the `.g4b` file).
		p_file_ref->set_buffer_view_index(-1);
		if (_model_g4mf_state.is_valid()) {
			// Repeated conversion and writing across states are acceptable; sharing the output path avoids duplicate files.
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
	PackedByteArray model_bytes;
	if (_model_g4mf_state.is_valid()) {
		// Embedded payloads use the current containing directory and have no external filename.
		_model_g4mf_state->set_g4mf_base_path(p_g4mf_state->get_g4mf_base_path());
		_model_g4mf_state->set_g4mf_filename(String());
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
	constexpr int64_t MODEL_ALIGNMENT = 16;
	p_file_ref->export_store_file_bytes(model_bytes);
	return p_file_ref->export_write_to_buffer(p_g4mf_state, MODEL_ALIGNMENT, p_deduplicate);
}

Ref<G4MFModel4D> G4MFModel4D::from_dictionary(const Dictionary &p_dict) {
	Ref<G4MFModel4D> model;
	model.instantiate();
	model->read_item_entries_from_dictionary(p_dict);
	if (p_dict.has("file")) {
		model->set_file_index(p_dict["file"]);
	}
	if (p_dict.has("nodeAdditionalChildren")) {
		Dictionary node_add_children_json = p_dict["nodeAdditionalChildren"];
		// TODO: Consider changing this to TypedDictionary in a future Godot version.
		Dictionary node_add_children_godot;
		const Array node_add_children_name_keys = node_add_children_json.keys();
		for (int i = 0; i < node_add_children_name_keys.size(); i++) {
			const String node_name = node_add_children_name_keys[i];
			const PackedInt32Array node_add_children_packed = json_array_to_int32_array(node_add_children_json[node_name]);
			node_add_children_godot[node_name] = node_add_children_packed;
		}
		model->set_node_additional_children(node_add_children_godot);
	}
	if (p_dict.has("overrides")) {
		// TODO: Consider changing this to TypedDictionary in a future Godot version.
		model->set_overrides(p_dict["overrides"]);
	}
	return model;
}

Dictionary G4MFModel4D::to_dictionary() const {
	Dictionary dict = write_item_entries_to_dictionary();
	if (_file_index >= 0) {
		dict["file"] = _file_index;
	}
	if (!_node_additional_children.is_empty()) {
		Dictionary node_add_children_json_obj;
		const Array node_add_children_name_keys = _node_additional_children.keys();
		for (int i = 0; i < node_add_children_name_keys.size(); i++) {
			const String node_name = node_add_children_name_keys[i];
			const Array node_add_children_json_arr = int32_array_to_json_array(_node_additional_children[node_name]);
			node_add_children_json_obj[node_name] = node_add_children_json_arr;
		}
		dict["nodeAdditionalChildren"] = node_add_children_json_obj;
	}
	if (!_overrides.is_empty()) {
		dict["overrides"] = _overrides;
	}
	return dict;
}

void G4MFModel4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_file_index"), &G4MFModel4D::get_file_index);
	ClassDB::bind_method(D_METHOD("set_file_index", "file_index"), &G4MFModel4D::set_file_index);
	ClassDB::bind_method(D_METHOD("get_node_additional_children"), &G4MFModel4D::get_node_additional_children);
	ClassDB::bind_method(D_METHOD("set_node_additional_children", "node_additional_children"), &G4MFModel4D::set_node_additional_children);
	ClassDB::bind_method(D_METHOD("get_overrides"), &G4MFModel4D::get_overrides);
	ClassDB::bind_method(D_METHOD("set_overrides", "overrides"), &G4MFModel4D::set_overrides);

	ClassDB::bind_method(D_METHOD("get_model_g4mf_document"), &G4MFModel4D::get_model_g4mf_document);
	ClassDB::bind_method(D_METHOD("set_model_g4mf_document", "model_g4mf_document"), &G4MFModel4D::set_model_g4mf_document);

	ClassDB::bind_method(D_METHOD("import_instantiate_model", "g4mf_state"), &G4MFModel4D::import_instantiate_model);
	ClassDB::bind_method(D_METHOD("import_finalize_instance", "model_root", "scene_root"), &G4MFModel4D::import_finalize_instance, DEFVAL(Variant()));
	ClassDB::bind_method(D_METHOD("import_parse_file_data", "g4mf_state"), &G4MFModel4D::import_parse_file_data);
	ClassDB::bind_static_method("G4MFModel4D", D_METHOD("export_pack_nodes_into_model", "g4mf_document", "g4mf_state", "node", "deduplicate"), &G4MFModel4D::export_pack_nodes_into_model, DEFVAL(true));

	ClassDB::bind_static_method("G4MFModel4D", D_METHOD("from_dictionary", "dict"), &G4MFModel4D::from_dictionary);
	ClassDB::bind_method(D_METHOD("to_dictionary"), &G4MFModel4D::to_dictionary);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "file_index"), "set_file_index", "get_file_index");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "node_additional_children"), "set_node_additional_children", "get_node_additional_children");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "overrides"), "set_overrides", "get_overrides");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "model_g4mf_document", PROPERTY_HINT_RESOURCE_TYPE, "G4MFDocument4D"), "set_model_g4mf_document", "get_model_g4mf_document");
}
