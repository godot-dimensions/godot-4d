#include "editor_import_plugin_off_mesh_3d.h"

#include "../../../model/off/off_document_4d.h"

String EditorImportPluginOFFMesh3D::GDEXTMOD_GET_IMPORTER_NAME() const {
	return "godot_4d.off_geometry_format.array_mesh_3d";
}

String EditorImportPluginOFFMesh3D::GDEXTMOD_GET_RESOURCE_TYPE() const {
	return "ArrayMesh";
}

String EditorImportPluginOFFMesh3D::GDEXTMOD_GET_VISIBLE_NAME() const {
	return "ArrayMesh (3D)";
}

#if GDEXTENSION
TypedArray<Dictionary> EditorImportPluginOFFMesh3D::_get_import_options(const String &p_path, int32_t p_preset_index) const {
	TypedArray<Dictionary> options;
	Dictionary per_face_vertices;
	per_face_vertices["name"] = "per_face_vertices";
	per_face_vertices["type"] = Variant::BOOL;
	per_face_vertices["default_value"] = true;
	options.append(per_face_vertices);
	Dictionary force_outward_normals;
	force_outward_normals["name"] = "force_outward_normals";
	force_outward_normals["type"] = Variant::BOOL;
	force_outward_normals["default_value"] = false;
	options.append(force_outward_normals);
	return options;
}

Error EditorImportPluginOFFMesh3D::_import(const String &p_source_file, const String &p_save_path, const Dictionary &p_options, const TypedArray<String> &p_platform_variants, const TypedArray<String> &p_gen_files) const {
	Ref<OFFDocument4D> off_doc = OFFDocument4D::import_load_from_file(p_source_file);
	ERR_FAIL_COND_V(off_doc.is_null(), ERR_FILE_CANT_OPEN);
	Ref<ArrayMesh> mesh_3d = off_doc->import_generate_mesh_3d(p_options[StringName("per_face_vertices")], p_options[StringName("force_outward_normals")]);
	ERR_FAIL_COND_V(mesh_3d.is_null(), ERR_FILE_CORRUPT);
	mesh_3d->set_name(p_source_file.get_file());
	Error err = ResourceSaver::get_singleton()->save(mesh_3d, p_save_path + String(".res"));
	return err;
}
#elif GODOT_MODULE
void EditorImportPluginOFFMesh3D::get_import_options(const String &p_path, List<ImportOption> *r_options, int p_preset) const {
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "per_face_vertices"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "force_outward_normals"), false));
}

#if VERSION_HEX < 0x040400
Error EditorImportPluginOFFMesh3D::import(const String &p_source_file, const String &p_save_path, const HashMap<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata)
#else
Error EditorImportPluginOFFMesh3D::import(ResourceUID::ID p_source_id, const String &p_source_file, const String &p_save_path, const HashMap<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata)
#endif
{
	Ref<OFFDocument4D> off_doc = OFFDocument4D::import_load_from_file(p_source_file);
	ERR_FAIL_COND_V(off_doc.is_null(), ERR_FILE_CANT_OPEN);
	Ref<ArrayMesh> mesh_3d = off_doc->import_generate_mesh_3d(p_options[StringName("per_face_vertices")], p_options[StringName("force_outward_normals")]);
	ERR_FAIL_COND_V(mesh_3d.is_null(), ERR_FILE_CORRUPT);
	mesh_3d->set_name(p_source_file.get_file());
	Error err = ResourceSaver::save(mesh_3d, p_save_path + String(".res"));
	return err;
}
#endif
