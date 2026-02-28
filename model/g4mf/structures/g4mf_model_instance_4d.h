#pragma once

#include "g4mf_item_4d.h"

class G4MFState4D;

class G4MFModelInstance4D : public G4MFItem4D {
	GDCLASS(G4MFModelInstance4D, G4MFItem4D);

	Dictionary _node_additional_children; // TypedDictionary<String, PackedInt32Array>
	Dictionary _node_overrides; // TypedDictionary<String, Dictionary>
	int _model_index = -1;

	Node *_find_node_by_name(Node *p_node, const String &p_node_name) const;

protected:
	static void _bind_methods();

public:
	Dictionary get_node_additional_children() const { return _node_additional_children; }
	void set_node_additional_children(const Dictionary &p_node_additional_children) { _node_additional_children = p_node_additional_children; }

	Dictionary get_node_overrides() const { return _node_overrides; }
	void set_node_overrides(const Dictionary &p_node_overrides) { _node_overrides = p_node_overrides; }

	int get_model_index() const { return _model_index; }
	void set_model_index(const int p_model_index) { _model_index = p_model_index; }

	Node *import_instantiate_model(const Ref<G4MFState4D> &p_g4mf_state) const;
	static Ref<G4MFModelInstance4D> export_pack_nodes_into_model_instance(const Variant p_g4mf_document, const Ref<G4MFState4D> &p_g4mf_state, Node *p_node, const bool p_deduplicate = true);

	static Ref<G4MFModelInstance4D> from_dictionary(const Dictionary &p_dict);
	Dictionary to_dictionary() const;
};
