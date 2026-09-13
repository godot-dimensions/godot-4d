#include "voxel_data.h"

#include "../generators/tiger_test_generator.h"
#include "../voxel_constants.h"

void VoxelData::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_voxel_defined", "voxel"), &VoxelData::is_voxel_defined);
	ClassDB::bind_method(D_METHOD("get_density", "voxel"), &VoxelData::get_density);
}

bool VoxelData::is_voxel_defined(const Vector4i &p_voxel) const {
	const VoxelDataTree *node = _tree->find_deepest_node(p_voxel);
	return node != nullptr && !node->is_undefined();
}

bool VoxelData::is_region_defined(const Rect4i &p_region) const {
	if (!_tree->get_bounds().encloses_inclusive(p_region)) {
		return false;
	}
	return _tree->is_region_defined(p_region);
}

VoxelValue VoxelData::get_value(const Vector4i &p_voxel) const {
	return _tree->get_value(p_voxel);
}

int VoxelData::get_density(const Vector4i &p_voxel) const {
	return _tree->get_value(p_voxel).density;
}

Vector4 VoxelData::get_edge_normal(const Vector4i &p_voxel, const int p_axis) const {
	return _tree->get_edge_normal(p_voxel, p_axis);
}

Vector4i VoxelData::get_chunk_position(const Vector4i &p_voxel) const {
	const Vector4i relative = p_voxel - get_bounds().position;
	Vector4i chunk_position = p_voxel;
	for (int i = 0; i < 4; i++) {
		chunk_position[i] -= (int32_t)Math::posmod(relative[i], VOXEL_DATA_CHUNK_SIZE);
	}
	return chunk_position;
}

VoxelDataTree *VoxelData::generate_chunk_content(const Vector4i &p_voxel) const {
	VoxelDataTree *chunk = memnew(VoxelDataTree(Rect4i(get_chunk_position(p_voxel), VOXEL_DATA_CHUNK_SIZE_VECTOR)));
	chunk->generate(_generator);
	return chunk;
}

void VoxelData::apply_generated_chunk(VoxelDataTree *p_chunk) {
	_tree->apply_generated_chunk(p_chunk);
}

void VoxelData::load_all_chunks() {
	_tree->generate(_generator);
}

VoxelData::VoxelData() {
	// Temporary: a fixed 8x8x8x8-chunk region centered on the origin, filled
	// with hard-coded test data as its chunks are loaded.
	const int32_t size = 8 * VOXEL_DATA_CHUNK_SIZE;
	_tree = memnew(VoxelDataTree(Rect4i(-size / 2, -size / 2, -size / 2, -size / 2, size, size, size, size)));
	Ref<TigerTestGenerator> generator;
	generator.instantiate();
	_generator = generator;
}

VoxelData::~VoxelData() {
	memdelete(_tree);
}
