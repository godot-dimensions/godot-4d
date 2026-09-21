#include "voxel_data_tree.h"

#include "../edit/voxel_edit.h"
#include "voxel_data_leaf.h"

bool VoxelDataTree::has_voxel(const Vector4i &p_voxel) const {
	return _bounds.has_point(p_voxel);
}

void VoxelDataTree::clear() {
	switch (_type) {
		case TYPE_UNDEFINED: {
		} break;
		case TYPE_PARENT: {
			for (int i = 0; i < CHILD_COUNT; i++) {
				_children[i].~VoxelDataTree();
			}
			memfree(_children);
		} break;
		case TYPE_LEAF: {
			memdelete(_data);
		} break;
		case TYPE_CONSTANT: {
		} break;
	}
	_type = TYPE_UNDEFINED;
	_children = nullptr;
}

VoxelDataTree *VoxelDataTree::subdivide() {
	ERR_FAIL_COND_V_MSG(_type != TYPE_UNDEFINED, nullptr, "VoxelDataTree can only subdivide an undefined node. Call clear() first to discard its contents.");
	ERR_FAIL_COND_V_MSG(_bounds.size.x < 2, nullptr, "VoxelDataTree cannot subdivide a node the size of a single voxel.");
	const Vector4i half_size = _bounds.size / 2;
	_children = (VoxelDataTree *)memalloc(sizeof(VoxelDataTree) * CHILD_COUNT);
	_type = TYPE_PARENT;
	for (int i = 0; i < CHILD_COUNT; i++) {
		const Vector4i offset = Vector4i(
				(i & 1) ? half_size.x : 0,
				(i & 2) ? half_size.y : 0,
				(i & 4) ? half_size.z : 0,
				(i & 8) ? half_size.w : 0);
		memnew_placement(&_children[i], VoxelDataTree(Rect4i(_bounds.position + offset, half_size)));
	}
	return _children;
}

VoxelDataTree *VoxelDataTree::get_children() {
	ERR_FAIL_COND_V_MSG(_type != TYPE_PARENT, nullptr, "VoxelDataTree node is not subdivided, it has no children.");
	return _children;
}

const VoxelDataTree *VoxelDataTree::get_children() const {
	ERR_FAIL_COND_V_MSG(_type != TYPE_PARENT, nullptr, "VoxelDataTree node is not subdivided, it has no children.");
	return _children;
}

VoxelDataTree *VoxelDataTree::get_child(const int p_index) {
	ERR_FAIL_COND_V_MSG(_type != TYPE_PARENT, nullptr, "VoxelDataTree node is not subdivided, it has no children.");
	ERR_FAIL_INDEX_V(p_index, CHILD_COUNT, nullptr);
	return &_children[p_index];
}

const VoxelDataTree *VoxelDataTree::get_child(const int p_index) const {
	ERR_FAIL_COND_V_MSG(_type != TYPE_PARENT, nullptr, "VoxelDataTree node is not subdivided, it has no children.");
	ERR_FAIL_INDEX_V(p_index, CHILD_COUNT, nullptr);
	return &_children[p_index];
}

int VoxelDataTree::get_child_index_containing(const Vector4i &p_voxel) const {
	const Vector4i center = _bounds.position + _bounds.size / 2;
	return int(p_voxel.x >= center.x) | (int(p_voxel.y >= center.y) << 1) | (int(p_voxel.z >= center.z) << 2) | (int(p_voxel.w >= center.w) << 3);
}

VoxelDataTree *VoxelDataTree::get_child_containing(const Vector4i &p_voxel) {
	if (_type != TYPE_PARENT || !has_voxel(p_voxel)) {
		return nullptr;
	}
	return &_children[get_child_index_containing(p_voxel)];
}

const VoxelDataTree *VoxelDataTree::get_child_containing(const Vector4i &p_voxel) const {
	if (_type != TYPE_PARENT || !has_voxel(p_voxel)) {
		return nullptr;
	}
	return &_children[get_child_index_containing(p_voxel)];
}

