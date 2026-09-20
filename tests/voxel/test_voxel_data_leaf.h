#pragma once

#include "../../voxel/data/voxel_data_leaf.h"

#include "tests/test_macros.h"

namespace TestVoxelDataLeaf {
TEST_CASE("[VoxelDataLeaf] Get and set materials") {
	VoxelDataLeaf leaf;
	const Vector4i voxel = Vector4i(1, 2, 3, 0);
	CHECK_MESSAGE(leaf.get_material(voxel) == VoxelMaterial::UNDEFINED, "VoxelDataLeaf voxels should start out undefined.");
	leaf.set_material(voxel, VoxelMaterial::SOLID);
	CHECK_MESSAGE(leaf.get_material(voxel) == VoxelMaterial::SOLID, "VoxelDataLeaf get_material should return what set_material stored.");
	CHECK_MESSAGE(leaf.get_material(Vector4i(2, 2, 3, 0)) == VoxelMaterial::UNDEFINED, "VoxelDataLeaf set_material should not affect neighboring voxels.");

	CHECK_MESSAGE(VoxelDataLeaf::has_voxel(Vector4i(0, 0, 0, 0)), "VoxelDataLeaf has_voxel should include the origin.");
	CHECK_MESSAGE(!VoxelDataLeaf::has_voxel(Vector4i(VOXEL_DATA_CHUNK_SIZE, 0, 0, 0)), "VoxelDataLeaf has_voxel should exclude coordinates at the chunk size.");
	CHECK_MESSAGE(!VoxelDataLeaf::has_voxel(Vector4i(0, -1, 0, 0)), "VoxelDataLeaf has_voxel should exclude negative coordinates.");
	ERR_PRINT_OFF;
	CHECK_MESSAGE(leaf.get_material(Vector4i(-1, 0, 0, 0)) == VoxelMaterial::UNDEFINED, "VoxelDataLeaf get_material outside the chunk should return UNDEFINED.");
	ERR_PRINT_ON;
}

TEST_CASE("[VoxelDataLeaf] Edge data") {
	VoxelDataLeaf leaf;
	const Vector4i voxel = Vector4i(1, 2, 3, 0);
	const VoxelEdgeData data_a = VoxelEdgeData::encode(Vector4(0, 0, 1, 0), 0.25f);
	const VoxelEdgeData data_b = VoxelEdgeData::encode(Vector4(1, 0, 0, 0), 0.75f);
	const VoxelEdgeData data_c = VoxelEdgeData::encode(Vector4(0, 0, -1, 0), 0.5f);
	CHECK_MESSAGE(!leaf.has_edge_data(voxel, 2), "VoxelDataLeaf edges should start out without data.");
	CHECK_MESSAGE(leaf.get_edge_data_count() == 0, "VoxelDataLeaf should start out with no edge data stored.");

	leaf.set_edge_data(voxel, 2, data_a);
	CHECK_MESSAGE(leaf.has_edge_data(voxel, 2), "VoxelDataLeaf set_edge_data should make the edge active.");
	CHECK_MESSAGE(!leaf.has_edge_data(voxel, 3), "VoxelDataLeaf set_edge_data should not affect the voxel's other edges.");
	CHECK_MESSAGE(leaf.get_edge_data(voxel, 2).data == data_a.data, "VoxelDataLeaf get_edge_data should return what set_edge_data stored.");

	// Inserting an edge with a lower index shifts the compact array.
	leaf.set_edge_data(Vector4i(0, 0, 0, 0), 0, data_b);
	CHECK_MESSAGE(leaf.get_edge_data(Vector4i(0, 0, 0, 0), 0).data == data_b.data, "VoxelDataLeaf should store edge data inserted before another entry.");
	CHECK_MESSAGE(leaf.get_edge_data(voxel, 2).data == data_a.data, "VoxelDataLeaf should keep existing edge data when another entry is inserted before it.");
	CHECK_MESSAGE(leaf.get_edge_data_count() == 2, "VoxelDataLeaf should store one entry per active edge.");

	leaf.set_edge_data(voxel, 2, data_c);
	CHECK_MESSAGE(leaf.get_edge_data(voxel, 2).data == data_c.data, "VoxelDataLeaf set_edge_data on an active edge should replace its data.");
	CHECK_MESSAGE(leaf.get_edge_data_count() == 2, "VoxelDataLeaf set_edge_data on an active edge should not grow the array.");

	leaf.clear_edge_data(voxel, 2);
	CHECK_MESSAGE(!leaf.has_edge_data(voxel, 2), "VoxelDataLeaf clear_edge_data should remove the edge's data.");
	CHECK_MESSAGE(leaf.has_edge_data(Vector4i(0, 0, 0, 0), 0), "VoxelDataLeaf clear_edge_data should not affect other edges.");
	CHECK_MESSAGE(leaf.get_edge_data_count() == 1, "VoxelDataLeaf clear_edge_data should shrink the array.");

	leaf.clear_all_edge_data();
	CHECK_MESSAGE(!leaf.has_edge_data(Vector4i(0, 0, 0, 0), 0), "VoxelDataLeaf clear_all_edge_data should remove all edge data.");
	CHECK_MESSAGE(leaf.get_edge_data_count() == 0, "VoxelDataLeaf clear_all_edge_data should empty the array.");
}
} // namespace TestVoxelDataLeaf
