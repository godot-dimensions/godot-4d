#pragma once

#include "../voxel_constants.h"
#include "voxel_edge_data.h"
#include "voxel_material.h"

#if GDEXTENSION
#include <godot_cpp/templates/local_vector.hpp>
#elif GODOT_MODULE
#include "core/templates/local_vector.h"
#endif

// A dense cubic chunk of voxel data, VOXEL_DATA_CHUNK_SIZE on every axis.
// Coordinates are local to the chunk, the VoxelDataTree leaf node that owns
// the chunk determines where that chunk is located in the world.
//
// The chunk also stores compressed surface data, a normal and a crossing
// position, for each of its active edges: grid edges with a different
// material on either end. The edge from a voxel to its neighbor one step
// along an axis belongs to the chunk that contains the lower voxel, so a
// chunk's edges are addressed by lower voxel and axis, and edges on the
// chunk's upper borders lead into the next chunk. Active edges are sparse,
// so their data is kept in a compact array indexed through a bitmask of
// which edges have an entry.
class VoxelDataLeaf {
public:
	static constexpr int32_t EDGE_COUNT = VOXEL_DATA_CHUNK_HYPERVOLUME * 4;
	static constexpr int32_t EDGE_WORD_COUNT = EDGE_COUNT / 64;

private:
	VoxelMaterial _voxels[VOXEL_DATA_CHUNK_HYPERVOLUME] = {};
	// Bit i is set when edge i has an entry in _edge_data.
	uint64_t _edge_data_bits[EDGE_WORD_COUNT] = {};
	// The number of set bits in all of the words before each word, so that an
	// edge's entry in _edge_data can be found without a full scan.
	uint16_t _edge_data_ranks[EDGE_WORD_COUNT] = {};
	// The surface data of the active edges, in edge index order.
	LocalVector<PackedVoxelEdgeData> _edge_data;

	int32_t _get_edge_data_index(const int32_t p_edge_index) const;

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

	VoxelMaterial get_material(const Vector4i &p_local_voxel) const {
		ERR_FAIL_COND_V(!has_voxel(p_local_voxel), VoxelMaterial::UNDEFINED);
		return _voxels[get_voxel_index(p_local_voxel)];
	}

	void set_material(const Vector4i &p_local_voxel, const VoxelMaterial p_material) {
		ERR_FAIL_COND(!has_voxel(p_local_voxel));
		_voxels[get_voxel_index(p_local_voxel)] = p_material;
	}

	static int32_t get_edge_index(const Vector4i &p_local_voxel, const int p_axis) {
		return get_voxel_index(p_local_voxel) * 4 + p_axis;
	}

	bool has_edge_data(const Vector4i &p_local_voxel, const int p_axis) const;
	VoxelEdgeData get_edge_data(const Vector4i &p_local_voxel, const int p_axis) const;
	void set_edge_data(const Vector4i &p_local_voxel, const int p_axis, const VoxelEdgeData &p_edge_data);
	// Removes the data stored for the edge, if there is any.
	void clear_edge_data(const Vector4i &p_local_voxel, const int p_axis);
	int64_t get_edge_data_count() const { return _edge_data.size(); }
	void clear_all_edge_data();
};
