#include "mesh_instance_4d.h"

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

void MeshInstance4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_material_override"), &MeshInstance4D::get_material_override);
	ClassDB::bind_method(D_METHOD("set_material_override", "material"), &MeshInstance4D::set_material_override);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "material_override", PROPERTY_HINT_RESOURCE_TYPE, "Material4D"), "set_material_override", "get_material_override");

	ClassDB::bind_method(D_METHOD("get_mesh"), &MeshInstance4D::get_mesh);
	ClassDB::bind_method(D_METHOD("set_mesh", "mesh"), &MeshInstance4D::set_mesh);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "mesh", PROPERTY_HINT_RESOURCE_TYPE, "Mesh4D"), "set_mesh", "get_mesh");
}
