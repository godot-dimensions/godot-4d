#include "mesh_instance_4d.h"

#include "../../render/rendering_server_4d.h"
#include "tetra/tetra_mesh_4d.h"

void MeshInstance4D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			RenderingServer4D::get_singleton()->register_mesh_instance(this);
		} break;
		case NOTIFICATION_EXIT_TREE: {
			RenderingServer4D::get_singleton()->unregister_mesh_instance(this);
		} break;
	}
}

Ref<Material4D> MeshInstance4D::get_active_material() const {
	Ref<Material4D> material;
	if (_material_override.is_valid()) {
		material = _material_override;
	} else if (_mesh.is_valid()) {
		material = _mesh->get_material();
	}
	if (material.is_valid()) {
		_mesh->validate_material_for_mesh(material);
	}
	return material;
}

Ref<Material4D> MeshInstance4D::get_material_override() const {
	return _material_override;
}

void MeshInstance4D::set_material_override(const Ref<Material4D> &p_material) {
	_material_override = p_material;
}

Ref<Mesh4D> MeshInstance4D::get_mesh() const {
	return _mesh;
}

void MeshInstance4D::set_mesh(const Ref<Mesh4D> &p_mesh) {
	_mesh = p_mesh;
}

Rect4 MeshInstance4D::get_rect_bounds_local(const Transform4D &p_to_target) const {
	Rect4 bounds = Rect4(p_to_target.origin, Vector4());
	const Ref<Mesh4D> mesh = get_mesh();
	if (mesh.is_null()) {
		return bounds;
	}
	const Transform4D to_target = p_to_target;
	const PackedVector4Array vertices = mesh->get_vertices();
	for (int vert_index = 0; vert_index < vertices.size(); vert_index++) {
		bounds = bounds.expand_to_point(to_target * vertices[vert_index]);
	}
	return bounds;
}

Dictionary MeshInstance4D::raycast_intersects_local(const Vector4 &p_local_from, const Vector4 &p_local_direction, const bool p_inside_is_zero) const {
	const Ref<TetraMesh4D> tetra_mesh = _mesh;
	if (tetra_mesh.is_valid()) {
		// Use the full version for the general `MeshInstance4D::raycast_intersects_local` function, which returns the distance and normal of the hit point.
		return tetra_mesh->raycast_intersects(p_local_from, p_local_direction);
	}
	// If the mesh is not a tetra mesh, fallback to using the local Rect4 bounds.
	const Rect4 local_bounds = get_rect_bounds_local();
	return local_bounds.raycast_intersects_dict(p_local_from, p_local_direction, p_inside_is_zero);
}

void MeshInstance4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_active_material"), &MeshInstance4D::get_active_material);

	ClassDB::bind_method(D_METHOD("get_material_override"), &MeshInstance4D::get_material_override);
	ClassDB::bind_method(D_METHOD("set_material_override", "material"), &MeshInstance4D::set_material_override);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "material_override", PROPERTY_HINT_RESOURCE_TYPE, "Material4D"), "set_material_override", "get_material_override");

	ClassDB::bind_method(D_METHOD("get_mesh"), &MeshInstance4D::get_mesh);
	ClassDB::bind_method(D_METHOD("set_mesh", "mesh"), &MeshInstance4D::set_mesh);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "mesh", PROPERTY_HINT_RESOURCE_TYPE, "Mesh4D"), "set_mesh", "get_mesh");
}
