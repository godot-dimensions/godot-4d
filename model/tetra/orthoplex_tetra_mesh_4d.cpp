#include "orthoplex_tetra_mesh_4d.h"

#include "../wire/orthoplex_wire_mesh_4d.h"

constexpr real_t _1D6 = 1.0 / 6.0;
constexpr real_t _5D6 = 5.0 / 6.0;

// We have to use #define below because static constants will crash GDExtension.
// This code is much harder to read when clang-format messes with the manual alignment,
// so disabling clang-format just for the next hundred lines or so.
/* clang-format off */
#define ORTHOPLEX_CELL_NORMALS PackedVector4Array { \
	Vector4(-0.5f, -0.5f, -0.5f, -0.5f), \
	Vector4(+0.5f, -0.5f, -0.5f, -0.5f), \
	Vector4(+0.5f, +0.5f, -0.5f, -0.5f), \
	Vector4(-0.5f, +0.5f, -0.5f, -0.5f), \
	Vector4(-0.5f, +0.5f, +0.5f, -0.5f), \
	Vector4(+0.5f, +0.5f, +0.5f, -0.5f), \
	Vector4(+0.5f, -0.5f, +0.5f, -0.5f), \
	Vector4(-0.5f, -0.5f, +0.5f, -0.5f), \
	Vector4(-0.5f, -0.5f, +0.5f, +0.5f), \
	Vector4(+0.5f, -0.5f, +0.5f, +0.5f), \
	Vector4(+0.5f, +0.5f, +0.5f, +0.5f), \
	Vector4(-0.5f, +0.5f, +0.5f, +0.5f), \
	Vector4(-0.5f, +0.5f, -0.5f, +0.5f), \
	Vector4(+0.5f, +0.5f, -0.5f, +0.5f), \
	Vector4(+0.5f, -0.5f, -0.5f, +0.5f), \
	Vector4(-0.5f, -0.5f, -0.5f, +0.5f), \
}

#define ORTHOPLEX_CELL_INDICES PackedInt32Array { \
	6, 0, 2, 4, 1, 2, 4, 6, 6, 1, 3, 4, 0, 3, 4, 6, /* -W and -Z */ \
	6, 0, 3, 5, 1, 3, 5, 6, 6, 1, 2, 5, 0, 2, 5, 6, /* -W and +Z */ \
	7, 0, 2, 5, 1, 2, 5, 7, 7, 1, 3, 5, 0, 3, 5, 7, /* +W and +Z */ \
	7, 0, 3, 4, 1, 3, 4, 7, 7, 1, 2, 4, 0, 2, 4, 7, /* +W and -Z */ \
}

#define ORTHOPLEX_CELL_UVW_MAP PackedVector3Array { \
	/* -W and -Z */ \
	Vector3(_1D6, _1D6, _1D6), Vector3(0.0f, 0.5f, 0.5f), Vector3(0.5f, 0.0f, 0.5f), Vector3(0.5f, 0.5f, 0.0f), \
	Vector3(1.0f, 0.5f, 0.5f), Vector3(0.5f, 0.0f, 0.5f), Vector3(0.5f, 0.5f, 0.0f), Vector3(_5D6, _1D6, _1D6), \
	Vector3(_5D6, _5D6, _1D6), Vector3(1.0f, 0.5f, 0.5f), Vector3(0.5f, 1.0f, 0.5f), Vector3(0.5f, 0.5f, 0.0f), \
	Vector3(0.0f, 0.5f, 0.5f), Vector3(0.5f, 1.0f, 0.5f), Vector3(0.5f, 0.5f, 0.0f), Vector3(_1D6, _5D6, _1D6), \
	/* -W and +Z */ \
	Vector3(_1D6, _5D6, _5D6), Vector3(0.0f, 0.5f, 0.5f), Vector3(0.5f, 1.0f, 0.5f), Vector3(0.5f, 0.5f, 1.0f), \
	Vector3(1.0f, 0.5f, 0.5f), Vector3(0.5f, 1.0f, 0.5f), Vector3(0.5f, 0.5f, 1.0f), Vector3(_5D6, _5D6, _5D6), \
	Vector3(_5D6, _1D6, _5D6), Vector3(1.0f, 0.5f, 0.5f), Vector3(0.5f, 0.0f, 0.5f), Vector3(0.5f, 0.5f, 1.0f), \
	Vector3(0.0f, 0.5f, 0.5f), Vector3(0.5f, 0.0f, 0.5f), Vector3(0.5f, 0.5f, 1.0f), Vector3(_1D6, _1D6, _5D6), \
	/* +W and +Z */ \
	Vector3(0.5f, 0.5f, 0.5f), Vector3(0.0f, 0.5f, 0.5f), Vector3(0.5f, 0.0f, 0.5f), Vector3(0.5f, 0.5f, 1.0f), \
	Vector3(1.0f, 0.5f, 0.5f), Vector3(0.5f, 0.0f, 0.5f), Vector3(0.5f, 0.5f, 1.0f), Vector3(0.5f, 0.5f, 0.5f), \
	Vector3(0.5f, 0.5f, 0.5f), Vector3(1.0f, 0.5f, 0.5f), Vector3(0.5f, 1.0f, 0.5f), Vector3(0.5f, 0.5f, 1.0f), \
	Vector3(0.0f, 0.5f, 0.5f), Vector3(0.5f, 1.0f, 0.5f), Vector3(0.5f, 0.5f, 1.0f), Vector3(0.5f, 0.5f, 0.5f), \
	/* +W and -Z */ \
	Vector3(0.5f, 0.5f, 0.5f), Vector3(0.0f, 0.5f, 0.5f), Vector3(0.5f, 1.0f, 0.5f), Vector3(0.5f, 0.5f, 0.0f), \
	Vector3(1.0f, 0.5f, 0.5f), Vector3(0.5f, 1.0f, 0.5f), Vector3(0.5f, 0.5f, 0.0f), Vector3(0.5f, 0.5f, 0.5f), \
	Vector3(0.5f, 0.5f, 0.5f), Vector3(1.0f, 0.5f, 0.5f), Vector3(0.5f, 0.0f, 0.5f), Vector3(0.5f, 0.5f, 0.0f), \
	Vector3(0.0f, 0.5f, 0.5f), Vector3(0.5f, 0.0f, 0.5f), Vector3(0.5f, 0.5f, 0.0f), Vector3(0.5f, 0.5f, 0.5f), \
};
/* clang-format on */

