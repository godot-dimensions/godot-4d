#include "voxel_data_tree_4d.h"

#include "../edit/voxel_edit_4d.h"
#include "voxel_data_leaf_4d.h"

bool VoxelDataTree4D::has_voxel(const Vector4i &p_voxel) const {
	return _bounds.has_point(p_voxel);
}

void VoxelDataTree4D::clear() {
	switch (_type) {
		case TYPE_UNDEFINED: {
		} break;
		case TYPE_PARENT: {
			for (int i = 0; i < CHILD_COUNT; i++) {
				_children[i].~VoxelDataTree4D();
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
	_defined_or_pending = false;
}

VoxelDataTree4D *VoxelDataTree4D::subdivide() {
	ERR_FAIL_COND_V_MSG(_type != TYPE_UNDEFINED, nullptr, "VoxelDataTree4D can only subdivide an undefined node. Call clear() first to discard its contents.");
	ERR_FAIL_COND_V_MSG(_bounds.size.x < 2, nullptr, "VoxelDataTree4D cannot subdivide a node the size of a single voxel.");
	const Vector4i half_size = _bounds.size / 2;
	_children = (VoxelDataTree4D *)memalloc(sizeof(VoxelDataTree4D) * CHILD_COUNT);
	_type = TYPE_PARENT;
	for (int i = 0; i < CHILD_COUNT; i++) {
		const Vector4i offset = Vector4i(
				(i & 1) ? half_size.x : 0,
				(i & 2) ? half_size.y : 0,
				(i & 4) ? half_size.z : 0,
				(i & 8) ? half_size.w : 0);
		memnew_placement(&_children[i], VoxelDataTree4D(Rect4i(_bounds.position + offset, half_size)));
		// A pending node's request covers its whole region.
		_children[i]._defined_or_pending = _defined_or_pending;
	}
	return _children;
}

VoxelDataTree4D *VoxelDataTree4D::get_children() {
	ERR_FAIL_COND_V_MSG(_type != TYPE_PARENT, nullptr, "VoxelDataTree4D node is not subdivided, it has no children.");
	return _children;
}

const VoxelDataTree4D *VoxelDataTree4D::get_children() const {
	ERR_FAIL_COND_V_MSG(_type != TYPE_PARENT, nullptr, "VoxelDataTree4D node is not subdivided, it has no children.");
	return _children;
}

VoxelDataTree4D *VoxelDataTree4D::get_child(const int p_index) {
	ERR_FAIL_COND_V_MSG(_type != TYPE_PARENT, nullptr, "VoxelDataTree4D node is not subdivided, it has no children.");
	ERR_FAIL_INDEX_V(p_index, CHILD_COUNT, nullptr);
	return &_children[p_index];
}

const VoxelDataTree4D *VoxelDataTree4D::get_child(const int p_index) const {
	ERR_FAIL_COND_V_MSG(_type != TYPE_PARENT, nullptr, "VoxelDataTree4D node is not subdivided, it has no children.");
	ERR_FAIL_INDEX_V(p_index, CHILD_COUNT, nullptr);
	return &_children[p_index];
}

int VoxelDataTree4D::get_child_index_containing(const Vector4i &p_voxel) const {
	const Vector4i center = _bounds.position + _bounds.size / 2;
	return int(p_voxel.x >= center.x) | (int(p_voxel.y >= center.y) << 1) | (int(p_voxel.z >= center.z) << 2) | (int(p_voxel.w >= center.w) << 3);
}

VoxelDataTree4D *VoxelDataTree4D::get_child_containing(const Vector4i &p_voxel) {
	if (_type != TYPE_PARENT || !has_voxel(p_voxel)) {
		return nullptr;
	}
	return &_children[get_child_index_containing(p_voxel)];
}

const VoxelDataTree4D *VoxelDataTree4D::get_child_containing(const Vector4i &p_voxel) const {
	if (_type != TYPE_PARENT || !has_voxel(p_voxel)) {
		return nullptr;
	}
	return &_children[get_child_index_containing(p_voxel)];
}

void VoxelDataTree4D::set_leaf_data(VoxelDataLeaf4D *p_data) {
	clear();
	if (p_data == nullptr) {
		return;
	}
	const Vector4i position = _bounds.position;
	const int32_t size_mask = _bounds.size.x - 1;
	if ((position.x & size_mask) != 0 || (position.y & size_mask) != 0 || (position.z & size_mask) != 0 || (position.w & size_mask) != 0) {
		memdelete(p_data);
		ERR_FAIL_MSG("VoxelDataTree4D leaf positions must be a multiple of their size on every axis.");
	}
	_data = p_data;
	_type = TYPE_LEAF;
	_defined_or_pending = true;
}

VoxelDataLeaf4D *VoxelDataTree4D::get_leaf_data() {
	ERR_FAIL_COND_V_MSG(_type != TYPE_LEAF, nullptr, "VoxelDataTree4D node is not a leaf, it has no leaf data.");
	return _data;
}

const VoxelDataLeaf4D *VoxelDataTree4D::get_leaf_data() const {
	ERR_FAIL_COND_V_MSG(_type != TYPE_LEAF, nullptr, "VoxelDataTree4D node is not a leaf, it has no leaf data.");
	return _data;
}

void VoxelDataTree4D::set_constant_material(const VoxelMaterial4D p_material) {
	clear();
	_constant_material = p_material;
	_type = TYPE_CONSTANT;
	_defined_or_pending = true;
}

VoxelMaterial4D VoxelDataTree4D::get_constant_material() const {
	ERR_FAIL_COND_V_MSG(_type != TYPE_CONSTANT, VoxelMaterial4D::UNDEFINED, "VoxelDataTree4D node is not constant, it has no constant value.");
	return _constant_material;
}

VoxelDataTree4D *VoxelDataTree4D::find_deepest_node(const Vector4i &p_voxel) {
	if (!has_voxel(p_voxel)) {
		return nullptr;
	}
	VoxelDataTree4D *node = this;
	while (node->_type == TYPE_PARENT) {
		node = &node->_children[node->get_child_index_containing(p_voxel)];
	}
	return node;
}

const VoxelDataTree4D *VoxelDataTree4D::find_deepest_node(const Vector4i &p_voxel) const {
	return const_cast<VoxelDataTree4D *>(this)->find_deepest_node(p_voxel);
}

bool VoxelDataTree4D::merge_constant_children() {
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

void VoxelDataTree4D::mark_edited(const Rect4i &p_edited_region) {
	if (!_bounds.grow(1).intersects_exclusive(p_edited_region)) {
		return;
	}
	if (_type == TYPE_LEAF) {
		_parent_needs_update = true;
	} else if (_type == TYPE_PARENT) {
		_parent_needs_update = true;
		for (int i = 0; i < CHILD_COUNT; i++) {
			_children[i].mark_edited(p_edited_region);
		}
	}
}

void VoxelDataTree4D::mark_region_pending(const Rect4i &p_region) {
	if (_defined_or_pending || !_bounds.intersects_exclusive(p_region)) {
		return;
	}
	if (_type == TYPE_UNDEFINED) {
		// Loads are chunk-granular, so a partially overlapped chunk is
		// requested whole; larger nodes split around the region's border.
		if (_bounds.size.x <= VOXEL_4D_DATA_CHUNK_SIZE || p_region.encloses_inclusive(_bounds)) {
			_defined_or_pending = true;
			return;
		}
		subdivide();
	}
	bool collapsible = true;
	bool all_defined_or_pending = true;
	for (int i = 0; i < CHILD_COUNT; i++) {
		_children[i].mark_region_pending(p_region);
		if (!_children[i].is_undefined() || !_children[i]._defined_or_pending) {
			collapsible = false;
		}
		all_defined_or_pending = all_defined_or_pending && _children[i]._defined_or_pending;
	}
	if (collapsible) {
		// Every child is now a pending mark, so one mark covers them all.
		clear();
		_defined_or_pending = true;
		return;
	}
	_defined_or_pending = all_defined_or_pending;
}

bool VoxelDataTree4D::_borders_different_material(const Ref<VoxelGenerator4D> &p_generator, const Rect4i &p_bounds, const VoxelMaterial4D p_material) {
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
						if (VoxelMaterialUtil4D::overlay_material(p_generator->get_material(outside), VoxelMaterial4D::AIR) != p_material) {
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}

void VoxelDataTree4D::generate(const Ref<VoxelGenerator4D> &p_generator) {
	ERR_FAIL_COND(p_generator.is_null());
	clear();
	if (_bounds.size.x > VOXEL_4D_DATA_CHUNK_SIZE) {
		subdivide();
		for (int i = 0; i < CHILD_COUNT; i++) {
			_children[i].generate(p_generator);
		}
		merge_constant_children();
		_defined_or_pending = true;
		return;
	}
	VoxelDataLeaf4D *leaf = memnew(VoxelDataLeaf4D);
	const VoxelMaterial4D first_material = VoxelMaterialUtil4D::overlay_material(p_generator->get_material(_bounds.position), VoxelMaterial4D::AIR);
	bool uniform = true;
	for (int32_t w = 0; w < _bounds.size.w; w++) {
		for (int32_t z = 0; z < _bounds.size.z; z++) {
			for (int32_t y = 0; y < _bounds.size.y; y++) {
				for (int32_t x = 0; x < _bounds.size.x; x++) {
					const Vector4i local_voxel = Vector4i(x, y, z, w);
					// A generator returning UNDEFINED here is not valid,
					// but as a fallback, UNDEFINED is replaced with air.
					const VoxelMaterial4D material = VoxelMaterialUtil4D::overlay_material(p_generator->get_material(_bounds.position + local_voxel), VoxelMaterial4D::AIR);
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
					const VoxelMaterial4D material = leaf->get_material(local_voxel);
					for (int axis = 0; axis < 4; axis++) {
						Vector4i neighbor_local = local_voxel;
						neighbor_local[axis] += 1;
						const VoxelMaterial4D neighbor_material = neighbor_local[axis] < _bounds.size[axis] ? leaf->get_material(neighbor_local) : VoxelMaterialUtil4D::overlay_material(p_generator->get_material(_bounds.position + neighbor_local), VoxelMaterial4D::AIR);
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

void VoxelDataTree4D::_take_contents(VoxelDataTree4D &p_donor) {
	ERR_FAIL_COND_MSG(_type != TYPE_UNDEFINED, "VoxelDataTree4D can only take contents into an undefined node.");
	ERR_FAIL_COND_MSG(_bounds != p_donor._bounds, "VoxelDataTree4D can only take the contents of a node with identical bounds.");
	_type = p_donor._type;
	_defined_or_pending = p_donor._defined_or_pending;
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
	p_donor._defined_or_pending = false;
}

bool VoxelDataTree4D::clear_chunk(const Vector4i &p_voxel) {
	if (!has_voxel(p_voxel) || (_type == TYPE_UNDEFINED && !_defined_or_pending)) {
		return false;
	}
	if (_bounds.size.x == VOXEL_4D_DATA_CHUNK_SIZE) {
		// Canceling a chunk's pending mark removes no data.
		const bool had_data = _type != TYPE_UNDEFINED;
		clear();
		return had_data;
	}
	if (_type == TYPE_CONSTANT) {
		split_constant();
	} else if (_type == TYPE_UNDEFINED) {
		// A pending mark covering more than the chunk splits, so that the
		// chunk's part of it can be canceled alone.
		subdivide();
	}
	ERR_FAIL_COND_V_MSG(_type != TYPE_PARENT, false, "VoxelDataTree4D nodes larger than a chunk should be parents, constants, or undefined.");
	const bool unloaded = get_child_containing(p_voxel)->clear_chunk(p_voxel);
	// Collapse the children if they are all undefined with matching pending
	// marks. Either way, keep this node's defined-or-pending mark exact: left
	// stale, it could make the newly missing chunk look still covered.
	bool collapsible = true;
	bool all_defined_or_pending = true;
	for (int i = 0; i < CHILD_COUNT; i++) {
		if (!_children[i].is_undefined() || _children[i]._defined_or_pending != _children[0]._defined_or_pending) {
			collapsible = false;
		}
		all_defined_or_pending = all_defined_or_pending && _children[i]._defined_or_pending;
	}
	if (collapsible) {
		clear();
	}
	_defined_or_pending = all_defined_or_pending;
	return unloaded;
}

bool VoxelDataTree4D::is_region_defined(const Rect4i &p_region) const {
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

VoxelMaterial4D VoxelDataTree4D::get_material(const Vector4i &p_voxel) const {
	const VoxelDataTree4D *node = find_deepest_node(p_voxel);
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
	return VoxelMaterial4D::UNDEFINED;
}

VoxelEdgeData4D VoxelDataTree4D::get_edge_data(const Vector4i &p_voxel, const int p_axis) const {
	const VoxelDataTree4D *node = find_deepest_node(p_voxel);
	if (node == nullptr || node->_type != TYPE_LEAF) {
		return VoxelEdgeData4D();
	}
	const Vector4i local_voxel = p_voxel - node->_bounds.position;
	if (!node->_data->has_edge_data(local_voxel, p_axis)) {
		return VoxelEdgeData4D();
	}
	return node->_data->get_edge_data(local_voxel, p_axis);
}

VoxelDataTree4D::VoxelDataTree4D(const Rect4i &p_bounds) :
		_bounds(p_bounds) {
	const Vector4i size = p_bounds.size;
	ERR_FAIL_COND_MSG(size.x < 1 || (size.x & (size.x - 1)) != 0 || size.y != size.x || size.z != size.x || size.w != size.x, "VoxelDataTree4D bounds size must be the same power of two on every axis.");
	const Vector4i position = p_bounds.position;
	const int32_t half_size_mask = (size.x >> 1) - 1;
	ERR_FAIL_COND_MSG(size.x > 1 && ((position.x & half_size_mask) != 0 || (position.y & half_size_mask) != 0 || (position.z & half_size_mask) != 0 || (position.w & half_size_mask) != 0), "VoxelDataTree4D bounds position must be a multiple of half the size on every axis.");
}

VoxelDataNeighborhood4D VoxelDataNeighborhood4D::get_child(const int p_index) const {
	VoxelDataNeighborhood4D child;
	ERR_FAIL_NULL_V(node, child);
	child.node = node->get_child(p_index);
	if (child.node == nullptr) {
		return child;
	}
	for (int direction = 0; direction < DIRECTION_COUNT; direction++) {
		if (direction == CENTER_DIRECTION) {
			continue;
		}
		int parent_direction = 0;
		int target_child_index = 0;
		for (int axis = 0, power = 1; axis < 4; axis++, power *= 3) {
			// The neighbor's position along this axis in units of the child's
			// size, relative to the original center node's lower corner, so
			// positions 0 and 1 are inside the center node.
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
		VoxelDataTree4D *target = parent_direction == CENTER_DIRECTION ? node : neighbors[parent_direction];
		if (target != nullptr && target->is_parent()) {
			target = target->get_child(target_child_index);
		}
		child.neighbors[direction] = target;
	}
	return child;
}

VoxelDataTree4D *VoxelDataNeighborhood4D::get_node_containing(const Vector4i &p_voxel) const {
	ERR_FAIL_NULL_V(node, nullptr);
	if (node->has_voxel(p_voxel)) {
		return node;
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
	return neighbors[direction];
}

VoxelMaterial4D VoxelDataNeighborhood4D::get_material(const Vector4i &p_voxel) const {
	const VoxelDataTree4D *containing = get_node_containing(p_voxel);
	return containing == nullptr ? VoxelMaterial4D::UNDEFINED : containing->get_material(p_voxel);
}

VoxelEdgeData4D VoxelDataNeighborhood4D::get_edge_data(const Vector4i &p_voxel, const int p_axis) const {
	const VoxelDataTree4D *containing = get_node_containing(p_voxel);
	return containing == nullptr ? VoxelEdgeData4D() : containing->get_edge_data(p_voxel, p_axis);
}

void VoxelDataTree4D::split_constant() {
	const VoxelMaterial4D constant_material = get_constant_material();
	if (_bounds.size.x > VOXEL_4D_DATA_CHUNK_SIZE) {
		clear();
		// The split parts still cover the node.
		_defined_or_pending = true;
		VoxelDataTree4D *constant_children = subdivide();
		for (int i = 0; i < CHILD_COUNT; i++) {
			constant_children[i].set_constant_material(constant_material);
		}
		return;
	}
	VoxelDataLeaf4D *leaf = memnew(VoxelDataLeaf4D);
	for (int32_t w = 0; w < _bounds.size.w; w++) {
		for (int32_t z = 0; z < _bounds.size.z; z++) {
			for (int32_t y = 0; y < _bounds.size.y; y++) {
				for (int32_t x = 0; x < _bounds.size.x; x++) {
					leaf->set_material(Vector4i(x, y, z, w), constant_material);
				}
			}
		}
	}
	set_leaf_data(leaf);
}

// Whether every defined voxel in the given region of the node has the given
// material. The node must be a leaf or a constant.
bool VoxelDataNeighborhood4D::_region_matches_material(const VoxelDataTree4D *p_node, const Rect4i &p_region, const VoxelMaterial4D p_material) {
	if (p_node->is_constant()) {
		const VoxelMaterial4D constant_material = p_node->get_constant_material();
		return constant_material == VoxelMaterial4D::UNDEFINED || constant_material == p_material;
	}
	const VoxelDataLeaf4D *leaf = p_node->get_leaf_data();
	const Vector4i origin = p_node->get_bounds().position;
	const Vector4i end = p_region.get_end();
	for (int32_t w = p_region.position.w; w < end.w; w++) {
		for (int32_t z = p_region.position.z; z < end.z; z++) {
			for (int32_t y = p_region.position.y; y < end.y; y++) {
				for (int32_t x = p_region.position.x; x < end.x; x++) {
					const VoxelMaterial4D material = leaf->get_material(Vector4i(x, y, z, w) - origin);
					if (material != VoxelMaterial4D::UNDEFINED && material != p_material) {
						return false;
					}
				}
			}
		}
	}
	return true;
}

// Makes the stored surface data of the edges crossing the border between two
// adjacent regions consistent with the materials at their ends, after edits
// may have changed one side relative to what the other side's generation
// assumed. If a surface must be added, it's flat on the border.
void VoxelDataNeighborhood4D::_reconcile_border(VoxelDataTree4D *p_lower, VoxelDataTree4D *p_upper, const int p_axis) {
	if (p_lower == nullptr || p_upper == nullptr || p_lower->is_undefined() || p_upper->is_undefined()) {
		return;
	}
	if (p_lower->is_parent() || p_upper->is_parent()) {
		// When both are parents they have equal sizes, so the facing children
		// pair up; a childless side is instead paired with each of the other
		// side's facing children.
		const int axis_bit = 1 << p_axis;
		for (int i = 0; i < VoxelDataTree4D::CHILD_COUNT; i++) {
			if ((i & axis_bit) != 0) {
				continue;
			}
			VoxelDataTree4D *lower_part = p_lower->is_parent() ? p_lower->get_child(i | axis_bit) : p_lower;
			VoxelDataTree4D *upper_part = p_upper->is_parent() ? p_upper->get_child(i) : p_upper;
			_reconcile_border(lower_part, upper_part, p_axis);
		}
		return;
	}
	// The layers of voxels on either side of the shared part of the border.
	Rect4i lower_layer = p_upper->get_bounds();
	lower_layer.position[p_axis] -= 1;
	lower_layer.size[p_axis] = 1;
	lower_layer = lower_layer.intersection(p_lower->get_bounds());
	Rect4i upper_layer = lower_layer;
	upper_layer.position[p_axis] += 1;
	// Constants cannot store edge data and may not border a different
	// material, so a constant with any mismatch on the border stops being one.
	if (p_lower->is_constant()) {
		if (_region_matches_material(p_upper, upper_layer, p_lower->get_constant_material())) {
			// Constants store no edge data, so there is nothing to remove.
			return;
		}
		p_lower->split_constant();
		_reconcile_border(p_lower, p_upper, p_axis);
		return;
	}
	if (p_upper->is_constant() && !_region_matches_material(p_lower, lower_layer, p_upper->get_constant_material())) {
		p_upper->split_constant();
		_reconcile_border(p_lower, p_upper, p_axis);
		return;
	}
	VoxelDataLeaf4D *lower_leaf = p_lower->get_leaf_data();
	const VoxelDataLeaf4D *upper_leaf = p_upper->is_leaf() ? p_upper->get_leaf_data() : nullptr;
	const Vector4i lower_origin = p_lower->get_bounds().position;
	const Vector4i upper_origin = p_upper->get_bounds().position;
	Vector4 border_normal;
	border_normal[p_axis] = 1.0f;
	const VoxelEdgeData4D border_crossing = VoxelEdgeData4D(border_normal, 0.5f);
	const Vector4i end = lower_layer.get_end();
	for (int32_t w = lower_layer.position.w; w < end.w; w++) {
		for (int32_t z = lower_layer.position.z; z < end.z; z++) {
			for (int32_t y = lower_layer.position.y; y < end.y; y++) {
				for (int32_t x = lower_layer.position.x; x < end.x; x++) {
					const Vector4i voxel = Vector4i(x, y, z, w);
					const Vector4i local_voxel = voxel - lower_origin;
					const VoxelMaterial4D lower_material = lower_leaf->get_material(local_voxel);
					Vector4i upper_voxel = voxel;
					upper_voxel[p_axis]++;
					const VoxelMaterial4D upper_material = upper_leaf != nullptr ? upper_leaf->get_material(upper_voxel - upper_origin) : p_upper->get_constant_material();
					if (lower_material == VoxelMaterial4D::UNDEFINED || upper_material == VoxelMaterial4D::UNDEFINED) {
						continue;
					}
					if (lower_material == upper_material) {
						lower_leaf->clear_edge_data(local_voxel, p_axis);
					} else if (!lower_leaf->has_edge_data(local_voxel, p_axis)) {
						lower_leaf->set_edge_data(local_voxel, p_axis, border_crossing);
					}
				}
			}
		}
	}
}

void VoxelDataNeighborhood4D::reconcile_borders() {
	ERR_FAIL_NULL(node);
	for (int axis = 0, power = 1; axis < 4; axis++, power *= 3) {
		_reconcile_border(neighbors[CENTER_DIRECTION - power], node, axis);
		_reconcile_border(node, neighbors[CENTER_DIRECTION + power], axis);
	}
}

void VoxelDataNeighborhood4D::apply_edit(const Ref<VoxelEdit4D> &p_edit) {
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
	node->_parent_needs_update = true;
	if (node->is_constant()) {
		// The edit may make the node non-uniform or give it normals; a larger
		// constant is only split, so that just the parts near the edit lose
		// their constant representation.
		node->split_constant();
	}
	if (node->is_parent()) {
		for (int i = 0; i < VoxelDataTree4D::CHILD_COUNT; i++) {
			// Checked here to skip building neighborhoods of irrelevant children.
			if (node->get_child(i)->get_bounds().intersects_exclusive(face_bounds)) {
				get_child(i).apply_edit(p_edit);
			}
		}
		return;
	}
	VoxelDataLeaf4D *leaf_data = node->get_leaf_data();
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
						const VoxelMaterial4D edit_lower = edit_bounds.has_point(voxel) ? p_edit->get_material(voxel) : VoxelMaterial4D::UNDEFINED;
						const VoxelMaterial4D edit_upper = edit_bounds.has_point(upper_voxel) ? p_edit->get_material(upper_voxel) : VoxelMaterial4D::UNDEFINED;
						const bool lower_defined = edit_lower != VoxelMaterial4D::UNDEFINED;
						const bool upper_defined = edit_upper != VoxelMaterial4D::UNDEFINED;
						if (!lower_defined && !upper_defined) {
							continue;
						}
						const VoxelMaterial4D old_lower = leaf_data->get_material(local_voxel);
						const VoxelMaterial4D old_upper = get_material(upper_voxel);
						const VoxelMaterial4D new_lower = VoxelMaterialUtil4D::overlay_material(edit_lower, old_lower);
						const VoxelMaterial4D new_upper = VoxelMaterialUtil4D::overlay_material(edit_upper, old_upper);
						if (new_lower == VoxelMaterial4D::UNDEFINED || new_upper == VoxelMaterial4D::UNDEFINED) {
							// The edge leads into an undefined chunk.
							continue;
						}
						if (new_lower == new_upper) {
							leaf_data->clear_edge_data(local_voxel, axis);
							continue;
						}
						const VoxelEdgeData4D edit_edge_data = p_edit->get_edge_data(voxel, axis);
						if (lower_defined != upper_defined && leaf_data->has_edge_data(local_voxel, axis)) {
							const bool same_material = lower_defined ? edit_lower == old_lower : edit_upper == old_upper;
							if (same_material) {
								// Writing a material over itself extends its
								// region, so the combined region's surface is
								// whichever crossing lies farther from the
								// edit-defined end.
								const real_t old_position = leaf_data->get_edge_data(local_voxel, axis).position;
								const real_t edit_position = edit_edge_data.position;
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
					leaf_data->set_material(local_voxel, VoxelMaterialUtil4D::overlay_material(p_edit->get_material(voxel), leaf_data->get_material(local_voxel)));
				}
			}
		}
	}
}

void VoxelDataNeighborhood4D::merge_edited_constants() {
	ERR_FAIL_NULL(node);
	if (!node->_parent_needs_update) {
		return;
	}
	node->_parent_needs_update = false;
	if (node->is_parent()) {
		for (int i = 0; i < VoxelDataTree4D::CHILD_COUNT; i++) {
			// Checked here to skip building neighborhoods of unmarked children.
			if (node->get_child(i)->_parent_needs_update) {
				get_child(i).merge_edited_constants();
			}
		}
		if (!node->merge_constant_children()) {
			bool all_defined_or_pending = true;
			for (int i = 0; i < VoxelDataTree4D::CHILD_COUNT; i++) {
				all_defined_or_pending = all_defined_or_pending && node->get_child(i)->_defined_or_pending;
			}
			node->_defined_or_pending = all_defined_or_pending;
		}
		return;
	}
	if (!node->is_leaf()) {
		return;
	}
	// A leaf can revert to a constant when its voxels are uniform, it stores
	// no edge data (constants cannot, and border data kept for undefined
	// neighbors must survive), and no defined voxel bordering it has a
	// different material (constants may not border one).
	const VoxelDataLeaf4D *leaf_data = node->get_leaf_data();
	if (leaf_data->get_edge_data_count() != 0) {
		return;
	}
	const VoxelMaterial4D material = leaf_data->get_material(Vector4i());
	const Rect4i bounds = node->get_bounds();
	for (int32_t w = 0; w < bounds.size.w; w++) {
		for (int32_t z = 0; z < bounds.size.z; z++) {
			for (int32_t y = 0; y < bounds.size.y; y++) {
				for (int32_t x = 0; x < bounds.size.x; x++) {
					if (leaf_data->get_material(Vector4i(x, y, z, w)) != material) {
						return;
					}
				}
			}
		}
	}
	for (int axis = 0; axis < 4; axis++) {
		for (int side = 0; side < 2; side++) {
			Rect4i outside_layer = bounds;
			outside_layer.position[axis] = side == 0 ? bounds.get_end()[axis] : bounds.position[axis] - 1;
			outside_layer.size[axis] = 1;
			const Vector4i end = outside_layer.get_end();
			for (int32_t w = outside_layer.position.w; w < end.w; w++) {
				for (int32_t z = outside_layer.position.z; z < end.z; z++) {
					for (int32_t y = outside_layer.position.y; y < end.y; y++) {
						for (int32_t x = outside_layer.position.x; x < end.x; x++) {
							const VoxelMaterial4D outside_material = get_material(Vector4i(x, y, z, w));
							if (outside_material != VoxelMaterial4D::UNDEFINED && outside_material != material) {
								return;
							}
						}
					}
				}
			}
		}
	}
	node->set_constant_material(material);
}
