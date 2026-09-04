#pragma once

#include "../../voxel/generators/tiger_test_generator.h"

#include "tests/test_macros.h"

namespace TestTigerTestGenerator {
TEST_CASE("[TigerTestGenerator] Values and normals") {
	Ref<TigerTestGenerator> generator;
	generator.instantiate();
	CHECK_MESSAGE(generator->get_value(Vector4i(10, 0, 10, 0)).is_opaque(), "TigerTestGenerator should be solid on the tiger's core circles.");
	CHECK_MESSAGE(!generator->get_value(Vector4i(0, 0, 0, 0)).is_opaque(), "TigerTestGenerator should be empty at the origin.");
	CHECK_MESSAGE(generator->get_value(Vector4i(0, 0, 0, 0)).density == 255, "TigerTestGenerator density should saturate far from the surface.");
	CHECK_MESSAGE(generator->get_value(Vector4i(14, 0, 10, 0)).density < 32, "TigerTestGenerator density should be low near the surface.");

	// The edge from (13, 0, 10, 0) to (14, 0, 10, 0) crosses the surface
	// heading out of the tube, away from the XY core circle, so its normal
	// should point mostly along +X.
	const VoxelValue solid_end = generator->get_value(Vector4i(13, 0, 10, 0));
	const VoxelValue air_end = generator->get_value(Vector4i(14, 0, 10, 0));
	REQUIRE(solid_end.is_opaque());
	REQUIRE(!air_end.is_opaque());
	const Vector4 normal = generator->get_normal(Vector4i(13, 0, 10, 0), 0, solid_end, air_end);
	CHECK_MESSAGE(normal.is_normalized(), "TigerTestGenerator normals should be unit vectors.");
	CHECK_MESSAGE(normal.x > 0.6, "TigerTestGenerator normals should point out of the solid material.");
}
} // namespace TestTigerTestGenerator
