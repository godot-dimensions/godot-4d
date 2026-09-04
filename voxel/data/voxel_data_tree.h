#pragma once

#include "../../math/rect4i.h"
#include "../generators/voxel_generator.h"
#include "voxel_value.h"

class VoxelDataLeaf;

// A sparse tree over 4D voxel space, the 4D analog of a quadtree or octree,
// so each subdivided node has 16 children ("16-tree"). Each node covers an
// axis-aligned box of integer voxel coordinates and is in one of four states:
// undefined (covers space with no voxel data, needs no storage), a parent
// with 16 children each covering one orthant of its bounds, a leaf holding
// a chunk of actual voxel data in a VoxelDataLeaf, or constant, holding a
// single value shared by every voxel in its bounds.
// Always a power-of-2 sized hypercube.
// Nodes own their children and leaf data, and free them when cleared or destroyed.
class VoxelDataTree {
public:
	enum Type {
		TYPE_UNDEFINED,
		TYPE_PARENT,
		TYPE_LEAF,
		TYPE_CONSTANT,
	};

	static constexpr int CHILD_COUNT = 16;

private:
	// The region of voxel space this node covers.
	const Rect4i _bounds;
	Type _type = TYPE_UNDEFINED;
	union {
		// Array of CHILD_COUNT children allocated with memalloc. Only valid when _type == TYPE_PARENT.
		VoxelDataTree *_children = nullptr;
		// Chunk of voxel data. Only valid when _type == TYPE_LEAF.
		VoxelDataLeaf *_data;
		// The value of every voxel in the bounds. Only valid when _type == TYPE_CONSTANT.
		VoxelValue _constant_value;
	};

public:
	Type get_type() const { return _type; }
	bool is_undefined() const { return _type == TYPE_UNDEFINED; }
	bool is_parent() const { return _type == TYPE_PARENT; }
	bool is_leaf() const { return _type == TYPE_LEAF; }
	bool is_constant() const { return _type == TYPE_CONSTANT; }

	const Rect4i &get_bounds() const { return _bounds; }
	bool has_voxel(const Vector4i &p_voxel) const;

	// Frees any children or leaf data and makes the node undefined again.
	void clear();

	// Turns an undefined node into a parent with 16 undefined children, one
	// per orthant of this node's bounds, and returns the child array.
	// The node must be larger than a single voxel.
	VoxelDataTree *subdivide();
	VoxelDataTree *get_children();
	const VoxelDataTree *get_children() const;
	VoxelDataTree *get_child(const int p_index);
	const VoxelDataTree *get_child(const int p_index) const;

	// The index of the child whose bounds contain the given voxel, assuming
	// this node were subdivided. Bit 0 is set for the upper half in X, bit 1
	// for Y, bit 2 for Z, and bit 3 for W. Only meaningful for voxels inside
	// this node's bounds (see has_voxel).
	int get_child_index_containing(const Vector4i &p_voxel) const;
	VoxelDataTree *get_child_containing(const Vector4i &p_voxel);
	const VoxelDataTree *get_child_containing(const Vector4i &p_voxel) const;

	// Turns the node into a leaf, taking ownership of the given data and
	// freeing any previous contents. Passing nullptr makes the node undefined.
	void set_leaf_data(VoxelDataLeaf *p_data);
	VoxelDataLeaf *get_leaf_data();
	const VoxelDataLeaf *get_leaf_data() const;

	// Turns the node into a constant, one value shared by every voxel in its
	// bounds, freeing any previous contents.
	void set_constant_value(const VoxelValue p_value);
	VoxelValue get_constant_value() const;

	// If this node is a parent whose children are all constants with the same
	// value, replaces them with a single constant node and returns true.
	bool merge_constant_children();

	// Discards any existing contents and fills the node at full detail with
	// the values the given generator returns for each voxel, and with the
	// surface normals of the active edges. Uniform regions are stored as
	// constant nodes, merged into larger ones where possible, except chunks
	// that border a different material, which stay leaves so that they can
	// store the surface data
	void generate(const Ref<VoxelGenerator> &p_generator);

	// Descends the tree to the deepest existing node whose bounds contain the
	// given voxel, which is never a parent. Returns nullptr if the voxel is
	// outside of this node's bounds.
	VoxelDataTree *find_deepest_node(const Vector4i &p_voxel);
	const VoxelDataTree *find_deepest_node(const Vector4i &p_voxel) const;

	// The value of the voxel at the given coordinates, or a value with the
	// UNDEFINED material if the voxel is undefined or outside of this node's
	// bounds.
	VoxelValue get_value(const Vector4i &p_voxel) const;

	// The stored surface normal of the edge from the given voxel to its
	// neighbor one step along the given axis, or Vector4() if no normal is
	// stored for that edge.
	Vector4 get_edge_normal(const Vector4i &p_voxel, const int p_axis) const;

	// Nodes own their children and leaf data, so copying is not allowed.
	VoxelDataTree(const VoxelDataTree &) = delete;
	VoxelDataTree &operator=(const VoxelDataTree &) = delete;

	explicit VoxelDataTree(const Rect4i &p_bounds);
	~VoxelDataTree() { clear(); }
};
