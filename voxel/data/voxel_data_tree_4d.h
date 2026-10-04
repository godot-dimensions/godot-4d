#pragma once

#include "../../math/rect4i.h"
#include "../generators/voxel_generator_4d.h"
#include "voxel_edge_data_4d.h"
#include "voxel_material_4d.h"

class VoxelDataLeaf4D;
class VoxelEdit4D;

// A sparse tree over 4D voxel space, the 4D analog of a quadtree or octree,
// so each subdivided node has 16 children ("16-tree"). Each node covers an
// axis-aligned box of integer voxel coordinates and is in one of four states:
// undefined (covers space with no voxel data, needs no storage), a parent
// with 16 children each covering one orthant of its bounds, a leaf holding
// a chunk of actual voxel data in a VoxelDataLeaf4D, or constant, holding a
// single material shared by every voxel in its bounds.
// Always a power-of-2 sized hypercube, positioned at a multiple of half its
// size on every axis; leaf positions are a multiple of their full size.
// Nodes own their children and leaf data, and free them when cleared or destroyed.
class VoxelDataTree4D {
	// VoxelData4D contracts and replaces the root in ways that only make sense
	// for a whole tree, working with the node internals directly.
	friend class VoxelData4D;
	// Edits set, and the update pass consumes, _parent_needs_update.
	friend struct VoxelDataNeighborhood4D;

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
	// Whether derived state in this subtree needs recomputing because it, or
	// the 1-voxel border around it, has changed since the last update pass
	// over it (see VoxelDataNeighborhood4D::merge_edited_constants).
	bool _parent_needs_update = false;
	// Whether every voxel in this node's bounds is either defined or pending a
	// load: always true for leaves and constants, true for a parent iff it is
	// true for all of its children, and set on an undefined node to mark its
	// region as pending. Mutations keep parents' marks either exact or
	// conservatively false; the update pass restores exactness.
	bool _defined_or_pending = false;
	union {
		// Array of CHILD_COUNT children allocated with memalloc. Only valid when _type == TYPE_PARENT.
		VoxelDataTree4D *_children = nullptr;
		// Chunk of voxel data. Only valid when _type == TYPE_LEAF.
		VoxelDataLeaf4D *_data;
		// The material of every voxel in the bounds. Only valid when _type == TYPE_CONSTANT.
		VoxelMaterial4D _constant_material;
	};

	// Moves the donor's contents into this node, which must be undefined and
	// have identical bounds. The donor is left undefined.
	void _take_contents(VoxelDataTree4D &p_donor);

public:
	Type get_type() const { return _type; }
	bool is_undefined() const { return _type == TYPE_UNDEFINED; }
	bool is_parent() const { return _type == TYPE_PARENT; }
	bool is_leaf() const { return _type == TYPE_LEAF; }
	bool is_constant() const { return _type == TYPE_CONSTANT; }
	bool is_defined_or_pending() const { return _defined_or_pending; }

	const Rect4i &get_bounds() const { return _bounds; }
	bool has_voxel(const Vector4i &p_voxel) const;

	// Frees any children or leaf data and makes the node undefined again.
	void clear();

	// Turns an undefined node into a parent with 16 undefined children, one
	// per orthant of this node's bounds, and returns the child array.
	// The node must be larger than a single voxel.
	VoxelDataTree4D *subdivide();
	VoxelDataTree4D *get_children();
	const VoxelDataTree4D *get_children() const;
	VoxelDataTree4D *get_child(const int p_index);
	const VoxelDataTree4D *get_child(const int p_index) const;

	// The index of the child whose bounds contain the given voxel, assuming
	// this node were subdivided. Bit 0 is set for the upper half in X, bit 1
	// for Y, bit 2 for Z, and bit 3 for W. Only meaningful for voxels inside
	// this node's bounds (see has_voxel).
	int get_child_index_containing(const Vector4i &p_voxel) const;
	VoxelDataTree4D *get_child_containing(const Vector4i &p_voxel);
	const VoxelDataTree4D *get_child_containing(const Vector4i &p_voxel) const;

	// Turns the node into a leaf, taking ownership of the given data and
	// freeing any previous contents. Passing nullptr makes the node undefined.
	void set_leaf_data(VoxelDataLeaf4D *p_data);
	VoxelDataLeaf4D *get_leaf_data();
	const VoxelDataLeaf4D *get_leaf_data() const;

	// Turns the node into a constant, one material shared by every voxel in
	// its bounds, freeing any previous contents.
	void set_constant_material(const VoxelMaterial4D p_material);
	VoxelMaterial4D get_constant_material() const;

	// If this node is a parent whose children are all constants with the same
	// material, replaces them with a single constant node and returns true.
	bool merge_constant_children();

	// Subdivides a constant node into constant children, or turns a
	// chunk-sized one into a leaf, so that parts of it can diverge or store
	// edge data.
	void split_constant();

