#include "voxel_data.h"

#include "../edit/voxel_edit.h"
#include "../voxel_constants.h"

// Whether the node covers nothing at all: neither data nor a pending load
// mark.
static bool _is_tree_empty(const VoxelDataTree &p_node) {
	return p_node.is_undefined() && !p_node.is_defined_or_pending();
}

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

bool VoxelData::is_voxel_defined_or_pending(const Vector4i &p_voxel) const {
	if (_tree == nullptr) {
		return false;
	}
	const VoxelDataTree *node = _tree->find_deepest_node(p_voxel);
	return node != nullptr && node->is_defined_or_pending();
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

VoxelDataNeighbourhood VoxelData::find_region_neighbourhood(const Rect4i &p_region) {
	VoxelDataNeighbourhood neighbourhood;
	if (_tree == nullptr || !_tree->get_bounds().encloses_inclusive(p_region)) {
		return neighbourhood;
	}
	neighbourhood.node = _tree;
	while (neighbourhood.node->is_parent()) {
		const int child_index = neighbourhood.node->get_child_index_containing(p_region.position);
		if (!neighbourhood.node->get_child(child_index)->get_bounds().encloses_inclusive(p_region)) {
			break;
		}
		neighbourhood = neighbourhood.get_child(child_index);
	}
	return neighbourhood;
}

VoxelDataTree *VoxelData::generate_chunk_content(const Vector4i &p_voxel) const {
	ERR_FAIL_COND_V(_generator.is_null(), nullptr);
	VoxelDataTree *chunk = memnew(VoxelDataTree(Rect4i(get_chunk_position(p_voxel), VOXEL_DATA_CHUNK_SIZE_VECTOR)));
	chunk->generate(_generator);
	return chunk;
}

void VoxelData::set_generator(const Ref<VoxelGenerator> &p_generator) {
	_generator = p_generator;
}

void VoxelData::apply_generated_chunk(VoxelDataTree *p_chunk) {
	ERR_FAIL_NULL(p_chunk);
	if (_tree == nullptr) {
		_tree = p_chunk;
		return;
	}
	const Rect4i chunk_bounds = p_chunk->get_bounds();
	while (!_tree->get_bounds().encloses_inclusive(chunk_bounds)) {
		expand_bounds(chunk_bounds.position);
	}
	VoxelDataNeighbourhood target{ _tree };
	while (target.node->get_bounds() != chunk_bounds) {
		if (target.node->get_bounds().size.x <= chunk_bounds.size.x) {
			memdelete(p_chunk);
			ERR_FAIL_MSG("VoxelDataTree chunks must line up with the tree's subdivisions.");
		}
		if (target.node->is_undefined()) {
			target.node->subdivide();
		} else if (!target.node->is_parent()) {
			break;
		}
		target = target.get_child(target.node->get_child_index_containing(chunk_bounds.position));
	}
	if (!target.node->is_undefined()) {
		// That part of the tree is already defined; discard the chunk.
		memdelete(p_chunk);
		return;
	}
	target.node->_take_contents(*p_chunk);
	memdelete(p_chunk);
	// The new node generated its contents assuming the generator's materials
	// around it, but edits may have changed them, so the surface data of the
	// edges crossing its borders needs reconciling on both sides.
	target.reconcile_borders();
	// Loading counts as an edit for the constant-merging pass: it can complete
	// a parent's set of constant children, and reconciliation can clear stale
	// edge data in bordering chunks.
	_tree->mark_edited(chunk_bounds);
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
		} else if (_tree->is_undefined() && _tree->is_defined_or_pending()) {
			// Likewise split a pending mark; the children inherit it.
			_tree->subdivide();
		}
		// A half-aligned leaf cannot exist under the alignment invariant, so
		// the old root is now a parent, or empty with nothing to move.
		if (_tree->is_parent()) {
			VoxelDataTree *old_children = _tree->_children;
			for (int i = 0; i < VoxelDataTree::CHILD_COUNT; i++) {
				if (_is_tree_empty(old_children[i])) {
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

void VoxelData::mark_region_pending(const Rect4i &p_region) {
	if (_tree == nullptr) {
		_tree = memnew(VoxelDataTree(Rect4i(get_chunk_position(p_region.position), VOXEL_DATA_CHUNK_SIZE_VECTOR)));
	}
	while (!_tree->get_bounds().encloses_inclusive(p_region)) {
		expand_bounds(p_region.position);
	}
	_tree->mark_region_pending(p_region);
	trim_bounds();
}

void VoxelData::merge_edited_constants() {
	if (_tree == nullptr) {
		return;
	}
	VoxelDataNeighbourhood{ _tree }.merge_edited_constants();
}

bool VoxelData::unload_chunk(const Vector4i &p_voxel) {
	if (_tree == nullptr) {
		return false;
	}
	const bool unloaded = _tree->clear_chunk(p_voxel);
	// Cancelling a pending mark can also free up the bounds, so trim
	// regardless of whether any data was removed.
	trim_bounds();
	return unloaded;
}

void VoxelData::trim_bounds() {
	while (_tree != nullptr) {
		if (_is_tree_empty(*_tree)) {
			memdelete(_tree);
			_tree = nullptr;
			return;
		}
		if (!_tree->is_parent()) {
			return;
		}
		// The root can contract to half its size if its content, including
		// pending marks, fits in a half-sized hypercube: either half of the
		// bounds along each axis, or the middle half, whose half-aligned
		// position the alignment invariant permits only for a root.
		VoxelDataTree *children = _tree->_children;
		bool children_all_parent_or_empty = true;
		for (int i = 0; i < VoxelDataTree::CHILD_COUNT; i++) {
			if (!_is_tree_empty(children[i]) && !children[i].is_parent()) {
				children_all_parent_or_empty = false;
				break;
			}
		}
		// The quarter of the old bounds, along each axis, where the half-sized
		// target begins: 0 and 2 select an existing half, 1 the middle.
		int offset_quarters[4];
		bool any_middle = false;
		for (int axis = 0; axis < 4; axis++) {
			const int axis_bit = 1 << axis;
			bool low_occupied = false;
			bool high_occupied = false;
			for (int i = 0; i < VoxelDataTree::CHILD_COUNT; i++) {
				if (!_is_tree_empty(children[i])) {
					((i & axis_bit) ? high_occupied : low_occupied) = true;
				}
			}
			if (!high_occupied) {
				offset_quarters[axis] = 0;
			} else if (!low_occupied) {
				offset_quarters[axis] = 2;
			} else {
				// Middle contraction along this axis moves grandchildren, so
				// every occupied child must be a parent, and each child's
				// grandchildren on its outer side along this axis must be
				// empty.
				if (!children_all_parent_or_empty) {
					return;
				}
				for (int i = 0; i < VoxelDataTree::CHILD_COUNT; i++) {
					if (!children[i].is_parent()) {
						continue;
					}
					for (int j = 0; j < VoxelDataTree::CHILD_COUNT; j++) {
						if ((j & axis_bit) == (i & axis_bit) && !_is_tree_empty(children[i]._children[j])) {
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
			// The target coincides with one child; every other child is empty.
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
				if (_is_tree_empty(*grandchild)) {
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
	ERR_FAIL_COND(_generator.is_null());
	if (_tree != nullptr) {
		memdelete(_tree);
	}
	const int32_t size = 8 * VOXEL_DATA_CHUNK_SIZE;
	_tree = memnew(VoxelDataTree(Rect4i(-size / 2, -size / 2, -size / 2, -size / 2, size, size, size, size)));
	_tree->generate(_generator);
}

VoxelData::~VoxelData() {
	if (_tree != nullptr) {
		memdelete(_tree);
	}
}
