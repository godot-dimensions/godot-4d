#include "box_wire_mesh_4d.h"

#include "../tetra/box_tetra_mesh_4d.h"

Vector4 BoxWireMesh4D::get_half_extents() const {
	return _size * 0.5f;
}

void BoxWireMesh4D::set_half_extents(const Vector4 &p_half_extents) {
	set_size(p_half_extents * 2.0f);
}

Vector4 BoxWireMesh4D::get_size() const {
	return _size;
}

void BoxWireMesh4D::set_size(const Vector4 &p_size) {
	if (p_size != _size) {
		_size = p_size;
		_vertices_cache.clear();
		wire_mesh_clear_cache();
	}
}

PackedInt32Array BoxWireMesh4D::get_edge_indices() {
	return BOX_EDGE_INDICES;
}

PackedVector4Array BoxWireMesh4D::get_edge_positions() {
	if (_edge_positions_cache.is_empty()) {
		for (const int i : BOX_EDGE_INDICES) {
			_edge_positions_cache.append(get_vertices()[i]);
		}
	}
	return _edge_positions_cache;
}

PackedVector4Array BoxWireMesh4D::get_vertices() {
	if (_vertices_cache.is_empty()) {
		const Vector4 he = get_half_extents();
		_vertices_cache.append(Vector4(-he.x, -he.y, -he.z, -he.w));
		_vertices_cache.append(Vector4(+he.x, -he.y, -he.z, -he.w));
		_vertices_cache.append(Vector4(-he.x, +he.y, -he.z, -he.w));
		_vertices_cache.append(Vector4(+he.x, +he.y, -he.z, -he.w));
		_vertices_cache.append(Vector4(-he.x, -he.y, +he.z, -he.w));
		_vertices_cache.append(Vector4(+he.x, -he.y, +he.z, -he.w));
		_vertices_cache.append(Vector4(-he.x, +he.y, +he.z, -he.w));
		_vertices_cache.append(Vector4(+he.x, +he.y, +he.z, -he.w));
		_vertices_cache.append(Vector4(-he.x, -he.y, -he.z, +he.w));
		_vertices_cache.append(Vector4(+he.x, -he.y, -he.z, +he.w));
		_vertices_cache.append(Vector4(-he.x, +he.y, -he.z, +he.w));
		_vertices_cache.append(Vector4(+he.x, +he.y, -he.z, +he.w));
		_vertices_cache.append(Vector4(-he.x, -he.y, +he.z, +he.w));
		_vertices_cache.append(Vector4(+he.x, -he.y, +he.z, +he.w));
		_vertices_cache.append(Vector4(-he.x, +he.y, +he.z, +he.w));
		_vertices_cache.append(Vector4(+he.x, +he.y, +he.z, +he.w));
	}
	return _vertices_cache;
}

Ref<BoxTetraMesh4D> BoxWireMesh4D::to_tetra_mesh() const {
	Ref<BoxTetraMesh4D> tetra_mesh;
	tetra_mesh.instantiate();
	tetra_mesh->set_size(_size);
	tetra_mesh->set_material(get_material());
	return tetra_mesh;
}

Ref<BoxWireMesh4D> BoxWireMesh4D::from_tetra_mesh(const Ref<BoxTetraMesh4D> &p_tetra_mesh) {
	Ref<BoxWireMesh4D> wire_mesh;
	wire_mesh.instantiate();
	wire_mesh->set_size(p_tetra_mesh->get_size());
	wire_mesh->set_material(p_tetra_mesh->get_material());
	return wire_mesh;
}

void BoxWireMesh4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_half_extents"), &BoxWireMesh4D::get_half_extents);
	ClassDB::bind_method(D_METHOD("set_half_extents", "half_extents"), &BoxWireMesh4D::set_half_extents);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "half_extents", PROPERTY_HINT_NONE, "suffix:m", PROPERTY_USAGE_NONE), "set_half_extents", "get_half_extents");

	ClassDB::bind_method(D_METHOD("get_size"), &BoxWireMesh4D::get_size);
	ClassDB::bind_method(D_METHOD("set_size", "size"), &BoxWireMesh4D::set_size);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "size", PROPERTY_HINT_NONE, "suffix:m"), "set_size", "get_size");

	ClassDB::bind_method(D_METHOD("to_tetra_mesh"), &BoxWireMesh4D::to_tetra_mesh);
	ClassDB::bind_static_method("BoxWireMesh4D", D_METHOD("from_tetra_mesh", "tetra_mesh"), &BoxWireMesh4D::from_tetra_mesh);
}
