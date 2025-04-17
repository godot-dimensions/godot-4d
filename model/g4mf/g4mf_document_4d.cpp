#include "g4mf_document_4d.h"

#include "../mesh_instance_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/classes/marshalls.hpp>
#include <godot_cpp/core/version.hpp>
#elif GODOT_MODULE
#include "core/core_bind.h"
#include "core/io/json.h"
using namespace core_bind;
#endif

// Export process.

Error G4MFDocument4D::_export_convert_scene_node(Ref<G4MFState4D> p_g4mf_state, Node *p_current_node, const int p_parent_index) {
	Ref<G4MFNode4D> g4mf_node = G4MFNode4D::from_godot_node(p_current_node);
	g4mf_node->set_parent_index(p_parent_index);
	// Convert node types.
	_export_convert_builtin_node(p_g4mf_state, g4mf_node, p_current_node);
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

Error G4MFDocument4D::_export_convert_builtin_node(Ref<G4MFState4D> p_g4mf_state, Ref<G4MFNode4D> &p_g4mf_node, Node *p_current_node) {
	const MeshInstance4D *mesh_instance = Object::cast_to<MeshInstance4D>(p_current_node);
	if (mesh_instance) {
		const Ref<Mesh4D> mesh = mesh_instance->get_mesh();
		if (mesh.is_valid()) {
			const int mesh_index = G4MFMesh4D::convert_mesh_into_state(p_g4mf_state, mesh);
			p_g4mf_node->set_mesh_index(mesh_index);
		}
	}
	return OK;
}

Error G4MFDocument4D::_export_serialize_json_data(Ref<G4MFState4D> p_g4mf_state) {
	Dictionary g4mf_json;
	p_g4mf_state->set_g4mf_json(g4mf_json);
	_export_serialize_asset_header(p_g4mf_state, g4mf_json);
	_export_serialize_buffers_accessors(p_g4mf_state, g4mf_json);
	_export_serialize_meshes(p_g4mf_state, g4mf_json);
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

Error G4MFDocument4D::_export_serialize_buffers_accessors(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json) {
	// Serialize buffers.
	TypedArray<PackedByteArray> state_buffers = p_g4mf_state->get_buffers();
	const int buffer_count = state_buffers.size();
	if (buffer_count == 0) {
		return OK; // No buffers to serialize (implies no buffer views or accessors).
	}
	Array serialized_buffers;
	serialized_buffers.resize(buffer_count);
	for (int i = 0; i < buffer_count; i++) {
		const PackedByteArray state_buffer = state_buffers[i];
		const String base64_buffer = Marshalls::get_singleton()->raw_to_base64(state_buffer);
		Dictionary buffer_dict;
		buffer_dict["byteLength"] = state_buffer.size();
		buffer_dict["uri"] = String("data:application/octet-stream;base64,") + base64_buffer;
		serialized_buffers[i] = buffer_dict;
	}
	if (!serialized_buffers.is_empty()) {
		p_g4mf_json["buffers"] = serialized_buffers;
	}
	// Serialize buffer views (only relevant if buffers exist).
	TypedArray<G4MFBufferView4D> state_buffer_views = p_g4mf_state->get_buffer_views();
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
	TypedArray<G4MFAccessor4D> state_accessors = p_g4mf_state->get_accessors();
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

// Import process.

Error G4MFDocument4D::_import_parse_json_data(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json) {
	Error err = _import_parse_asset_header(p_g4mf_state, p_g4mf_json);
	ERR_FAIL_COND_V_MSG(err != OK, err, "G4MF import: Failed to parse asset header.");
	err = _import_parse_buffers_accessors(p_g4mf_state, p_g4mf_json);
	ERR_FAIL_COND_V_MSG(err != OK, err, "G4MF import: Failed to parse buffers and accessors.");
	err = _import_parse_meshes(p_g4mf_state, p_g4mf_json);
	ERR_FAIL_COND_V_MSG(err != OK, err, "G4MF import: Failed to parse meshes.");
	err = _import_parse_nodes(p_g4mf_state, p_g4mf_json);
	Ref<G4MFAccessor4D> accessor = p_g4mf_state->get_accessors()[1];
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

Error G4MFDocument4D::_import_parse_buffers_accessors(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json) {
	if (p_g4mf_json.has("buffers")) {
		const Array json_buffers = p_g4mf_json["buffers"];
		const int buffer_count = json_buffers.size();
		if (buffer_count == 0) {
			return OK; // No buffers to parse.
		}
		TypedArray<PackedByteArray> buffers;
		buffers.resize(buffer_count);
		for (int i = 0; i < buffer_count; i++) {
			const Dictionary json_buffer = json_buffers[i];
			ERR_FAIL_COND_V_MSG(!json_buffer.has("byteLength"), ERR_INVALID_DATA, "G4MF import: Buffer is missing required field 'byteLength'. Aborting file import.");
			const int byte_length = json_buffer["byteLength"];
			PackedByteArray buffer;
			if (json_buffer.has("uri")) {
				const String uri = json_buffer["uri"];
				if (uri.begins_with("data:")) {
					PackedStringArray split = uri.split(";base64,", true, 1);
					ERR_FAIL_COND_V_MSG(split.size() != 2, ERR_INVALID_DATA, "G4MF import: Buffer URI is malformed. Expected 'data:application/octet-stream;base64,<base64 data>'. Aborting file import.");
					buffer = Marshalls::get_singleton()->base64_to_raw(split[1]);
				} else {
					// TODO: Load from external file.
				}
			}
			buffer.resize(byte_length);
			buffers[i] = buffer;
		}
		p_g4mf_state->set_buffers(buffers);
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
		p_g4mf_state->set_buffer_views(buffer_views);
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
		p_g4mf_state->set_accessors(accessors);
	}
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
	TypedArray<G4MFMesh4D> g4mf_meshes;
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

Node4D *G4MFDocument4D::_import_generate_scene_node(Ref<G4MFState4D> p_g4mf_state, const int p_node_index, Node *p_parent_node, Node *p_scene_root) {
	TypedArray<G4MFNode4D> state_g4mf_nodes = p_g4mf_state->get_g4mf_nodes();
	ERR_FAIL_INDEX_V(p_node_index, state_g4mf_nodes.size(), nullptr);
	Ref<G4MFNode4D> g4mf_node = state_g4mf_nodes[p_node_index];
	Node4D *godot_node = nullptr;
	// First, check if any extension wants to generate a node.
	// If no extension generated a node, check the built-in types.
	if (godot_node == nullptr) {
		godot_node = _import_generate_builtin_node(p_g4mf_state, g4mf_node);
		if (godot_node == nullptr) {
			godot_node = memnew(Node4D);
		}
	}
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
		godot_node->set_transform(Transform4D());
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

Node4D *G4MFDocument4D::_import_generate_builtin_node(const Ref<G4MFState4D> p_g4mf_state, const Ref<G4MFNode4D> p_g4mf_node) {
	const int mesh_index = p_g4mf_node->get_mesh_index();
	if (mesh_index >= 0) {
		TypedArray<G4MFMesh4D> state_g4mf_meshes = p_g4mf_state->get_g4mf_meshes();
		ERR_FAIL_INDEX_V(mesh_index, state_g4mf_meshes.size(), nullptr);
		Ref<G4MFMesh4D> g4mf_mesh = state_g4mf_meshes[mesh_index];
		Ref<Mesh4D> mesh = g4mf_mesh->generate_mesh(p_g4mf_state, _force_wireframe);
		if (mesh.is_valid()) {
			MeshInstance4D *mesh_instance = memnew(MeshInstance4D);
			mesh_instance->set_mesh(mesh);
			return mesh_instance;
		}
	}
	return nullptr;
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
	return _export_convert_scene_node(p_g4mf_state, p_scene_root, -1);
}

Error G4MFDocument4D::export_append_from_godot_mesh(Ref<G4MFState4D> p_g4mf_state, const Ref<Mesh4D> &p_mesh) {
	ERR_FAIL_COND_V_MSG(p_mesh.is_null(), ERR_INVALID_PARAMETER, "G4MF export: Cannot export a null mesh.");
	const int mesh_index = G4MFMesh4D::convert_mesh_into_state(p_g4mf_state, p_mesh);
	ERR_FAIL_COND_V_MSG(mesh_index < 0, ERR_INVALID_PARAMETER, "G4MF export: Failed to convert mesh into G4MF state.");
	return OK;
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
			mesh_instance->set_name(p_g4mf_state->get_filename().get_basename());
		} else {
			mesh_instance->set_name(mesh_name);
		}
		return mesh_instance;
	}
	TypedArray<Node4D> state_godot_nodes;
	state_godot_nodes.resize(state_g4mf_nodes.size());
	p_g4mf_state->set_godot_nodes(state_godot_nodes);
	return _import_generate_scene_node(p_g4mf_state, 0, nullptr, nullptr);
}

Ref<Mesh4D> G4MFDocument4D::import_generate_godot_mesh(Ref<G4MFState4D> p_g4mf_state, const bool p_include_invisible) {
	const TypedArray<G4MFMesh4D> state_g4mf_meshes = p_g4mf_state->get_g4mf_meshes();
	const int mesh_count = state_g4mf_meshes.size();
	ERR_FAIL_COND_V_MSG(mesh_count == 0, Ref<Mesh4D>(), "G4MF import: This G4MF file has no meshes, so it cannot be imported as a mesh.");
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

void G4MFDocument4D::_bind_methods() {
	// Main import and export functions.
	ClassDB::bind_method(D_METHOD("export_append_from_godot_scene", "g4mf_state", "scene_root"), &G4MFDocument4D::export_append_from_godot_scene);
	ClassDB::bind_method(D_METHOD("export_append_from_godot_mesh", "g4mf_state", "mesh"), &G4MFDocument4D::export_append_from_godot_mesh);
	ClassDB::bind_method(D_METHOD("export_write_to_file", "g4mf_state", "path"), &G4MFDocument4D::export_write_to_file);
	ClassDB::bind_method(D_METHOD("import_read_from_file", "g4mf_state", "path"), &G4MFDocument4D::import_read_from_file);
	ClassDB::bind_method(D_METHOD("import_generate_godot_scene", "g4mf_state"), &G4MFDocument4D::import_generate_godot_scene);
	ClassDB::bind_method(D_METHOD("import_generate_godot_mesh", "g4mf_state", "include_invisible"), &G4MFDocument4D::import_generate_godot_mesh, DEFVAL(false));

	// Settings for the import process.
	ClassDB::bind_method(D_METHOD("get_force_wireframe"), &G4MFDocument4D::get_force_wireframe);
	ClassDB::bind_method(D_METHOD("set_force_wireframe", "force_wireframe"), &G4MFDocument4D::set_force_wireframe);

	// Properties.
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "force_wireframe"), "set_force_wireframe", "get_force_wireframe");
}
