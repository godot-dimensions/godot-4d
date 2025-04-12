#pragma once

#include "g4mf_state_4d.h"

class G4MFDocument4D : public Resource {
	GDCLASS(G4MFDocument4D, Resource);

	// Export process.
	Error _export_convert_scene_node(Ref<G4MFState4D> p_g4mf_state, Node *p_current_node, const int p_parent_index);
	Error _export_serialize_json_data(Ref<G4MFState4D> p_g4mf_state);
	void _export_serialize_asset_header(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json);
	Error _export_serialize_nodes(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json);

	// Import process.
	Error _import_parse_json_data(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json);
	Error _import_parse_asset_header(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json);
	Error _import_parse_nodes(Ref<G4MFState4D> p_g4mf_state, Dictionary &p_g4mf_json);
	Node4D *_import_generate_scene_node(Ref<G4MFState4D> p_g4mf_state, const int p_node_index, Node *p_parent_node, Node *p_scene_root);

protected:
	static void _bind_methods();

public:
	Error export_append_from_godot_scene(Ref<G4MFState4D> p_g4mf_state, Node *p_scene_root);
	Error export_write_to_file(Ref<G4MFState4D> p_g4mf_state, const String &p_path);
	Error import_read_from_file(Ref<G4MFState4D> p_g4mf_state, const String &p_path);
	Node4D *import_generate_godot_scene(Ref<G4MFState4D> p_g4mf_state);
};
