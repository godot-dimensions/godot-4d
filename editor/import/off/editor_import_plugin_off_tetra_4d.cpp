#include "editor_import_plugin_off_tetra_4d.h"

#include "../../../model/off/off_document_4d.h"

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
	Dictionary compute_normals_mode;
	compute_normals_mode["name"] = "compute_normals_mode";
	compute_normals_mode["type"] = Variant::INT;
	compute_normals_mode["default_value"] = ArrayPolyMesh4D::COMPUTE_NORMALS_MODE_CELL_ORIENTATION_ONLY;
	compute_normals_mode["property_hint"] = PROPERTY_HINT_ENUM;
	compute_normals_mode["hint_string"] = "Cell Orientation Only,Force Outward and Fix Cell Orientation,Force Outward and Override Cell Orientation";
	options.append(compute_normals_mode);
	Dictionary double_sided;
	double_sided["name"] = "double_sided";
	double_sided["type"] = Variant::BOOL;
	double_sided["default_value"] = false;
	options.append(double_sided);
	return options;
}

Error EditorImportPluginOFFTetra4D::_import(const String &p_source_file, const String &p_save_path, const Dictionary &p_options, const TypedArray<String> &p_platform_variants, const TypedArray<String> &p_gen_files) const {
	Ref<OFFDocument4D> off_doc = OFFDocument4D::import_load_from_file(p_source_file);
	ERR_FAIL_COND_V(off_doc.is_null(), ERR_FILE_CANT_OPEN);
	Ref<ArrayPolyMesh4D> poly_mesh = off_doc->import_generate_poly_mesh_4d();
	ERR_FAIL_COND_V(poly_mesh.is_null(), ERR_FILE_CORRUPT);
	poly_mesh->calculate_boundary_normals(ArrayPolyMesh4D::ComputeNormalsMode(int(p_options["compute_normals_mode"])), false);
	Ref<ArrayTetraMesh4D> tetra_mesh = poly_mesh->to_array_tetra_mesh();
	tetra_mesh->set_name(p_source_file.get_file());
	Error err = ResourceSaver::get_singleton()->save(tetra_mesh, p_save_path + String(".res"));
	return err;
}
#elif GODOT_MODULE
void EditorImportPluginOFFTetra4D::get_import_options(const String &p_path, List<ImportOption> *r_options, int p_preset) const {
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "compute_normals_mode", PROPERTY_HINT_ENUM, "Cell Orientation Only,Force Outward and Fix Cell Orientation,Force Outward and Override Cell Orientation"), ArrayPolyMesh4D::COMPUTE_NORMALS_MODE_CELL_ORIENTATION_ONLY));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "double_sided"), false));
}

#if VERSION_HEX < 0x040400
Error EditorImportPluginOFFTetra4D::import(const String &p_source_file, const String &p_save_path, const HashMap<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata)
#else
Error EditorImportPluginOFFTetra4D::import(ResourceUID::ID p_source_id, const String &p_source_file, const String &p_save_path, const HashMap<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata)
#endif
{
	Ref<OFFDocument4D> off_doc = OFFDocument4D::import_load_from_file(p_source_file);
	ERR_FAIL_COND_V(off_doc.is_null(), ERR_FILE_CANT_OPEN);
	Ref<ArrayPolyMesh4D> poly_mesh = off_doc->import_generate_poly_mesh_4d();
	ERR_FAIL_COND_V(poly_mesh.is_null(), ERR_FILE_CORRUPT);
	poly_mesh->calculate_boundary_normals(ArrayPolyMesh4D::ComputeNormalsMode(int(p_options["compute_normals_mode"])), false);
	Ref<ArrayTetraMesh4D> tetra_mesh = poly_mesh->to_array_tetra_mesh();
	tetra_mesh->set_name(p_source_file.get_file());
	Error err = ResourceSaver::save(tetra_mesh, p_save_path + String(".res"));
	return err;
}
#endif
