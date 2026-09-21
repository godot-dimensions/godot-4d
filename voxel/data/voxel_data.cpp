#include "voxel_data.h"

#include "../edit/voxel_edit.h"
#include "../generators/tiger_test_generator.h"
#include "../voxel_constants.h"

void VoxelData::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_voxel_defined", "voxel"), &VoxelData::is_voxel_defined);
}

bool VoxelData::is_voxel_defined(const Vector4i &p_voxel) const {
	if (_tree == nullptr) {
		return false;
	}
	const VoxelDataTree *node = _tree->find_deepest_node(p_voxel);
	return node != nullptr && !node->is_undefined();
}

bool VoxelData::is_region_defined(const Rect4i &p_region) const {
	if (_tree == nullptr || !_tree->get_bounds().encloses_inclusive(p_region)) {
		return false;
	}
	return _tree->is_region_defined(p_region);
}

VoxelMaterial VoxelData::get_material(const Vector4i &p_voxel) const {
	return _tree == nullptr ? VoxelMaterial::UNDEFINED : _tree->get_material(p_voxel);
}

VoxelEdgeData VoxelData::get_edge_data(const Vector4i &p_voxel, const int p_axis) const {
	return _tree == nullptr ? VoxelEdgeData() : _tree->get_edge_data(p_voxel, p_axis);
}

Vector4i VoxelData::get_chunk_position(const Vector4i &p_voxel) const {
	Vector4i chunk_position = p_voxel;
	for (int i = 0; i < 4; i++) {
		chunk_position[i] -= (int32_t)Math::posmod(p_voxel[i], VOXEL_DATA_CHUNK_SIZE);
	}
	return chunk_position;
}

const VoxelDataTree *VoxelData::find_region_node(const Rect4i &p_region) const {
	if (_tree == nullptr || !_tree->has_voxel(p_region.position)) {
		return nullptr;
	}
	const VoxelDataTree *node = _tree;
	while (node->get_bounds() != p_region) {
		if (node->get_bounds().size.x <= p_region.size.x || !node->is_parent()) {
			return nullptr;
		}
		node = node->get_child_containing(p_region.position);
	}
	return node;
}

VoxelDataTree *VoxelData::generate_chunk_content(const Vector4i &p_voxel) const {
	VoxelDataTree *chunk = memnew(VoxelDataTree(Rect4i(get_chunk_position(p_voxel), VOXEL_DATA_CHUNK_SIZE_VECTOR)));
	chunk->generate(_generator);
	return chunk;
}

void VoxelData::apply_generated_chunk(VoxelDataTree *p_chunk) {
	if (_tree == nullptr) {
		_tree = p_chunk;
		return;
	}
	const Rect4i chunk_bounds = p_chunk->get_bounds();
	while (!_tree->get_bounds().encloses_inclusive(chunk_bounds)) {
		expand_bounds(chunk_bounds.position);
	}
	_tree->apply_generated_chunk(p_chunk);
	// Sometimes bounds of the same size as they were, just shifted, suffice.
	trim_bounds();
}

void VoxelData::expand_bounds(const Vector4i &p_toward) {
	const Rect4i old_bounds = _tree->get_bounds();
	const int32_t old_size = old_bounds.size.x;
	const Vector4i old_center = old_bounds.position + old_bounds.size / 2;
	Vector4i new_position = old_bounds.position;
	bool any_half_aligned = false;
	for (int axis = 0; axis < 4; axis++) {
		if ((old_bounds.position[axis] & (old_size - 1)) != 0) {
			// Half-aligned: keep the old center, so that the old root covers
			// the middle half of the new root along this axis.
			new_position[axis] -= old_size / 2;
			any_half_aligned = true;
		} else if (p_toward[axis] < old_center[axis]) {
			new_position[axis] -= old_size;
		}
	}
	VoxelDataTree *new_root = memnew(VoxelDataTree(Rect4i(new_position, Vector4i(old_size * 2, old_size * 2, old_size * 2, old_size * 2))));
	VoxelDataTree *new_children = new_root->subdivide();
	if (!any_half_aligned) {
		// The old root coincides with one child of the new root.
		new_children[new_root->get_child_index_containing(old_bounds.position)]._take_contents(*_tree);
	} else {
		// The old root straddles children of the new root, so each of the old
		// root's children coincides with a grandchild of the new root instead.
		if (_tree->is_constant()) {
			// Split the constant so that its parts can move separately.
			const VoxelMaterial constant_material = _tree->get_constant_material();
			_tree->clear();
			VoxelDataTree *split = _tree->subdivide();
			for (int i = 0; i < VoxelDataTree::CHILD_COUNT; i++) {
				split[i].set_constant_material(constant_material);
			}
		}
		// A half-aligned leaf cannot exist under the alignment invariant, so
		// the old root is now a parent, or undefined with nothing to move.
		if (_tree->is_parent()) {
			VoxelDataTree *old_children = _tree->_children;
			for (int i = 0; i < VoxelDataTree::CHILD_COUNT; i++) {
				if (old_children[i].is_undefined()) {
					continue;
				}
				const Vector4i old_child_position = old_children[i].get_bounds().position;
				VoxelDataTree *new_child = new_root->get_child_containing(old_child_position);
				if (new_child->is_undefined()) {
					new_child->subdivide();
				}
				new_child->get_child_containing(old_child_position)->_take_contents(old_children[i]);
			}
		}
	}
	memdelete(_tree);
	_tree = new_root;
}

