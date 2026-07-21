#pragma once

#include "../../../physics/shapes/capsule_shape_4d.h"

#include "tests/test_macros.h"

namespace TestCapsuleShape4D {
TEST_CASE("[CapsuleShape4D] Raycast from outside - cylinder middle") {
	Ref<CapsuleShape4D> capsule;
	capsule.instantiate();
	capsule->set_radius(1.0f);
	capsule->set_full_height(2.0f);
	// Ray from outside pointing at cylinder middle along X axis.
	Dictionary result = capsule->raycast_intersects(Vector4(-2, 0, 0, 0), Vector4(1, 0, 0, 0).normalized());
	CHECK_MESSAGE((bool)result["hit"] == true, "Raycast from outside should hit capsule cylinder middle");
	CHECK_MESSAGE((real_t)result["distance"] == doctest::Approx(1.0f), "Distance should be 1.0 (radius)");
}

TEST_CASE("[CapsuleShape4D] Raycast from outside - cap") {
	Ref<CapsuleShape4D> capsule;
	capsule.instantiate();
	capsule->set_radius(1.0f);
	capsule->set_full_height(2.0f);
	// Ray from outside pointing at positive Y cap.
	Dictionary result = capsule->raycast_intersects(Vector4(0, 3, 0, 0), Vector4(0, -1, 0, 0).normalized());
	CHECK_MESSAGE((bool)result["hit"] == true, "Raycast from outside should hit capsule cap");
	// Ray from outside pointing at negative Y cap.
	result = capsule->raycast_intersects(Vector4(0, -3, 0, 0), Vector4(0, 1, 0, 0).normalized());
	CHECK_MESSAGE((bool)result["hit"] == true, "Raycast from outside should hit capsule cap");
}

TEST_CASE("[CapsuleShape4D] Raycast from inside") {
	Ref<CapsuleShape4D> capsule;
	capsule.instantiate();
	capsule->set_radius(1.0f);
	capsule->set_full_height(2.0f);
	// Ray from inside center - behavior when starting inside is well-defined but complex.
	// Just verify it returns a hit (may be exit face or may have other semantics).
	Dictionary result = capsule->raycast_intersects(Vector4(0, 0, 0, 0), Vector4(1, 0, 0, 0).normalized());
	CHECK_MESSAGE((bool)result["hit"] == true, "Raycast from inside should hit");
}

TEST_CASE("[CapsuleShape4D] Raycast missing") {
	Ref<CapsuleShape4D> capsule;
	capsule.instantiate();
	capsule->set_radius(1.0f);
	capsule->set_full_height(2.0f);
	// Ray missing the capsule.
	Dictionary result = capsule->raycast_intersects(Vector4(-2, 3, 0, 0), Vector4(1, 0, 0, 0).normalized());
	CHECK_MESSAGE((bool)result["hit"] == false, "Raycast missing capsule should not hit");
	// Ray pointing away.
	result = capsule->raycast_intersects(Vector4(2, 0, 0, 0), Vector4(1, 0, 0, 0).normalized());
	CHECK_MESSAGE((bool)result["hit"] == false, "Raycast pointing away should not hit");
}
} // namespace TestCapsuleShape4D
