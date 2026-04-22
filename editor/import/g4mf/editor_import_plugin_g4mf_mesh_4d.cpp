#include "editor_import_plugin_g4mf_mesh_4d.h"

#include "../../../model/g4mf/g4mf_document_4d.h"
#include "../../../model/g4mf/g4mf_state_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#elif GODOT_MODULE
#include "scene/resources/packed_scene.h"
#endif

String EditorImportPluginG4MFMesh4D::GDEXTMOD_GET_IMPORTER_NAME() const {
	return "godot_4d.g4mf.mesh_4d";
}

String EditorImportPluginG4MFMesh4D::GDEXTMOD_GET_RESOURCE_TYPE() const {
	return "Mesh4D";
}

String EditorImportPluginG4MFMesh4D::GDEXTMOD_GET_SAVE_EXTENSION() const {
	return "res";
}

String EditorImportPluginG4MFMesh4D::GDEXTMOD_GET_VISIBLE_NAME() const {
	return "4D Mesh";
}

#if GDEXTENSION
TypedArray<Dictionary> EditorImportPluginG4MFMesh4D::_get_import_options(const String &p_path, int32_t p_preset_index) const {
	TypedArray<Dictionary> options;
	Dictionary preferred_mesh_format;
	preferred_mesh_format["name"] = "preferred_mesh_format";
	preferred_mesh_format["type"] = Variant::INT;
	preferred_mesh_format["default_value"] = G4MFMesh4D::MESH_FORMAT_POLYTOPE;
	preferred_mesh_format["hint"] = PropertyHint::PROPERTY_HINT_ENUM;
	preferred_mesh_format["hint_string"] = "Polytope,Tetrahedral,Wireframe";
	options.append(preferred_mesh_format);
	Dictionary include_invisible;
	include_invisible["name"] = "include_invisible";
	include_invisible["type"] = Variant::BOOL;
	include_invisible["default_value"] = false;
	options.append(include_invisible);
	return options;
}

Error EditorImportPluginG4MFMesh4D::_import(const String &p_source_file, const String &p_save_path, const Dictionary &p_options, const TypedArray<String> &p_platform_variants, const TypedArray<String> &p_gen_files) const
#elif GODOT_MODULE
void EditorImportPluginG4MFMesh4D::get_import_options(const String &p_path, List<ImportOption> *r_options, int p_preset) const {
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "preferred_mesh_format", PROPERTY_HINT_ENUM, "Polytope,Tetrahedral,Wireframe"), G4MFMesh4D::MESH_FORMAT_POLYTOPE));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "include_invisible"), false));
}

#if VERSION_HEX < 0x040400
Error EditorImportPluginG4MFMesh4D::import(const String &p_source_file, const String &p_save_path, const HashMap<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata)
#else
Error EditorImportPluginG4MFMesh4D::import(ResourceUID::ID p_source_id, const String &p_source_file, const String &p_save_path, const HashMap<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata)
#endif
#endif
{
	Ref<G4MFDocument4D> g4mf_doc;
	Ref<G4MFState4D> g4mf_state;
	g4mf_doc.instantiate();
	g4mf_state.instantiate();
	g4mf_state->set_preferred_mesh_format(G4MFMesh4D::MeshFormat(int(p_options[StringName("preferred_mesh_format")])));
	Error err = g4mf_doc->import_read_from_file(g4mf_state, p_source_file);
	ERR_FAIL_COND_V_MSG(err != OK, err, "Editor: Failed to read G4MF document from file. Aborting file import.");
	const bool include_invisible = p_options[StringName("include_invisible")];
	Ref<Mesh4D> mesh;
	String mesh_name;
	// Directly generate mesh 0 if there is only one mesh, otherwise generate a combined mesh with the nodes.
	if (g4mf_state->get_g4mf_meshes().size() == 1) {
		Ref<G4MFMesh4D> g4mf_mesh = g4mf_state->get_g4mf_meshes()[0];
		mesh_name = g4mf_mesh->get_name();
		mesh = g4mf_doc->import_generate_godot_mesh(g4mf_state, 0, include_invisible);
	} else {
		mesh = g4mf_doc->import_generate_godot_mesh(g4mf_state, -1, include_invisible);
	}
	ERR_FAIL_COND_V_MSG(mesh.is_null(), ERR_INVALID_DATA, "Editor: Failed to generate mesh from G4MF document.");
	// Use the file name if either there are multiple meshes or the mesh is unnamed.
	if (mesh_name.is_empty()) {
		mesh_name = p_save_path.get_file().get_basename();
	}
	mesh->set_name(mesh_name);
	const String save_file = p_save_path + String(".res");
#if GDEXTENSION
	err = ResourceSaver::get_singleton()->save(mesh, save_file);
#elif GODOT_MODULE
	err = ResourceSaver::save(mesh, save_file);
#endif
	return err;
}
