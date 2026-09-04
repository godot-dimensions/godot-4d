#pragma once

#include "../../voxel/data/voxel_data_leaf.h"

#include "tests/test_macros.h"

namespace TestVoxelDataLeaf {
TEST_CASE("[VoxelDataLeaf] Get and set voxels") {
	VoxelDataLeaf leaf;
	const Vector4i voxel = Vector4i(1, 2, 3, 0);
	const VoxelValue solid_value = { VoxelMaterial::SOLID, 200 };
	CHECK_MESSAGE(leaf.get_value(voxel) == VoxelValue(), "VoxelDataLeaf voxels should start out with the default value.");
	CHECK_MESSAGE(leaf.get_value(voxel).material == VoxelMaterial::UNDEFINED, "VoxelDataLeaf voxels should start out undefined.");
	leaf.set_value(voxel, solid_value);
	CHECK_MESSAGE(leaf.get_value(voxel) == solid_value, "VoxelDataLeaf get_value should return what set_value stored.");
	CHECK_MESSAGE(leaf.get_value(Vector4i(2, 2, 3, 0)) == VoxelValue(), "VoxelDataLeaf set_value should not affect neighboring voxels.");

	CHECK_MESSAGE(VoxelDataLeaf::has_voxel(Vector4i(0, 0, 0, 0)), "VoxelDataLeaf has_voxel should include the origin.");
	CHECK_MESSAGE(!VoxelDataLeaf::has_voxel(Vector4i(VOXEL_DATA_CHUNK_SIZE, 0, 0, 0)), "VoxelDataLeaf has_voxel should exclude coordinates at the chunk size.");
	CHECK_MESSAGE(!VoxelDataLeaf::has_voxel(Vector4i(0, -1, 0, 0)), "VoxelDataLeaf has_voxel should exclude negative coordinates.");
	ERR_PRINT_OFF;
	CHECK_MESSAGE(leaf.get_value(Vector4i(-1, 0, 0, 0)) == VoxelValue(), "VoxelDataLeaf get_value outside the chunk should return the default value.");
	ERR_PRINT_ON;
}

TEST_CASE("[VoxelDataLeaf] Edge normals") {
	VoxelDataLeaf leaf;
	const Vector4i voxel = Vector4i(1, 2, 3, 0);
	CHECK_MESSAGE(!leaf.has_edge_normal(voxel, 2), "VoxelDataLeaf edges should start out without normals.");
	CHECK_MESSAGE(leaf.get_edge_normal_count() == 0, "VoxelDataLeaf should start out with no edge normals stored.");

	leaf.set_edge_normal(voxel, 2, Vector4(0, 0, 1, 0));
	CHECK_MESSAGE(leaf.has_edge_normal(voxel, 2), "VoxelDataLeaf set_edge_normal should make the edge active.");
	CHECK_MESSAGE(!leaf.has_edge_normal(voxel, 3), "VoxelDataLeaf set_edge_normal should not affect the voxel's other edges.");
	CHECK_MESSAGE(leaf.get_edge_normal(voxel, 2) == Vector4(0, 0, 1, 0), "VoxelDataLeaf should store axis-aligned edge normals exactly.");

	// Inserting an edge with a lower index shifts the compact array.
	leaf.set_edge_normal(Vector4i(0, 0, 0, 0), 0, Vector4(1, 0, 0, 0));
	CHECK_MESSAGE(leaf.get_edge_normal(Vector4i(0, 0, 0, 0), 0) == Vector4(1, 0, 0, 0), "VoxelDataLeaf should store an edge normal inserted before another.");
	CHECK_MESSAGE(leaf.get_edge_normal(voxel, 2) == Vector4(0, 0, 1, 0), "VoxelDataLeaf should keep existing edge normals when another is inserted before them.");
	CHECK_MESSAGE(leaf.get_edge_normal_count() == 2, "VoxelDataLeaf should store one normal per active edge.");

	leaf.set_edge_normal(voxel, 2, Vector4(0, 0, -1, 0));
	CHECK_MESSAGE(leaf.get_edge_normal(voxel, 2) == Vector4(0, 0, -1, 0), "VoxelDataLeaf set_edge_normal on an active edge should replace its normal.");
	CHECK_MESSAGE(leaf.get_edge_normal_count() == 2, "VoxelDataLeaf set_edge_normal on an active edge should not grow the array.");

	const Vector4 diagonal = Vector4(1, 1, 0, 0).normalized();
	leaf.set_edge_normal(voxel, 0, diagonal);
	CHECK_MESSAGE(leaf.get_edge_normal(voxel, 0).dot(diagonal) > 0.999, "VoxelDataLeaf compressed normals should be accurate to well under a degree.");

	leaf.clear_edge_normals();
	CHECK_MESSAGE(!leaf.has_edge_normal(voxel, 2), "VoxelDataLeaf clear_edge_normals should remove all edge normals.");
	CHECK_MESSAGE(leaf.get_edge_normal_count() == 0, "VoxelDataLeaf clear_edge_normals should empty the array.");
}
} // namespace TestVoxelDataLeaf
