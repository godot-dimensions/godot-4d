#include "mesh_instance_4d.h"

#include "../../render/rendering_server_4d.h"
#include "multi_surface_mesh_4d.h"
#include "tetra/tetra_mesh_4d.h"

void MeshInstance4D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			RenderingServer4D::get_singleton()->register_mesh_instance(this);
		} break;
		case NOTIFICATION_EXIT_TREE: {
			// The singleton is already gone if the module was uninitialized first.
			RenderingServer4D *rendering_server = RenderingServer4D::get_singleton();
			if (rendering_server != nullptr) {
				rendering_server->unregister_mesh_instance(this);
			}
		} break;
	}
}

void MeshInstance4D::_validate_property(PropertyInfo &p_property) const {
	// For material override(s): Always show the single version if the array is empty,
	// always show the plural if the array has more than one element, and if the array
	// has exactly one element, show based on whether the mesh is a MultiSurfaceMesh4D.
	if (p_property.name == StringName("material_override") || p_property.name == StringName("material_overrides")) {
		bool show_plural;
		if (_material_overrides.size() == 0) {
			show_plural = false;
		} else if (_material_overrides.size() > 1) {
			show_plural = true;
		} else {
			const Ref<MultiSurfaceMesh4D> multi_surface_mesh = _mesh;
			show_plural = multi_surface_mesh.is_valid();
		}
		if (p_property.name == StringName("material_override")) {
			p_property.usage = show_plural ? PROPERTY_USAGE_NONE : PROPERTY_USAGE_DEFAULT;
		} else { // "material_overrides"
			p_property.usage = show_plural ? PROPERTY_USAGE_DEFAULT : PROPERTY_USAGE_NONE;
		}
	}
}

Ref<Material4D> MeshInstance4D::_get_valid_active_material_for_surface(const Ref<SingleSurfaceMesh4D> &p_surface_mesh, Ref<Material4D> p_material) {
	if (p_material.is_null()) {
		p_material = p_surface_mesh->get_material();
		if (p_material.is_null()) {
			// Use the fallback material from the surface mesh. Don't validate fallback materials ever, so return.
			return p_surface_mesh->get_fallback_material();
		}
	}
	// If both the surface mesh and material are valid, ensure the material is compatible with the mesh.
	if (p_material.is_valid()) {
		p_surface_mesh->validate_material_for_mesh(p_material);
	}
	return p_material;
}

Ref<Material4D> MeshInstance4D::get_active_material(const int p_surface_index) const {
	Ref<Material4D> material;
	const int material_override_count = _material_overrides.size();
	// Check the material overrides. Null here means to use the material from the mesh itself.
	if (material_override_count == 1) {
		// If there is a single material override, use it for all surfaces.
		material = _material_overrides[0];
	} else if (p_surface_index < material_override_count && 0 <= p_surface_index) {
		// If there are multiple material overrides, use the one corresponding to the surface index.
		material = _material_overrides[p_surface_index];
	}
	// Check the mesh for a material, and/or for material validity, as needed.
	if (_mesh.is_valid()) {
		// If the overrides have not provided a material, try to get it from the mesh itself.
		const Ref<SingleSurfaceMesh4D> single_surface_mesh = _mesh;
		if (single_surface_mesh.is_valid()) {
			// Single-surface mesh: These have materials and fallback materials defined, so use it if no override is provided.
			material = _get_valid_active_material_for_surface(single_surface_mesh, material);
		} else {
			// Multi-surface mesh: These do not have materials, but their surfaces do.
			const Ref<MultiSurfaceMesh4D> multi_surface_mesh = _mesh;
			if (multi_surface_mesh.is_valid()) {
				const Vector<Ref<SingleSurfaceMesh4D>> &surface_meshes = multi_surface_mesh->get_surface_meshes();
				if (p_surface_index < surface_meshes.size() && 0 <= p_surface_index) {
					const Ref<SingleSurfaceMesh4D> &surface_mesh = surface_meshes[p_surface_index];
					if (surface_mesh.is_valid()) {
						material = _get_valid_active_material_for_surface(surface_mesh, material);
					}
				}
			}
		}
	}
	// Note: It is possible that the returned material is still null, since meshes can return a null fallback material.
	// Therefore, rendering engines MUST handle the case of this returning null.
	return material;
}

Ref<Material4D> MeshInstance4D::get_material_override() const {
	if (_material_overrides.is_empty()) {
		return Ref<Material4D>();
	}
	return _material_overrides[0];
}

