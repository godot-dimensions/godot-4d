#include "editor_import_plugin_off_poly_4d.h"

#include "../../../math/math_4d.h"
#include "../../../model/off/off_document_4d.h"

String EditorImportPluginOFFPoly4D::GDEXTMOD_GET_IMPORTER_NAME() const {
	return "godot_4d.off_geometry_format.array_poly_mesh_4d";
}

String EditorImportPluginOFFPoly4D::GDEXTMOD_GET_RESOURCE_TYPE() const {
	return "ArrayPolyMesh4D";
}

String EditorImportPluginOFFPoly4D::GDEXTMOD_GET_VISIBLE_NAME() const {
	return "ArrayPolyMesh4D";
}

void EditorImportPluginOFFPoly4D::_make_single_convex_3d_cell(Ref<ArrayPolyMesh4D> p_mesh) {
	Vector<Vector<PackedInt32Array>> poly_cell_indices = p_mesh->get_poly_cell_indices();
	poly_cell_indices.resize(2);
	poly_cell_indices.set(1, Vector<PackedInt32Array>{ p_mesh->make_single_cell_from_all_faces() });
	p_mesh->set_poly_cell_indices(poly_cell_indices);
}

void EditorImportPluginOFFPoly4D::_make_single_convex_4d_volume(Ref<ArrayPolyMesh4D> p_mesh) {
	Vector<Vector<PackedInt32Array>> poly_cell_indices = p_mesh->get_poly_cell_indices();
	poly_cell_indices.resize(3);
	poly_cell_indices.set(2, Vector<PackedInt32Array>{ p_mesh->make_single_volume_from_all_cells() });
	p_mesh->set_poly_cell_indices(poly_cell_indices);
}

#if GDEXTENSION
TypedArray<Dictionary> EditorImportPluginOFFPoly4D::_get_import_options(const String &p_path, int32_t p_preset_index) const {
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
	Dictionary single_convex_volume;
	single_convex_volume["name"] = "single_convex_volume";
	single_convex_volume["type"] = Variant::BOOL;
	single_convex_volume["default_value"] = false;
	options.append(single_convex_volume);
	return options;
}

Error EditorImportPluginOFFPoly4D::_import(const String &p_source_file, const String &p_save_path, const Dictionary &p_options, const TypedArray<String> &p_platform_variants, const TypedArray<String> &p_gen_files) const {
	Ref<OFFDocument4D> off_doc = OFFDocument4D::import_load_from_file(p_source_file);
	ERR_FAIL_COND_V(off_doc.is_null(), ERR_FILE_CANT_OPEN);
	Ref<ArrayPolyMesh4D> poly_mesh = off_doc->import_generate_poly_mesh_4d();
	ERR_FAIL_COND_V(poly_mesh.is_null(), ERR_FILE_CORRUPT);
	poly_mesh->set_name(p_source_file.get_file());
	const Vector<Vector<PackedInt32Array>> &poly_cell_indices = poly_mesh->get_poly_cell_indices();
	if (poly_cell_indices.size() > 1) {
		poly_mesh->calculate_boundary_normals(ArrayPolyMesh4D::ComputeNormalsMode(int(p_options["compute_normals_mode"])), false);
		if (bool(p_options["double_sided"])) {
			poly_mesh->make_double_sided(false);
		}
		if (bool(p_options["single_convex_volume"])) {
			_make_single_convex_4d_volume(poly_mesh);
		}
	} else {
		if (bool(p_options["double_sided"])) {
			WARN_PRINT("EditorImportPluginOFFPoly4D: Ignoring double_sided option because the imported OFF file does not contain any cells.");
		}
		if (bool(p_options["single_convex_volume"])) {
			if (poly_cell_indices.size() == 1) {
				_make_single_convex_3d_cell(poly_mesh);
			} else {
				WARN_PRINT("EditorImportPluginOFFPoly4D: Ignoring single_convex_volume option because the imported OFF file does not contain any cells or faces.");
			}
		}
	}
	Error err = ResourceSaver::get_singleton()->save(poly_mesh, p_save_path + String(".res"));
	return err;
}
#elif GODOT_MODULE
void EditorImportPluginOFFPoly4D::get_import_options(const String &p_path, List<ImportOption> *r_options, int p_preset) const {
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "compute_normals_mode", PROPERTY_HINT_ENUM, "Cell Orientation Only,Force Outward and Fix Cell Orientation,Force Outward and Override Cell Orientation"), ArrayPolyMesh4D::COMPUTE_NORMALS_MODE_CELL_ORIENTATION_ONLY));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "double_sided"), false));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "single_convex_volume"), false));
}

#if VERSION_HEX < 0x040400
Error EditorImportPluginOFFPoly4D::import(const String &p_source_file, const String &p_save_path, const HashMap<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata)
#else
Error EditorImportPluginOFFPoly4D::import(ResourceUID::ID p_source_id, const String &p_source_file, const String &p_save_path, const HashMap<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata)
#endif
{
	Ref<OFFDocument4D> off_doc = OFFDocument4D::import_load_from_file(p_source_file);
	ERR_FAIL_COND_V(off_doc.is_null(), ERR_FILE_CANT_OPEN);
	Ref<ArrayPolyMesh4D> poly_mesh = off_doc->import_generate_poly_mesh_4d();
	ERR_FAIL_COND_V(poly_mesh.is_null(), ERR_FILE_CORRUPT);
	poly_mesh->set_name(p_source_file.get_file());
	const Vector<Vector<PackedInt32Array>> &poly_cell_indices = poly_mesh->get_poly_cell_indices();
	if (poly_cell_indices.size() > 1) {
		poly_mesh->calculate_boundary_normals(ArrayPolyMesh4D::ComputeNormalsMode(int(p_options["compute_normals_mode"])), false);
		if (bool(p_options["double_sided"])) {
			poly_mesh->make_double_sided(false);
		}
		if (bool(p_options["single_convex_volume"])) {
			_make_single_convex_4d_volume(poly_mesh);
		}
	} else {
		if (bool(p_options["double_sided"])) {
			WARN_PRINT("EditorImportPluginOFFPoly4D: Ignoring double_sided option because the imported OFF file does not contain any cells.");
		}
		if (bool(p_options["single_convex_volume"])) {
			if (poly_cell_indices.size() == 1) {
				_make_single_convex_3d_cell(poly_mesh);
			} else {
				WARN_PRINT("EditorImportPluginOFFPoly4D: Ignoring single_convex_volume option because the imported OFF file does not contain any cells or faces.");
			}
		}
	}
	Error err = ResourceSaver::save(poly_mesh, p_save_path + String(".res"));
	return err;
}
#endif
