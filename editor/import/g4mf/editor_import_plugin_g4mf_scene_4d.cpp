#include "editor_import_plugin_g4mf_scene_4d.h"

#include "../../../model/g4mf/g4mf_document_4d.h"
#include "../../../model/g4mf/g4mf_state_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#elif GODOT_MODULE
#include "scene/resources/packed_scene.h"
#endif

String EditorImportPluginG4MFScene4D::GDEXTMOD_GET_IMPORTER_NAME() const {
	return "godot_4d.g4mf.scene_4d";
}

String EditorImportPluginG4MFScene4D::GDEXTMOD_GET_RESOURCE_TYPE() const {
	return "PackedScene";
}

String EditorImportPluginG4MFScene4D::GDEXTMOD_GET_SAVE_EXTENSION() const {
	return "scn";
}

String EditorImportPluginG4MFScene4D::GDEXTMOD_GET_VISIBLE_NAME() const {
	return "4D Scene";
}

#if GDEXTENSION
TypedArray<Dictionary> EditorImportPluginG4MFScene4D::_get_import_options(const String &p_path, int32_t p_preset_index) const {
	TypedArray<Dictionary> options;
	Dictionary force_wireframe;
	force_wireframe["name"] = "force_wireframe";
	force_wireframe["type"] = Variant::BOOL;
	force_wireframe["default_value"] = false;
	options.append(force_wireframe);
	return options;
}

Error EditorImportPluginG4MFScene4D::_import(const String &p_source_file, const String &p_save_path, const Dictionary &p_options, const TypedArray<String> &p_platform_variants, const TypedArray<String> &p_gen_files) const
#elif GODOT_MODULE
void EditorImportPluginG4MFScene4D::get_import_options(const String &p_path, List<ImportOption> *r_options, int p_preset) const {
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "force_wireframe"), false));
}

#if VERSION_HEX < 0x040400
Error EditorImportPluginG4MFScene4D::import(const String &p_source_file, const String &p_save_path, const HashMap<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata)
#else
Error EditorImportPluginG4MFScene4D::import(ResourceUID::ID p_source_id, const String &p_source_file, const String &p_save_path, const HashMap<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata)
#endif
#endif
{
	Ref<G4MFDocument4D> g4mf_doc;
	Ref<G4MFState4D> g4mf_state;
	g4mf_doc.instantiate();
	g4mf_state.instantiate();
	g4mf_doc->set_force_wireframe(p_options[StringName("force_wireframe")]);
	Error err = g4mf_doc->import_read_from_file(g4mf_state, p_source_file);
	ERR_FAIL_COND_V_MSG(err != OK, err, "Editor: Failed to read G4MF document from file.");
	Node *node = g4mf_doc->import_generate_godot_scene(g4mf_state);
	ERR_FAIL_NULL_V_MSG(node, ERR_INVALID_DATA, "Editor: Failed to generate scene from G4MF document.");
	Ref<PackedScene> packed_scene;
	packed_scene.instantiate();
	err = packed_scene->pack(node);
	memdelete(node);
	ERR_FAIL_COND_V(err != OK, err);
	const String save_file = p_save_path + String(".scn");
#if GDEXTENSION
	err = ResourceSaver::get_singleton()->save(packed_scene, save_file);
#elif GODOT_MODULE
	err = ResourceSaver::save(packed_scene, save_file);
#endif
	return err;
}
