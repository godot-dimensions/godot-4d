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

VoxelValue VoxelData::get_value(const Vector4i &p_voxel) const {
	return _tree->get_value(p_voxel);
}

int VoxelData::get_density(const Vector4i &p_voxel) const {
	return _tree->get_value(p_voxel).density;
}

Vector4 VoxelData::get_edge_normal(const Vector4i &p_voxel, const int p_axis) const {
	return _tree->get_edge_normal(p_voxel, p_axis);
}

VoxelData::VoxelData() {
	// Temporary: fill an 8x8x8x8-chunk region centered on the origin with
	// hard-coded test data, so that early systems have something to show.
	const int32_t size = 8 * VOXEL_DATA_CHUNK_SIZE;
	_tree = memnew(VoxelDataTree(Rect4i(-size / 2, -size / 2, -size / 2, -size / 2, size, size, size, size)));
	Ref<TigerTestGenerator> generator;
	generator.instantiate();
	_generator = generator;
	_tree->generate(_generator);
}

VoxelData::~VoxelData() {
	memdelete(_tree);
}
