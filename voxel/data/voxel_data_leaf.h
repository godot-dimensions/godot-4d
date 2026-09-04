#pragma once

#include "../voxel_constants.h"
#include "voxel_edge_normal.h"
#include "voxel_value.h"

// A dense cubic chunk of voxel data, VOXEL_DATA_CHUNK_SIZE on every axis.
// Coordinates are local to the chunk, the VoxelDataTree leaf node that owns
// the chunk determines where that chunk is located in the world.
//
// The chunk also stores a compressed surface normal for each of its active
// edges: grid edges with a different material on either end. The edge from a
// voxel to its neighbor one step along an axis belongs to the chunk that
// contains the lower voxel, so a chunk's edges are addressed by lower voxel
// and axis, and edges on the chunk's upper borders lead into the next chunk.
// Active edges are sparse, so the normals are kept in a compact array
// indexed through a bitmask of which edges have one.
class VoxelDataLeaf {
public:
	static constexpr int32_t EDGE_COUNT = VOXEL_DATA_CHUNK_HYPERVOLUME * 4;
	static constexpr int32_t EDGE_WORD_COUNT = EDGE_COUNT / 64;

private:
	VoxelValue _voxels[VOXEL_DATA_CHUNK_HYPERVOLUME] = {};
	// Bit i is set when edge i has a normal in _edge_normals.
	uint64_t _edge_normal_bits[EDGE_WORD_COUNT] = {};
	// The number of set bits in all of the words before each word, so that an
	// edge's position in _edge_normals can be found without a full scan.
	uint16_t _edge_normal_ranks[EDGE_WORD_COUNT] = {};
	// The normals of the active edges, in edge index order.
	LocalVector<VoxelEdgeNormal> _edge_normals;

	int32_t _get_edge_normal_position(const int32_t p_edge_index) const;

public:
	static bool has_voxel(const Vector4i &p_local_voxel) {
		return p_local_voxel.x >= 0 && p_local_voxel.x < VOXEL_DATA_CHUNK_SIZE &&
				p_local_voxel.y >= 0 && p_local_voxel.y < VOXEL_DATA_CHUNK_SIZE &&
				p_local_voxel.z >= 0 && p_local_voxel.z < VOXEL_DATA_CHUNK_SIZE &&
				p_local_voxel.w >= 0 && p_local_voxel.w < VOXEL_DATA_CHUNK_SIZE;
	}

	static int get_voxel_index(const Vector4i &p_local_voxel) {
		return p_local_voxel.x + VOXEL_DATA_CHUNK_SIZE * (p_local_voxel.y + VOXEL_DATA_CHUNK_SIZE * (p_local_voxel.z + VOXEL_DATA_CHUNK_SIZE * p_local_voxel.w));
	}

	VoxelValue get_value(const Vector4i &p_local_voxel) const {
		ERR_FAIL_COND_V(!has_voxel(p_local_voxel), VoxelValue());
		return _voxels[get_voxel_index(p_local_voxel)];
	}

	void set_value(const Vector4i &p_local_voxel, const VoxelValue p_value) {
		ERR_FAIL_COND(!has_voxel(p_local_voxel));
		_voxels[get_voxel_index(p_local_voxel)] = p_value;
	}

	static int32_t get_edge_index(const Vector4i &p_local_voxel, const int p_axis) {
		return get_voxel_index(p_local_voxel) * 4 + p_axis;
	}

	bool has_edge_normal(const Vector4i &p_local_voxel, const int p_axis) const;
	Vector4 get_edge_normal(const Vector4i &p_local_voxel, const int p_axis) const;
	void set_edge_normal(const Vector4i &p_local_voxel, const int p_axis, const Vector4 &p_normal);
	int64_t get_edge_normal_count() const { return _edge_normals.size(); }
	void clear_edge_normals();
};
