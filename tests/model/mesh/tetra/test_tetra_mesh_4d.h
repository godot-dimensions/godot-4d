#pragma once

#include "../../../../model/mesh/tetra/array_tetra_mesh_4d.h"
#include "../../../../model/mesh/tetra/box_tetra_mesh_4d.h"

#include "core/version.h"
#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR < 6
#include "servers/rendering_server.h"
#else
#include "servers/rendering/rendering_server.h"
#endif
#include "tests/test_macros.h"

namespace TestTetraMesh4D {
TEST_CASE("[SceneTree][TetraMesh4D] Proxy mesh size constants match Godot's surface layout") {
	Ref<BoxTetraMesh4D> box;
	box.instantiate();
	const int64_t tet_count = box->get_simplex_cell_vertex_indices().size() / 4;
	REQUIRE(tet_count > 0);
	const Ref<ArrayMesh> proxy = box->get_proxy_mesh_3d();
	REQUIRE(proxy.is_valid());
	REQUIRE(proxy->get_surface_count() == 1);
	const Array arrays = proxy->surface_get_arrays(0);
	CHECK(PackedVector3Array(arrays[Mesh::ARRAY_VERTEX]).size() == tet_count * TetraMesh4D::PROXY_VERTS_PER_TET);
	// Ask the rendering server how many bytes each vertex takes in this surface's actual format.
	uint32_t offsets[RSE::ARRAY_MAX];
	uint32_t vertex_element_size = 0;
	uint32_t normal_element_size = 0;
	uint32_t attrib_element_size = 0;
	uint32_t skin_element_size = 0;
	RS::get_singleton()->mesh_surface_make_offsets_from_format(proxy->surface_get_format(0), 1, 0, offsets, vertex_element_size, normal_element_size, attrib_element_size, skin_element_size);
	CHECK((int64_t)(vertex_element_size + normal_element_size) == TetraMesh4D::PROXY_VERTEX_BYTES_PER_VERT);
	CHECK((int64_t)attrib_element_size == TetraMesh4D::PROXY_ATTRIBUTE_BYTES_PER_VERT);
	CHECK(skin_element_size == 0);
	// The limit sits right below the rendering server's 32-bit overflow of the largest buffer.
	CHECK(TetraMesh4D::PROXY_ATTRIBUTE_BYTES_PER_VERT >= TetraMesh4D::PROXY_VERTEX_BYTES_PER_VERT);
	CHECK(TetraMesh4D::PROXY_MAX_VERTS_PER_SURFACE * TetraMesh4D::PROXY_ATTRIBUTE_BYTES_PER_VERT <= (int64_t)INT32_MAX);
	CHECK((TetraMesh4D::PROXY_MAX_VERTS_PER_SURFACE + 1) * TetraMesh4D::PROXY_ATTRIBUTE_BYTES_PER_VERT > (int64_t)INT32_MAX);
	CHECK(TetraMesh4D::PROXY_MAX_TETS_PER_SURFACE * TetraMesh4D::PROXY_VERTS_PER_TET <= TetraMesh4D::PROXY_MAX_VERTS_PER_SURFACE);
	CHECK((TetraMesh4D::PROXY_MAX_TETS_PER_SURFACE + 1) * TetraMesh4D::PROXY_VERTS_PER_TET > TetraMesh4D::PROXY_MAX_VERTS_PER_SURFACE);
}

TEST_CASE("[TetraMesh4D] Raycast basic functionality") {
	Ref<ArrayTetraMesh4D> mesh;
	mesh.instantiate();
	// Test empty mesh raycast.
	Dictionary result = mesh->raycast_intersects(Vector4(0, 0, 0, 0), Vector4(1, 0, 0, 0).normalized());
	CHECK_MESSAGE((bool)result["hit"] == false, "Raycast on empty mesh should not hit");
	// Test fast raycast on empty mesh.
	bool hit = mesh->raycast_intersects_fast(Vector4(0, 0, 0, 0), Vector4(1, 0, 0, 0).normalized());
	CHECK_MESSAGE(hit == false, "Fast raycast on empty mesh should return false");
}

TEST_CASE("[TetraMesh4D] Small box raycast") {
	Ref<BoxTetraMesh4D> mesh;
	mesh.instantiate();
	mesh->set_size(Vector4(0.05, 0.05, 0.3, 0.05));
	mesh->populate_inverse_metric_cache();

	Dictionary result = mesh->raycast_intersects(Vector4(-1, 0, 0, 0), Vector4(1, 0, 0, 0));
	CHECK((bool)result["hit"]);
	CHECK(Math::is_equal_approx((double)result["distance"], 0.975));
}
} // namespace TestTetraMesh4D
