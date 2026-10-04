#include "box_voxel_edit_4d.h"

bool BoxVoxelEdit4D::_contains_point(const Vector4 &p_point) const {
	const Vector4 end = _rect.get_end();
	for (int axis = 0; axis < 4; axis++) {
		if (p_point[axis] < _rect.position[axis] || p_point[axis] > end[axis]) {
			return false;
		}
	}
	return true;
}

void BoxVoxelEdit4D::_update_bounds() {
	// Covers exactly the voxels whose centers are inside the box.
	const Vector4 half = Vector4(0.5f, 0.5f, 0.5f, 0.5f);
	const Vector4i min_voxel = Vector4i((_rect.position - half).ceil());
	const Vector4i max_voxel = Vector4i((_rect.get_end() - half).floor());
	_bounds = Rect4i(min_voxel, (max_voxel - min_voxel + Vector4i(1, 1, 1, 1)).maxi(0));
}

VoxelMaterial4D BoxVoxelEdit4D::get_material(const Vector4i &p_voxel) const {
	const Vector4 center = Vector4(p_voxel) + Vector4(0.5f, 0.5f, 0.5f, 0.5f);
	return _contains_point(center) ? _material : VoxelMaterial4D::UNDEFINED;
}

VoxelEdgeData4D BoxVoxelEdit4D::get_edge_data(const Vector4i &p_voxel, const int p_axis) const {
	const Vector4 center = Vector4(p_voxel) + Vector4(0.5f, 0.5f, 0.5f, 0.5f);
	// An active edge crosses the box's surface through a face perpendicular
	// to the edge: the face toward the axis if the edge starts inside, or
	// away from it if it ends inside.
	const real_t face = _contains_point(center) ? _rect.get_end()[p_axis] : _rect.position[p_axis];
	Vector4 normal = Vector4();
	normal[p_axis] = 1.0f;
	return VoxelEdgeData4D(normal, face - center[p_axis]);
}

void BoxVoxelEdit4D::set_rect(const Rect4 &p_rect) {
	_rect = p_rect;
	_update_bounds();
}

void BoxVoxelEdit4D::set_position(const Vector4 &p_position) {
	_rect.position = p_position;
	_update_bounds();
}

void BoxVoxelEdit4D::set_size(const Vector4 &p_size) {
	_rect.size = p_size;
	_update_bounds();
}

void BoxVoxelEdit4D::set_fill_material(const int p_fill_material) {
	const VoxelMaterial4D material = (VoxelMaterial4D)p_fill_material;
	ERR_FAIL_COND_MSG((int)material != p_fill_material || material == VoxelMaterial4D::UNDEFINED, "BoxVoxelEdit4D fill material must be a defined material. Refusing to set.");
	_material = material;
}

void BoxVoxelEdit4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_position"), &BoxVoxelEdit4D::get_position);
	ClassDB::bind_method(D_METHOD("set_position", "position"), &BoxVoxelEdit4D::set_position);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "position"), "set_position", "get_position");

	ClassDB::bind_method(D_METHOD("get_size"), &BoxVoxelEdit4D::get_size);
	ClassDB::bind_method(D_METHOD("set_size", "size"), &BoxVoxelEdit4D::set_size);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "size"), "set_size", "get_size");

	ClassDB::bind_method(D_METHOD("get_fill_material"), &BoxVoxelEdit4D::get_fill_material);
	ClassDB::bind_method(D_METHOD("set_fill_material", "fill_material"), &BoxVoxelEdit4D::set_fill_material);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "fill_material", PROPERTY_HINT_RANGE, "0,254,1"), "set_fill_material", "get_fill_material");
}
