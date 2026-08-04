#pragma once

#include "../../../../model/mesh/tetra/array_tetra_mesh_4d.h"
#include "../../../../model/mesh/tetra/box_tetra_mesh_4d.h"

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
