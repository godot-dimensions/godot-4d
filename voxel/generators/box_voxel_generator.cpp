#include "box_voxel_generator.h"

bool BoxVoxelGenerator::_contains_point(const Vector4 &p_point) const {
	const Vector4 offset = (p_point - _center).abs();
	for (int i = 0; i < 4; i++) {
		if (offset[i] > _size[i] * 0.5f) {
			return false;
		}
	}
	return true;
}

VoxelMaterial BoxVoxelGenerator::get_material(const Vector4i &p_voxel) const {
	return _contains_point(get_voxel_center(p_voxel)) ? _material_inner : _material_outer;
}

VoxelEdgeData BoxVoxelGenerator::get_edge_data(const Vector4i &p_voxel, const int p_axis) const {
	const Vector4 center = get_voxel_center(p_voxel);
	// An active edge crosses the box's surface through a face perpendicular
	// to the edge: the face toward the axis if the edge starts inside, or
	// away from it if it ends inside.
	const real_t face = _center[p_axis] + (_contains_point(center) ? _size[p_axis] : -_size[p_axis]) * 0.5f;
	Vector4 normal = Vector4();
	normal[p_axis] = 1.0f;
	return VoxelEdgeData(normal, face - center[p_axis]);
}

void BoxVoxelGenerator::set_center(const Vector4 &p_center) {
	_center = p_center;
	emit_changed();
}

void BoxVoxelGenerator::set_size(const Vector4 &p_size) {
	ERR_FAIL_COND_MSG(p_size.x < 0.0f || p_size.y < 0.0f || p_size.z < 0.0f || p_size.w < 0.0f, "BoxVoxelGenerator size must not be negative. Refusing to set.");
	_size = p_size;
	emit_changed();
}

void BoxVoxelGenerator::set_material_inner(const int p_material) {
	ERR_FAIL_COND_MSG((int)(VoxelMaterial)p_material != p_material, "BoxVoxelGenerator materials must fit in the material range. Refusing to set.");
	_material_inner = (VoxelMaterial)p_material;
	emit_changed();
}

void BoxVoxelGenerator::set_material_outer(const int p_material) {
	ERR_FAIL_COND_MSG((int)(VoxelMaterial)p_material != p_material, "BoxVoxelGenerator materials must fit in the material range. Refusing to set.");
	_material_outer = (VoxelMaterial)p_material;
	emit_changed();
}

void BoxVoxelGenerator::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_center"), &BoxVoxelGenerator::get_center);
	ClassDB::bind_method(D_METHOD("set_center", "center"), &BoxVoxelGenerator::set_center);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "center"), "set_center", "get_center");

	ClassDB::bind_method(D_METHOD("get_size"), &BoxVoxelGenerator::get_size);
	ClassDB::bind_method(D_METHOD("set_size", "size"), &BoxVoxelGenerator::set_size);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "size"), "set_size", "get_size");

	ClassDB::bind_method(D_METHOD("get_material_inner"), &BoxVoxelGenerator::get_material_inner);
	ClassDB::bind_method(D_METHOD("set_material_inner", "material"), &BoxVoxelGenerator::set_material_inner);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "material_inner", PROPERTY_HINT_RANGE, "0,255,1"), "set_material_inner", "get_material_inner");

	ClassDB::bind_method(D_METHOD("get_material_outer"), &BoxVoxelGenerator::get_material_outer);
	ClassDB::bind_method(D_METHOD("set_material_outer", "material"), &BoxVoxelGenerator::set_material_outer);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "material_outer", PROPERTY_HINT_RANGE, "0,255,1"), "set_material_outer", "get_material_outer");
}
