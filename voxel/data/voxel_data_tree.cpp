#include "voxel_data_tree.h"

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
	if (p_data != nullptr) {
		_data = p_data;
		_type = TYPE_LEAF;
	}
}

VoxelDataLeaf *VoxelDataTree::get_leaf_data() {
	ERR_FAIL_COND_V_MSG(_type != TYPE_LEAF, nullptr, "VoxelDataTree node is not a leaf, it has no leaf data.");
	return _data;
}

const VoxelDataLeaf *VoxelDataTree::get_leaf_data() const {
	ERR_FAIL_COND_V_MSG(_type != TYPE_LEAF, nullptr, "VoxelDataTree node is not a leaf, it has no leaf data.");
	return _data;
}

void VoxelDataTree::set_constant_value(const VoxelValue p_value) {
	clear();
	_constant_value = p_value;
	_type = TYPE_CONSTANT;
}

VoxelValue VoxelDataTree::get_constant_value() const {
	ERR_FAIL_COND_V_MSG(_type != TYPE_CONSTANT, VoxelValue(), "VoxelDataTree node is not constant, it has no constant value.");
	return _constant_value;
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
		if (_children[i]._type != TYPE_CONSTANT || _children[i]._constant_value != _children[0]._constant_value) {
			return false;
		}
	}
	set_constant_value(_children[0]._constant_value);
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
						if (p_generator->get_value(outside).material != p_material) {
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
	const VoxelValue first_value = p_generator->get_value(_bounds.position);
	bool uniform = true;
	for (int32_t w = 0; w < _bounds.size.w; w++) {
		for (int32_t z = 0; z < _bounds.size.z; z++) {
			for (int32_t y = 0; y < _bounds.size.y; y++) {
				for (int32_t x = 0; x < _bounds.size.x; x++) {
					const Vector4i local_voxel = Vector4i(x, y, z, w);
					const VoxelValue value = p_generator->get_value(_bounds.position + local_voxel);
					leaf->set_value(local_voxel, value);
					uniform = uniform && value == first_value;
				}
			}
		}
	}
	// A chunk bordering a different material has active edges, whose surface
	// normals can only be stored in a leaf, so it must stay a leaf even when
	// its own voxels are uniform.
	if (uniform && !_borders_different_material(p_generator, _bounds, first_value.material)) {
		memdelete(leaf);
		set_constant_value(first_value);
		return;
	}
	// Store the surface normals of the leaf's active edges. Edges are visited
	// in edge index order, so each insert appends to the end of the array.
	for (int32_t w = 0; w < _bounds.size.w; w++) {
		for (int32_t z = 0; z < _bounds.size.z; z++) {
			for (int32_t y = 0; y < _bounds.size.y; y++) {
				for (int32_t x = 0; x < _bounds.size.x; x++) {
					const Vector4i local_voxel = Vector4i(x, y, z, w);
					const VoxelValue value = leaf->get_value(local_voxel);
					for (int axis = 0; axis < 4; axis++) {
						Vector4i neighbor_local = local_voxel;
						neighbor_local[axis] += 1;
						const VoxelValue neighbor_value = neighbor_local[axis] < _bounds.size[axis] ? leaf->get_value(neighbor_local) : p_generator->get_value(_bounds.position + neighbor_local);
						if (value.material != neighbor_value.material) {
							leaf->set_edge_normal(local_voxel, axis, p_generator->get_normal(_bounds.position + local_voxel, axis, value, neighbor_value));
						}
					}
				}
			}
		}
	}
	set_leaf_data(leaf);
}

VoxelValue VoxelDataTree::get_value(const Vector4i &p_voxel) const {
	const VoxelDataTree *node = find_deepest_node(p_voxel);
	if (node != nullptr) {
		switch (node->_type) {
			case TYPE_UNDEFINED:
			case TYPE_PARENT: {
			} break;
			case TYPE_LEAF: {
				return node->_data->get_value(p_voxel - node->_bounds.position);
			} break;
			case TYPE_CONSTANT: {
				return node->_constant_value;
			} break;
		}
	}
	return VoxelValue();
}

Vector4 VoxelDataTree::get_edge_normal(const Vector4i &p_voxel, const int p_axis) const {
	const VoxelDataTree *node = find_deepest_node(p_voxel);
	if (node == nullptr || node->_type != TYPE_LEAF) {
		return Vector4();
	}
	const Vector4i local_voxel = p_voxel - node->_bounds.position;
	if (!node->_data->has_edge_normal(local_voxel, p_axis)) {
		return Vector4();
	}
	return node->_data->get_edge_normal(local_voxel, p_axis);
}

VoxelDataTree::VoxelDataTree(const Rect4i &p_bounds) :
		_bounds(p_bounds) {
	const Vector4i size = p_bounds.size;
	ERR_FAIL_COND_MSG(size.x < 1 || (size.x & (size.x - 1)) != 0 || size.y != size.x || size.z != size.x || size.w != size.x, "VoxelDataTree bounds size must be the same power of two on every axis.");
}