void MeshInstance4D::set_material_override(const Ref<Material4D> &p_material) {
	if (p_material.is_valid()) {
		// Set the material override for all surfaces by using an array with a single element.
		_material_overrides.resize(1);
		_material_overrides.set(0, p_material);
	} else {
		// Set no material override for all surfaces by clearing the array.
		_material_overrides.clear();
	}
	notify_property_list_changed();
}

Vector<Ref<Material4D>> MeshInstance4D::get_material_overrides() const {
	return _material_overrides;
}

void MeshInstance4D::set_material_overrides(const Vector<Ref<Material4D>> &p_material_overrides) {
	_material_overrides = p_material_overrides;
	notify_property_list_changed();
}

TypedArray<Material4D> MeshInstance4D::get_material_overrides_bind() const {
	TypedArray<Material4D> bind;
	bind.resize(_material_overrides.size());
	for (int i = 0; i < _material_overrides.size(); ++i) {
		bind[i] = _material_overrides[i];
	}
	return bind;
}

void MeshInstance4D::set_material_overrides_bind(const TypedArray<Material4D> &p_material_overrides) {
	_material_overrides.resize(p_material_overrides.size());
	for (int i = 0; i < p_material_overrides.size(); ++i) {
		_material_overrides.set(i, p_material_overrides[i]);
	}
	notify_property_list_changed();
}

Ref<Mesh4D> MeshInstance4D::get_mesh() const {
	return _mesh;
}

void MeshInstance4D::set_mesh(const Ref<Mesh4D> &p_mesh) {
	// The property list only depends on whether the mesh is a MultiSurfaceMesh4D, see `_validate_property`.
	// Only notify when that changes. Notifying on every assignment would rebuild the inspector each time, which
	// interrupts dragging an inspector slider on any node that regenerates its mesh from its other properties.
	const bool was_multi_surface = Ref<MultiSurfaceMesh4D>(_mesh).is_valid();
	_mesh = p_mesh;
	const bool is_multi_surface = Ref<MultiSurfaceMesh4D>(_mesh).is_valid();
	if (was_multi_surface != is_multi_surface) {
		notify_property_list_changed();
	}
}

Rect4 MeshInstance4D::get_rect_bounds_local(const Transform4D &p_to_target) const {
	const Ref<Mesh4D> mesh = get_mesh();
	if (mesh.is_null()) {
		return Rect4(p_to_target.origin, Vector4());
	}
	return p_to_target.xform_rect(mesh->get_rect_bounds());
}

Dictionary MeshInstance4D::raycast_intersects_local(const Vector4 &p_local_from, const Vector4 &p_local_direction, const real_t p_max_distance, const bool p_inside_is_zero) const {
	const Ref<TetraMesh4D> tetra_mesh = _mesh;
	if (tetra_mesh.is_valid()) {
		// Use the full version for the general `MeshInstance4D::raycast_intersects_local` function, which returns the distance and normal of the hit point.
		// This code path ignores the `p_inside_is_zero` parameter, since tetra meshes are concave and have no true concept of "inside" or "outside" the mesh.
		return tetra_mesh->raycast_intersects(p_local_from, p_local_direction, p_max_distance);
	}
	// If the mesh is not a tetra mesh, fallback to using the local Rect4 bounds.
	const Rect4 local_bounds = get_rect_bounds_local();
	return local_bounds.raycast_intersects_dict(p_local_from, p_local_direction, p_max_distance, p_inside_is_zero);
}

void MeshInstance4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_active_material", "surface_index"), &MeshInstance4D::get_active_material, DEFVAL(0));

	ClassDB::bind_method(D_METHOD("get_material_override"), &MeshInstance4D::get_material_override);
	ClassDB::bind_method(D_METHOD("set_material_override", "material"), &MeshInstance4D::set_material_override);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "material_override", PROPERTY_HINT_RESOURCE_TYPE, "Material4D"), "set_material_override", "get_material_override");

	ClassDB::bind_method(D_METHOD("get_material_overrides"), &MeshInstance4D::get_material_overrides_bind);
	ClassDB::bind_method(D_METHOD("set_material_overrides", "material_overrides"), &MeshInstance4D::set_material_overrides_bind);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "material_overrides", PROPERTY_HINT_ARRAY_TYPE, "Material4D", PROPERTY_USAGE_NONE), "set_material_overrides", "get_material_overrides");

	ClassDB::bind_method(D_METHOD("get_mesh"), &MeshInstance4D::get_mesh);
	ClassDB::bind_method(D_METHOD("set_mesh", "mesh"), &MeshInstance4D::set_mesh);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "mesh", PROPERTY_HINT_RESOURCE_TYPE, "Mesh4D"), "set_mesh", "get_mesh");
}
