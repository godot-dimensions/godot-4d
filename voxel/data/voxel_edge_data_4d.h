#pragma once

#include "../../godot_4d_defines.h"

// The surface data of one active voxel grid edge: the surface normal, whose
// overall sign carries no meaning, and the position along the edge where the
// surface crosses it, from 0 at the lower voxel's center to 1 at the upper's.
struct VoxelEdgeData4D {
	Vector4 normal;
	real_t position = 0.0f;

	VoxelEdgeData4D() {}
	VoxelEdgeData4D(const Vector4 &p_normal, const real_t p_position) :
			normal(p_normal),
			position(p_position) {}
};

// Edge surface data compressed to the 32 bits in which chunks store it.
//
// The normal is stored as a point on the surface of a hypercube. 2 bits pick
// the face it lies on (the normal's overall sign is not stored), and 7 bits
// per remaining axis store that coordinate of the point, in steps of 1/64
// from -1 to +63/64. Despite that asymmetric range, every point on a 2-face
// shared between two faces has exactly one encoding, because the coordinates
// of axes below the face axis are stored negated: for axes A < B, the 2-face
// where their components agree in sign is representable only on face B,
// where A's reversed coordinate is -1, and the 2-face where they differ only
// on face A. Edges and vertices may not be exactly representable, however, so
// the face whose rounded point lies nearest the normal's direction is
// stored. The remaining 9 bits store the crossing position, from 0 to 1 in
// steps of 1/511.
struct PackedVoxelEdgeData4D {
	uint32_t data = 0;

	static PackedVoxelEdgeData4D encode(const VoxelEdgeData4D &p_edge_data) {
		const Vector4 &normal = p_edge_data.normal;
		PackedVoxelEdgeData4D encoded;
		int best_face = -1;
		int32_t best_steps[3] = { 0, 0, 0 };
		real_t best_alignment = (real_t)-1.0f;
		for (int face = 0; face < 4; face++) {
			if (normal[face] == (real_t)0.0f) {
				continue;
			}
			int32_t steps[3];
			Vector4 point;
			point[face] = 1.0f;
			int slot = 0;
			for (int axis = 0; axis < 4; axis++) {
				if (axis == face) {
					continue;
				}
				// Dividing by the signed component also folds away the sign.
				real_t coordinate = normal[axis] / normal[face];
				if (axis < face) {
					coordinate = -coordinate;
				}
				const int32_t rounded = CLAMP((int32_t)Math::round(coordinate * (real_t)64.0f), -64, 63);
				steps[slot++] = rounded;
				const real_t stored = (real_t)rounded / (real_t)64.0f;
				point[axis] = axis < face ? -stored : stored;
			}
			const real_t alignment = Math::abs(normal.dot(point)) / point.length();
			if (alignment > best_alignment) {
				best_alignment = alignment;
				best_face = face;
				best_steps[0] = steps[0];
				best_steps[1] = steps[1];
				best_steps[2] = steps[2];
			}
		}
		ERR_FAIL_COND_V_MSG(best_face < 0, encoded, "PackedVoxelEdgeData4D cannot encode a zero normal.");
		const uint32_t position_bits = (uint32_t)Math::round(CLAMP(p_edge_data.position, (real_t)0.0f, (real_t)1.0f) * (real_t)511.0f);
		encoded.data = ((uint32_t)best_face << 30) | ((uint32_t)(best_steps[2] + 64) << 23) | ((uint32_t)(best_steps[1] + 64) << 16) | ((uint32_t)(best_steps[0] + 64) << 9) | position_bits;
		return encoded;
	}

	// The decoded normal is normalized, with its component along the stored
	// face axis always positive.
	VoxelEdgeData4D decode() const {
		const int face = data >> 30;
		Vector4 normal = Vector4();
		normal[face] = 1.0f;
		int slot = 0;
		for (int axis = 0; axis < 4; axis++) {
			if (axis == face) {
				continue;
			}
			real_t coordinate = (real_t)((int32_t)((data >> (9 + 7 * slot)) & 127) - 64) / (real_t)64.0f;
			if (axis < face) {
				coordinate = -coordinate;
			}
			normal[axis] = coordinate;
			slot++;
		}
		return VoxelEdgeData4D(normal.normalized(), (real_t)(data & 511) / (real_t)511.0f);
	}
};

static_assert(sizeof(PackedVoxelEdgeData4D) == 4);
