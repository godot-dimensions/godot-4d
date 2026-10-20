#pragma once

#include "../g4mf_defines_4d.h"

#include "../../off/off_document_4d.h"
#include "g4mf_file_reference_4d.h"

#if GDEXTENSION
// Assume that GDExtension users have all the built-in modules enabled.
#define MODULE_GLTF_ENABLED
#include <godot_cpp/classes/gltf_document.hpp>
#include <godot_cpp/classes/gltf_state.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>
#elif GODOT_MODULE
#include "core/templates/hash_map.h"
#include "core/templates/vector.h"
#include "modules/modules_enabled.gen.h"
#ifdef MODULE_GLTF_ENABLED
#include "modules/gltf/gltf_document.h"
#include "modules/gltf/gltf_state.h"
#endif // MODULE_GLTF_ENABLED
#endif

class G4MFModel4D : public G4MFItem4D {
	GDCLASS(G4MFModel4D, G4MFItem4D);

	Dictionary _node_additional_children; // TypedDictionary<String, PackedInt32Array>
	Dictionary _overrides; // TypedDictionary<String, Dictionary>
	int _file_index = -1;

	// Pending scene integration is tracked per generated model root, since one model can have multiple usages.
	// Object IDs allow callers to free parts of an instance before finalizing it.
	struct G4MFModelImportInstanceData4D {
		Vector<uint64_t> nodes_to_own;
		// Includes customized models generated inside additional-child subtrees.
		Vector<uint64_t> editable_instances;
	};
	HashMap<uint64_t, G4MFModelImportInstanceData4D> _import_instances;

	// Only models converted from an eligible instance may be reused by later exports.
	bool _export_can_deduplicate = false;
	String _export_source_path;

	// Optional. If provided, use this document for import settings.
	// This needs to be a Variant to avoid circular dependency issues.
	// This is a limitation of GDExtension C++, forward declarations don't work here.
	Variant _model_g4mf_document = Variant();
	Ref<G4MFState4D> _model_g4mf_state;
	Ref<OFFDocument4D> _model_off_document;
#ifdef MODULE_GLTF_ENABLED
	Ref<GLTFDocument> _model_gltf_document;
	Ref<GLTFState> _model_gltf_state;
#endif

	static Node *_find_node_by_name(Node *p_owner, Node *p_node, const String &p_node_name, const bool p_exclude_foreign_owned_nodes);
	static bool _export_can_deduplicate_node(Node *p_node);
	static String _export_normalize_source_path(const String &p_source_abs_path);
	static String _get_model_path_stem(const String &p_path);
	Node *_import_instantiate_g4mf(const Ref<G4MFState4D> &p_parent_g4mf_state, bool &r_has_any_override_applied);
	Error _export_prepare_file(const Ref<G4MFState4D> &p_g4mf_state, const Ref<G4MFFileReference4D> &p_file_ref, const bool p_deduplicate);

protected:
	static void _bind_methods();
	virtual Error _export_prepare_for_serialization_internal(const Ref<G4MFState4D> &p_g4mf_state, const bool p_deduplicate) override;

public:
	Variant get_model_g4mf_document() const;
	void set_model_g4mf_document(const Variant p_model_g4mf_document);

	Dictionary get_node_additional_children() const { return _node_additional_children; }
	void set_node_additional_children(const Dictionary &p_node_additional_children) { _node_additional_children = p_node_additional_children; }

	Dictionary get_overrides() const { return _overrides; }
	void set_overrides(const Dictionary &p_overrides) { _overrides = p_overrides; }

	int get_file_index() const { return _file_index; }
	void set_file_index(const int p_file_index) { _file_index = p_file_index; }

	virtual Error import_parse_file_data(const Ref<G4MFState4D> &p_g4mf_state);
	Node *import_instantiate_model(const Ref<G4MFState4D> &p_g4mf_state);
	void import_finalize_instance(Node *p_model_root, Node *p_scene_root = nullptr);
	// Native helper that transfers and consumes a nested instance's record instead of applying it.
	void import_take_instance_finalization(Node *p_model_root, Vector<uint64_t> &r_nodes_to_own, Vector<uint64_t> &r_editable_instances);

	static int export_pack_nodes_into_model(const Variant p_g4mf_document, const Ref<G4MFState4D> &p_g4mf_state, Node *p_node, const bool p_deduplicate = true);

	static Ref<G4MFModel4D> from_dictionary(const Dictionary &p_dict);
	Dictionary to_dictionary() const;
};
