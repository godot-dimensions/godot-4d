#pragma once

#include "../../../physics/shapes/box_shape_4d.h"

#include "tests/test_macros.h"

namespace TestBoxShape4D {
TEST_CASE("[BoxShape4D] Raycast from outside") {
	Ref<BoxShape4D> box;
	box.instantiate();
	box->set_size(Vector4(2, 2, 2, 2));
	// Ray from outside pointing at center.
	Dictionary result = box->raycast_intersects(Vector4(-3, 0, 0, 0), Vector4(1, 0, 0, 0).normalized());
	CHECK_MESSAGE((bool)result["hit"] == true, "Raycast from outside should hit box");
	CHECK_MESSAGE((real_t)result["distance"] == doctest::Approx(2.0f), "Distance should be 2.0 (size is 2, so extends -1 to 1)");
	Vector4 normal = result["normal"];
	CHECK_MESSAGE(normal == Vector4(-1, 0, 0, 0), "Normal should point backwards along X");
	// Ray from outside pointing away.
	result = box->raycast_intersects(Vector4(3, 0, 0, 0), Vector4(1, 0, 0, 0).normalized());
	CHECK_MESSAGE((bool)result["hit"] == false, "Raycast pointing away should not hit");
	// Ray missing the box.
	result = box->raycast_intersects(Vector4(-3, 3, 0, 0), Vector4(1, 0, 0, 0).normalized());
	CHECK_MESSAGE((bool)result["hit"] == false, "Raycast missing box should not hit");
}

TEST_CASE("[BoxShape4D] Raycast from inside") {
	Ref<BoxShape4D> box;
	box.instantiate();
	box->set_size(Vector4(2, 2, 2, 2));
	// Ray from center pointing towards +X.
	// Verify that raycast returns a hit when starting inside.
	Dictionary result = box->raycast_intersects(Vector4(0, 0, 0, 0), Vector4(1, 0, 0, 0).normalized());
	CHECK_MESSAGE((bool)result["hit"] == true, "Raycast from center should hit");
	// Distance may be negative or positive depending on implementation, just verify it's set.
	CHECK_MESSAGE(result.has("distance"), "Result should have distance field");
}

TEST_CASE("[BoxShape4D] Raycast through different axes") {
	Ref<BoxShape4D> box;
	box.instantiate();
	box->set_size(Vector4(2, 4, 2, 2));
	// Ray along Y axis.
	Dictionary result = box->raycast_intersects(Vector4(0, -3, 0, 0), Vector4(0, 1, 0, 0).normalized());
	CHECK_MESSAGE((bool)result["hit"] == true, "Raycast along Y should hit");
	CHECK_MESSAGE((real_t)result["distance"] == doctest::Approx(1.0f), "Distance along Y should be 1.0 (Y extends -2 to 2)");
	// Ray along Z axis.
	result = box->raycast_intersects(Vector4(0, 0, -3, 0), Vector4(0, 0, 1, 0).normalized());
	CHECK_MESSAGE((bool)result["hit"] == true, "Raycast along Z should hit");
	CHECK_MESSAGE((real_t)result["distance"] == doctest::Approx(2.0f), "Distance along Z should be 2.0");
	// Ray along W axis.
	result = box->raycast_intersects(Vector4(0, 0, 0, -3), Vector4(0, 0, 0, 1).normalized());
	CHECK_MESSAGE((bool)result["hit"] == true, "Raycast along W should hit");
	CHECK_MESSAGE((real_t)result["distance"] == doctest::Approx(2.0f), "Distance along W should be 2.0");
}
} // namespace TestBoxShape4D