void VoxelDataTree::set_leaf_data(VoxelDataLeaf *p_data) {
	clear();
	if (p_data == nullptr) {
		return;
	}
	const Vector4i position = _bounds.position;
	const int32_t size_mask = _bounds.size.x - 1;
	if ((position.x & size_mask) != 0 || (position.y & size_mask) != 0 || (position.z & size_mask) != 0 || (position.w & size_mask) != 0) {
		memdelete(p_data);
		ERR_FAIL_MSG("VoxelDataTree leaf positions must be a multiple of their size on every axis.");
	}
	_data = p_data;
	_type = TYPE_LEAF;
}

VoxelDataLeaf *VoxelDataTree::get_leaf_data() {
	ERR_FAIL_COND_V_MSG(_type != TYPE_LEAF, nullptr, "VoxelDataTree node is not a leaf, it has no leaf data.");
	return _data;
}

const VoxelDataLeaf *VoxelDataTree::get_leaf_data() const {
	ERR_FAIL_COND_V_MSG(_type != TYPE_LEAF, nullptr, "VoxelDataTree node is not a leaf, it has no leaf data.");
	return _data;
}

void VoxelDataTree::set_constant_material(const VoxelMaterial p_material) {
	clear();
	_constant_material = p_material;
	_type = TYPE_CONSTANT;
}

VoxelMaterial VoxelDataTree::get_constant_material() const {
	ERR_FAIL_COND_V_MSG(_type != TYPE_CONSTANT, VoxelMaterial::UNDEFINED, "VoxelDataTree node is not constant, it has no constant value.");
	return _constant_material;
}

VoxelDataTree *VoxelDataTree::find_deepest_node(const Vector4i &p_voxel) {
	if (!has_voxel(p_voxel)) {
		return nullptr;
	}
	VoxelDataTree *node = this;
	while (node->_type == TYPE_PARENT) {
		node = &node->_children[node->get_child_index_containing(p_voxel)];
	}
	return node;
}

const VoxelDataTree *VoxelDataTree::find_deepest_node(const Vector4i &p_voxel) const {
	return const_cast<VoxelDataTree *>(this)->find_deepest_node(p_voxel);
}

bool VoxelDataTree::merge_constant_children() {
	if (_type != TYPE_PARENT) {
		return false;
	}
	for (int i = 0; i < CHILD_COUNT; i++) {
		if (_children[i]._type != TYPE_CONSTANT || _children[i]._constant_material != _children[0]._constant_material) {
			return false;
		}
	}
	set_constant_material(_children[0]._constant_material);
	return true;
}

