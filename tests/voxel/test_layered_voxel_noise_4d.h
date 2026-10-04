#pragma once

#include "../../voxel/generators/layered_voxel_noise_4d.h"

#include "core/math/random_number_generator.h"
#include "core/templates/hash_set.h"
#include "tests/test_macros.h"

namespace TestLayeredVoxelNoise4D {

TEST_CASE("[LayeredVoxelNoise4D] Gradient and value bound") {
	const LayeredVoxelNoise4D noise = { 123, 3, 0.6f, Vector4(2.3f, 1.7f, 3.1f, 2.9f), 2.5f };
	const real_t bound = noise.max_value();
	// Seeded, so any failure is reproducible.
	Ref<RandomNumberGenerator> rng;
	rng.instantiate();
	rng->set_seed(815412);
	const real_t step = 0.01f;
	bool values_bounded = true;
	bool gradients_match = true;
	for (int i = 0; i < 64; i++) {
		const Vector4 point = Vector4(rng->randf_range(-50.0f, 50.0f), rng->randf_range(-50.0f, 50.0f), rng->randf_range(-50.0f, 50.0f), rng->randf_range(-50.0f, 50.0f));
		Vector4 gradient;
		const real_t value = noise.sample(point, &gradient);
		values_bounded = values_bounded && Math::abs(value) <= bound;
		for (int axis = 0; axis < 4; axis++) {
			Vector4 low = point;
			Vector4 high = point;
			low[axis] -= step;
			high[axis] += step;
			const real_t finite_difference = (noise.sample(high, nullptr) - noise.sample(low, nullptr)) / (2.0f * step);
			gradients_match = gradients_match && Math::abs(finite_difference - gradient[axis]) < (real_t)0.02f;
		}
	}
	CHECK_MESSAGE(values_bounded, "LayeredVoxelNoise4D samples should stay within max_value.");
	CHECK_MESSAGE(gradients_match, "LayeredVoxelNoise4D analytic gradients should match central finite differences.");
}

TEST_CASE("[LayeredVoxelNoise4D] Seed derivation") {
	// The derived seeds must never collide for distinct indices of one seed,
	// and the fixed seeds and chains here are deterministic, so the
	// probabilistic collisions are either absent forever or caught here.
	bool all_distinct = true;
	for (const uint32_t seed : { 0u, 1u, 3735928559u }) {
		HashSet<uint32_t> seen;
		seen.insert(seed);
		for (uint32_t index = 0; index < 1000; index++) {
			const uint32_t derived = derive_seed(seed, index);
			all_distinct = all_distinct && !seen.has(derived);
			seen.insert(derived);
		}
		// Chains of derivations in different orders also stay distinct.
		for (uint32_t a = 0; a < 8; a++) {
			for (uint32_t b = 0; b < 8; b++) {
				const uint32_t derived = derive_seed(derive_seed(seed, a), b);
				all_distinct = all_distinct && !seen.has(derived);
				seen.insert(derived);
			}
		}
	}
	CHECK_MESSAGE(all_distinct, "derive_seed should produce distinct seeds across indices and derivation chains.");
}
} // namespace TestLayeredVoxelNoise4D
