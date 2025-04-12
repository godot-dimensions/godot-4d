#include "g4mf_document_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/core/version.hpp>
#elif GODOT_MODULE
#include "core/io/json.h"
#endif

// Export process.

Error G4MFDocument4D::_export_convert_scene_node(Ref<G4MFState4D> p_g4mf_state, Node *p_current_node, const int p_parent_index) {
	Ref<G4MFNode4D> g4mf_node = G4MFNode4D::from_godot_node(p_current_node);
	g4mf_node->set_parent_index(p_parent_index);
	// Allow excluding a node from export by setting the parent index to -2.
	if (g4mf_node->get_parent_index() < -1) {
		return OK;
	}
	TypedArray<G4MFNode4D> state_g4mf_nodes = p_g4mf_state->get_g4mf_nodes();
	TypedArray<Node4D> state_godot_nodes = p_g4mf_state->get_godot_nodes();
	const int new_node_index = state_g4mf_nodes.size();
	p_g4mf_state->append_g4mf_node(g4mf_node);
	state_godot_nodes.resize(new_node_index + 1);
	state_godot_nodes[new_node_index] = p_current_node;
	PackedInt32Array children_indices;
	for (int i = 0; i < p_current_node->get_child_count(); i++) {
		children_indices.append(state_g4mf_nodes.size());
		Node *child = p_current_node->get_child(i);
		Error err = _export_convert_scene_node(p_g4mf_state, child, new_node_index);
		if (err != OK) {
			return err;
		}
	}
	g4mf_node->set_children_indices(children_indices);
	return OK;
}

Error G4MFDocument4D::_export_serialize_json_data(Ref<G4MFState4D> p_g4mf_state) {
	Dictionary g4mf_json;
	p_g4mf_state->set_g4mf_json(g4mf_json);
	_export_serialize_asset_header(p_g4mf_state, g4mf_json);
	_export_serialize_nodes(p_g4mf_state, g4mf_json);
	return OK;
}

void G4MFDocument4D::_export_serialize_asset_header(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json) {
	Dictionary asset_header;
	asset_header["dimension"] = 4;
	const String godot_version = itos(GODOT_VERSION_MAJOR) + "." + itos(GODOT_VERSION_MINOR) + "." + itos(GODOT_VERSION_PATCH);
	asset_header["generator"] = "Godot Engine " + godot_version + " with Godot 4D";
	p_g4mf_json["asset"] = asset_header;
}

Error G4MFDocument4D::_export_serialize_nodes(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json) {
	TypedArray<G4MFNode4D> state_g4mf_nodes = p_g4mf_state->get_g4mf_nodes();
	const int node_count = state_g4mf_nodes.size();
	Array serialized_nodes;
	serialized_nodes.resize(node_count);
	for (int i = 0; i < node_count; i++) {
		Ref<G4MFNode4D> g4mf_node = state_g4mf_nodes[i];
		Dictionary serialized_node = g4mf_node->to_dictionary();
		serialized_nodes[i] = serialized_node;
	}
	Dictionary g4mf_json = p_g4mf_state->get_g4mf_json();
	g4mf_json["nodes"] = serialized_nodes;
	p_g4mf_state->set_g4mf_json(g4mf_json);
	return OK;
}

// Import process.

Error G4MFDocument4D::_import_parse_json_data(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json) {
	Error err = _import_parse_asset_header(p_g4mf_state, p_g4mf_json);
	if (err != OK) {
		return err;
	}
	err = _import_parse_nodes(p_g4mf_state, p_g4mf_json);
	return err;
}

Error G4MFDocument4D::_import_parse_asset_header(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json) {
	ERR_FAIL_COND_V_MSG(!p_g4mf_json.has("asset"), ERR_INVALID_DATA, "G4MF import: Missing required 'asset' header. Aborting file import.");
	const Dictionary asset_header = p_g4mf_json["asset"];
	ERR_FAIL_COND_V_MSG(!asset_header.has("dimension"), ERR_INVALID_DATA, "G4MF import: Missing required asset header field 'dimension'. Aborting file import.");
	int dimension = asset_header["dimension"];
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
	p_g4mf_state->set_g4mf_nodes(g4mf_nodes);
	return OK;
}

