#pragma once

#include "../../godot_4d_defines.h"

#if GDEXTENSION
#include <godot_cpp/templates/hashfuncs.hpp>
#elif GODOT_MODULE
#include "core/templates/hashfuncs.h"
#endif

// Derives a stream of independent seeds from one seed. For a fixed seed the
// derived seeds are all distinct, since the hash round is a bijection in the
// index; any other collision, such as between seeds derived from different
// seeds or by different chains of derivations, has probability 2^-32.
inline uint32_t derive_seed(const uint32_t p_seed, const uint32_t p_index) {
	return hash_fmix32(hash_murmur3_one_32(p_index, p_seed));
}

// Layered 4D value noise: octaves of smooth lattice noise summed with a
// fixed lacunarity of 2, so each octave has twice the scale of the next
// finer one. Generators may hold these and sample them as fields. This
// is value noise, not something band-limited like Perlin or simplex,
// because band-limiting doesn't work well in 4D anyway.
struct LayeredVoxelNoise4D {
	uint32_t seed = 0;
	// Must be between 1 and 16.
	int octaves = 4;
	// The factor between the amplitudes of consecutive octaves: each finer
	// octave's amplitude is the previous amplitude times this.
	real_t persistence = 0.5f;
	// The scale of the finest octave along each axis, in voxels.
	Vector4 scale = Vector4(1, 1, 1, 1);
	// The amplitude of the coarsest octave.
	real_t intensity = 1.0f;

	// The summed octaves at the given point. The gradient, when requested,
	// is exact.
	real_t sample(const Vector4 &p_point, Vector4 *r_gradient) const;

	// The largest absolute value a sample can take: the octaves' summed
	// amplitudes, reached only where all their lattice values align.
	real_t max_value() const {
		real_t total = 0.0f;
		real_t amplitude = intensity;
		for (int i = 0; i < octaves; i++) {
			total += Math::abs(amplitude);
			amplitude *= persistence;
		}
		return total;
	}
};
