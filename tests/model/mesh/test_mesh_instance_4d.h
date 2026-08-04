#pragma once

#include "../../../model/mesh/mesh_instance_4d.h"
#include "../../../model/mesh/wire/array_wire_mesh_4d.h"

#include "tests/test_macros.h"

namespace TestMeshInstance4D {
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
} // namespace TestMeshInstance4D
