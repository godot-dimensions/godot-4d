#pragma once

#include "../../voxel/data/voxel_data.h"

#include "tests/test_macros.h"

namespace TestVoxelData {
TEST_CASE("[VoxelData] Hard-coded test data") {
	Ref<VoxelData> data;
	data.instantiate();
	CHECK_MESSAGE(data->get_bounds().size == Vector4i(), "VoxelData should start with no defined region.");
	CHECK_MESSAGE(!data->is_voxel_defined(Vector4i(0, 0, 0, 0)), "VoxelData voxels should start out undefined before their chunk is loaded.");

	data->load_all_chunks();
	const Rect4i test_region = Rect4i(VOXEL_DATA_CHUNK_SIZE_VECTOR * -4, VOXEL_DATA_CHUNK_SIZE_VECTOR * 8);
	const Vector4i last_voxel = test_region.get_end() - Vector4i(1, 1, 1, 1);
	CHECK_MESSAGE(data->get_bounds() == test_region, "VoxelData load_all_chunks should generate the hard-coded 8x8x8x8 chunk test region.");
	CHECK_MESSAGE(data->is_voxel_defined(Vector4i(0, 0, 0, 0)), "VoxelData voxels inside the test region should be defined.");
	CHECK_MESSAGE(data->is_voxel_defined(Vector4i(test_region.position.x, last_voxel.y, test_region.position.z, last_voxel.w)), "VoxelData voxels at the test region's corners should be defined.");
	CHECK_MESSAGE(!data->is_voxel_defined(Vector4i(test_region.get_end().x, 0, 0, 0)), "VoxelData is_voxel_defined should be false outside the bounds.");
	CHECK_MESSAGE(data->is_region_defined(test_region), "VoxelData the whole test region should be defined.");
	CHECK_MESSAGE(!data->is_region_defined(Rect4i(0, 0, 0, 0, test_region.get_end().x + 1, 1, 1, 1)), "VoxelData regions reaching outside the bounds should not be fully defined.");
	CHECK_MESSAGE(!data->is_region_defined(Rect4i(test_region.get_end().x + 1, 0, 0, 0, 1, 1, 1, 1)), "VoxelData regions entirely outside the bounds should not be defined.");
	// The remaining samples are fixed locations on the test tiger, inside the
	// test region for any chunk size of at least 4.
	CHECK_MESSAGE(!data->get_value(Vector4i(0, 0, 0, 0)).is_opaque(), "VoxelData test shape should be empty at the origin.");
	CHECK_MESSAGE(data->get_value(Vector4i(10, 0, 10, 0)).is_opaque(), "VoxelData test shape should be solid on the tiger's core circles.");
	CHECK_MESSAGE(data->get_value(Vector4i(100000, -5, 3, 12)) == VoxelValue(), "VoxelData get_value outside the bounds should return the default value.");
	CHECK_MESSAGE(data->get_density(Vector4i(14, 0, 10, 0)) < data->get_density(Vector4i(0, 0, 0, 0)), "VoxelData density should be lower near the surface than deep inside a material.");
	CHECK_MESSAGE(data->get_density(Vector4i(10, 0, 10, 0)) > 128, "VoxelData density should be high deep inside the solid.");
}

static bool _bounds_alignment_holds(const Rect4i &p_bounds) {
	if (p_bounds.size.x < 2) {
		return true;
	}
	const int32_t half_mask = (p_bounds.size.x >> 1) - 1;
	return (p_bounds.position.x & half_mask) == 0 && (p_bounds.position.y & half_mask) == 0 && (p_bounds.position.z & half_mask) == 0 && (p_bounds.position.w & half_mask) == 0;
}

TEST_CASE("[VoxelData] Bounds expansion and contraction") {
	const Rect4i origin_chunk_bounds = Rect4i(Vector4i(), VOXEL_DATA_CHUNK_SIZE_VECTOR);
	Ref<VoxelData> data;
	data.instantiate();
	data->apply_generated_chunk(data->generate_chunk_content(Vector4i(1, 2, 3, 0)));
	CHECK_MESSAGE(data->is_region_defined(origin_chunk_bounds), "VoxelData apply_generated_chunk should define the whole chunk containing the given voxel.");
	CHECK_MESSAGE(!data->is_voxel_defined(Vector4i(VOXEL_DATA_CHUNK_SIZE, 0, 0, 0)), "VoxelData apply_generated_chunk should not define neighboring chunks.");
	CHECK_MESSAGE(data->get_bounds() == origin_chunk_bounds, "VoxelData bounds should stay tight around the defined content.");

	// The content must survive every reshaping of the bounds below unchanged.
	const Vector4i samples[3] = { Vector4i(0, 0, 0, 0), Vector4i(3, 2, 1, 0), Vector4i(1, 3, 3, 3) };
	VoxelValue sample_values[3];
	for (int i = 0; i < 3; i++) {
		sample_values[i] = data->get_value(samples[i]);
	}

	const Vector4i far_voxel = Vector4i(65, 2, -7, 0);
	data->apply_generated_chunk(data->generate_chunk_content(far_voxel));
	CHECK_MESSAGE(data->is_voxel_defined(far_voxel), "VoxelData should expand its bounds to store chunks outside of them.");
	CHECK_MESSAGE(data->get_bounds().encloses_inclusive(origin_chunk_bounds), "VoxelData expanded bounds should still cover the old content.");
	CHECK_MESSAGE(_bounds_alignment_holds(data->get_bounds()), "VoxelData expanded bounds should satisfy the alignment invariant.");
	for (int i = 0; i < 3; i++) {
		CHECK_MESSAGE(data->get_value(samples[i]) == sample_values[i], "VoxelData expansion should preserve the stored values.");
	}

	data->unload_chunk(far_voxel);
	CHECK_MESSAGE(data->get_bounds() == origin_chunk_bounds, "VoxelData bounds should contract back to the remaining content once the far chunk is unloaded.");
	for (int i = 0; i < 3; i++) {
		CHECK_MESSAGE(data->get_value(samples[i]) == sample_values[i], "VoxelData contraction should preserve the stored values.");
	}

	// A chunk on the other side of the origin: the two chunks together only
	// fit in a root that is half-aligned on the X axis.
	data->apply_generated_chunk(data->generate_chunk_content(Vector4i(-1, 0, 0, 0)));
	CHECK_MESSAGE(data->get_bounds().size == VOXEL_DATA_CHUNK_SIZE_VECTOR * 2, "VoxelData chunks straddling the origin should fit in a half-aligned root instead of one twice the size.");
	CHECK_MESSAGE(data->get_bounds().position.x == -VOXEL_DATA_CHUNK_SIZE, "VoxelData bounds straddling the origin should be half-aligned on that axis.");
	CHECK_MESSAGE(_bounds_alignment_holds(data->get_bounds()), "VoxelData half-aligned bounds should satisfy the alignment invariant.");

	// Expanding from and contracting back to a half-aligned root exercises
	// the middle cases of both: the old root becomes the middle half of the
	// new root, and later contractions gather grandchildren back into it.
	data->apply_generated_chunk(data->generate_chunk_content(far_voxel));
	CHECK_MESSAGE(data->is_voxel_defined(far_voxel), "VoxelData should expand half-aligned bounds to store far away chunks.");
	CHECK_MESSAGE(_bounds_alignment_holds(data->get_bounds()), "VoxelData bounds expanded from a half-aligned root should satisfy the alignment invariant.");
	data->unload_chunk(far_voxel);
	CHECK_MESSAGE(data->get_bounds().size == VOXEL_DATA_CHUNK_SIZE_VECTOR * 2, "VoxelData bounds should contract back around the straddling chunks.");
	CHECK_MESSAGE(data->get_bounds().position.x == -VOXEL_DATA_CHUNK_SIZE, "VoxelData contraction should restore the half-aligned root around the straddling chunks.");
	CHECK_MESSAGE(data->is_voxel_defined(Vector4i(-1, 0, 0, 0)), "VoxelData contraction should keep both straddling chunks.");
	for (int i = 0; i < 3; i++) {
		CHECK_MESSAGE(data->get_value(samples[i]) == sample_values[i], "VoxelData middle expansion and contraction should preserve the stored values.");
	}

	data->unload_chunk(Vector4i(-1, 0, 0, 0));
	data->unload_chunk(Vector4i(0, 0, 0, 0));
	CHECK_MESSAGE(data->get_bounds().size == Vector4i(), "VoxelData bounds should become empty once everything is unloaded.");
	CHECK_MESSAGE(!data->is_voxel_defined(Vector4i(0, 0, 0, 0)), "VoxelData voxels should be undefined once everything is unloaded.");
	CHECK_MESSAGE(data->get_value(Vector4i(0, 0, 0, 0)) == VoxelValue(), "VoxelData get_value should return the default value once everything is unloaded.");

	data->apply_generated_chunk(data->generate_chunk_content(Vector4i(0, 0, 0, 0)));
	CHECK_MESSAGE(data->get_bounds() == origin_chunk_bounds, "VoxelData should accept chunks again after everything was unloaded.");
	for (int i = 0; i < 3; i++) {
		CHECK_MESSAGE(data->get_value(samples[i]) == sample_values[i], "VoxelData chunks reloaded after a full unload should regenerate the same values.");
	}
}
} // namespace TestVoxelData
