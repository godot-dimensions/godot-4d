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
	CHECK((int64_t)skin_element_size == TetraMesh4D::PROXY_SKIN_BYTES_PER_VERT);
	// The limit sits right below the rendering server's 32-bit overflow of the largest buffer.
	CHECK(TetraMesh4D::PROXY_ATTRIBUTE_BYTES_PER_VERT >= TetraMesh4D::PROXY_VERTEX_BYTES_PER_VERT);
	CHECK(TetraMesh4D::PROXY_MAX_VERTS_PER_SURFACE * TetraMesh4D::PROXY_ATTRIBUTE_BYTES_PER_VERT <= (int64_t)INT32_MAX);
	CHECK((TetraMesh4D::PROXY_MAX_VERTS_PER_SURFACE + 1) * TetraMesh4D::PROXY_ATTRIBUTE_BYTES_PER_VERT > (int64_t)INT32_MAX);
	CHECK(TetraMesh4D::PROXY_MAX_TETS_PER_SURFACE * TetraMesh4D::PROXY_VERTS_PER_TET <= TetraMesh4D::PROXY_MAX_VERTS_PER_SURFACE);
	CHECK((TetraMesh4D::PROXY_MAX_TETS_PER_SURFACE + 1) * TetraMesh4D::PROXY_VERTS_PER_TET > TetraMesh4D::PROXY_MAX_VERTS_PER_SURFACE);
}

TEST_CASE("[SceneTree][TetraMesh4D] Proxy mesh texture coordinates survive the attribute packing") {
	// The proxy packs each tetrahedron's four UVWs into vertex attributes of mixed precision: the second vertex is
	// exact, the first vertex's W and the whole fourth vertex are 16-bit codes relative to it in the bone weights, and
	// the third vertex's U and V ride in the normal. This decodes them the way the shaders do and compares them with
	// a texture map that reaches well outside the 0 to 1 range.
	Ref<BoxTetraMesh4D> box;
	box.instantiate();
	Ref<ArrayTetraMesh4D> mesh = box->to_array_tetra_mesh();
	PackedVector3Array texture_map_values = mesh->get_texture_map_values();
	REQUIRE(!texture_map_values.is_empty());
	for (int64_t i = 0; i < texture_map_values.size(); i++) {
		texture_map_values.set(i, texture_map_values[i] * Vector3(3, 5, 7) + Vector3(-2, -1, 0.5));
	}
	mesh->set_texture_map_values(texture_map_values);
	const PackedInt32Array texture_map_indices = mesh->get_simplex_cell_texture_map_indices();
	const int64_t tet_count = texture_map_indices.size() / 4;
	REQUIRE(tet_count > 0);
	const Ref<ArrayMesh> proxy = mesh->get_proxy_mesh_3d();
	REQUIRE(proxy.is_valid());
	REQUIRE(proxy->get_surface_count() == 1);
	const Array arrays = proxy->surface_get_arrays(0);
	const PackedVector3Array positions = arrays[Mesh::ARRAY_VERTEX];
	const PackedVector3Array normals = arrays[Mesh::ARRAY_NORMAL];
	const PackedVector2Array uvs = arrays[Mesh::ARRAY_TEX_UV];
	const PackedVector2Array uv2s = arrays[Mesh::ARRAY_TEX_UV2];
	const PackedInt32Array bones = arrays[Mesh::ARRAY_BONES];
	const PackedFloat32Array weights = arrays[Mesh::ARRAY_WEIGHTS];
	REQUIRE(positions.size() == tet_count * TetraMesh4D::PROXY_VERTS_PER_TET);
	REQUIRE(bones.size() == positions.size() * 4);
	REQUIRE(weights.size() == positions.size() * 4);
	for (int64_t tet = 0; tet < tet_count; tet++) {
		const Vector3 expected1 = texture_map_values[texture_map_indices[tet * 4]];
		const Vector3 expected2 = texture_map_values[texture_map_indices[tet * 4 + 1]];
		const Vector3 expected3 = texture_map_values[texture_map_indices[tet * 4 + 2]];
		const Vector3 expected4 = texture_map_values[texture_map_indices[tet * 4 + 3]];
		for (int64_t corner = 0; corner < TetraMesh4D::PROXY_VERTS_PER_TET; corner++) {
			const int64_t v = tet * TetraMesh4D::PROXY_VERTS_PER_TET + corner;
			const Vector3 uvw2 = Vector3(uv2s[v].x, uv2s[v].y, positions[v].y);
			CHECK(uvw2.is_equal_approx(expected2));
			const real_t range = Math::pow((real_t)2.0, (real_t)(bones[v * 4] - 64));
			const real_t tolerance = range * (real_t)5e-5 + (real_t)1e-6;
			auto decode = [&](const int64_t p_slot) -> real_t {
				const real_t code = Math::round((real_t)weights[v * 4 + p_slot] * (real_t)65535.0);
				return (code / (real_t)65535.0 * (real_t)2.0 - (real_t)1.0) * range;
			};
			CHECK(Math::is_equal_approx(uvs[v].x, expected1.x));
			CHECK(Math::is_equal_approx(uvs[v].y, expected1.y));
			CHECK(Math::abs(uvw2.z + decode(0) - expected1.z) <= tolerance);
			const Vector3 uvw4 = uvw2 + Vector3(decode(1), decode(2), decode(3));
			CHECK((uvw4 - expected4).length() <= tolerance * (real_t)2.0);
			// The third vertex's U and V come back through the normal's octahedral compression, the least precise slot.
			const Vector3 normal = normals[v];
			REQUIRE(normal.z > (real_t)0.0);
			CHECK(Math::abs(normal.x / normal.z - expected3.x) < (real_t)0.01);
			CHECK(Math::abs(normal.y / normal.z - expected3.y) < (real_t)0.01);
			CHECK(Math::is_equal_approx(positions[v].z, expected3.z));
		}
	}
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

TEST_CASE("[TetraMesh4D] A degenerate tetrahedron does not break closest-point queries for the whole mesh") {
	Ref<ArrayTetraMesh4D> mesh;
	mesh.instantiate();
	mesh->append_tetra_cell_points(Vector4(0, 0, 0, 0), Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0), Vector4(0, 0, 1, 0), true);
	// A flat tetrahedron: all four vertices lie in the XY plane.
	mesh->append_tetra_cell_points(Vector4(5, 0, 0, 0), Vector4(6, 0, 0, 0), Vector4(5, 1, 0, 0), Vector4(5, 2, 0, 0), true);
	ERR_PRINT_OFF;
	mesh->populate_inverse_metric_cache();
	ERR_PRINT_ON;
	// The point is 1 unit along W from the valid tetrahedron's corner, and further from the flat one.
	const real_t distance = mesh->get_signed_distance_to_mesh(Vector4(0, 0, 0, 1), nullptr, nullptr);
	CHECK_MESSAGE(Math::is_finite(distance), "The closest-point cache must still be usable when one tetrahedron is degenerate.");
	CHECK_MESSAGE(Math::is_equal_approx(Math::abs(distance), (real_t)1.0), "The distance must come from the valid tetrahedron.");
}
} // namespace TestTetraMesh4D
