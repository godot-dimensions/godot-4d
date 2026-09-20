#pragma once

#include "../../voxel/data/voxel_data_leaf.h"
#include "../../voxel/data/voxel_data_tree.h"

#include "tests/test_macros.h"

namespace TestVoxelDataTree {

constexpr VoxelMaterial SOLID_MATERIAL = VoxelMaterial::SOLID;

class UniformSolidGenerator : public VoxelGenerator {
public:
	virtual VoxelMaterial get_material(const Vector4i &) const override { return VoxelMaterial::SOLID; }
	virtual VoxelEdgeData get_edge_data(const Vector4i &, const int) const override { return VoxelEdgeData(); }
};

// Solid where X is non-negative, so the boundary is halfway between the
// voxels on either side of it.
class HalfSpaceGenerator : public VoxelGenerator {
public:
	virtual VoxelMaterial get_material(const Vector4i &p_voxel) const override {
		return p_voxel.x >= 0 ? VoxelMaterial::SOLID : VoxelMaterial::AIR;
	}
	virtual VoxelEdgeData get_edge_data(const Vector4i &, const int) const override {
		return VoxelEdgeData::encode(Vector4(-1, 0, 0, 0), 0.5f);
	}
};

// Solid where X + Y is non-negative.
class DiagonalHalfSpaceGenerator : public VoxelGenerator {
public:
	virtual VoxelMaterial get_material(const Vector4i &p_voxel) const override {
		return p_voxel.x + p_voxel.y >= 0 ? VoxelMaterial::SOLID : VoxelMaterial::AIR;
	}
	virtual VoxelEdgeData get_edge_data(const Vector4i &, const int) const override {
		return VoxelEdgeData::encode(Vector4(-1, -1, 0, 0), 0.5f);
	}
};

TEST_CASE("[VoxelDataTree] Node types and clearing") {
	// Fully aligned so that the root itself may become a leaf.
	VoxelDataTree tree = VoxelDataTree(Rect4i(-16, -16, -16, -16, 16, 16, 16, 16));
	CHECK_MESSAGE(tree.is_undefined(), "VoxelDataTree nodes should start out undefined.");

	tree.set_leaf_data(memnew(VoxelDataLeaf));
	CHECK_MESSAGE(tree.is_leaf(), "VoxelDataTree set_leaf_data with data should make the node a leaf.");
	CHECK_MESSAGE(tree.get_leaf_data() != nullptr, "VoxelDataTree get_leaf_data should return the data of a leaf node.");

	tree.clear();
	CHECK_MESSAGE(tree.is_undefined(), "VoxelDataTree clear should make the node undefined again.");

	tree.set_constant_material(SOLID_MATERIAL);
	CHECK_MESSAGE(tree.is_constant(), "VoxelDataTree set_constant_material should make the node constant.");
	CHECK_MESSAGE(tree.get_constant_material() == SOLID_MATERIAL, "VoxelDataTree get_constant_material should return the material that was set.");
	tree.set_leaf_data(memnew(VoxelDataLeaf));
	CHECK_MESSAGE(tree.is_leaf(), "VoxelDataTree set_leaf_data should replace a constant node with a leaf.");

	tree.clear();
	CHECK_MESSAGE(tree.is_undefined(), "VoxelDataTree clear should make a leaf node undefined again.");

	tree.subdivide();
	CHECK_MESSAGE(tree.is_parent(), "VoxelDataTree subdivide should make the node a parent.");
	CHECK_MESSAGE(tree.get_children() != nullptr, "VoxelDataTree get_children should return the children of a parent node.");
	tree.clear();
	CHECK_MESSAGE(tree.is_undefined(), "VoxelDataTree clear should free the children and make the node undefined again.");
}

TEST_CASE("[VoxelDataTree] Voxel containment and lookup") {
	VoxelDataTree tree = VoxelDataTree(Rect4i(0, 0, 0, 0, 16, 16, 16, 16));
	CHECK_MESSAGE(tree.has_voxel(Vector4i(0, 0, 0, 0)), "VoxelDataTree has_voxel should include the position corner.");
	CHECK_MESSAGE(tree.has_voxel(Vector4i(15, 15, 15, 15)), "VoxelDataTree has_voxel should include the last voxel before the end.");
	CHECK_MESSAGE(!tree.has_voxel(Vector4i(16, 0, 0, 0)), "VoxelDataTree has_voxel should exclude the end, voxel cells are half-open ranges.");
	CHECK_MESSAGE(!tree.has_voxel(Vector4i(0, -1, 0, 0)), "VoxelDataTree has_voxel should exclude voxels below the position.");

	CHECK_MESSAGE(tree.get_child_index_containing(Vector4i(0, 0, 0, 0)) == 0, "VoxelDataTree voxels in the lower orthant should map to child 0.");
	CHECK_MESSAGE(tree.get_child_index_containing(Vector4i(8, 7, 8, 7)) == 0b0101, "VoxelDataTree child index bits should be X, Y, Z, W from lowest to highest.");
	CHECK_MESSAGE(tree.get_child_index_containing(Vector4i(15, 15, 15, 15)) == 15, "VoxelDataTree voxels in the upper orthant should map to child 15.");

	CHECK_MESSAGE(tree.find_deepest_node(Vector4i(3, 3, 3, 3)) == &tree, "VoxelDataTree find_deepest_node on an unsubdivided node should return the node itself.");
	CHECK_MESSAGE(tree.find_deepest_node(Vector4i(16, 3, 3, 3)) == nullptr, "VoxelDataTree find_deepest_node should return null for a voxel outside the bounds.");

	tree.subdivide();
	VoxelDataTree *child = tree.get_child_containing(Vector4i(8, 7, 8, 7));
	REQUIRE(child != nullptr);
	CHECK_MESSAGE(child == tree.get_child(0b0101), "VoxelDataTree get_child_containing should agree with get_child_index_containing.");
	child->subdivide();
	VoxelDataTree *deepest = tree.find_deepest_node(Vector4i(8, 7, 8, 7));
	CHECK_MESSAGE(deepest == child->get_child_containing(Vector4i(8, 7, 8, 7)), "VoxelDataTree find_deepest_node should descend through parents to the deepest non-parent node.");
	CHECK_MESSAGE(deepest->is_undefined(), "VoxelDataTree find_deepest_node should never return a parent node.");
	CHECK_MESSAGE(deepest->get_bounds() == Rect4i(8, 4, 8, 4, 4, 4, 4, 4), "VoxelDataTree find_deepest_node should return the grandchild whose bounds contain the voxel.");
}

TEST_CASE("[VoxelDataTree] Voxel value lookup") {
	VoxelDataTree tree = VoxelDataTree(Rect4i(0, 0, 0, 0, 8, 8, 8, 8));
	CHECK_MESSAGE(tree.get_material(Vector4i(1, 2, 3, 4)) == VoxelMaterial::UNDEFINED, "VoxelDataTree get_material of an undefined node should return UNDEFINED.");
	CHECK_MESSAGE(tree.get_material(Vector4i(0, 0, 0, 8)) == VoxelMaterial::UNDEFINED, "VoxelDataTree get_material outside the bounds should return UNDEFINED.");

	VoxelDataTree *children = tree.subdivide();
	children[0].set_constant_material(SOLID_MATERIAL);
	CHECK_MESSAGE(tree.get_material(Vector4i(3, 3, 3, 3)) == SOLID_MATERIAL, "VoxelDataTree get_material should return the constant value of a constant descendant.");
	CHECK_MESSAGE(tree.get_material(Vector4i(4, 3, 3, 3)) == VoxelMaterial::UNDEFINED, "VoxelDataTree get_material in an undefined sibling should return UNDEFINED.");

	VoxelDataLeaf *leaf = memnew(VoxelDataLeaf);
	leaf->set_material(Vector4i(1, 0, 0, 0), SOLID_MATERIAL);
	children[15].set_leaf_data(leaf);
	CHECK_MESSAGE(tree.get_material(Vector4i(5, 4, 4, 4)) == SOLID_MATERIAL, "VoxelDataTree get_material should read a chunk-sized leaf at full detail, relative to the leaf node's position.");
	CHECK_MESSAGE(tree.get_material(Vector4i(6, 4, 4, 4)) == VoxelMaterial::UNDEFINED, "VoxelDataTree get_material should not smear leaf materials across neighboring voxels at full detail.");
}

TEST_CASE("[VoxelDataTree] Region definedness") {
	VoxelDataTree tree = VoxelDataTree(Rect4i(0, 0, 0, 0, 8, 8, 8, 8));
	CHECK_MESSAGE(!tree.is_region_defined(Rect4i(1, 1, 1, 1, 2, 2, 2, 2)), "VoxelDataTree regions inside an undefined node should not be defined.");
	VoxelDataTree *children = tree.subdivide();
	children[0].set_constant_material(SOLID_MATERIAL);
	children[1].set_leaf_data(memnew(VoxelDataLeaf));
	CHECK_MESSAGE(tree.is_region_defined(Rect4i(0, 0, 0, 0, 4, 4, 4, 4)), "VoxelDataTree regions inside a constant node should be defined.");
	CHECK_MESSAGE(tree.is_region_defined(Rect4i(2, 1, 1, 1, 4, 2, 2, 2)), "VoxelDataTree regions spanning a constant node and a leaf should be defined.");
	CHECK_MESSAGE(!tree.is_region_defined(Rect4i(1, 1, 1, 1, 2, 4, 2, 2)), "VoxelDataTree regions overlapping any undefined node should not be defined.");
}

TEST_CASE("[VoxelDataTree] Generate") {
	// Four chunks per axis, so that along X there are two chunks on each side
	// of the material boundary at 0: one touching it and one away from it.
	const Vector4i corner = VOXEL_DATA_CHUNK_SIZE_VECTOR * -2;
	VoxelDataTree tree = VoxelDataTree(Rect4i(corner, VOXEL_DATA_CHUNK_SIZE_VECTOR * 4));
	Ref<UniformSolidGenerator> solid_generator;
	solid_generator.instantiate();
	tree.generate(solid_generator);
	CHECK_MESSAGE(tree.is_constant(), "VoxelDataTree generate should merge a uniform region into a single constant node, even across multiple levels.");
	CHECK_MESSAGE(tree.get_constant_material() == SOLID_MATERIAL, "VoxelDataTree generate should merge to the generated value.");

	Ref<HalfSpaceGenerator> half_space_generator;
	half_space_generator.instantiate();
	tree.generate(half_space_generator);
	CHECK_MESSAGE(tree.is_parent(), "VoxelDataTree generate should not merge children with different values.");
	CHECK_MESSAGE(tree.find_deepest_node(corner)->is_constant(), "VoxelDataTree generate should make uniform chunks away from a material boundary constant.");
	CHECK_MESSAGE(tree.find_deepest_node(Vector4i(-1, corner.y, corner.z, corner.w))->is_leaf(), "VoxelDataTree generate should keep a chunk that borders a different material as a leaf.");
	CHECK_MESSAGE(tree.find_deepest_node(Vector4i(0, corner.y, corner.z, corner.w))->is_leaf(), "VoxelDataTree generate should keep a solid chunk that borders air as a leaf.");
	CHECK_MESSAGE(tree.get_material(Vector4i(-1, 3, -3, 5)) == VoxelMaterial::AIR, "VoxelDataTree generate should store the generated materials.");
	CHECK_MESSAGE(tree.get_material(Vector4i(0, 3, -3, 5)) == VoxelMaterial::SOLID, "VoxelDataTree generate should store the generated materials.");

	VoxelDataLeaf *border_leaf = tree.find_deepest_node(Vector4i(-1, corner.y, corner.z, corner.w))->get_leaf_data();
	REQUIRE(border_leaf != nullptr);
	const Vector4i crossing_voxel = Vector4i(VOXEL_DATA_CHUNK_SIZE - 1, 0, 0, 0);
	CHECK_MESSAGE(border_leaf->has_edge_data(crossing_voxel, 0), "VoxelDataTree generate should store data on an edge crossing the material boundary.");
	CHECK_MESSAGE(!border_leaf->has_edge_data(crossing_voxel, 1), "VoxelDataTree generate should not store data on edges between voxels of the same material.");
	CHECK_MESSAGE(border_leaf->get_edge_data(crossing_voxel, 0).decode_normal() == Vector4(1, 0, 0, 0), "VoxelDataTree generate should store the generator's normal for an active edge, up to its sign.");
	CHECK_MESSAGE(Math::abs(border_leaf->get_edge_data(crossing_voxel, 0).decode_position() - 0.5f) < 0.5f / 31.0f + 0.0001f, "VoxelDataTree generate should store the generator's crossing position for an active edge.");
	CHECK_MESSAGE(border_leaf->get_edge_data_count() == VOXEL_DATA_CHUNK_SIZE * VOXEL_DATA_CHUNK_SIZE * VOXEL_DATA_CHUNK_SIZE, "VoxelDataTree generate should store one entry per active edge, including edges crossing into the next chunk.");
	VoxelDataLeaf *solid_leaf = tree.find_deepest_node(Vector4i(0, corner.y, corner.z, corner.w))->get_leaf_data();
	REQUIRE(solid_leaf != nullptr);
	CHECK_MESSAGE(solid_leaf->get_edge_data_count() == 0, "VoxelDataTree generate should not store data in a chunk that owns no active edges, since edges belong to the chunk of their lower voxel.");

	Ref<DiagonalHalfSpaceGenerator> diagonal_generator;
	diagonal_generator.instantiate();
	tree.generate(diagonal_generator);
	CHECK_MESSAGE(tree.find_deepest_node(Vector4i(2, -2, 0, 0))->is_leaf(), "VoxelDataTree generate should make a leaf for a chunk with mixed values.");
	CHECK_MESSAGE(tree.get_material(Vector4i(2, -2, 0, 0)) == VoxelMaterial::SOLID, "VoxelDataTree generate should store mixed chunks at full detail.");
	CHECK_MESSAGE(tree.get_material(Vector4i(2, -3, 0, 0)) == VoxelMaterial::AIR, "VoxelDataTree generate should store mixed chunks at full detail.");
}
} // namespace TestVoxelDataTree