Node4D *G4MFDocument4D::_import_generate_scene_node(Ref<G4MFState4D> p_g4mf_state, const int p_node_index, Node *p_parent_node, Node *p_scene_root) {
	TypedArray<G4MFNode4D> state_g4mf_nodes = p_g4mf_state->get_g4mf_nodes();
	ERR_FAIL_INDEX_V(p_node_index, state_g4mf_nodes.size(), nullptr);
	Ref<G4MFNode4D> g4mf_node = state_g4mf_nodes[p_node_index];
	Node4D *godot_node;
	godot_node = memnew(Node4D);
	// Set up the generated Godot node.
	g4mf_node->apply_to_godot_node_4d(godot_node);
	TypedArray<Node4D> state_godot_nodes = p_g4mf_state->get_godot_nodes();
	state_godot_nodes[p_node_index] = godot_node;
	// Add the Godot node to the tree and set the owner.
	if (p_parent_node) {
		p_parent_node->add_child(godot_node);
		Array args;
		args.append(p_scene_root);
		godot_node->propagate_call(StringName("set_owner"), args);
	} else {
		p_parent_node = godot_node;
		p_scene_root = godot_node;
		// If multiple nodes were generated under the root node, ensure they have the owner set.
		if (unlikely(godot_node->get_child_count() > 0)) {
			Array args;
			args.append(p_scene_root);
			for (int i = 0; i < godot_node->get_child_count(); i++) {
				Node *child = godot_node->get_child(i);
				child->propagate_call(StringName("set_owner"), args);
			}
		}
	}
	// Generate children.
	const PackedInt32Array children_indices = g4mf_node->get_children_indices();
	for (int32_t child_index : children_indices) {
		_import_generate_scene_node(p_g4mf_state, child_index, godot_node, p_scene_root);
	}
	return godot_node;
}

// Public functions.

Error G4MFDocument4D::export_append_from_godot_scene(Ref<G4MFState4D> p_g4mf_state, Node *p_scene_root) {
	return _export_convert_scene_node(p_g4mf_state, p_scene_root, -1);
}

Error G4MFDocument4D::export_write_to_file(Ref<G4MFState4D> p_g4mf_state, const String &p_path) {
	Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::WRITE);
	ERR_FAIL_COND_V_MSG(file.is_null(), ERR_CANT_OPEN, "G4MF export: Failed to open file for writing.");
	p_g4mf_state->set_base_path(p_path.get_base_dir());
	p_g4mf_state->set_filename(p_path.get_file());
	Error err = _export_serialize_json_data(p_g4mf_state);
	ERR_FAIL_COND_V_MSG(err != OK, err, "G4MF export: Failed to serialize G4MF data.");
	const Dictionary g4mf_json = p_g4mf_state->get_g4mf_json();
	const String json_string = JSON::stringify(g4mf_json, "", true);
	file->store_string(json_string + String("\n"));
	return OK;
}

Error G4MFDocument4D::import_read_from_file(Ref<G4MFState4D> p_g4mf_state, const String &p_path) {
	Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::READ);
	ERR_FAIL_COND_V_MSG(file.is_null(), ERR_CANT_OPEN, "G4MF import: Failed to open file for reading.");
	p_g4mf_state->set_base_path(p_path.get_base_dir());
	p_g4mf_state->set_filename(p_path.get_file());
	// TODO: Handle binary files.
	Dictionary g4mf_json = JSON::parse_string(file->get_as_text(true));
	p_g4mf_state->set_g4mf_json(g4mf_json);
	return _import_parse_json_data(p_g4mf_state, g4mf_json);
}

Node4D *G4MFDocument4D::import_generate_godot_scene(Ref<G4MFState4D> p_g4mf_state) {
	TypedArray<G4MFNode4D> state_g4mf_nodes = p_g4mf_state->get_g4mf_nodes();
	ERR_FAIL_COND_V_MSG(state_g4mf_nodes.is_empty(), nullptr, "G4MF import: This G4MF file has no nodes, so it cannot be imported as a scene.");
	TypedArray<Node4D> state_godot_nodes;
	state_godot_nodes.resize(state_g4mf_nodes.size());
	p_g4mf_state->set_godot_nodes(state_godot_nodes);
	return _import_generate_scene_node(p_g4mf_state, 0, nullptr, nullptr);
}

void G4MFDocument4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("export_append_from_godot_scene", "g4mf_state", "scene_root"), &G4MFDocument4D::export_append_from_godot_scene);
	ClassDB::bind_method(D_METHOD("export_write_to_file", "g4mf_state", "path"), &G4MFDocument4D::export_write_to_file);
	ClassDB::bind_method(D_METHOD("import_read_from_file", "g4mf_state", "path"), &G4MFDocument4D::import_read_from_file);
	ClassDB::bind_method(D_METHOD("import_generate_godot_scene", "g4mf_state"), &G4MFDocument4D::import_generate_godot_scene);
}
