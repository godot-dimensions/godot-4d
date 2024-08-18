#include "orthoplex_wire_mesh_4d.h"

#include "../tetra/orthoplex_tetra_mesh_4d.h"

const PackedInt32Array ORTHOPLEX_EDGE_INDICES = {
	0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, // -X connecting to YZW
	1, 2, 1, 3, 1, 4, 1, 5, 1, 6, 1, 7, // +X connecting to YZW
	2, 4, 2, 5, 2, 6, 2, 7, // -Y connecting to ZW
	3, 4, 3, 5, 3, 6, 3, 7, // +Y connecting to ZW
	4, 6, 4, 7, // -Z connecting to W
	5, 6, 5, 7, // +Z connecting to W
};

Vector4 OrthoplexWireMesh4D::get_half_extents() const {
	return _size * 0.5f;
}

void OrthoplexWireMesh4D::set_half_extents(const Vector4 &p_half_extents) {
	set_size(p_half_extents * 2.0f);
}

Vector4 OrthoplexWireMesh4D::get_size() const {
	return _size;
}

void OrthoplexWireMesh4D::set_size(const Vector4 &p_size) {
	if (p_size != _size) {
		_size = p_size;
		_vertices_cache.clear();
		wire_mesh_clear_cache();
	}
}

PackedInt32Array OrthoplexWireMesh4D::get_edge_indices() {
	return ORTHOPLEX_EDGE_INDICES;
}

PackedVector4Array OrthoplexWireMesh4D::get_edge_positions() {
	if (_edge_positions_cache.is_empty()) {
		for (const int i : ORTHOPLEX_EDGE_INDICES) {
			_edge_positions_cache.append(get_vertices()[i]);
		}
	}
	return _edge_positions_cache;
}

PackedVector4Array OrthoplexWireMesh4D::get_vertices() {
	if (_vertices_cache.is_empty()) {
		const Vector4 he = get_half_extents();
		_vertices_cache.append(Vector4(-he.x, 0.0f, 0.0f, 0.0f));
		_vertices_cache.append(Vector4(+he.x, 0.0f, 0.0f, 0.0f));
		_vertices_cache.append(Vector4(0.0f, -he.y, 0.0f, 0.0f));
		_vertices_cache.append(Vector4(0.0f, +he.y, 0.0f, 0.0f));
		_vertices_cache.append(Vector4(0.0f, 0.0f, -he.z, 0.0f));
		_vertices_cache.append(Vector4(0.0f, 0.0f, +he.z, 0.0f));
		_vertices_cache.append(Vector4(0.0f, 0.0f, 0.0f, -he.w));
		_vertices_cache.append(Vector4(0.0f, 0.0f, 0.0f, +he.w));
	}
	return _vertices_cache;
}

Ref<OrthoplexWireMesh4D> OrthoplexWireMesh4D::from_tetra_mesh(const Ref<OrthoplexTetraMesh4D> &p_tetra_mesh) {
	Ref<OrthoplexWireMesh4D> wire_mesh;
	wire_mesh.instantiate();
	wire_mesh->set_size(p_tetra_mesh->get_size());
	wire_mesh->set_material(p_tetra_mesh->get_material());
	return wire_mesh;
}

Ref<OrthoplexTetraMesh4D> OrthoplexWireMesh4D::to_tetra_mesh() const {
	Ref<OrthoplexTetraMesh4D> tetra_mesh;
	tetra_mesh.instantiate();
	tetra_mesh->set_size(_size);
	tetra_mesh->set_material(get_material());
	return tetra_mesh;
}

Ref<WireMesh4D> OrthoplexWireMesh4D::to_wire_mesh() {
	Ref<OrthoplexWireMesh4D> wire_mesh;
	wire_mesh.instantiate();
	wire_mesh->set_size(_size);
	wire_mesh->set_material(get_material());
	return wire_mesh;
}

void OrthoplexWireMesh4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_half_extents"), &OrthoplexWireMesh4D::get_half_extents);
	ClassDB::bind_method(D_METHOD("set_half_extents", "half_extents"), &OrthoplexWireMesh4D::set_half_extents);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "half_extents", PROPERTY_HINT_NONE, "suffix:m", PROPERTY_USAGE_NONE), "set_half_extents", "get_half_extents");

	ClassDB::bind_method(D_METHOD("get_size"), &OrthoplexWireMesh4D::get_size);
	ClassDB::bind_method(D_METHOD("set_size", "size"), &OrthoplexWireMesh4D::set_size);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "size", PROPERTY_HINT_NONE, "suffix:m"), "set_size", "get_size");

	ClassDB::bind_static_method("OrthoplexWireMesh4D", D_METHOD("from_tetra_mesh", "tetra_mesh"), &OrthoplexWireMesh4D::from_tetra_mesh);
	ClassDB::bind_method(D_METHOD("to_tetra_mesh"), &OrthoplexWireMesh4D::to_tetra_mesh);
}
