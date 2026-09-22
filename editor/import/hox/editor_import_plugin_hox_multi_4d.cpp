#include "editor_import_plugin_hox_multi_4d.h"

#include "../../../math/math_4d.h"
#include "../../../model/hox/hox_document_4d.h"
#include "../../../model/mesh/multi_surface_mesh_4d.h"

String EditorImportPluginHoxMulti4D::GDEXTMOD_GET_IMPORTER_NAME() const {
	return "godot_4d.hoxel_draw.multi_surface_mesh_4d";
}

String EditorImportPluginHoxMulti4D::GDEXTMOD_GET_RESOURCE_TYPE() const {
	return "MultiSurfaceMesh4D";
}

String EditorImportPluginHoxMulti4D::GDEXTMOD_GET_VISIBLE_NAME() const {
	return "MultiSurfaceMesh4D";
}

#if GDEXTENSION
TypedArray<Dictionary> EditorImportPluginHoxMulti4D::_get_import_options(const String &p_path, int32_t p_preset_index) const {
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
	Dictionary mesh_format;
	mesh_format["name"] = "mesh_format";
	mesh_format["type"] = Variant::INT;
	mesh_format["default_value"] = HoxDocument4D::HOX_MESH_FORMAT_POLYTOPE;
	mesh_format["hint"] = PropertyHint::PROPERTY_HINT_ENUM;
	mesh_format["hint_string"] = "Polytope,Tetrahedral";
	options.append(mesh_format);
	return options;
}

Error EditorImportPluginHoxMulti4D::_import(const String &p_source_file, const String &p_save_path, const Dictionary &p_options, const TypedArray<String> &p_platform_variants, const TypedArray<String> &p_gen_files) const
#elif GODOT_MODULE
void EditorImportPluginHoxMulti4D::get_import_options(const String &p_path, List<ImportOption> *r_options, int p_preset) const {
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "convert_z_up_to_y_up"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "include_interior"), false));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "mesh_format", PROPERTY_HINT_ENUM, "Polytope,Tetrahedral"), HoxDocument4D::HOX_MESH_FORMAT_POLYTOPE));
}

#if VERSION_HEX < 0x040400
Error EditorImportPluginHoxMulti4D::import(const String &p_source_file, const String &p_save_path, const HashMap<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata)
#else
Error EditorImportPluginHoxMulti4D::import(ResourceUID::ID p_source_id, const String &p_source_file, const String &p_save_path, const HashMap<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata)
#endif
#endif
{
	Ref<HoxDocument4D> hox_doc = HoxDocument4D::import_read_from_file(p_source_file);
	ERR_FAIL_COND_V(hox_doc.is_null(), ERR_FILE_CANT_OPEN);
	const bool include_interior = p_options[StringName("include_interior")];
	const HoxDocument4D::HoxMeshFormat mesh_format = HoxDocument4D::HoxMeshFormat(int(p_options[StringName("mesh_format")]));
	Ref<MultiSurfaceMesh4D> multi_surface_mesh = hox_doc->import_generate_multi_surface_mesh_4d(include_interior, mesh_format);
	ERR_FAIL_COND_V(multi_surface_mesh.is_null(), ERR_FILE_CORRUPT);
	ERR_FAIL_COND_V(!multi_surface_mesh->is_mesh_data_valid(), ERR_FILE_CORRUPT);
	const bool convert_z_up_to_y_up = p_options[StringName("convert_z_up_to_y_up")];
	if (convert_z_up_to_y_up) {
		multi_surface_mesh->transform_mesh(CONVERT_Z_UP_TO_Y_UP_TRANSFORM);
	}
	multi_surface_mesh->set_name(p_source_file.get_file());
	const String save_file = p_save_path + String(".res");
#if GDEXTENSION
	Error err = ResourceSaver::get_singleton()->save(multi_surface_mesh, save_file);
#elif GODOT_MODULE
	Error err = ResourceSaver::save(multi_surface_mesh, save_file);
#endif
	return err;
}
