#pragma once

#include "../../voxel/data/voxel_edge_data.h"

#include "core/math/random_number_generator.h"
#include "tests/test_macros.h"

namespace TestVoxelEdgeData {
TEST_CASE("[VoxelEdgeData] Encoding round trips") {
	// Seeded, so any failure is reproducible.
	Ref<RandomNumberGenerator> rng;
	rng.instantiate();
	rng->set_seed(566744);
	for (int i = 0; i < 64; i++) {
		Vector4 normal;
		do {
			normal = Vector4(rng->randf_range(-1.0f, 1.0f), rng->randf_range(-1.0f, 1.0f), rng->randf_range(-1.0f, 1.0f), rng->randf_range(-1.0f, 1.0f));
		} while (normal.length() < 0.1f);
		normal = normal.normalized();
		const real_t position = rng->randf();
		const VoxelEdgeData encoded = VoxelEdgeData::encode(normal, position);
		// Each of the three face coordinates rounds by at most half of its
		// 1/4 step, giving a direction error within ~0.22 radians. The sign
		// is not stored, so compare up to it.
		CHECK_MESSAGE(Math::abs(encoded.decode_normal().dot(normal)) > Math::cos(0.25), "VoxelEdgeData decoded normals should be within the quantization error of the original.");
		CHECK_MESSAGE(Math::abs(encoded.decode_position() - position) < 0.5f / 31.0f + 0.0001f, "VoxelEdgeData decoded positions should be within half of a quantization step.");
	}

	// The centers of the hypercube's 2-faces, the diagonals between two axes,
	// are exactly representable on whichever face the orientation convention
	// assigns them to, for every combination of signs.
	for (int axis_a = 0; axis_a < 4; axis_a++) {
		for (int axis_b = axis_a + 1; axis_b < 4; axis_b++) {
			for (int signs = 0; signs < 4; signs++) {
				Vector4 normal;
				normal[axis_a] = (signs & 1) ? -1.0f : 1.0f;
				normal[axis_b] = (signs & 2) ? -1.0f : 1.0f;
				const VoxelEdgeData encoded = VoxelEdgeData::encode(normal, 0.5f);
				CHECK_MESSAGE(Math::abs(encoded.decode_normal().dot(normal.normalized())) > 0.9999f, "VoxelEdgeData should represent the centers of the hypercube's 2-faces exactly.");
			}
		}
	}

	CHECK_MESSAGE(VoxelEdgeData::encode(Vector4(0, 1, 0, 0), 0.5f).decode_normal() == Vector4(0, 1, 0, 0), "VoxelEdgeData should store axis-aligned normals exactly.");
	CHECK_MESSAGE(VoxelEdgeData::encode(Vector4(1, 0, 0, 0), 0.0f).decode_position() == 0.0f, "VoxelEdgeData should store a crossing at the edge's start exactly.");
	CHECK_MESSAGE(VoxelEdgeData::encode(Vector4(1, 0, 0, 0), 1.0f).decode_position() == 1.0f, "VoxelEdgeData should store a crossing at the edge's end exactly.");
}
} // namespace TestVoxelEdgeData
