#include "g4mf_mesh_instance_4d.h"

#include "../../mesh/mesh_instance_4d.h"
#include "../../mesh/multi_surface_mesh_4d.h"
#include "../../mesh/single_surface_mesh_4d.h"
#include "../g4mf_state_4d.h"

MeshInstance4D *G4MFMeshInstance4D::import_generate_mesh_instance(const Ref<G4MFState4D> &p_g4mf_state) const {
	MeshInstance4D *ret_node = memnew(MeshInstance4D);
	if (!get_item_name().is_empty()) {
		ret_node->set_name(get_item_name());
	}
	Ref<G4MFMesh4D> g4mf_mesh;
	Ref<Mesh4D> godot_mesh_4d;
	if (_mesh_index >= 0) {
		const TypedArray<G4MFMesh4D> state_g4mf_meshes = p_g4mf_state->get_g4mf_meshes();
		ERR_FAIL_INDEX_V(_mesh_index, state_g4mf_meshes.size(), ret_node);
		g4mf_mesh = state_g4mf_meshes[_mesh_index];
		ERR_FAIL_COND_V(g4mf_mesh.is_null(), ret_node);
		godot_mesh_4d = g4mf_mesh->import_get_or_generate_mesh(p_g4mf_state);
		if (godot_mesh_4d.is_valid()) {
			ret_node->set_mesh(godot_mesh_4d);
		}
	}
	if (g4mf_mesh.is_null()) {
		return ret_node;
	}
	const int64_t material_count = _material_indices.size();
	if (material_count == 0) {
		return ret_node; // No material overrides.
	}
	const Ref<MultiSurfaceMesh4D> godot_multi_surface_mesh_4d = godot_mesh_4d;
	const TypedArray<G4MFMeshSurface4D> g4mf_mesh_surfaces = g4mf_mesh->get_surfaces();
	const TypedArray<G4MFMaterial4D> state_g4mf_materials = p_g4mf_state->get_g4mf_materials();
	const int64_t surface_count = g4mf_mesh_surfaces.size();
	// G4MF and MeshInstance4D share the same rule: a single index overrides every surface,
	// otherwise there is one index per surface, with -1 meaning that surface is not overridden.
	if (material_count != 1 && material_count != surface_count) {
		WARN_PRINT("G4MF import: Mesh instance '" + get_item_name() + "' has " + itos(material_count) + " material overrides for a mesh with " + itos(surface_count) + " surfaces. The array should have one entry, or one entry per surface.");
	}
	Vector<Ref<Material4D>> material_overrides;
	material_overrides.resize(surface_count);
	bool has_any_override = false;
	for (int64_t override_index = 0; override_index < material_overrides.size(); override_index++) {
		const int material_index = material_count == 1 ? _material_indices[0] : (override_index < material_count ? _material_indices[override_index] : -1);
		if (material_index == -1) {
			continue; // Not overriding a material is allowed.
		}
		ERR_FAIL_INDEX_V(material_index, state_g4mf_materials.size(), ret_node);
		const Ref<G4MFMaterial4D> g4mf_material = state_g4mf_materials[material_index];
		ERR_FAIL_COND_V(g4mf_material.is_null(), ret_node);
		// The override material's class must match the kind of mesh surface that was generated.
		// Even a single G4MF override may need different runtime material classes for different surfaces.
		const int64_t surface_index = override_index;
		Ref<Mesh4D> godot_surface_mesh_4d = godot_mesh_4d;
		if (godot_multi_surface_mesh_4d.is_valid() && surface_index >= 0 && surface_index < godot_multi_surface_mesh_4d->get_surface_meshes().size()) {
			godot_surface_mesh_4d = godot_multi_surface_mesh_4d->get_surface_meshes()[surface_index];
		}
		if (godot_surface_mesh_4d.is_null()) {
			continue;
		}
		const Ref<PolyMesh4D> godot_poly_mesh_4d = godot_surface_mesh_4d;
		const Ref<WireMesh4D> godot_wire_mesh_4d = godot_surface_mesh_4d;
		Ref<Material4D> material;
		if (godot_poly_mesh_4d.is_valid()) {
			material = g4mf_material->import_get_or_generate_poly_material(p_g4mf_state);
		} else if (godot_wire_mesh_4d.is_valid()) {
			material = g4mf_material->import_get_or_generate_wire_material(p_g4mf_state);
		} else {
			material = g4mf_material->import_get_or_generate_tetra_material(p_g4mf_state);
		}
		if (material.is_valid()) {
			material_overrides.set(override_index, material);
			has_any_override = true;
		}
	}
	if (has_any_override) {
		// Keep the single-override representation when every surface can use the same cached material.
		if (material_count == 1 && material_overrides.size() > 1) {
			bool all_same = true;
			for (int64_t i = 1; i < material_overrides.size(); i++) {
				if (material_overrides[i] != material_overrides[0]) {
					all_same = false;
					break;
				}
			}
			if (all_same) {
				material_overrides.resize(1);
			}
		}
		ret_node->set_material_overrides(material_overrides);
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
		const Vector<Ref<Material4D>> material_overrides = p_mesh_instance->get_material_overrides();
		if (!material_overrides.is_empty()) {
			// G4MF and MeshInstance4D share the same rule: a single index overrides every surface, otherwise
			// there must be one index per surface of the mesh, with -1 meaning that surface is not overridden.
			PackedInt32Array material_indices;
			bool has_any_override = false;
			if (material_overrides.size() == 1) {
				const int material_index = material_overrides[0].is_valid() ? G4MFMaterial4D::export_convert_material_into_state(p_g4mf_state, material_overrides[0], true) : -1;
				material_indices.append(material_index);
				has_any_override = material_index >= 0;
			} else {
				// Null surfaces of a MultiSurfaceMesh4D are not exported, so walk the mesh's surfaces and skip
				// the null ones to keep the material indices aligned with the G4MF surfaces that were exported.
				Vector<Ref<SingleSurfaceMesh4D>> surface_meshes;
				const Ref<MultiSurfaceMesh4D> multi_surface_mesh = mesh;
				if (multi_surface_mesh.is_valid()) {
					surface_meshes = multi_surface_mesh->get_surface_meshes();
				} else {
					surface_meshes.append(mesh);
				}
				for (int64_t surface_index = 0; surface_index < surface_meshes.size(); surface_index++) {
					if (surface_meshes[surface_index].is_null()) {
						continue;
					}
					int material_index = -1;
					if (surface_index < material_overrides.size() && material_overrides[surface_index].is_valid()) {
						material_index = G4MFMaterial4D::export_convert_material_into_state(p_g4mf_state, material_overrides[surface_index], true);
					}
					material_indices.append(material_index);
					has_any_override = has_any_override || material_index >= 0;
				}
			}
			if (has_any_override) {
				ret->set_material_indices(material_indices);
			}
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
