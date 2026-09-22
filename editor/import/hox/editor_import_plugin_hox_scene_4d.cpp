#include "editor_import_plugin_hox_scene_4d.h"

#include "../../../math/math_4d.h"
#include "../../../model/hox/hox_document_4d.h"
#include "../../../model/mesh/mesh_instance_4d.h"
#include "../../../nodes/node_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/packed_scene.hpp>
#elif GODOT_MODULE
#include "scene/resources/packed_scene.h"
#endif

String EditorImportPluginHoxScene4D::GDEXTMOD_GET_IMPORTER_NAME() const {
	return "godot_4d.hoxel_draw.scene_4d";
}

String EditorImportPluginHoxScene4D::GDEXTMOD_GET_RESOURCE_TYPE() const {
	return "PackedScene";
}

String EditorImportPluginHoxScene4D::GDEXTMOD_GET_SAVE_EXTENSION() const {
	return "scn";
}

String EditorImportPluginHoxScene4D::GDEXTMOD_GET_VISIBLE_NAME() const {
	return "4D Scene";
}

#if GDEXTENSION
TypedArray<Dictionary> EditorImportPluginHoxScene4D::_get_import_options(const String &p_path, int32_t p_preset_index) const {
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

Error EditorImportPluginHoxScene4D::_import(const String &p_source_file, const String &p_save_path, const Dictionary &p_options, const TypedArray<String> &p_platform_variants, const TypedArray<String> &p_gen_files) const
#elif GODOT_MODULE
void EditorImportPluginHoxScene4D::get_import_options(const String &p_path, List<ImportOption> *r_options, int p_preset) const {
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "convert_z_up_to_y_up"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "include_interior"), false));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "mesh_format", PROPERTY_HINT_ENUM, "Polytope,Tetrahedral"), HoxDocument4D::HOX_MESH_FORMAT_POLYTOPE));
}

#if VERSION_HEX < 0x040400
Error EditorImportPluginHoxScene4D::import(const String &p_source_file, const String &p_save_path, const HashMap<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata)
#else
Error EditorImportPluginHoxScene4D::import(ResourceUID::ID p_source_id, const String &p_source_file, const String &p_save_path, const HashMap<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata)
#endif
#endif
{
	Ref<HoxDocument4D> hox_doc = HoxDocument4D::import_read_from_file(p_source_file);
	ERR_FAIL_COND_V(hox_doc.is_null(), ERR_FILE_CANT_OPEN);
	const bool include_interior = p_options[StringName("include_interior")];
	const HoxDocument4D::HoxMeshFormat mesh_format = HoxDocument4D::HoxMeshFormat(int(p_options[StringName("mesh_format")]));
	Node4D *node_4d = hox_doc->import_generate_scene(include_interior, mesh_format);
	ERR_FAIL_NULL_V(node_4d, ERR_FILE_CORRUPT);
	const bool convert_z_up_to_y_up = p_options[StringName("convert_z_up_to_y_up")];
	if (convert_z_up_to_y_up) {
		MeshInstance4D *mesh_instance_4d = Object::cast_to<MeshInstance4D>(node_4d);
		if (mesh_instance_4d) {
			// Single mesh node case: Transform the mesh.
			const Ref<Mesh4D> mesh = mesh_instance_4d->get_mesh();
			if (mesh.is_valid()) {
				const Ref<MultiSurfaceMesh4D> multi_surface_mesh = mesh;
				if (multi_surface_mesh.is_valid()) {
					multi_surface_mesh->transform_mesh(CONVERT_Z_UP_TO_Y_UP_TRANSFORM);
				} else {
					const Ref<ArrayPolyMesh4D> array_poly_mesh = mesh;
					if (array_poly_mesh.is_valid()) {
						array_poly_mesh->transform_mesh(CONVERT_Z_UP_TO_Y_UP_TRANSFORM);
					} else {
						ERR_PRINT("Unsupported mesh type for Z-up to Y-up conversion in Hox import.");
					}
				}
			}
		} else {
			// Multiple mesh node case: Transform the child nodes, leave the meshes alone.
			for (int child_index = 0; child_index < node_4d->get_child_count(); child_index++) {
				Node4D *child_node_4d = Object::cast_to<Node4D>(node_4d->get_child(child_index));
				if (child_node_4d) {
					child_node_4d->set_transform(CONVERT_Z_UP_TO_Y_UP_TRANSFORM * child_node_4d->get_transform());
				}
			}
		}
	}
	node_4d->set_name(p_source_file.get_file());
	Ref<PackedScene> packed_scene;
	packed_scene.instantiate();
	Error err = packed_scene->pack(node_4d);
	memdelete(node_4d);
	ERR_FAIL_COND_V(err != OK, err);
	const String save_file = p_save_path + String(".scn");
#if GDEXTENSION
	err = ResourceSaver::get_singleton()->save(packed_scene, save_file);
#elif GODOT_MODULE
	err = ResourceSaver::save(packed_scene, save_file);
#endif
	return err;
}