	// Marks every leaf and parent whose bounds, grown by a 1-voxel border,
	// intersect the given edited region, so that the next update pass rechecks
	// them.
	void mark_edited(const Rect4i &p_edited_region);

	// Records that a region is being loaded: marks every undefined chunk
	// overlapping it as pending, at chunk granularity. Defined parts are
	// unaffected.
	void mark_region_pending(const Rect4i &p_region);

	// Discards any existing contents and fills the node at full detail with
	// the data the given generator returns for each voxel. Uniform regions
	// are stored as constant nodes, merged into larger ones where possible,
	// except chunks that border a different material, which stay leaves so
	// that they can store the surface data.
	void generate(const Ref<VoxelGenerator4D> &p_generator);

	// Makes the chunk containing the given voxel undefined again and cancels
	// its pending mark, splitting constants and pending marks that cover more
	// than the chunk, and collapsing parents whose children become all
	// undefined with matching marks. Returns whether any data was unloaded;
	// canceling a pending mark alone does not count.
	bool clear_chunk(const Vector4i &p_voxel);

	// Descends the tree to the deepest existing node whose bounds contain the
	// given voxel, which is never a parent. Returns nullptr if the voxel is
	// outside of this node's bounds.
	VoxelDataTree4D *find_deepest_node(const Vector4i &p_voxel);
	const VoxelDataTree4D *find_deepest_node(const Vector4i &p_voxel) const;

	// Whether no voxel of the given region inside this node's bounds is
	// undefined. Parts of the region outside the bounds are not considered;
	// the caller is responsible for them.
	bool is_region_defined(const Rect4i &p_region) const;

	// The material of the voxel at the given coordinates, or the UNDEFINED
	// material if the voxel is undefined or outside of this node's bounds.
	VoxelMaterial4D get_material(const Vector4i &p_voxel) const;

	// The stored surface data of the edge from the given voxel to its
	// neighbor one step along the given axis, still encoded, or arbitrary data
	// if none is stored for that edge.
	VoxelEdgeData4D get_edge_data(const Vector4i &p_voxel, const int p_axis) const;

	// Nodes own their children and leaf data, so copying is not allowed.
	VoxelDataTree4D(const VoxelDataTree4D &) = delete;
	VoxelDataTree4D &operator=(const VoxelDataTree4D &) = delete;

	explicit VoxelDataTree4D(const Rect4i &p_bounds);
	~VoxelDataTree4D() { clear(); }
};

// A temporary view of a tree node together with the nodes bordering it, for
// operations that read across node boundaries. While it exists, it borrows
// ownership of the nodes, so they may not be modified except through it.
// Generally, modifying a neighborhood means modifying its center node, but
// sometimes things like the surface data for the neighbors may also change.
struct VoxelDataNeighborhood4D {
	// Neighbor directions are indexed in base 3: the node one step in
	// direction d, where each component of d is -1, 0, or +1, is at index
	// (d.x + 1) + (d.y + 1) * 3 + (d.z + 1) * 9 + (d.w + 1) * 27.
	static constexpr int DIRECTION_COUNT = 81;
	// The zero direction; its neighbors entry is unused.
	static constexpr int CENTER_DIRECTION = 40;

	VoxelDataTree4D *node = nullptr;
	// The nodes bordering the center node, including diagonally.
	// Null where the neighboring region is outside the entire tree.
	// Each neighbor either is the same size as the center node, or is larger
	// and doesn't have children.
	VoxelDataTree4D *neighbors[DIRECTION_COUNT] = {};

	// The neighborhood of the given child of the center node.
	VoxelDataNeighborhood4D get_child(const int p_index) const;

	// The center node or the neighbor whose region contains the given voxel,
	// or null where the neighborhood does not cover it. Only meaningful for
	// voxels inside the center node or one neighbor step outside it.
	VoxelDataTree4D *get_node_containing(const Vector4i &p_voxel) const;

	// The material of the given voxel, read from the node containing it.
	VoxelMaterial4D get_material(const Vector4i &p_voxel) const;

	// The stored surface data of the edge from the given voxel to its neighbor
	// one step along the given axis, read from the node containing the voxel.
	VoxelEdgeData4D get_edge_data(const Vector4i &p_voxel, const int p_axis) const;

	// Makes the stored surface data of the edges crossing the center node's
	// borders consistent with the materials on their two ends, and splits any
	// constant that turns out to border a different material.
	void reconcile_borders();

	// Updates the node by overlaying the edit's data where it's defined.
	void apply_edit(const Ref<VoxelEdit4D> &p_edit);

	// Simplifies nodes that edits have made representable as constants: leaves
	// whose voxels are uniform, store no edge data, and border no defined
	// voxel of a different material, and parents whose children all become
	// constants of one material. Also refreshes the parents' defined-or-pending
	// marks. Visits only subtrees marked with _parent_needs_update, clearing
	// the marks.
	void merge_edited_constants();
};
