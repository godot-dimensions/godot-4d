#pragma once

#include "../../voxel/generators/tiger_test_generator_4d.h"

#include "tests/test_macros.h"

namespace TestTigerTestGenerator4D {
constexpr VoxelMaterial4D SOLID_MATERIAL = (VoxelMaterial4D)0;

TEST_CASE("[TigerTestGenerator4D] Materials and edge data") {
	Ref<TigerTestGenerator4D> generator;
	generator.instantiate();
	CHECK_MESSAGE(generator->get_material(Vector4i(10, 0, 10, 0)) == SOLID_MATERIAL, "TigerTestGenerator4D should be solid on the tiger's core circles.");
	CHECK_MESSAGE(generator->get_material(Vector4i(0, 0, 0, 0)) == VoxelMaterial4D::AIR, "TigerTestGenerator4D should be empty at the origin.");

	// The edge from (13, 0, 10, 0) to (14, 0, 10, 0) crosses the surface
	// heading out of the tube, away from the XY core circle, so its normal
	// should point mostly along X.
	REQUIRE(generator->get_material(Vector4i(13, 0, 10, 0)) == SOLID_MATERIAL);
	REQUIRE(generator->get_material(Vector4i(14, 0, 10, 0)) == VoxelMaterial4D::AIR);
	const VoxelEdgeData4D edge_data = generator->get_edge_data(Vector4i(13, 0, 10, 0), 0);
	const Vector4 normal = edge_data.normal.normalized();
	CHECK_MESSAGE(normal.x > 0.6, "TigerTestGenerator4D normals should point along the surface gradient.");

	// The tolerance covers the crossing's linear approximation.
	Vector4 crossing_point = Vector4(13.5, 0.5, 10.5, 0.5);
	crossing_point.x += edge_data.position;
	const double xy = Math::sqrt(crossing_point.x * crossing_point.x + crossing_point.y * crossing_point.y) - 10.0;
	const double zw = Math::sqrt(crossing_point.z * crossing_point.z + crossing_point.w * crossing_point.w) - 10.0;
	const double signed_distance = 4.5 - Math::sqrt(xy * xy + zw * zw);
	CHECK_MESSAGE(Math::abs(signed_distance) < 0.1, "TigerTestGenerator4D edge crossings should lie on the tiger's surface.");
}
} // namespace TestTigerTestGenerator4D
