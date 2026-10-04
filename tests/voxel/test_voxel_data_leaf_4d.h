#pragma once

#include "../../voxel/data/voxel_data_leaf_4d.h"

#include "tests/test_macros.h"

namespace TestVoxelDataLeaf4D {
constexpr VoxelMaterial4D SOLID_MATERIAL = (VoxelMaterial4D)0;

TEST_CASE("[VoxelDataLeaf4D] Get and set materials") {
	VoxelDataLeaf4D leaf;
	const Vector4i voxel = Vector4i(1, 2, 3, 0);
	const Vector4i neighbor = Vector4i(2, 2, 3, 0);
	leaf.set_material(neighbor, VoxelMaterial4D::AIR);
	leaf.set_material(voxel, SOLID_MATERIAL);
	CHECK_MESSAGE(leaf.get_material(voxel) == SOLID_MATERIAL, "VoxelDataLeaf4D get_material should return what set_material stored.");
	CHECK_MESSAGE(leaf.get_material(neighbor) == VoxelMaterial4D::AIR, "VoxelDataLeaf4D set_material should not affect neighboring voxels.");

	CHECK_MESSAGE(VoxelDataLeaf4D::has_voxel(Vector4i(0, 0, 0, 0)), "VoxelDataLeaf4D has_voxel should include the origin.");
	CHECK_MESSAGE(!VoxelDataLeaf4D::has_voxel(Vector4i(VOXEL_4D_DATA_CHUNK_SIZE, 0, 0, 0)), "VoxelDataLeaf4D has_voxel should exclude coordinates at the chunk size.");
	CHECK_MESSAGE(!VoxelDataLeaf4D::has_voxel(Vector4i(0, -1, 0, 0)), "VoxelDataLeaf4D has_voxel should exclude negative coordinates.");
	ERR_PRINT_OFF;
	CHECK_MESSAGE(leaf.get_material(Vector4i(-1, 0, 0, 0)) == VoxelMaterial4D::UNDEFINED, "VoxelDataLeaf4D get_material outside the chunk should return UNDEFINED.");
	ERR_PRINT_ON;
}

// The normal's sign carries no meaning and both fields are quantized for
// storage, so equality holds up to the sign and the storage precision.
static bool _edge_data_equal(const VoxelEdgeData4D &p_a, const VoxelEdgeData4D &p_b) {
	return Math::abs(p_a.normal.dot(p_b.normal)) > (real_t)0.999 && Math::abs(p_a.position - p_b.position) < (real_t)0.002;
}

TEST_CASE("[VoxelDataLeaf4D] Edge data") {
	VoxelDataLeaf4D leaf;
	const Vector4i voxel = Vector4i(1, 2, 3, 0);
	// Values distinguishable well beyond the storage precision, so that
	// reads can tell the entries apart.
	const VoxelEdgeData4D data_a = VoxelEdgeData4D(Vector4(0, 0, 1, 0), 0.0f);
	const VoxelEdgeData4D data_b = VoxelEdgeData4D(Vector4(1, 0, 0, 0), 1.0f);
	const VoxelEdgeData4D data_c = VoxelEdgeData4D(Vector4(0, 0, -1, 0), 0.26f);
	CHECK_MESSAGE(!leaf.has_edge_data(voxel, 2), "VoxelDataLeaf4D edges should start out without data.");
	CHECK_MESSAGE(leaf.get_edge_data_count() == 0, "VoxelDataLeaf4D should start out with no edge data stored.");

	leaf.set_edge_data(voxel, 2, data_a);
	CHECK_MESSAGE(leaf.has_edge_data(voxel, 2), "VoxelDataLeaf4D set_edge_data should make the edge active.");
	CHECK_MESSAGE(!leaf.has_edge_data(voxel, 3), "VoxelDataLeaf4D set_edge_data should not affect the voxel's other edges.");
	CHECK_MESSAGE(_edge_data_equal(leaf.get_edge_data(voxel, 2), data_a), "VoxelDataLeaf4D get_edge_data should return what set_edge_data stored.");

	// Inserting an edge with a lower index shifts the compact array.
	leaf.set_edge_data(Vector4i(0, 0, 0, 0), 0, data_b);
	CHECK_MESSAGE(_edge_data_equal(leaf.get_edge_data(Vector4i(0, 0, 0, 0), 0), data_b), "VoxelDataLeaf4D should store edge data inserted before another entry.");
	CHECK_MESSAGE(_edge_data_equal(leaf.get_edge_data(voxel, 2), data_a), "VoxelDataLeaf4D should keep existing edge data when another entry is inserted before it.");
	CHECK_MESSAGE(leaf.get_edge_data_count() == 2, "VoxelDataLeaf4D should store one entry per active edge.");

	leaf.set_edge_data(voxel, 2, data_c);
	CHECK_MESSAGE(_edge_data_equal(leaf.get_edge_data(voxel, 2), data_c), "VoxelDataLeaf4D set_edge_data on an active edge should replace its data.");
	CHECK_MESSAGE(leaf.get_edge_data_count() == 2, "VoxelDataLeaf4D set_edge_data on an active edge should not grow the array.");

	leaf.clear_edge_data(voxel, 2);
	CHECK_MESSAGE(!leaf.has_edge_data(voxel, 2), "VoxelDataLeaf4D clear_edge_data should remove the edge's data.");
	CHECK_MESSAGE(leaf.has_edge_data(Vector4i(0, 0, 0, 0), 0), "VoxelDataLeaf4D clear_edge_data should not affect other edges.");
	CHECK_MESSAGE(leaf.get_edge_data_count() == 1, "VoxelDataLeaf4D clear_edge_data should shrink the array.");

	leaf.clear_all_edge_data();
	CHECK_MESSAGE(!leaf.has_edge_data(Vector4i(0, 0, 0, 0), 0), "VoxelDataLeaf4D clear_all_edge_data should remove all edge data.");
	CHECK_MESSAGE(leaf.get_edge_data_count() == 0, "VoxelDataLeaf4D clear_all_edge_data should empty the array.");
}
} // namespace TestVoxelDataLeaf4D
