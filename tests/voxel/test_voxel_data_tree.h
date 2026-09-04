#pragma once

#include "../../voxel/data/voxel_data_leaf.h"
#include "../../voxel/data/voxel_data_tree.h"

#include "tests/test_macros.h"

namespace TestVoxelDataTree {

constexpr VoxelValue SOLID_VALUE = { VoxelMaterial::SOLID, 255 };
constexpr VoxelValue AIR_VALUE = { VoxelMaterial::AIR, 255 };

class UniformSolidGenerator : public VoxelGenerator {
public:
	virtual VoxelValue get_value(const Vector4i &) const override { return SOLID_VALUE; }
	virtual Vector4 get_normal(const Vector4i &, const int, const VoxelValue &, const VoxelValue &) const override { return Vector4(); }
};

// Solid where X is non-negative, with the surface normal pointing towards -X.
class HalfSpaceGenerator : public VoxelGenerator {
public:
	virtual VoxelValue get_value(const Vector4i &p_voxel) const override { return p_voxel.x >= 0 ? SOLID_VALUE : AIR_VALUE; }
	virtual Vector4 get_normal(const Vector4i &, const int, const VoxelValue &, const VoxelValue &) const override { return Vector4(-1, 0, 0, 0); }
};

// Solid where X + Y is non-negative.
class DiagonalHalfSpaceGenerator : public VoxelGenerator {
public:
	virtual VoxelValue get_value(const Vector4i &p_voxel) const override { return p_voxel.x + p_voxel.y >= 0 ? SOLID_VALUE : AIR_VALUE; }
	virtual Vector4 get_normal(const Vector4i &, const int, const VoxelValue &, const VoxelValue &) const override { return Vector4(-1, -1, 0, 0).normalized(); }
};

TEST_CASE("[VoxelDataTree] Node types and clearing") {
	VoxelDataTree tree = VoxelDataTree(Rect4i(-8, -8, -8, -8, 16, 16, 16, 16));
	CHECK_MESSAGE(tree.is_undefined(), "VoxelDataTree nodes should start out undefined.");

	tree.set_leaf_data(memnew(VoxelDataLeaf));
	CHECK_MESSAGE(tree.is_leaf(), "VoxelDataTree set_leaf_data with data should make the node a leaf.");
	CHECK_MESSAGE(tree.get_leaf_data() != nullptr, "VoxelDataTree get_leaf_data should return the data of a leaf node.");

	tree.clear();
	CHECK_MESSAGE(tree.is_undefined(), "VoxelDataTree clear should make the node undefined again.");

	tree.set_constant_value(SOLID_VALUE);
	CHECK_MESSAGE(tree.is_constant(), "VoxelDataTree set_constant_value should make the node constant.");
	CHECK_MESSAGE(tree.get_constant_value() == SOLID_VALUE, "VoxelDataTree get_constant_value should return the value that was set.");
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
	CHECK_MESSAGE(tree.get_value(Vector4i(1, 2, 3, 4)) == VoxelValue(), "VoxelDataTree get_value of an undefined node should return the default value.");
	CHECK_MESSAGE(tree.get_value(Vector4i(0, 0, 0, 8)) == VoxelValue(), "VoxelDataTree get_value outside the bounds should return the default value.");

	VoxelDataTree *children = tree.subdivide();
	children[0].set_constant_value(SOLID_VALUE);
	CHECK_MESSAGE(tree.get_value(Vector4i(3, 3, 3, 3)) == SOLID_VALUE, "VoxelDataTree get_value should return the constant value of a constant descendant.");
	CHECK_MESSAGE(tree.get_value(Vector4i(4, 3, 3, 3)) == VoxelValue(), "VoxelDataTree get_value in an undefined sibling should return the default value.");

	VoxelDataLeaf *leaf = memnew(VoxelDataLeaf);
	leaf->set_value(Vector4i(1, 0, 0, 0), SOLID_VALUE);
	children[15].set_leaf_data(leaf);
	CHECK_MESSAGE(tree.get_value(Vector4i(5, 4, 4, 4)) == SOLID_VALUE, "VoxelDataTree get_value should read a chunk-sized leaf at full detail, relative to the leaf node's position.");
	CHECK_MESSAGE(tree.get_value(Vector4i(6, 4, 4, 4)) == VoxelValue(), "VoxelDataTree get_value should not smear leaf values across neighboring voxels at full detail.");
}

TEST_CASE("[VoxelDataTree] Generate") {
	VoxelDataTree tree = VoxelDataTree(Rect4i(-8, -8, -8, -8, 16, 16, 16, 16));
	Ref<UniformSolidGenerator> solid_generator;
	solid_generator.instantiate();
	tree.generate(solid_generator);
	CHECK_MESSAGE(tree.is_constant(), "VoxelDataTree generate should merge a uniform region into a single constant node, even across multiple levels.");
	CHECK_MESSAGE(tree.get_constant_value() == SOLID_VALUE, "VoxelDataTree generate should merge to the generated value.");

	Ref<HalfSpaceGenerator> half_space_generator;
	half_space_generator.instantiate();
	tree.generate(half_space_generator);
	CHECK_MESSAGE(tree.is_parent(), "VoxelDataTree generate should not merge children with different values.");
	CHECK_MESSAGE(tree.find_deepest_node(Vector4i(-8, -8, -8, -8))->is_constant(), "VoxelDataTree generate should make uniform chunks away from a material boundary constant.");
	CHECK_MESSAGE(tree.find_deepest_node(Vector4i(-1, -8, -8, -8))->is_leaf(), "VoxelDataTree generate should keep a uniform chunk as a leaf when it borders a different material.");
	CHECK_MESSAGE(tree.find_deepest_node(Vector4i(0, -8, -8, -8))->is_leaf(), "VoxelDataTree generate should keep a uniform solid chunk as a leaf when it borders air.");
	CHECK_MESSAGE(!tree.get_value(Vector4i(-1, 3, -3, 5)).is_opaque(), "VoxelDataTree generate should store the generated values.");
	CHECK_MESSAGE(tree.get_value(Vector4i(0, 3, -3, 5)).is_opaque(), "VoxelDataTree generate should store the generated values.");

	VoxelDataLeaf *border_leaf = tree.find_deepest_node(Vector4i(-1, -8, -8, -8))->get_leaf_data();
	REQUIRE(border_leaf != nullptr);
	CHECK_MESSAGE(border_leaf->has_edge_normal(Vector4i(3, 0, 0, 0), 0), "VoxelDataTree generate should store a normal on an edge crossing the material boundary.");
	CHECK_MESSAGE(!border_leaf->has_edge_normal(Vector4i(3, 0, 0, 0), 1), "VoxelDataTree generate should not store normals on edges between voxels of the same material.");
	CHECK_MESSAGE(border_leaf->get_edge_normal(Vector4i(3, 0, 0, 0), 0) == Vector4(-1, 0, 0, 0), "VoxelDataTree generate should store the generator's normal for an active edge.");
	CHECK_MESSAGE(border_leaf->get_edge_normal_count() == VOXEL_DATA_CHUNK_SIZE * VOXEL_DATA_CHUNK_SIZE * VOXEL_DATA_CHUNK_SIZE, "VoxelDataTree generate should store one normal per active edge, including edges crossing into the next chunk.");
	VoxelDataLeaf *solid_leaf = tree.find_deepest_node(Vector4i(0, -8, -8, -8))->get_leaf_data();
	REQUIRE(solid_leaf != nullptr);
	CHECK_MESSAGE(solid_leaf->get_edge_normal_count() == 0, "VoxelDataTree generate should not store normals in a chunk that owns no active edges, since edges belong to the chunk of their lower voxel.");

	Ref<DiagonalHalfSpaceGenerator> diagonal_generator;
	diagonal_generator.instantiate();
	tree.generate(diagonal_generator);
	CHECK_MESSAGE(tree.find_deepest_node(Vector4i(2, -2, 0, 0))->is_leaf(), "VoxelDataTree generate should make a leaf for a chunk with mixed values.");
	CHECK_MESSAGE(tree.get_value(Vector4i(2, -2, 0, 0)).is_opaque(), "VoxelDataTree generate should store mixed chunks at full detail.");
	CHECK_MESSAGE(!tree.get_value(Vector4i(2, -3, 0, 0)).is_opaque(), "VoxelDataTree generate should store mixed chunks at full detail.");
}
} // namespace TestVoxelDataTree
