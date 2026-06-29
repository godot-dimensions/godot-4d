#pragma once

#include "../../../../model/mesh/tetra/array_tetra_mesh_4d.h"

#include "tests/test_macros.h"

namespace TestTetraMesh4D {
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
} // namespace TestTetraMesh4D
