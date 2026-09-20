#pragma once

#include "../../godot_4d_defines.h"

// The surface data of one active voxel grid edge, compressed to 16 bits: the
// surface normal and the position along the edge where the surface crosses.
//
// The normal is stored as a point on the surface of a hypercube. 2 bits pick
// the face its largest component points at (the normal's overall sign is not
// stored), and 3 bits per remaining axis store that coordinate of the point,
// in quarters from -1 to +3/4. Despite that asymmetric range, every point on
// a 2-face shared between two faces has exactly one encoding, because the
// coordinates of axes below the face axis are stored negated: for axes A < B,
// the 2-face where their components agree in sign is representable only on
// face B, where A's reversed coordinate is -1, and the 2-face where they
// differ only on face A. Edges and vertices may not be exactly representable
// however. The remaining 5 bits store the crossing position, from 0 to 1
// in steps of 1/31.
struct VoxelEdgeData {
	uint16_t data = 0;

	static VoxelEdgeData encode(const Vector4 &p_normal, const real_t p_position) {
		int face = 0;
		for (int axis = 1; axis < 4; axis++) {
			if (Math::abs(p_normal[axis]) > Math::abs(p_normal[face])) {
				face = axis;
			}
		}
		VoxelEdgeData encoded;
		ERR_FAIL_COND_V_MSG(p_normal[face] == (real_t)0.0f, encoded, "VoxelEdgeData cannot encode a zero normal.");
		int32_t quarters[3] = { 0, 0, 0 };
		for (int attempt = 0;; attempt++) {
			int overflow_axis = -1;
			int slot = 0;
			for (int axis = 0; axis < 4; axis++) {
				if (axis == face) {
					continue;
				}
				// Dividing by the signed component also folds away the sign.
				real_t coordinate = p_normal[axis] / p_normal[face];
				if (axis < face) {
					coordinate = -coordinate;
				}
				const int32_t rounded = (int32_t)Math::round(coordinate * (real_t)4.0f);
				if (rounded > 3 && overflow_axis < 0) {
					overflow_axis = axis;
				}
				quarters[slot++] = CLAMP(rounded, -4, 3);
			}
			if (overflow_axis < 0 || attempt == 1) {
				break;
			}
			// A coordinate on the +1 boundary belongs to the adjacent face,
			// where it is exactly representable; the exactly 45-degree
			// normals of common shapes land there.
			face = overflow_axis;
		}
		const uint16_t position_bits = (uint16_t)Math::round(CLAMP(p_position, (real_t)0.0f, (real_t)1.0f) * (real_t)31.0f);
		encoded.data = (uint16_t)((face << 14) | ((quarters[2] + 4) << 11) | ((quarters[1] + 4) << 8) | ((quarters[0] + 4) << 5) | position_bits);
		return encoded;
	}

	// The decoded normal's sign is not meaningful: the component along the
	// stored face axis is always positive.
	Vector4 decode_normal() const {
		const int face = data >> 14;
		Vector4 normal = Vector4();
		normal[face] = 1.0f;
		int slot = 0;
		for (int axis = 0; axis < 4; axis++) {
			if (axis == face) {
				continue;
			}
			real_t coordinate = (real_t)((int32_t)((data >> (5 + 3 * slot)) & 7) - 4) * (real_t)0.25f;
			if (axis < face) {
				coordinate = -coordinate;
			}
			normal[axis] = coordinate;
			slot++;
		}
		return normal.normalized();
	}

	real_t decode_position() const {
		return (real_t)(data & 31) / (real_t)31.0f;
	}
};

static_assert(sizeof(VoxelEdgeData) == 2);