void OrthoplexTetraMesh4D::_clear_caches() {
	_cell_positions_cache.clear();
	_vertices_cache.clear();
}

Vector4 OrthoplexTetraMesh4D::get_half_extents() const {
	return _size * 0.5f;
}

void OrthoplexTetraMesh4D::set_half_extents(const Vector4 &p_half_extents) {
	_size = p_half_extents * 2.0f;
	_clear_caches();
}

Vector4 OrthoplexTetraMesh4D::get_size() const {
	return _size;
}

void OrthoplexTetraMesh4D::set_size(const Vector4 &p_size) {
	_size = p_size;
	_clear_caches();
}

PackedInt32Array OrthoplexTetraMesh4D::get_cell_indices() {
	return ORTHOPLEX_CELL_INDICES;
}

PackedVector4Array OrthoplexTetraMesh4D::get_cell_positions() {
	if (_cell_positions_cache.is_empty()) {
		const PackedVector4Array vertices = get_vertices();
		for (const int i : ORTHOPLEX_CELL_INDICES) {
			_cell_positions_cache.append(vertices[i]);
		}
	}
	return _cell_positions_cache;
}

PackedVector4Array OrthoplexTetraMesh4D::get_cell_normals() {
	return ORTHOPLEX_CELL_NORMALS;
}

PackedVector3Array OrthoplexTetraMesh4D::get_cell_uvw_map() {
	return ORTHOPLEX_CELL_UVW_MAP;
}

PackedVector4Array OrthoplexTetraMesh4D::get_vertices() {
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

Ref<OrthoplexTetraMesh4D> OrthoplexTetraMesh4D::from_orthoplex_wire_mesh(const Ref<OrthoplexWireMesh4D> &p_wire_mesh) {
	Ref<OrthoplexTetraMesh4D> tetra_mesh;
	tetra_mesh.instantiate();
	tetra_mesh->set_size(p_wire_mesh->get_size());
	tetra_mesh->set_material(p_wire_mesh->get_material());
	return tetra_mesh;
}

Ref<OrthoplexWireMesh4D> OrthoplexTetraMesh4D::to_orthoplex_wire_mesh() const {
	Ref<OrthoplexWireMesh4D> wire_mesh;
	wire_mesh.instantiate();
	wire_mesh->set_size(_size);
	wire_mesh->set_material(get_material());
	return wire_mesh;
}

Ref<WireMesh4D> OrthoplexTetraMesh4D::to_wire_mesh() {
	return to_orthoplex_wire_mesh();
}

void OrthoplexTetraMesh4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_half_extents"), &OrthoplexTetraMesh4D::get_half_extents);
	ClassDB::bind_method(D_METHOD("set_half_extents", "half_extents"), &OrthoplexTetraMesh4D::set_half_extents);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "half_extents", PROPERTY_HINT_NONE, "suffix:m", PROPERTY_USAGE_NONE), "set_half_extents", "get_half_extents");

	ClassDB::bind_method(D_METHOD("get_size"), &OrthoplexTetraMesh4D::get_size);
	ClassDB::bind_method(D_METHOD("set_size", "size"), &OrthoplexTetraMesh4D::set_size);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "size", PROPERTY_HINT_NONE, "suffix:m"), "set_size", "get_size");

	ClassDB::bind_static_method("OrthoplexTetraMesh4D", D_METHOD("from_orthoplex_wire_mesh", "wire_mesh"), &OrthoplexTetraMesh4D::from_orthoplex_wire_mesh);
	ClassDB::bind_method(D_METHOD("to_orthoplex_wire_mesh"), &OrthoplexTetraMesh4D::to_orthoplex_wire_mesh);
}
