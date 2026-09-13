#pragma once

#include "../../voxel/data/voxel_data.h"

#include "tests/test_macros.h"

namespace TestVoxelData {
TEST_CASE("[VoxelData] Hard-coded test data") {
	Ref<VoxelData> data;
	data.instantiate();
	CHECK_MESSAGE(data->get_bounds() == Rect4i(-16, -16, -16, -16, 32, 32, 32, 32), "VoxelData should start with the hard-coded 8x8x8x8 chunk test region.");
	CHECK_MESSAGE(!data->is_voxel_defined(Vector4i(0, 0, 0, 0)), "VoxelData voxels should start out undefined before their chunk is loaded.");

	VoxelDataTree *chunk = data->generate_chunk_content(Vector4i(1, 2, 3, 0));
	data->apply_generated_chunk(chunk);
	CHECK_MESSAGE(data->is_region_defined(Rect4i(0, 0, 0, 0, 4, 4, 4, 4)), "VoxelData apply_generated_chunk should define the whole chunk containing the given voxel.");
	CHECK_MESSAGE(!data->is_voxel_defined(Vector4i(4, 0, 0, 0)), "VoxelData apply_generated_chunk should not define neighboring chunks.");

	data->load_all_chunks();
	CHECK_MESSAGE(data->is_voxel_defined(Vector4i(0, 0, 0, 0)), "VoxelData voxels inside the test region should be defined.");
	CHECK_MESSAGE(data->is_voxel_defined(Vector4i(-16, 15, -16, 15)), "VoxelData voxels at the test region's corners should be defined.");
	CHECK_MESSAGE(!data->is_voxel_defined(Vector4i(16, 0, 0, 0)), "VoxelData is_voxel_defined should be false outside the bounds.");
	CHECK_MESSAGE(data->is_region_defined(Rect4i(-16, -16, -16, -16, 32, 32, 32, 32)), "VoxelData the whole test region should be defined.");
	CHECK_MESSAGE(!data->is_region_defined(Rect4i(0, 0, 0, 0, 17, 1, 1, 1)), "VoxelData regions reaching outside the bounds should not be fully defined.");
	CHECK_MESSAGE(!data->is_region_defined(Rect4i(20, 0, 0, 0, 1, 1, 1, 1)), "VoxelData regions entirely outside the bounds should not be defined.");
	CHECK_MESSAGE(!data->get_value(Vector4i(0, 0, 0, 0)).is_opaque(), "VoxelData test shape should be empty at the origin.");
	CHECK_MESSAGE(data->get_value(Vector4i(10, 0, 10, 0)).is_opaque(), "VoxelData test shape should be solid on the tiger's core circles.");
	CHECK_MESSAGE(data->get_value(Vector4i(100000, -5, 3, 12)) == VoxelValue(), "VoxelData get_value outside the bounds should return the default value.");
	CHECK_MESSAGE(data->get_density(Vector4i(14, 0, 10, 0)) < data->get_density(Vector4i(0, 0, 0, 0)), "VoxelData density should be lower near the surface than deep inside a material.");
	CHECK_MESSAGE(data->get_density(Vector4i(10, 0, 10, 0)) > 128, "VoxelData density should be high deep inside the solid.");
}
} // namespace TestVoxelData
