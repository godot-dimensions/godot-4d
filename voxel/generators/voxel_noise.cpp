#include "voxel_noise.h"

// The pseudo-random value in [-1, 1] of one lattice corner: the cell's
// coordinates offset by the corner's bits, mixed with the seed through an
// integer avalanche.
static real_t _corner_value(const int32_t p_cell[4], const int p_corner_bits, const uint32_t p_seed) {
	uint32_t hash = p_seed;
	for (int axis = 0; axis < 4; axis++) {
		hash = (hash ^ (uint32_t)(p_cell[axis] + ((p_corner_bits >> axis) & 1))) * 0x9E3779B1u;
	}
	hash ^= hash >> 16;
	hash *= 0x7FEB352Du;
	hash ^= hash >> 15;
	hash *= 0x846CA68Bu;
	hash ^= hash >> 16;
	return (real_t)hash * (real_t)(2.0 / 4294967295.0) - (real_t)1.0f;
}

// 4D value noise: pseudo-random lattice corner values interpolated with the
// C² quintic fade, giving a value in [-1, 1]. The gradient, when requested,
// is in lattice units.
// This has poor isotropy (both under rotation, since its spectrum is roughly
// confined to a hypercube, and under translation, since it's very different
// at lattice points). Once generation with caching instead of one point at
// a time is implemented, this should be replaced.
static real_t _value_noise(const Vector4 &p_position, const uint32_t p_seed, Vector4 *r_gradient) {
	int32_t cell[4];
	real_t fade[4];
	real_t fade_derivative[4];
	for (int axis = 0; axis < 4; axis++) {
		const real_t floored = Math::floor(p_position[axis]);
		cell[axis] = (int32_t)floored;
		const real_t t = p_position[axis] - floored;
		fade[axis] = t * t * t * (t * (t * (real_t)6.0f - (real_t)15.0f) + (real_t)10.0f);
		const real_t t_less_1 = t - (real_t)1.0f;
		fade_derivative[axis] = (real_t)30.0f * t * t * t_less_1 * t_less_1;
	}
	real_t values[16];
	for (int corner = 0; corner < 16; corner++) {
		values[corner] = _corner_value(cell, corner, p_seed);
	}
	// Collapse one axis at a time; the pending axis is always the pairs'
	// lowest index bit.
	Vector4 gradients[8];
	int count = 16;
	for (int axis = 0; axis < 4; axis++) {
		count >>= 1;
		for (int k = 0; k < count; k++) {
			const real_t low = values[2 * k];
			const real_t high = values[2 * k + 1];
			values[k] = low + fade[axis] * (high - low);
			if (r_gradient != nullptr) {
				Vector4 gradient = axis == 0 ? Vector4() : gradients[2 * k].lerp(gradients[2 * k + 1], fade[axis]);
				gradient[axis] += fade_derivative[axis] * (high - low);
				gradients[k] = gradient;
			}
		}
	}
	if (r_gradient != nullptr) {
		*r_gradient = gradients[0];
	}
	return values[0];
}

real_t LayeredVoxelNoise::sample(const Vector4 &p_point, Vector4 *r_gradient) const {
	real_t total = (real_t)0.0f;
	if (r_gradient != nullptr) {
		*r_gradient = Vector4();
	}
	real_t amplitude = intensity;
	Vector4 octave_scale = scale * (real_t)(1 << (octaves - 1));
	for (int octave = 0; octave < octaves; octave++) {
		Vector4 octave_gradient;
		const real_t noise = _value_noise(p_point / octave_scale, derive_seed(seed, (uint32_t)octave), r_gradient != nullptr ? &octave_gradient : nullptr);
		total += amplitude * noise;
		if (r_gradient != nullptr) {
			*r_gradient += amplitude * (octave_gradient / octave_scale);
		}
		octave_scale *= (real_t)0.5f;
		amplitude *= persistence;
	}
	return total;
}
