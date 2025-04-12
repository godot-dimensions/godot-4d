#include "g4mf_state_4d.h"

void G4MFState4D::append_g4mf_node(Ref<G4MFNode4D> p_node) {
	const String requested_name = p_node->get_name();
	if (!requested_name.is_empty()) {
		p_node->set_name(reserve_unique_name(requested_name));
	}
	_g4mf_nodes.push_back(p_node);
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
	ClassDB::bind_method(D_METHOD("get_g4mf_nodes"), &G4MFState4D::get_g4mf_nodes);
	ClassDB::bind_method(D_METHOD("set_g4mf_nodes", "g4mf_nodes"), &G4MFState4D::set_g4mf_nodes);
	ClassDB::bind_method(D_METHOD("get_godot_nodes"), &G4MFState4D::get_godot_nodes);
	ClassDB::bind_method(D_METHOD("set_godot_nodes", "godot_nodes"), &G4MFState4D::set_godot_nodes);
	ClassDB::bind_method(D_METHOD("get_base_path"), &G4MFState4D::get_base_path);
	ClassDB::bind_method(D_METHOD("set_base_path", "base_path"), &G4MFState4D::set_base_path);
	ClassDB::bind_method(D_METHOD("get_filename"), &G4MFState4D::get_filename);
	ClassDB::bind_method(D_METHOD("set_filename", "filename"), &G4MFState4D::set_filename);

	ClassDB::bind_method(D_METHOD("append_g4mf_node", "node"), &G4MFState4D::append_g4mf_node);
	ClassDB::bind_method(D_METHOD("reserve_unique_name", "requested"), &G4MFState4D::reserve_unique_name);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "g4mf_nodes", PROPERTY_HINT_ARRAY_TYPE, "G4MFNode4D"), "set_g4mf_nodes", "get_g4mf_nodes");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "godot_nodes", PROPERTY_HINT_ARRAY_TYPE, "Node4D"), "set_godot_nodes", "get_godot_nodes");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "base_path"), "set_base_path", "get_base_path");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "filename"), "set_filename", "get_filename");
}
