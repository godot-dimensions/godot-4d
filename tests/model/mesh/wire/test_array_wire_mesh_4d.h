#pragma once

#include "../../../../model/mesh/wire/array_wire_mesh_4d.h"

#include "core/version.h"
#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR < 6
#include "servers/rendering_server.h"
#else
#include "servers/rendering/rendering_server.h"
#endif
#include "tests/test_macros.h"

namespace TestArrayWireMesh4D {
TEST_CASE("[ArrayWireMesh4D] Merge Meshes") {
}

TEST_CASE("[SceneTree][WireMesh4D] Proxy mesh size constants match Godot's surface layout") {
	Ref<ArrayWireMesh4D> wire;
	wire.instantiate();
	wire->set_vertex_positions({ Vector4(0, 0, 0, 0), Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) });
	wire->set_edge_indices({ 0, 1, 1, 2, 2, 0 });
	REQUIRE(wire->is_mesh_data_valid());
	const int64_t edge_count = wire->get_edge_indices().size() / 2;
	const Ref<ArrayMesh> proxy = wire->get_proxy_mesh_3d();
	REQUIRE(proxy.is_valid());
	REQUIRE(proxy->get_surface_count() == 1);
	const Array arrays = proxy->surface_get_arrays(0);
	CHECK(PackedVector3Array(arrays[Mesh::ARRAY_VERTEX]).size() == edge_count * WireMesh4D::PROXY_VERTS_PER_EDGE);
	// Ask the rendering server how many bytes each vertex takes in this surface's actual format.
	uint32_t offsets[RS::ARRAY_MAX];
	uint32_t vertex_element_size = 0;
	uint32_t normal_element_size = 0;
	uint32_t attrib_element_size = 0;
	uint32_t skin_element_size = 0;
	RS::get_singleton()->mesh_surface_make_offsets_from_format(proxy->surface_get_format(0), 1, 0, offsets, vertex_element_size, normal_element_size, attrib_element_size, skin_element_size);
	CHECK((int64_t)(vertex_element_size + normal_element_size) == WireMesh4D::PROXY_VERTEX_BYTES_PER_VERT);
	CHECK((int64_t)attrib_element_size == WireMesh4D::PROXY_ATTRIBUTE_BYTES_PER_VERT);
	CHECK(skin_element_size == 0);
	// The limit sits right below the rendering server's 32-bit overflow of the largest buffer.
	CHECK(WireMesh4D::PROXY_ATTRIBUTE_BYTES_PER_VERT >= WireMesh4D::PROXY_VERTEX_BYTES_PER_VERT);
	CHECK(WireMesh4D::PROXY_MAX_VERTS_PER_SURFACE * WireMesh4D::PROXY_ATTRIBUTE_BYTES_PER_VERT <= (int64_t)INT32_MAX);
	CHECK((WireMesh4D::PROXY_MAX_VERTS_PER_SURFACE + 1) * WireMesh4D::PROXY_ATTRIBUTE_BYTES_PER_VERT > (int64_t)INT32_MAX);
	CHECK(WireMesh4D::PROXY_MAX_EDGES_PER_SURFACE * WireMesh4D::PROXY_VERTS_PER_EDGE <= WireMesh4D::PROXY_MAX_VERTS_PER_SURFACE);
}
} // namespace TestArrayWireMesh4D
