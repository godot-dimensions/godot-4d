#pragma once

#include "../../../physics/shapes/sphere_shape_4d.h"

#include "tests/test_macros.h"

namespace TestSphereShape4D {
TEST_CASE("[SphereShape4D] Raycast from outside") {
	Ref<SphereShape4D> sphere;
	sphere.instantiate();
	sphere->set_radius(1.0f);
	// Ray from outside pointing at center.
	Dictionary result = sphere->raycast_intersects(Vector4(-2, 0, 0, 0), Vector4(1, 0, 0, 0).normalized());
	CHECK_MESSAGE((bool)result["hit"] == true, "Raycast from outside should hit sphere");
	CHECK_MESSAGE((real_t)result["distance"] == doctest::Approx(1.0f), "Distance should be 1.0");
	Vector4 normal = result["normal"];
	CHECK_MESSAGE(Math::is_equal_approx(normal.length(), (real_t)1.0), "Normal should be normalized");
	// Ray from outside pointing away.
	result = sphere->raycast_intersects(Vector4(2, 0, 0, 0), Vector4(1, 0, 0, 0).normalized());
	CHECK_MESSAGE((bool)result["hit"] == false, "Raycast pointing away should not hit");
	// Ray missing the sphere.
	result = sphere->raycast_intersects(Vector4(-2, 3, 0, 0), Vector4(1, 0, 0, 0).normalized());
	CHECK_MESSAGE((bool)result["hit"] == false, "Raycast missing sphere should not hit");
}

TEST_CASE("[SphereShape4D] Raycast from inside") {
	Ref<SphereShape4D> sphere;
	sphere.instantiate();
	sphere->set_radius(1.0f);
	// Ray from inside the sphere.
	Dictionary result = sphere->raycast_intersects(Vector4(0, 0, 0, 0), Vector4(1, 0, 0, 0).normalized());
	CHECK_MESSAGE((bool)result["hit"] == true, "Raycast from center should hit");
	CHECK_MESSAGE((real_t)result["distance"] == doctest::Approx(1.0f), "Distance from center should be 1.0 (radius)");
	// Ray from inside at offset.
	result = sphere->raycast_intersects(Vector4(0.5, 0, 0, 0), Vector4(1, 0, 0, 0).normalized());
	CHECK_MESSAGE((bool)result["hit"] == true, "Raycast from inside should hit");
	CHECK_MESSAGE((real_t)result["distance"] == doctest::Approx(0.5f), "Distance should be 0.5");
}

TEST_CASE("[SphereShape4D] Raycast at various radii") {
	Ref<SphereShape4D> sphere;
	sphere.instantiate();
	// Small sphere.
	sphere->set_radius(0.5f);
	Dictionary result = sphere->raycast_intersects(Vector4(-1, 0, 0, 0), Vector4(1, 0, 0, 0).normalized());
	CHECK_MESSAGE((bool)result["hit"] == true, "Raycast should hit small sphere");
	CHECK_MESSAGE((real_t)result["distance"] == doctest::Approx(0.5f), "Distance should be 0.5");
	// Large sphere.
	sphere->set_radius(5.0f);
	result = sphere->raycast_intersects(Vector4(-10, 0, 0, 0), Vector4(1, 0, 0, 0).normalized());
	CHECK_MESSAGE((bool)result["hit"] == true, "Raycast should hit large sphere");
	CHECK_MESSAGE((real_t)result["distance"] == doctest::Approx(5.0f), "Distance should be 5.0");
}
} // namespace TestSphereShape4D