void VoxelData::apply_edit(const Ref<VoxelEdit> &p_edit) {
	ERR_FAIL_COND(p_edit.is_null());
	if (_tree == nullptr) {
		return;
	}
	VoxelDataNeighbourhood{ _tree }.apply_edit(p_edit);
}

bool VoxelData::unload_chunk(const Vector4i &p_voxel) {
	if (_tree == nullptr || !_tree->clear_chunk(p_voxel)) {
		return false;
	}
	trim_bounds();
	return true;
}

void VoxelData::trim_bounds() {
	while (_tree != nullptr) {
		if (_tree->is_undefined()) {
			memdelete(_tree);
			_tree = nullptr;
			return;
		}
		if (!_tree->is_parent()) {
			return;
		}
		// The root can contract to half its size if its defined content fits
		// in a half-sized hypercube: either half of the bounds along each
		// axis, or the middle half, whose half-aligned position the alignment
		// invariant permits only for a root.
		VoxelDataTree *children = _tree->_children;
		bool children_all_parent_or_undefined = true;
		for (int i = 0; i < VoxelDataTree::CHILD_COUNT; i++) {
			if (!children[i].is_undefined() && !children[i].is_parent()) {
				children_all_parent_or_undefined = false;
				break;
			}
		}
		// The quarter of the old bounds, along each axis, where the half-sized
		// target begins: 0 and 2 select an existing half, 1 the middle.
		int offset_quarters[4];
		bool any_middle = false;
		for (int axis = 0; axis < 4; axis++) {
			const int axis_bit = 1 << axis;
			bool low_defined = false;
			bool high_defined = false;
			for (int i = 0; i < VoxelDataTree::CHILD_COUNT; i++) {
				if (!children[i].is_undefined()) {
					((i & axis_bit) ? high_defined : low_defined) = true;
				}
			}
			if (!high_defined) {
				offset_quarters[axis] = 0;
			} else if (!low_defined) {
				offset_quarters[axis] = 2;
			} else {
				// Middle contraction along this axis moves grandchildren, so
				// every defined child must be a parent, and each child's
				// grandchildren on its outer side along this axis must be
				// undefined.
				if (!children_all_parent_or_undefined) {
					return;
				}
				for (int i = 0; i < VoxelDataTree::CHILD_COUNT; i++) {
					if (!children[i].is_parent()) {
						continue;
					}
					for (int j = 0; j < VoxelDataTree::CHILD_COUNT; j++) {
						if ((j & axis_bit) == (i & axis_bit) && !children[i]._children[j].is_undefined()) {
							return;
						}
					}
				}
				offset_quarters[axis] = 1;
				any_middle = true;
			}
		}
		const Rect4i old_bounds = _tree->get_bounds();
		const int32_t half_size = old_bounds.size.x / 2;
		Vector4i new_position = old_bounds.position;
		for (int axis = 0; axis < 4; axis++) {
			new_position[axis] += (offset_quarters[axis] * half_size) / 2;
		}
		VoxelDataTree *new_root = memnew(VoxelDataTree(Rect4i(new_position, Vector4i(half_size, half_size, half_size, half_size))));
		if (!any_middle) {
			// The target coincides with one child; every other child is undefined.
			new_root->_take_contents(children[_tree->get_child_index_containing(new_position)]);
		} else {
			// The target straddles children, so each of its children is one of
			// the old root's grandchildren.
			VoxelDataTree *new_children = new_root->subdivide();
			for (int i = 0; i < VoxelDataTree::CHILD_COUNT; i++) {
				const Vector4i new_child_position = new_children[i].get_bounds().position;
				VoxelDataTree *child = _tree->get_child_containing(new_child_position);
				if (child->is_undefined()) {
					continue;
				}
				VoxelDataTree *grandchild = child->get_child_containing(new_child_position);
				if (grandchild->is_undefined()) {
					continue;
				}
				new_children[i]._take_contents(*grandchild);
			}
		}
		memdelete(_tree);
		_tree = new_root;
	}
}

void VoxelData::load_all_chunks() {
	if (_tree != nullptr) {
		memdelete(_tree);
	}
	const int32_t size = 8 * VOXEL_DATA_CHUNK_SIZE;
	_tree = memnew(VoxelDataTree(Rect4i(-size / 2, -size / 2, -size / 2, -size / 2, size, size, size, size)));
	_tree->generate(_generator);
}

VoxelData::VoxelData() {
	// Temporary: hard-coded test data.
	Ref<TigerTestGenerator> generator;
	generator.instantiate();
	_generator = generator;
}

VoxelData::~VoxelData() {
	if (_tree != nullptr) {
		memdelete(_tree);
	}
}