static bool _borders_different_material(const Ref<VoxelGenerator> &p_generator, const Rect4i &p_bounds, const VoxelMaterial p_material) {
	const Vector4i end = p_bounds.get_end();
	for (int axis = 0; axis < 4; axis++) {
		int other_axes[3];
		int other_axis_count = 0;
		for (int i = 0; i < 4; i++) {
			if (i != axis) {
				other_axes[other_axis_count++] = i;
			}
		}
		for (int side = 0; side < 2; side++) {
			Vector4i outside;
			outside[axis] = side == 0 ? p_bounds.position[axis] - 1 : end[axis];
			for (int32_t c0 = 0; c0 < p_bounds.size[other_axes[0]]; c0++) {
				outside[other_axes[0]] = p_bounds.position[other_axes[0]] + c0;
				for (int32_t c1 = 0; c1 < p_bounds.size[other_axes[1]]; c1++) {
					outside[other_axes[1]] = p_bounds.position[other_axes[1]] + c1;
					for (int32_t c2 = 0; c2 < p_bounds.size[other_axes[2]]; c2++) {
						outside[other_axes[2]] = p_bounds.position[other_axes[2]] + c2;
						if (p_generator->get_material(outside) != p_material) {
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}

void VoxelDataTree::generate(const Ref<VoxelGenerator> &p_generator) {
	ERR_FAIL_COND(p_generator.is_null());
	clear();
	if (_bounds.size.x > VOXEL_DATA_CHUNK_SIZE) {
		subdivide();
		for (int i = 0; i < CHILD_COUNT; i++) {
			_children[i].generate(p_generator);
		}
		merge_constant_children();
		return;
	}
	VoxelDataLeaf *leaf = memnew(VoxelDataLeaf);
	const VoxelMaterial first_material = p_generator->get_material(_bounds.position);
	bool uniform = true;
	for (int32_t w = 0; w < _bounds.size.w; w++) {
		for (int32_t z = 0; z < _bounds.size.z; z++) {
			for (int32_t y = 0; y < _bounds.size.y; y++) {
				for (int32_t x = 0; x < _bounds.size.x; x++) {
					const Vector4i local_voxel = Vector4i(x, y, z, w);
					const VoxelMaterial material = p_generator->get_material(_bounds.position + local_voxel);
					leaf->set_material(local_voxel, material);
					uniform = uniform && material == first_material;
				}
			}
		}
	}
	// A chunk bordering a different material has active edges, whose surface
	// normals can only be stored in a leaf, so it must stay a leaf even when
	// its own voxels are uniform.
	if (uniform && !_borders_different_material(p_generator, _bounds, first_material)) {
		memdelete(leaf);
		set_constant_material(first_material);
		return;
	}
	// Store the surface data of the leaf's active edges. Edges are visited
	// in edge index order, so each insert appends to the end of the array.
	for (int32_t w = 0; w < _bounds.size.w; w++) {
		for (int32_t z = 0; z < _bounds.size.z; z++) {
			for (int32_t y = 0; y < _bounds.size.y; y++) {
				for (int32_t x = 0; x < _bounds.size.x; x++) {
					const Vector4i local_voxel = Vector4i(x, y, z, w);
					const VoxelMaterial material = leaf->get_material(local_voxel);
					for (int axis = 0; axis < 4; axis++) {
						Vector4i neighbor_local = local_voxel;
						neighbor_local[axis] += 1;
						const VoxelMaterial neighbor_material = neighbor_local[axis] < _bounds.size[axis] ? leaf->get_material(neighbor_local) : p_generator->get_material(_bounds.position + neighbor_local);
						if (material != neighbor_material) {
							leaf->set_edge_data(local_voxel, axis, p_generator->get_edge_data(_bounds.position + local_voxel, axis));
						}
					}
				}
			}
		}
	}
	set_leaf_data(leaf);
}

void VoxelDataTree::apply_generated_chunk(VoxelDataTree *p_chunk) {
	ERR_FAIL_NULL(p_chunk);
	const Vector4i position = p_chunk->_bounds.position;
	if (!has_voxel(position)) {
		memdelete(p_chunk);
		ERR_FAIL_MSG("VoxelDataTree cannot store a chunk outside of its bounds.");
	}
	VoxelDataTree *node = this;
	while (node->_bounds != p_chunk->_bounds) {
		if (node->_bounds.size.x <= p_chunk->_bounds.size.x) {
			memdelete(p_chunk);
			ERR_FAIL_MSG("VoxelDataTree chunks must line up with the tree's subdivisions.");
		}
		if (node->_type == TYPE_UNDEFINED) {
			node->subdivide();
		} else if (node->_type != TYPE_PARENT) {
			break;
		}
		node = &node->_children[node->get_child_index_containing(position)];
	}
	if (node->_type != TYPE_UNDEFINED) {
		// That part of the tree is already defined; discard the chunk.
		memdelete(p_chunk);
		return;
	}
	node->_take_contents(*p_chunk);
	memdelete(p_chunk);
}

void VoxelDataTree::_take_contents(VoxelDataTree &p_donor) {
	ERR_FAIL_COND_MSG(_type != TYPE_UNDEFINED, "VoxelDataTree can only take contents into an undefined node.");
	ERR_FAIL_COND_MSG(_bounds != p_donor._bounds, "VoxelDataTree can only take the contents of a node with identical bounds.");
	_type = p_donor._type;
	switch (p_donor._type) {
		case TYPE_UNDEFINED: {
		} break;
		case TYPE_PARENT: {
			_children = p_donor._children;
		} break;
		case TYPE_LEAF: {
			_data = p_donor._data;
		} break;
		case TYPE_CONSTANT: {
			_constant_material = p_donor._constant_material;
		} break;
	}
	p_donor._type = TYPE_UNDEFINED;
	p_donor._children = nullptr;
}

bool VoxelDataTree::clear_chunk(const Vector4i &p_voxel) {
	if (_type == TYPE_UNDEFINED || !has_voxel(p_voxel)) {
		return false;
	}
	if (_bounds.size.x == VOXEL_DATA_CHUNK_SIZE) {
		clear();
		return true;
	}
	if (_type == TYPE_CONSTANT) {
		const VoxelMaterial constant_material = _constant_material;
		clear();
		VoxelDataTree *constant_children = subdivide();
		for (int i = 0; i < CHILD_COUNT; i++) {
			constant_children[i].set_constant_material(constant_material);
		}
	}
	ERR_FAIL_COND_V_MSG(_type != TYPE_PARENT, false, "VoxelDataTree nodes larger than a chunk should be parents, constants, or undefined.");
	const bool unloaded = get_child_containing(p_voxel)->clear_chunk(p_voxel);
	for (int i = 0; i < CHILD_COUNT; i++) {
		if (!_children[i].is_undefined()) {
			return unloaded;
		}
	}
	clear();
	return unloaded;
}

bool VoxelDataTree::is_region_defined(const Rect4i &p_region) const {
	if (!_bounds.intersects_exclusive(p_region)) {
		return true;
	}
	switch (_type) {
		case TYPE_UNDEFINED: {
			return false;
		} break;
		case TYPE_PARENT: {
			for (int i = 0; i < CHILD_COUNT; i++) {
				if (!_children[i].is_region_defined(p_region)) {
					return false;
				}
			}
			return true;
		} break;
		case TYPE_LEAF:
		case TYPE_CONSTANT: {
			return true;
		} break;
	}
	return false;
}

VoxelMaterial VoxelDataTree::get_material(const Vector4i &p_voxel) const {
	const VoxelDataTree *node = find_deepest_node(p_voxel);
	if (node != nullptr) {
		switch (node->_type) {
			case TYPE_UNDEFINED:
			case TYPE_PARENT: {
			} break;
			case TYPE_LEAF: {
				return node->_data->get_material(p_voxel - node->_bounds.position);
			} break;
			case TYPE_CONSTANT: {
				return node->_constant_material;
			} break;
		}
	}
	return VoxelMaterial::UNDEFINED;
}

VoxelEdgeData VoxelDataTree::get_edge_data(const Vector4i &p_voxel, const int p_axis) const {
	const VoxelDataTree *node = find_deepest_node(p_voxel);
	if (node == nullptr || node->_type != TYPE_LEAF) {
		return VoxelEdgeData();
	}
	const Vector4i local_voxel = p_voxel - node->_bounds.position;
	if (!node->_data->has_edge_data(local_voxel, p_axis)) {
		return VoxelEdgeData();
	}
	return node->_data->get_edge_data(local_voxel, p_axis);
}

VoxelDataTree::VoxelDataTree(const Rect4i &p_bounds) :
		_bounds(p_bounds) {
	const Vector4i size = p_bounds.size;
	ERR_FAIL_COND_MSG(size.x < 1 || (size.x & (size.x - 1)) != 0 || size.y != size.x || size.z != size.x || size.w != size.x, "VoxelDataTree bounds size must be the same power of two on every axis.");
	const Vector4i position = p_bounds.position;
	const int32_t half_size_mask = (size.x >> 1) - 1;
	ERR_FAIL_COND_MSG(size.x > 1 && ((position.x & half_size_mask) != 0 || (position.y & half_size_mask) != 0 || (position.z & half_size_mask) != 0 || (position.w & half_size_mask) != 0), "VoxelDataTree bounds position must be a multiple of half the size on every axis.");
}

VoxelDataNeighbourhood VoxelDataNeighbourhood::get_child(const int p_index) const {
	VoxelDataNeighbourhood child;
	ERR_FAIL_NULL_V(node, child);
	child.node = node->get_child(p_index);
	if (child.node == nullptr) {
		return child;
	}
	for (int direction = 0; direction < DIRECTION_COUNT; direction++) {
		if (direction == CENTRE_DIRECTION) {
			continue;
		}
		int parent_direction = 0;
		int target_child_index = 0;
		for (int axis = 0, power = 1; axis < 4; axis++, power *= 3) {
			// The neighbour's position along this axis in units of the child's
			// size, relative to the original centre node's lower corner, so
			// positions 0 and 1 are inside the centre node.
			const int position = ((p_index >> axis) & 1) + (direction / power) % 3 - 1;
			if (position < 0) {
				target_child_index |= 1 << axis;
			} else if (position < 2) {
				parent_direction += power;
				target_child_index |= position << axis;
			} else {
				parent_direction += 2 * power;
			}
		}
		VoxelDataTree *target = parent_direction == CENTRE_DIRECTION ? node : neighbours[parent_direction];
		if (target != nullptr && target->is_parent()) {
			target = target->get_child(target_child_index);
		}
		child.neighbours[direction] = target;
	}
	return child;
}

VoxelMaterial VoxelDataNeighbourhood::get_material(const Vector4i &p_voxel) const {
	ERR_FAIL_NULL_V(node, VoxelMaterial::UNDEFINED);
	if (node->has_voxel(p_voxel)) {
		return node->get_material(p_voxel);
	}
	const Rect4i bounds = node->get_bounds();
	const Vector4i end = bounds.get_end();
	int direction = 0;
	for (int axis = 0, power = 1; axis < 4; axis++, power *= 3) {
		if (p_voxel[axis] >= end[axis]) {
			direction += 2 * power;
		} else if (p_voxel[axis] >= bounds.position[axis]) {
			direction += power;
		}
	}
	VoxelDataTree *neighbour = neighbours[direction];
	return neighbour == nullptr ? VoxelMaterial::UNDEFINED : neighbour->get_material(p_voxel);
}

void VoxelDataNeighbourhood::apply_edit(const Ref<VoxelEdit> &p_edit) {
	ERR_FAIL_NULL(node);
	const Rect4i edit_bounds = p_edit->get_bounds();
	// Edges owned by the voxels one step below the edit bounds still reach
	// into them, so those voxels' chunks are affected too.
	const Rect4i edge_bounds = Rect4i(edit_bounds.position - Vector4i(1, 1, 1, 1), edit_bounds.size + Vector4i(1, 1, 1, 1));
	// Constant chunks are not permitted to be adjacent to voxels of a different
	// material to them.
	const Rect4i face_bounds = edit_bounds.grow(1);
	const Rect4i bounds = node->get_bounds();
	if (node->is_undefined() || !bounds.intersects_exclusive(face_bounds)) {
		return;
	}
	if (node->is_constant()) {
		const VoxelMaterial constant_material = node->get_constant_material();
		if (bounds.size.x > VOXEL_DATA_CHUNK_SIZE) {
			// Split, so that only the parts overlapping the edit lose their
			// constant representation.
			node->clear();
			VoxelDataTree *constant_children = node->subdivide();
			for (int i = 0; i < VoxelDataTree::CHILD_COUNT; i++) {
				constant_children[i].set_constant_material(constant_material);
			}
		} else {
			// The edit may make the chunk non-uniform or give it normals.
			VoxelDataLeaf *leaf = memnew(VoxelDataLeaf);
			for (int32_t w = 0; w < bounds.size.w; w++) {
				for (int32_t z = 0; z < bounds.size.z; z++) {
					for (int32_t y = 0; y < bounds.size.y; y++) {
						for (int32_t x = 0; x < bounds.size.x; x++) {
							leaf->set_material(Vector4i(x, y, z, w), constant_material);
						}
					}
				}
			}
			node->set_leaf_data(leaf);
		}
	}
	if (node->is_parent()) {
		for (int i = 0; i < VoxelDataTree::CHILD_COUNT; i++) {
			// Checked here to skip building neighbourhoods of irrelevant children.
			if (node->get_child(i)->get_bounds().intersects_exclusive(face_bounds)) {
				get_child(i).apply_edit(p_edit);
			}
		}
		return;
	}
	VoxelDataLeaf *leaf_data = node->get_leaf_data();
	// The surface data of the edges the edit touches, those where the edit is
	// not UNDEFINED on both sides, is updated before the materials, because
	// the rules below need the materials from before the edit. That holds for
	// upper voxels beyond this chunk too: children are visited in index
	// order, so the chunk containing an edge's upper voxel is always visited
	// after the chunk owning the edge.
	const Rect4i edge_affected = bounds.intersection(edge_bounds);
	const Vector4i edge_end = edge_affected.get_end();
	for (int32_t w = edge_affected.position.w; w < edge_end.w; w++) {
		for (int32_t z = edge_affected.position.z; z < edge_end.z; z++) {
			for (int32_t y = edge_affected.position.y; y < edge_end.y; y++) {
				for (int32_t x = edge_affected.position.x; x < edge_end.x; x++) {
					const Vector4i voxel = Vector4i(x, y, z, w);
					const Vector4i local_voxel = voxel - bounds.position;
					for (int axis = 0; axis < 4; axis++) {
						Vector4i upper_voxel = voxel;
						upper_voxel[axis]++;
						const VoxelMaterial edit_lower = edit_bounds.has_point(voxel) ? p_edit->get_material(voxel) : VoxelMaterial::UNDEFINED;
						const VoxelMaterial edit_upper = edit_bounds.has_point(upper_voxel) ? p_edit->get_material(upper_voxel) : VoxelMaterial::UNDEFINED;
						const bool lower_defined = edit_lower != VoxelMaterial::UNDEFINED;
						const bool upper_defined = edit_upper != VoxelMaterial::UNDEFINED;
						if (!lower_defined && !upper_defined) {
							continue;
						}
						const VoxelMaterial old_lower = leaf_data->get_material(local_voxel);
						const VoxelMaterial old_upper = get_material(upper_voxel);
						const VoxelMaterial new_lower = overlay_material(edit_lower, old_lower);
						const VoxelMaterial new_upper = overlay_material(edit_upper, old_upper);
						if (new_lower == VoxelMaterial::UNDEFINED || new_upper == VoxelMaterial::UNDEFINED) {
							// The edge leads into an undefined chunk.
							continue;
						}
						if (new_lower == new_upper) {
							leaf_data->clear_edge_data(local_voxel, axis);
							continue;
						}
						const VoxelEdgeData edit_edge_data = p_edit->get_edge_data(voxel, axis);
						if (lower_defined != upper_defined && leaf_data->has_edge_data(local_voxel, axis)) {
							const bool same_material = lower_defined ? edit_lower == old_lower : edit_upper == old_upper;
							if (same_material) {
								// Writing a material over itself extends its
								// region, so the combined region's surface is
								// whichever crossing lies farther from the
								// edit-defined end.
								const real_t old_position = leaf_data->get_edge_data(local_voxel, axis).decode_position();
								const real_t edit_position = edit_edge_data.decode_position();
								const bool old_is_farther = lower_defined ? old_position > edit_position : old_position < edit_position;
								if (old_is_farther) {
									continue;
								}
							}
						}
						leaf_data->set_edge_data(local_voxel, axis, edit_edge_data);
					}
				}
			}
		}
	}
	const Rect4i affected = bounds.intersection(edit_bounds);
	const Vector4i end = affected.get_end();
	for (int32_t w = affected.position.w; w < end.w; w++) {
		for (int32_t z = affected.position.z; z < end.z; z++) {
			for (int32_t y = affected.position.y; y < end.y; y++) {
				for (int32_t x = affected.position.x; x < end.x; x++) {
					const Vector4i voxel = Vector4i(x, y, z, w);
					const Vector4i local_voxel = voxel - bounds.position;
					leaf_data->set_material(local_voxel, overlay_material(p_edit->get_material(voxel), leaf_data->get_material(local_voxel)));
				}
			}
		}
	}
}
