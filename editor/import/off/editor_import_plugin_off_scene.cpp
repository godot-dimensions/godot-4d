#include "editor_import_plugin_off_scene.h"

#include "../../../model/off/off_document_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#elif GODOT_MODULE
#include "scene/resources/packed_scene.h"
#endif

String EditorImportPluginOFFScene::GDEXTMOD_GET_IMPORTER_NAME() const {
	return "godot_4d.off_geometry_format.scene";
}

String EditorImportPluginOFFScene::GDEXTMOD_GET_RESOURCE_TYPE() const {
	return "PackedScene";
}

String EditorImportPluginOFFScene::GDEXTMOD_GET_SAVE_EXTENSION() const {
	return "scn";
}

String EditorImportPluginOFFScene::GDEXTMOD_GET_VISIBLE_NAME() const {
	return "Scene";
}

#if GDEXTENSION
TypedArray<Dictionary> EditorImportPluginOFFScene::_get_import_options(const String &p_path, int32_t p_preset_index) const {
	TypedArray<Dictionary> options;
	Dictionary deduplicate_edges;
	deduplicate_edges["name"] = "deduplicate_edges";
	deduplicate_edges["type"] = Variant::BOOL;
	deduplicate_edges["default_value"] = true;
	options.append(deduplicate_edges);
	Dictionary per_face_vertices;
	per_face_vertices["name"] = "per_face_vertices";
	per_face_vertices["type"] = Variant::BOOL;
	per_face_vertices["default_value"] = true;
	options.append(per_face_vertices);
	return options;
}

Error EditorImportPluginOFFScene::_import(const String &p_source_file, const String &p_save_path, const Dictionary &p_options, const TypedArray<String> &p_platform_variants, const TypedArray<String> &p_gen_files) const {
	Ref<OFFDocument4D> off_doc = OFFDocument4D::load_from_file(p_source_file);
	ERR_FAIL_COND_V(off_doc.is_null(), ERR_FILE_CANT_OPEN);
	Node *node = off_doc->generate_node(p_options[StringName("deduplicate_edges")], p_options[StringName("per_face_vertices")]);
	String file = p_source_file.get_file();
	node->get("mesh").call("set_name", file);
	node->set_name(file.get_basename());
	Ref<PackedScene> packed_scene;
	packed_scene.instantiate();
	Error err = packed_scene->pack(node);
	ERR_FAIL_COND_V(err != OK, err);
	err = ResourceSaver::get_singleton()->save(packed_scene, p_save_path + String(".scn"));
	return err;
}
#elif GODOT_MODULE
void EditorImportPluginOFFScene::get_import_options(const String &p_path, List<ImportOption> *r_options, int p_preset) const {
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "deduplicate_edges"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "per_face_vertices"), true));
}

#if VERSION_HEX < 0x040400
Error EditorImportPluginOFFScene::import(const String &p_source_file, const String &p_save_path, const HashMap<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata)
#else
Error EditorImportPluginOFFScene::import(ResourceUID::ID p_source_id, const String &p_source_file, const String &p_save_path, const HashMap<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata)
#endif
{
	Ref<OFFDocument4D> off_doc = OFFDocument4D::load_from_file(p_source_file);
	ERR_FAIL_COND_V(off_doc.is_null(), ERR_FILE_CANT_OPEN);
	Node *node = off_doc->generate_node(p_options[StringName("deduplicate_edges")], p_options[StringName("per_face_vertices")]);
	String file = p_source_file.get_file();
	node->get("mesh").call("set_name", file);
	node->set_name(file.get_basename());
	Ref<PackedScene> packed_scene;
	packed_scene.instantiate();
	Error err = packed_scene->pack(node);
	ERR_FAIL_COND_V(err != OK, err);
	err = ResourceSaver::save(packed_scene, p_save_path + String(".scn"));
	return err;
}
#endif
