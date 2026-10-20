#pragma once

#include "../g4mf_defines_4d.h"

#include "../../off/off_document_4d.h"
#include "g4mf_file_reference_4d.h"

#if GDEXTENSION
// Assume that GDExtension users have all the built-in modules enabled.
#define MODULE_GLTF_ENABLED
#include <godot_cpp/classes/gltf_document.hpp>
#include <godot_cpp/classes/gltf_state.hpp>
#elif GODOT_MODULE
#include "modules/modules_enabled.gen.h"
#ifdef MODULE_GLTF_ENABLED
#include "modules/gltf/gltf_document.h"
#include "modules/gltf/gltf_state.h"
#endif // MODULE_GLTF_ENABLED
#endif

class G4MFModel4D : public G4MFFileReference4D {
	GDCLASS(G4MFModel4D, G4MFFileReference4D);

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
	String _model_file_uri_path = "";
	// Keep the preferred stem independent of generated extensions and collision suffixes.
	String _export_preferred_path_stem;
	// Distinguish a caller's URI edit from the result of the previous preparation, including embedding.
	String _export_last_file_uri;
	// Preserve the conversion caller's deduplication opt-out when preparing external files.
	bool _export_allow_file_reuse = true;

	static String _get_model_path_stem(const String &p_path);

protected:
	static void _bind_methods();
	virtual Error _export_prepare_for_serialization_internal(const Ref<G4MFState4D> &p_g4mf_state, const bool p_deduplicate) override;

public:
	Variant get_model_g4mf_document() const;
	void set_model_g4mf_document(const Variant p_model_g4mf_document);

	virtual Error import_parse_file_data(const Ref<G4MFState4D> &p_g4mf_state) override;
	Node *import_instantiate_model(const Ref<G4MFState4D> &p_g4mf_state) const;
	static int export_pack_nodes_into_model(const Variant p_g4mf_document, const Ref<G4MFState4D> &p_g4mf_state, Node *p_node, const bool p_deduplicate = true);

	static Ref<G4MFModel4D> from_dictionary(const Dictionary &p_dict);
	Dictionary to_dictionary() const;
};
