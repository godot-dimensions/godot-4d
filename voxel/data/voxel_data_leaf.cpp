#include "voxel_data_leaf.h"

static int32_t _count_set_bits(uint64_t p_bits) {
	p_bits = p_bits - ((p_bits >> 1) & 0x5555555555555555ULL);
	p_bits = (p_bits & 0x3333333333333333ULL) + ((p_bits >> 2) & 0x3333333333333333ULL);
	p_bits = (p_bits + (p_bits >> 4)) & 0x0F0F0F0F0F0F0F0FULL;
	return (int32_t)((p_bits * 0x0101010101010101ULL) >> 56);
}

int32_t VoxelDataLeaf::_get_edge_normal_position(const int32_t p_edge_index) const {
	const int32_t word = p_edge_index >> 6;
	const uint64_t bits_below = _edge_normal_bits[word] & (((uint64_t)1 << (p_edge_index & 63)) - 1);
	return _edge_normal_ranks[word] + _count_set_bits(bits_below);
}

bool VoxelDataLeaf::has_edge_normal(const Vector4i &p_local_voxel, const int p_axis) const {
	ERR_FAIL_INDEX_V(p_axis, 4, false);
	ERR_FAIL_COND_V(!has_voxel(p_local_voxel), false);
	const int32_t edge_index = get_edge_index(p_local_voxel, p_axis);
	return (_edge_normal_bits[edge_index >> 6] & ((uint64_t)1 << (edge_index & 63))) != 0;
}

Vector4 VoxelDataLeaf::get_edge_normal(const Vector4i &p_local_voxel, const int p_axis) const {
	ERR_FAIL_COND_V_MSG(!has_edge_normal(p_local_voxel, p_axis), Vector4(), "VoxelDataLeaf has no normal stored for this edge.");
	return _edge_normals[_get_edge_normal_position(get_edge_index(p_local_voxel, p_axis))].decode();
}

void VoxelDataLeaf::set_edge_normal(const Vector4i &p_local_voxel, const int p_axis, const Vector4 &p_normal) {
	ERR_FAIL_INDEX(p_axis, 4);
	ERR_FAIL_COND(!has_voxel(p_local_voxel));
	const int32_t edge_index = get_edge_index(p_local_voxel, p_axis);
	const VoxelEdgeNormal encoded = VoxelEdgeNormal::encode(p_normal);
	const int32_t position = _get_edge_normal_position(edge_index);
	const uint64_t bit = (uint64_t)1 << (edge_index & 63);
	if ((_edge_normal_bits[edge_index >> 6] & bit) != 0) {
		_edge_normals[position] = encoded;
		return;
	}
	_edge_normal_bits[edge_index >> 6] |= bit;
	for (int32_t word = (edge_index >> 6) + 1; word < EDGE_WORD_COUNT; word++) {
		_edge_normal_ranks[word]++;
	}
	_edge_normals.insert(position, encoded);
}

void VoxelDataLeaf::clear_edge_normals() {
	for (int32_t word = 0; word < EDGE_WORD_COUNT; word++) {
		_edge_normal_bits[word] = 0;
		_edge_normal_ranks[word] = 0;
	}
	_edge_normals.clear();
}
