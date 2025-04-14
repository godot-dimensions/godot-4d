#include "editor_import_plugin_off_tetra_4d.h"

#include "../../../model/off/off_document.h"

String EditorImportPluginOFFTetra4D::GDEXTMOD_GET_IMPORTER_NAME() const {
	return "godot_4d.off_geometry_format.array_tetra_mesh_4d";
}

String EditorImportPluginOFFTetra4D::GDEXTMOD_GET_RESOURCE_TYPE() const {
	return "ArrayTetraMesh4D";
}

String EditorImportPluginOFFTetra4D::GDEXTMOD_GET_VISIBLE_NAME() const {
	return "ArrayTetraMesh4D";
}

#if GDEXTENSION
TypedArray<Dictionary> EditorImportPluginOFFTetra4D::_get_import_options(const String &p_path, int32_t p_preset_index) const {
	TypedArray<Dictionary> options;
	return options;
}

Error EditorImportPluginOFFTetra4D::_import(const String &p_source_file, const String &p_save_path, const Dictionary &p_options, const TypedArray<String> &p_platform_variants, const TypedArray<String> &p_gen_files) const {
	Ref<OFFDocument> off_doc = OFFDocument::load_from_file(p_source_file);
	ERR_FAIL_COND_V(off_doc.is_null(), ERR_FILE_CANT_OPEN);
	Ref<ArrayTetraMesh4D> tetra_mesh = off_doc->generate_tetra_mesh_4d();
	ERR_FAIL_COND_V(tetra_mesh.is_null(), ERR_FILE_CORRUPT);
	tetra_mesh->set_name(p_source_file.get_file());
	Error err = ResourceSaver::get_singleton()->save(tetra_mesh, p_save_path + String(".res"));
	return err;
}
#elif GODOT_MODULE
void EditorImportPluginOFFTetra4D::get_import_options(const String &p_path, List<ImportOption> *r_options, int p_preset) const {
	// Do nothing.
}

#if VERSION_HEX < 0x040400
Error EditorImportPluginOFFTetra4D::import(const String &p_source_file, const String &p_save_path, const HashMap<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata)
#else
Error EditorImportPluginOFFTetra4D::import(ResourceUID::ID p_source_id, const String &p_source_file, const String &p_save_path, const HashMap<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata)
#endif
{
	Ref<OFFDocument> off_doc = OFFDocument::load_from_file(p_source_file);
	ERR_FAIL_COND_V(off_doc.is_null(), ERR_FILE_CANT_OPEN);
	Ref<ArrayTetraMesh4D> tetra_mesh = off_doc->generate_tetra_mesh_4d();
	ERR_FAIL_COND_V(tetra_mesh.is_null(), ERR_FILE_CORRUPT);
	tetra_mesh->set_name(p_source_file.get_file());
	Error err = ResourceSaver::save(tetra_mesh, p_save_path + String(".res"));
	return err;
}
#endif
