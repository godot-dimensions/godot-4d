#include "parallelogram_voxel_edit.h"

bool ParallelogramVoxelEdit::_contains_local_point(const Vector4 &p_local) const {
	for (int axis = 0; axis < 4; axis++) {
		if (p_local[axis] < (real_t)0.0f || p_local[axis] > (real_t)1.0f) {
			return false;
		}
	}
	return true;
}

void ParallelogramVoxelEdit::_update_cache() {
	_degenerate = _transform.basis.determinant() == (real_t)0.0f;
	if (_degenerate) {
		_bounds = Rect4i();
		return;
	}
	_inverse_transform = _transform.inverse();
	// The axis-aligned bounds of the transformed hypercube: each basis
	// column shifts one corner of the bounds by its positive or negative
	// components.
	Vector4 min_corner = _transform.origin;
	Vector4 max_corner = _transform.origin;
	for (int i = 0; i < 4; i++) {
		const Vector4 column = _transform.basis[i];
		for (int axis = 0; axis < 4; axis++) {
			if (column[axis] < (real_t)0.0f) {
				min_corner[axis] += column[axis];
			} else {
				max_corner[axis] += column[axis];
			}
		}
	}
	// Covers exactly the voxels whose centers are inside the AABB.
	const Vector4 half = Vector4(0.5f, 0.5f, 0.5f, 0.5f);
	const Vector4i min_voxel = Vector4i((min_corner - half).ceil());
	const Vector4i max_voxel = Vector4i((max_corner - half).floor());
	_bounds = Rect4i(min_voxel, (max_voxel - min_voxel + Vector4i(1, 1, 1, 1)).maxi(0));
}

VoxelMaterial ParallelogramVoxelEdit::get_material(const Vector4i &p_voxel) const {
	if (_degenerate) {
		return VoxelMaterial::UNDEFINED;
	}
	const Vector4 center = Vector4(p_voxel) + Vector4(0.5f, 0.5f, 0.5f, 0.5f);
	return _contains_local_point(_inverse_transform.xform(center)) ? _material : VoxelMaterial::UNDEFINED;
}

VoxelEdgeData ParallelogramVoxelEdit::get_edge_data(const Vector4i &p_voxel, const int p_axis) const {
	ERR_FAIL_COND_V(_degenerate, VoxelEdgeData());
	const Vector4 center = Vector4(p_voxel) + Vector4(0.5f, 0.5f, 0.5f, 0.5f);
	const Vector4 local_start = _inverse_transform.xform(center);
	// The edge's direction in local space, where the shape is the unit
	// hypercube and the edge crosses its surface where it enters or leaves
	// every axis' slab.
	const Vector4 local_direction = _inverse_transform.basis[p_axis];
	real_t enter = -(real_t)Math_INF;
	real_t exit = (real_t)Math_INF;
	int enter_axis = 0;
	int exit_axis = 0;
	for (int axis = 0; axis < 4; axis++) {
		if (local_direction[axis] == (real_t)0.0f) {
			continue;
		}
		const real_t to_zero = -local_start[axis] / local_direction[axis];
		const real_t to_one = ((real_t)1.0f - local_start[axis]) / local_direction[axis];
		const real_t low = MIN(to_zero, to_one);
		const real_t high = MAX(to_zero, to_one);
		if (low > enter) {
			enter = low;
			enter_axis = axis;
		}
		if (high < exit) {
			exit = high;
			exit_axis = axis;
		}
	}
	// An edge starting inside crosses where it leaves some slab; one starting
	// outside crosses where it has entered every slab.
	const bool starts_inside = _contains_local_point(local_start);
	const real_t crossing = starts_inside ? exit : enter;
	const int facet_axis = starts_inside ? exit_axis : enter_axis;
	// The facet's normal is the gradient of that local coordinate: the
	// corresponding row of the inverse basis.
	const Basis4D &inverse_basis = _inverse_transform.basis;
	const Vector4 normal = Vector4(inverse_basis[0][facet_axis], inverse_basis[1][facet_axis], inverse_basis[2][facet_axis], inverse_basis[3][facet_axis]);
	return VoxelEdgeData(normal.normalized(), crossing);
}

void ParallelogramVoxelEdit::set_transform(const Transform4D &p_transform) {
	_transform = p_transform;
	_update_cache();
}

void ParallelogramVoxelEdit::set_basis_bind(const Projection &p_basis) {
	_transform.basis = p_basis;
	_update_cache();
}

void ParallelogramVoxelEdit::set_position(const Vector4 &p_position) {
	_transform.origin = p_position;
	_update_cache();
}

void ParallelogramVoxelEdit::set_fill_material(const int p_fill_material) {
	const VoxelMaterial material = (VoxelMaterial)p_fill_material;
	ERR_FAIL_COND_MSG((int)material != p_fill_material || material == VoxelMaterial::UNDEFINED, "ParallelogramVoxelEdit fill material must be a defined material. Refusing to set.");
	_material = material;
}

void ParallelogramVoxelEdit::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_basis"), &ParallelogramVoxelEdit::get_basis_bind);
	ClassDB::bind_method(D_METHOD("set_basis", "basis"), &ParallelogramVoxelEdit::set_basis_bind);
	ADD_PROPERTY(PropertyInfo(Variant::PROJECTION, "basis"), "set_basis", "get_basis");

	ClassDB::bind_method(D_METHOD("get_position"), &ParallelogramVoxelEdit::get_position);
	ClassDB::bind_method(D_METHOD("set_position", "position"), &ParallelogramVoxelEdit::set_position);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "position"), "set_position", "get_position");

	ClassDB::bind_method(D_METHOD("get_fill_material"), &ParallelogramVoxelEdit::get_fill_material);
	ClassDB::bind_method(D_METHOD("set_fill_material", "fill_material"), &ParallelogramVoxelEdit::set_fill_material);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "fill_material", PROPERTY_HINT_RANGE, "0,254,1"), "set_fill_material", "get_fill_material");
}
