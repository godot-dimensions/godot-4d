#pragma once

#include "../../../model/mesh/mesh_instance_4d.h"
#include "../../../model/mesh/wire/array_wire_mesh_4d.h"

#include "tests/test_macros.h"

namespace TestMeshInstance4D {
TEST_CASE("[MeshInstance4D] Bounds follow mesh data and target transform") {
	Ref<ArrayWireMesh4D> mesh;
	mesh.instantiate();
	mesh->set_vertices(PackedVector4Array({ Vector4(-1, -1, -1, -1), Vector4(1, 1, 1, 1) }));

	MeshInstance4D mesh_instance;
	mesh_instance.set_mesh(mesh);
	CHECK(mesh_instance.get_rect_bounds_local() == Rect4(Vector4(-1, -1, -1, -1), Vector4(2, 2, 2, 2)));

	mesh->set_vertices(PackedVector4Array({ Vector4(-2, -2, -2, -2), Vector4(3, 3, 3, 3) }));
	CHECK(mesh_instance.get_rect_bounds_local() == Rect4(Vector4(-2, -2, -2, -2), Vector4(5, 5, 5, 5)));

	const Transform4D to_target = Transform4D(Basis4D(), Vector4(10, 20, 30, 40));
	CHECK(mesh_instance.get_rect_bounds_local(to_target) == Rect4(Vector4(8, 18, 28, 38), Vector4(5, 5, 5, 5)));
}

TEST_CASE("[MeshInstance4D] Raycast fallback to bounds") {
	Ref<ArrayWireMesh4D> mesh;
	mesh.instantiate();
	mesh->set_vertices(PackedVector4Array({ Vector4(-1, -1, -1, -1), Vector4(1, 1, 1, 1) }));

	MeshInstance4D mesh_instance;
	mesh_instance.set_mesh(mesh);

	const Vector4 outside_origin(-3, 0, 0, 0);
	const Vector4 direction(1, 0, 0, 0);
	Dictionary result = mesh_instance.raycast_intersects_local(outside_origin, direction, 2.0, false);
	CHECK_FALSE((bool)result["hit"]);
	result = mesh_instance.raycast_intersects_local(outside_origin, direction, 2.001, false);
	CHECK((bool)result["hit"]);
	CHECK(Math::is_equal_approx((double)result["distance"], 2.0));

	result = mesh_instance.raycast_intersects_local(Vector4(), direction, Math_INF, true);
	CHECK((bool)result["hit"]);
	CHECK(Math::is_zero_approx((double)result["distance"]));
	result = mesh_instance.raycast_intersects_local(Vector4(), direction, Math_INF, false);
	CHECK((bool)result["hit"]);
	CHECK(Math::is_equal_approx((double)result["distance"], 1.0));
}

TEST_CASE("[ArrayWireMesh4D] Bounds cache invalidation on deduplicate") {
	Ref<ArrayWireMesh4D> mesh;
	mesh.instantiate();
	// Create mesh with duplicate vertices
	mesh->set_vertices(PackedVector4Array({ Vector4(0, 0, 0, 0), Vector4(1, 1, 1, 1), Vector4(1, 1, 1, 1) }));
	mesh->append_edge_indices(0, 1, false);
	mesh->append_edge_indices(0, 2, false); // duplicate edge via different vertex

	Rect4 bounds_before = mesh->get_rect_bounds();
	CHECK(bounds_before == Rect4(Vector4(0, 0, 0, 0), Vector4(1, 1, 1, 1)));

	// Deduplicate should not change bounds (since duplicate vertex was at same position)
	mesh->deduplicate_all_elements();
	Rect4 bounds_after = mesh->get_rect_bounds();
	CHECK(bounds_after == bounds_before);
}

TEST_CASE("[ArrayWireMesh4D] Bounds cache invalidation on merge") {
	Ref<ArrayWireMesh4D> mesh1;
	mesh1.instantiate();
	mesh1->set_vertices(PackedVector4Array({ Vector4(-1, -1, -1, -1), Vector4(1, 1, 1, 1) }));
	mesh1->append_edge_indices(0, 1, false);

	Rect4 bounds1 = mesh1->get_rect_bounds();
	CHECK(bounds1 == Rect4(Vector4(-1, -1, -1, -1), Vector4(2, 2, 2, 2)));

	// Create second mesh with vertices outside first mesh's bounds
	Ref<ArrayWireMesh4D> mesh2;
	mesh2.instantiate();
	mesh2->set_vertices(PackedVector4Array({ Vector4(5, 5, 5, 5), Vector4(10, 10, 10, 10) }));
	mesh2->append_edge_indices(0, 1, false);

	// Merge mesh2 into mesh1
	mesh1->merge_with(mesh2);

	// Bounds should expand to include merged vertices
	Rect4 bounds_after_merge = mesh1->get_rect_bounds();
	CHECK(bounds_after_merge == Rect4(Vector4(-1, -1, -1, -1), Vector4(11, 11, 11, 11)));
}

TEST_CASE("[ArrayTetraMesh4D] Bounds cache invalidation on merge") {
	Ref<ArrayTetraMesh4D> tetra1;
	tetra1.instantiate();
	tetra1->set_vertices(PackedVector4Array({ Vector4(0, 0, 0, 0), Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0), Vector4(0, 0, 1, 0) }));
	tetra1->set_simplex_cell_indices(PackedInt32Array({ 0, 1, 2, 3 }));

	Rect4 bounds1 = tetra1->get_rect_bounds();
	CHECK(bounds1 == Rect4(Vector4(0, 0, 0, 0), Vector4(1, 1, 1, 0)));

	// Create second tetra mesh with vertices outside first mesh's bounds
	Ref<ArrayTetraMesh4D> tetra2;
	tetra2.instantiate();
	tetra2->set_vertices(PackedVector4Array({ Vector4(5, 5, 5, 5), Vector4(6, 5, 5, 5), Vector4(5, 6, 5, 5), Vector4(5, 5, 6, 5) }));
	tetra2->set_simplex_cell_indices(PackedInt32Array({ 0, 1, 2, 3 }));

	// Merge tetra2 into tetra1
	tetra1->merge_with(tetra2);

	// Bounds should expand to include merged vertices
	Rect4 bounds_after_merge = tetra1->get_rect_bounds();
	CHECK(bounds_after_merge == Rect4(Vector4(0, 0, 0, 0), Vector4(6, 6, 6, 5)));
}

TEST_CASE("[Mesh4D] Bounds cache persists across multiple accesses") {
	Ref<ArrayWireMesh4D> mesh;
	mesh.instantiate();
	mesh->set_vertices(PackedVector4Array({ Vector4(-2, -2, -2, -2), Vector4(3, 3, 3, 3) }));

	// Access bounds multiple times - should use cached value
	Rect4 bounds1 = mesh->get_rect_bounds();
	Rect4 bounds2 = mesh->get_rect_bounds();
	Rect4 bounds3 = mesh->get_rect_bounds();

	CHECK(bounds1 == bounds2);
	CHECK(bounds2 == bounds3);
	CHECK(bounds1 == Rect4(Vector4(-2, -2, -2, -2), Vector4(5, 5, 5, 5)));
}
} // namespace TestMeshInstance4D
