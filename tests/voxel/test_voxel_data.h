#pragma once

#include "../../voxel/data/voxel_data.h"

#include "tests/test_macros.h"

namespace TestVoxelData {
TEST_CASE("[VoxelData] Hard-coded test data") {
	Ref<VoxelData> data;
	data.instantiate();
	CHECK_MESSAGE(data->get_bounds() == Rect4i(-16, -16, -16, -16, 32, 32, 32, 32), "VoxelData should start with the hard-coded 8x8x8x8 chunk test region.");
	CHECK_MESSAGE(data->is_voxel_defined(Vector4i(0, 0, 0, 0)), "VoxelData voxels inside the test region should be defined.");
	CHECK_MESSAGE(data->is_voxel_defined(Vector4i(-16, 15, -16, 15)), "VoxelData voxels at the test region's corners should be defined.");
	CHECK_MESSAGE(!data->is_voxel_defined(Vector4i(16, 0, 0, 0)), "VoxelData is_voxel_defined should be false outside the bounds.");
	CHECK_MESSAGE(!data->get_value(Vector4i(0, 0, 0, 0)).is_opaque(), "VoxelData test shape should be empty at the origin.");
	CHECK_MESSAGE(data->get_value(Vector4i(10, 0, 10, 0)).is_opaque(), "VoxelData test shape should be solid on the tiger's core circles.");
	CHECK_MESSAGE(data->get_value(Vector4i(100000, -5, 3, 12)) == VoxelValue(), "VoxelData get_value outside the bounds should return the default value.");
	CHECK_MESSAGE(data->get_density(Vector4i(14, 0, 10, 0)) < data->get_density(Vector4i(0, 0, 0, 0)), "VoxelData density should be lower near the surface than deep inside a material.");
	CHECK_MESSAGE(data->get_density(Vector4i(10, 0, 10, 0)) > 128, "VoxelData density should be high deep inside the solid.");
}
} // namespace TestVoxelData
