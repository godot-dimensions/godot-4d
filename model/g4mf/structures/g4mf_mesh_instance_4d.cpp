#include "g4mf_mesh_instance_4d.h"

#include "../../mesh/mesh_instance_4d.h"
#include "../g4mf_state_4d.h"

MeshInstance4D *G4MFMeshInstance4D::import_generate_mesh_instance(const Ref<G4MFState4D> &p_g4mf_state) const {
	MeshInstance4D *ret_node = memnew(MeshInstance4D);
	Ref<G4MFMesh4D> g4mf_mesh;
	if (_mesh_index >= 0) {
		const TypedArray<G4MFMesh4D> state_g4mf_meshes = p_g4mf_state->get_g4mf_meshes();
		ERR_FAIL_INDEX_V(_mesh_index, state_g4mf_meshes.size(), ret_node);
		g4mf_mesh = state_g4mf_meshes[_mesh_index];
		ERR_FAIL_COND_V(g4mf_mesh.is_null(), ret_node);
		Ref<Mesh4D> mesh = g4mf_mesh->import_generate_mesh(p_g4mf_state);
		if (mesh.is_valid()) {
			ret_node->set_mesh(mesh);
		}
	}
	const TypedArray<G4MFMeshSurface4D> g4mf_mesh_surfaces = g4mf_mesh->get_surfaces();
	const TypedArray<G4MFMaterial4D> state_g4mf_materials = p_g4mf_state->get_g4mf_materials();
	for (int i = 0; i < _material_indices.size(); i++) {
		int material_index = _material_indices[i];
		if (material_index == -1) {
			continue; // Not overriding a material is allowed.
		}
		ERR_FAIL_INDEX_V(material_index, state_g4mf_materials.size(), ret_node);
		const Ref<G4MFMaterial4D> g4mf_material = state_g4mf_materials[material_index];
		const Ref<G4MFMeshSurface4D> surface = g4mf_mesh_surfaces[i];
		Ref<Material4D> material;
		if (surface->get_simplexes_accessor_index() < 0) {
			material = g4mf_material->generate_wire_material(p_g4mf_state);
		} else {
			material = g4mf_material->generate_tetra_material(p_g4mf_state);
		}
		if (material.is_valid()) {
			// TODO: Support per-surface material overrides instead of just the single override.
			ret_node->set_material_override(material);
		}
	}
	return ret_node;
}

Ref<G4MFMeshInstance4D> G4MFMeshInstance4D::export_convert_mesh_instance(const Ref<G4MFState4D> &p_g4mf_state, const MeshInstance4D *p_mesh_instance) {
	Ref<G4MFMeshInstance4D> ret;
	ret.instantiate();
	ERR_FAIL_COND_V(p_g4mf_state.is_null(), ret);
	ERR_FAIL_NULL_V(p_mesh_instance, ret);
	const Ref<Mesh4D> mesh = p_mesh_instance->get_mesh();
	if (mesh.is_valid()) {
		const int mesh_index = G4MFMesh4D::export_convert_mesh_into_state(p_g4mf_state, mesh, true);
		ret->set_mesh_index(mesh_index);
		Ref<Material4D> material = p_mesh_instance->get_material_override();
		if (material.is_valid()) {
			PackedInt32Array material_indices;
			// TODO: Support per-surface material overrides instead of just the single override.
			material_indices.append(G4MFMaterial4D::convert_material_into_state(p_g4mf_state, material, true));
			ret->set_material_indices(material_indices);
		}
	}
	return ret;
}

Ref<G4MFMeshInstance4D> G4MFMeshInstance4D::from_dictionary(const Dictionary &p_dict) {
	Ref<G4MFMeshInstance4D> mesh;
	mesh.instantiate();
	mesh->read_item_entries_from_dictionary(p_dict);
	if (p_dict.has("mesh")) {
		mesh->set_mesh_index(p_dict["mesh"]);
	}
	if (p_dict.has("materials")) {
		Array material_indices_array = p_dict["materials"];
		PackedInt32Array material_indices_packed;
		for (int i = 0; i < material_indices_array.size(); i++) {
			material_indices_packed.append(material_indices_array[i]);
		}
		mesh->set_material_indices(material_indices_packed);
	}
	return mesh;
}

Dictionary G4MFMeshInstance4D::to_dictionary() const {
	Dictionary dict = write_item_entries_to_dictionary();
	if (_mesh_index >= 0) {
		dict["mesh"] = _mesh_index;
	}
	if (_material_indices.size() > 0) {
		Array material_indices_array;
		for (int i = 0; i < _material_indices.size(); i++) {
			material_indices_array.append(_material_indices[i]);
		}
		dict["materials"] = material_indices_array;
	}
	return dict;
}

void G4MFMeshInstance4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_material_indices"), &G4MFMeshInstance4D::get_material_indices);
	ClassDB::bind_method(D_METHOD("set_material_indices", "material_indices"), &G4MFMeshInstance4D::set_material_indices);
	ClassDB::bind_method(D_METHOD("get_mesh_index"), &G4MFMeshInstance4D::get_mesh_index);
	ClassDB::bind_method(D_METHOD("set_mesh_index", "mesh_index"), &G4MFMeshInstance4D::set_mesh_index);

	ClassDB::bind_method(D_METHOD("import_generate_mesh_instance", "g4mf_state"), &G4MFMeshInstance4D::import_generate_mesh_instance);
	ClassDB::bind_static_method("G4MFMeshInstance4D", D_METHOD("export_convert_mesh_instance", "g4mf_state", "mesh_instance"), &G4MFMeshInstance4D::export_convert_mesh_instance);

	ClassDB::bind_static_method("G4MFMeshInstance4D", D_METHOD("from_dictionary", "dict"), &G4MFMeshInstance4D::from_dictionary);
	ClassDB::bind_method(D_METHOD("to_dictionary"), &G4MFMeshInstance4D::to_dictionary);

	ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT32_ARRAY, "material_indices"), "set_material_indices", "get_material_indices");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "mesh_index"), "set_mesh_index", "get_mesh_index");
}
