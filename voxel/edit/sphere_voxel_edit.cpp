#include "sphere_voxel_edit.h"

void SphereVoxelEdit::set_center(const Vector4 &p_center) {
	_center = p_center;
	_update_bounds();
}

void SphereVoxelEdit::set_radius(const double p_radius) {
	ERR_FAIL_COND_MSG(p_radius < 0.0, "SphereVoxelEdit radius must not be negative. Refusing to set.");
	_radius = p_radius;
	_update_bounds();
}

void SphereVoxelEdit::set_material(const int p_material) {
	ERR_FAIL_COND_MSG(p_material != (int)VoxelMaterial::AIR && p_material != (int)VoxelMaterial::SOLID, "SphereVoxelEdit material must be AIR or SOLID. Refusing to set.");
	_material = (VoxelMaterial)p_material;
}

void SphereVoxelEdit::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_center"), &SphereVoxelEdit::get_center);
	ClassDB::bind_method(D_METHOD("set_center", "center"), &SphereVoxelEdit::set_center);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "center"), "set_center", "get_center");

	ClassDB::bind_method(D_METHOD("get_radius"), &SphereVoxelEdit::get_radius);
	ClassDB::bind_method(D_METHOD("set_radius", "radius"), &SphereVoxelEdit::set_radius);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius", PROPERTY_HINT_RANGE, "0,100,0.001,or_greater"), "set_radius", "get_radius");

	ClassDB::bind_method(D_METHOD("get_material"), &SphereVoxelEdit::get_material);
	ClassDB::bind_method(D_METHOD("set_material", "material"), &SphereVoxelEdit::set_material);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "material", PROPERTY_HINT_ENUM, "Air:1,Solid:2"), "set_material", "get_material");
}

void SphereVoxelEdit::_update_bounds() {
	ERR_FAIL_COND_MSG(_material == VoxelMaterial::UNDEFINED, "SphereVoxelEdit cannot apply the UNDEFINED material.");
	// Covers every voxel within 1 of the sphere's surface, where the edit has
	// an effect, plus the surrounding layer of UNDEFINED no-effect voxels.
	const real_t reach = _radius + 2.5f;
	const Vector4 extents = Vector4(reach, reach, reach, reach);
	const Vector4i min_voxel = Vector4i((_center - extents).floor());
	const Vector4i max_voxel = Vector4i((_center + extents).ceil());
	_bounds = Rect4i(min_voxel, max_voxel - min_voxel + Vector4i(1, 1, 1, 1));
}

VoxelValue SphereVoxelEdit::get_value(const Vector4i &p_voxel) const {
	const Vector4 point = Vector4(p_voxel.x + 0.5f, p_voxel.y + 0.5f, p_voxel.z + 0.5f, p_voxel.w + 0.5f);
	const double signed_distance = _radius - point.distance_to(_center);
	VoxelValue value;
	value.material = signed_distance > 0.0 ? _material : VoxelMaterial::UNDEFINED;
	value.density = (uint8_t)MIN(Math::abs(signed_distance) * 255.0, 255.0);
	return value;
}

Vector4 SphereVoxelEdit::get_normal(const Vector4i &p_voxel, const int p_axis, const VoxelValue &p_value_1, const VoxelValue &p_value_2) const {
	// The surface crosses the edge a / (a + b) of the way along it.
	const double a = p_value_1.density;
	const double b = p_value_2.density;
	const double crossing = a + b > 0.0 ? a / (a + b) : 0.5;
	Vector4 point = Vector4(p_voxel.x + 0.5, p_voxel.y + 0.5, p_voxel.z + 0.5, p_voxel.w + 0.5);
	point[p_axis] += crossing;
	const Vector4 radial = (point - _center).normalized();
	// The normal points out of the solid: away from the sphere when it is
	// filled with solid material, into it when it carves air out of a solid.
	return _material == VoxelMaterial::SOLID ? radial : -radial;
}
