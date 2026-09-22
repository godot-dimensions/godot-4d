#include "editor_import_plugin_4do_multi_4d.h"

#include "../../../math/math_4d.h"
#include "../../../model/4do/4do_document_4d.h"
#include "../../../model/mesh/multi_surface_mesh_4d.h"

String EditorImportPlugin4DOMulti4D::GDEXTMOD_GET_IMPORTER_NAME() const {
	return "godot_4d.4do.multi_surface_mesh_4d";
}

String EditorImportPlugin4DOMulti4D::GDEXTMOD_GET_RESOURCE_TYPE() const {
	return "MultiSurfaceMesh4D";
}

String EditorImportPlugin4DOMulti4D::GDEXTMOD_GET_VISIBLE_NAME() const {
	return "MultiSurfaceMesh4D";
}

#if GDEXTENSION
TypedArray<Dictionary> EditorImportPlugin4DOMulti4D::_get_import_options(const String &p_path, int32_t p_preset_index) const {
	TypedArray<Dictionary> options;
	Dictionary convert_z_up_to_y_up;
	convert_z_up_to_y_up["name"] = "convert_z_up_to_y_up";
	convert_z_up_to_y_up["type"] = Variant::BOOL;
	convert_z_up_to_y_up["default_value"] = true;
	options.append(convert_z_up_to_y_up);
	return options;
}

Error EditorImportPlugin4DOMulti4D::_import(const String &p_source_file, const String &p_save_path, const Dictionary &p_options, const TypedArray<String> &p_platform_variants, const TypedArray<String> &p_gen_files) const
#elif GODOT_MODULE
void EditorImportPlugin4DOMulti4D::get_import_options(const String &p_path, List<ImportOption> *r_options, int p_preset) const {
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "convert_z_up_to_y_up"), true));
}

#if VERSION_HEX < 0x040400
Error EditorImportPlugin4DOMulti4D::import(const String &p_source_file, const String &p_save_path, const HashMap<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata)
#else
Error EditorImportPlugin4DOMulti4D::import(ResourceUID::ID p_source_id, const String &p_source_file, const String &p_save_path, const HashMap<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata)
#endif
#endif
{
	Ref<FourDODocument4D> the_4do_doc = FourDODocument4D::import_read_from_file(p_source_file);
	ERR_FAIL_COND_V(the_4do_doc.is_null(), ERR_FILE_CANT_OPEN);
	Ref<MultiSurfaceMesh4D> multi_surface_mesh = the_4do_doc->import_generate_multi_surface_mesh_4d();
	ERR_FAIL_COND_V(multi_surface_mesh.is_null(), ERR_FILE_CORRUPT);
	ERR_FAIL_COND_V(!multi_surface_mesh->is_mesh_data_valid(), ERR_FILE_CORRUPT);
	const bool convert_z_up_to_y_up = p_options[StringName("convert_z_up_to_y_up")];
	if (convert_z_up_to_y_up) {
		multi_surface_mesh->transform_mesh(CONVERT_Z_UP_TO_Y_UP_TRANSFORM);
	}
	multi_surface_mesh->set_name(p_source_file.get_file());
	const String save_path_with_ext = p_save_path + String(".res");
#if GDEXTENSION
	Error err = ResourceSaver::get_singleton()->save(multi_surface_mesh, save_path_with_ext);
#elif GODOT_MODULE
	Error err = ResourceSaver::save(multi_surface_mesh, save_path_with_ext);
#endif
	return err;
}
