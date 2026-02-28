#include "g4mf_model_instance_4d.h"

#include "../g4mf_state_4d.h"

Node *G4MFModelInstance4D::_find_node_by_name(Node *p_node, const String &p_node_name) const {
	if (p_node->get_name() == p_node_name) {
		return p_node;
	}
	for (int i = 0; i < p_node->get_child_count(); i++) {
		Node *child = p_node->get_child(i);
		Node *result = _find_node_by_name(child, p_node_name);
		if (result != nullptr) {
			return result;
		}
	}
	return nullptr;
}

Node *G4MFModelInstance4D::import_instantiate_model(const Ref<G4MFState4D> &p_g4mf_state) const {
	ERR_FAIL_COND_V(p_g4mf_state.is_null(), nullptr);
	// Get the model and create an instance of it.
	const TypedArray<G4MFModel4D> &state_g4mf_models = p_g4mf_state->get_g4mf_models();
	ERR_FAIL_INDEX_V(_model_index, state_g4mf_models.size(), nullptr);
	const Ref<G4MFModel4D> g4mf_model = state_g4mf_models[_model_index];
	ERR_FAIL_COND_V(g4mf_model.is_null(), nullptr);
	Node *ret = g4mf_model->import_instantiate_model(p_g4mf_state);
	ERR_FAIL_NULL_V(ret, nullptr);
	// Apply any node overrides to the instance.
	const Array node_override_name_keys = _node_overrides.keys();
	for (int i = 0; i < node_override_name_keys.size(); i++) {
		const String node_name = node_override_name_keys[i];
		Node *node = _find_node_by_name(ret, node_name);
		ERR_CONTINUE_MSG(node == nullptr, "G4MF import: Tried to override node with name '" + node_name + "', but no node with that name was found.");
		ret->set_editable_instance(node, true);
		const Dictionary node_override = _node_overrides[node_name];
		// TODO: Node overrides.
	}
	if (!_node_additional_children.is_empty()) {
		const TypedArray<G4MFNode4D> &parent_g4mf_nodes = p_g4mf_state->get_g4mf_nodes();
		const Array node_additional_children_name_keys = _node_additional_children.keys();
		for (int i = 0; i < node_additional_children_name_keys.size(); i++) {
			const String node_name = node_additional_children_name_keys[i];
			Node *node = _find_node_by_name(ret, node_name);
			ERR_CONTINUE_MSG(node == nullptr, "G4MF import: Tried to add children to node with name '" + node_name + "', but no node with that name was found.");
			ret->set_editable_instance(node, true);
			const PackedInt32Array node_add_children = _node_additional_children[node_name];
			for (const int32_t &child_index : node_add_children) {
				ERR_FAIL_INDEX_V(child_index, parent_g4mf_nodes.size(), ret);
				const Ref<G4MFNode4D> child_g4mf_node = parent_g4mf_nodes[child_index];
				ERR_FAIL_COND_V(child_g4mf_node.is_null(), ret);
				Node *child_node = child_g4mf_node->import_generate_godot_node(p_g4mf_state, node);
				ERR_FAIL_NULL_V(child_node, ret);
				node->add_child(child_node);
			}
		}
	}
	return ret;
}

Ref<G4MFModelInstance4D> G4MFModelInstance4D::export_pack_nodes_into_model_instance(const Variant p_g4mf_document, const Ref<G4MFState4D> &p_g4mf_state, Node *p_node, const bool p_deduplicate) {
	Ref<G4MFModelInstance4D> ret;
	ERR_FAIL_COND_V(p_g4mf_state.is_null(), ret);
	ERR_FAIL_NULL_V(p_node, ret);
	ret.instantiate();
	ret->set_model_index(G4MFModel4D::export_pack_nodes_into_model(p_g4mf_document, p_g4mf_state, p_node, p_deduplicate));
	return ret;
}

Ref<G4MFModelInstance4D> G4MFModelInstance4D::from_dictionary(const Dictionary &p_dict) {
	Ref<G4MFModelInstance4D> model;
	model.instantiate();
	model->read_item_entries_from_dictionary(p_dict);
	if (p_dict.has("model")) {
		model->set_model_index(p_dict["model"]);
	}
	if (p_dict.has("nodeAdditionalChildren")) {
		Dictionary node_add_children_json = p_dict["nodeAdditionalChildren"];
		// TODO: Consider changing this to TypedDictionary in a future Godot version.
		Dictionary node_add_children_godot;
		const Array node_add_children_name_keys = node_add_children_json.keys();
		for (int i = 0; i < node_add_children_name_keys.size(); i++) {
			const String node_name = node_add_children_name_keys[i];
			const PackedInt32Array node_add_children_packed = node_add_children_json[node_name];
			node_add_children_godot[node_name] = node_add_children_packed;
		}
		model->set_node_additional_children(node_add_children_godot);
	}
	if (p_dict.has("nodeOverrides")) {
		// TODO: Consider changing this to TypedDictionary in a future Godot version.
		model->set_node_overrides(p_dict["nodeOverrides"]);
	}
	return model;
}

Dictionary G4MFModelInstance4D::to_dictionary() const {
	Dictionary dict = write_item_entries_to_dictionary();
	if (_model_index >= 0) {
		dict["model"] = _model_index;
	}
	if (!_node_additional_children.is_empty()) {
		Dictionary node_add_children_json_obj;
		const Array node_add_children_name_keys = _node_additional_children.keys();
		for (int i = 0; i < node_add_children_name_keys.size(); i++) {
			const String node_name = node_add_children_name_keys[i];
			const Array node_add_children_json_arr = _node_additional_children[node_name];
			node_add_children_json_obj[node_name] = node_add_children_json_arr;
		}
		dict["nodeAdditionalChildren"] = node_add_children_json_obj;
	}
	if (!_node_overrides.is_empty()) {
		dict["nodeOverrides"] = _node_overrides;
	}
	return dict;
}

void G4MFModelInstance4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_node_additional_children"), &G4MFModelInstance4D::get_node_additional_children);
	ClassDB::bind_method(D_METHOD("set_node_additional_children", "node_additional_children"), &G4MFModelInstance4D::set_node_additional_children);
	ClassDB::bind_method(D_METHOD("get_node_overrides"), &G4MFModelInstance4D::get_node_overrides);
	ClassDB::bind_method(D_METHOD("set_node_overrides", "node_overrides"), &G4MFModelInstance4D::set_node_overrides);
	ClassDB::bind_method(D_METHOD("get_model_index"), &G4MFModelInstance4D::get_model_index);
	ClassDB::bind_method(D_METHOD("set_model_index", "model_index"), &G4MFModelInstance4D::set_model_index);

	ClassDB::bind_method(D_METHOD("import_instantiate_model", "g4mf_state"), &G4MFModelInstance4D::import_instantiate_model);
	ClassDB::bind_static_method("G4MFModelInstance4D", D_METHOD("export_pack_nodes_into_model_instance", "g4mf_document", "g4mf_state", "node", "deduplicate"), &G4MFModelInstance4D::export_pack_nodes_into_model_instance, DEFVAL(true));

	ClassDB::bind_static_method("G4MFModelInstance4D", D_METHOD("from_dictionary", "dict"), &G4MFModelInstance4D::from_dictionary);
	ClassDB::bind_method(D_METHOD("to_dictionary"), &G4MFModelInstance4D::to_dictionary);

	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "node_additional_children"), "set_node_additional_children", "get_node_additional_children");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "node_overrides"), "set_node_overrides", "get_node_overrides");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "model_index"), "set_model_index", "get_model_index");
}
