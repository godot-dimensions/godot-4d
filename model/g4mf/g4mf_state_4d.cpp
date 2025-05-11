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

void G4MFState4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_base_path"), &G4MFState4D::get_base_path);
	ClassDB::bind_method(D_METHOD("set_base_path", "base_path"), &G4MFState4D::set_base_path);
	ClassDB::bind_method(D_METHOD("get_buffers"), &G4MFState4D::get_buffers);
	ClassDB::bind_method(D_METHOD("set_buffers", "buffers"), &G4MFState4D::set_buffers);
	ClassDB::bind_method(D_METHOD("get_filename"), &G4MFState4D::get_filename);
	ClassDB::bind_method(D_METHOD("set_filename", "filename"), &G4MFState4D::set_filename);
	ClassDB::bind_method(D_METHOD("get_g4mf_json"), &G4MFState4D::get_g4mf_json);
	ClassDB::bind_method(D_METHOD("set_g4mf_json", "g4mf_json"), &G4MFState4D::set_g4mf_json);
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

	ClassDB::bind_method(D_METHOD("append_g4mf_node", "node"), &G4MFState4D::append_g4mf_node);
	ClassDB::bind_method(D_METHOD("get_node_index", "node"), &G4MFState4D::get_node_index);
	ClassDB::bind_method(D_METHOD("reserve_unique_name", "requested"), &G4MFState4D::reserve_unique_name);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "base_path"), "set_base_path", "get_base_path");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "buffers", PROPERTY_HINT_ARRAY_TYPE, "PackedByteArray"), "set_buffers", "get_buffers");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "filename"), "set_filename", "get_filename");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "g4mf_json"), "set_g4mf_json", "get_g4mf_json");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "g4mf_textures", PROPERTY_HINT_ARRAY_TYPE, "G4MFTexture4D"), "set_g4mf_textures", "get_g4mf_textures");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "g4mf_materials", PROPERTY_HINT_ARRAY_TYPE, "G4MFMaterial4D"), "set_g4mf_materials", "get_g4mf_materials");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "g4mf_meshes", PROPERTY_HINT_ARRAY_TYPE, "G4MFMesh4D"), "set_g4mf_meshes", "get_g4mf_meshes");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "g4mf_shapes", PROPERTY_HINT_ARRAY_TYPE, "G4MFShape4D"), "set_g4mf_shapes", "get_g4mf_shapes");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "g4mf_nodes", PROPERTY_HINT_ARRAY_TYPE, "G4MFNode4D"), "set_g4mf_nodes", "get_g4mf_nodes");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "godot_nodes", PROPERTY_HINT_ARRAY_TYPE, "Node4D"), "set_godot_nodes", "get_godot_nodes");
}
