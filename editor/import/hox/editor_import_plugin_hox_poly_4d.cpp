#include "editor_import_plugin_hox_poly_4d.h"

#include "../../../math/math_4d.h"
#include "../../../model/hox/hox_document_4d.h"

String EditorImportPluginHoxPoly4D::GDEXTMOD_GET_IMPORTER_NAME() const {
	return "godot_4d.hoxel_draw.array_poly_mesh_4d";
}

String EditorImportPluginHoxPoly4D::GDEXTMOD_GET_RESOURCE_TYPE() const {
	return "ArrayPolyMesh4D";
}

String EditorImportPluginHoxPoly4D::GDEXTMOD_GET_VISIBLE_NAME() const {
	return "Merged ArrayPolyMesh4D";
}

#if GDEXTENSION
TypedArray<Dictionary> EditorImportPluginHoxPoly4D::_get_import_options(const String &p_path, int32_t p_preset_index) const {
	TypedArray<Dictionary> options;
	Dictionary convert_z_up_to_y_up;
	convert_z_up_to_y_up["name"] = "convert_z_up_to_y_up";
	convert_z_up_to_y_up["type"] = Variant::BOOL;
	convert_z_up_to_y_up["default_value"] = true;
	options.append(convert_z_up_to_y_up);
	Dictionary include_interior;
	include_interior["name"] = "include_interior";
	include_interior["type"] = Variant::BOOL;
	include_interior["default_value"] = false;
	options.append(include_interior);
	return options;
}

Error EditorImportPluginHoxPoly4D::_import(const String &p_source_file, const String &p_save_path, const Dictionary &p_options, const TypedArray<String> &p_platform_variants, const TypedArray<String> &p_gen_files) const
#elif GODOT_MODULE
void EditorImportPluginHoxPoly4D::get_import_options(const String &p_path, List<ImportOption> *r_options, int p_preset) const {
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "convert_z_up_to_y_up"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "include_interior"), false));
}

#if VERSION_HEX < 0x040400
Error EditorImportPluginHoxPoly4D::import(const String &p_source_file, const String &p_save_path, const HashMap<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata)
#else
Error EditorImportPluginHoxPoly4D::import(ResourceUID::ID p_source_id, const String &p_source_file, const String &p_save_path, const HashMap<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata)
#endif
#endif
{
	Ref<HoxDocument4D> hox_doc = HoxDocument4D::import_read_from_file(p_source_file);
	ERR_FAIL_COND_V(hox_doc.is_null(), ERR_FILE_CANT_OPEN);
	const bool include_interior = p_options[StringName("include_interior")];
	Ref<ArrayPolyMesh4D> poly_mesh = hox_doc->import_generate_poly_mesh_4d(include_interior);
	ERR_FAIL_COND_V(poly_mesh.is_null(), ERR_FILE_CORRUPT);
	ERR_FAIL_COND_V(!poly_mesh->is_mesh_data_valid(), ERR_FILE_CORRUPT);
	const bool convert_z_up_to_y_up = p_options[StringName("convert_z_up_to_y_up")];
	if (convert_z_up_to_y_up) {
		poly_mesh->transform_mesh(CONVERT_Z_UP_TO_Y_UP_TRANSFORM);
	}
	poly_mesh->set_name(p_source_file.get_file());
	const String save_file = p_save_path + String(".res");
#if GDEXTENSION
	Error err = ResourceSaver::get_singleton()->save(poly_mesh, save_file);
#elif GODOT_MODULE
	Error err = ResourceSaver::save(poly_mesh, save_file);
#endif
	return err;
}
