#include "sphere_voxel_edit.h"

void SphereVoxelEdit::set_center(const Vector4 &p_center) {
	_center = p_center;
	_update_bounds();
}

void SphereVoxelEdit::set_radius(const real_t p_radius) {
	ERR_FAIL_COND_MSG(p_radius < 0.0, "SphereVoxelEdit radius must not be negative. Refusing to set.");
	_radius = p_radius;
	_update_bounds();
}

void SphereVoxelEdit::set_fill_material(const int p_fill_material) {
	ERR_FAIL_COND_MSG(p_fill_material != (int)VoxelMaterial::AIR && p_fill_material != (int)VoxelMaterial::SOLID, "SphereVoxelEdit fill material must be AIR or SOLID. Refusing to set.");
	_material = (VoxelMaterial)p_fill_material;
}

void SphereVoxelEdit::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_center"), &SphereVoxelEdit::get_center);
	ClassDB::bind_method(D_METHOD("set_center", "center"), &SphereVoxelEdit::set_center);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "center"), "set_center", "get_center");

	ClassDB::bind_method(D_METHOD("get_radius"), &SphereVoxelEdit::get_radius);
	ClassDB::bind_method(D_METHOD("set_radius", "radius"), &SphereVoxelEdit::set_radius);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius", PROPERTY_HINT_RANGE, "0,100,0.001,or_greater"), "set_radius", "get_radius");

	ClassDB::bind_method(D_METHOD("get_fill_material"), &SphereVoxelEdit::get_fill_material);
	ClassDB::bind_method(D_METHOD("set_fill_material", "fill_material"), &SphereVoxelEdit::set_fill_material);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "fill_material", PROPERTY_HINT_ENUM, "Air:1,Solid:2"), "set_fill_material", "get_fill_material");
}

void SphereVoxelEdit::_update_bounds() {
	ERR_FAIL_COND_MSG(_material == VoxelMaterial::UNDEFINED, "SphereVoxelEdit cannot apply the UNDEFINED material.");
	// Covers exactly the voxels whose centers are inside the sphere.
	const Vector4 half = Vector4(0.5f, 0.5f, 0.5f, 0.5f);
	const Vector4 extents = Vector4(_radius, _radius, _radius, _radius);
	const Vector4i min_voxel = Vector4i((_center - half - extents).ceil());
	const Vector4i max_voxel = Vector4i((_center - half + extents).floor());
	_bounds = Rect4i(min_voxel, (max_voxel - min_voxel + Vector4i(1, 1, 1, 1)).maxi(0));
}

VoxelMaterial SphereVoxelEdit::get_material(const Vector4i &p_voxel) const {
	const Vector4 point = Vector4(p_voxel.x + 0.5f, p_voxel.y + 0.5f, p_voxel.z + 0.5f, p_voxel.w + 0.5f);
	const real_t signed_distance = _radius - point.distance_to(_center);
	return signed_distance > 0.0 ? _material : VoxelMaterial::UNDEFINED;
}

VoxelEdgeData SphereVoxelEdit::get_edge_data(const Vector4i &p_voxel, const int p_axis) const {
	Vector4 point = Vector4(p_voxel.x + 0.5, p_voxel.y + 0.5, p_voxel.z + 0.5, p_voxel.w + 0.5);
	const Vector4 from_center = point - _center;
	// Solving |from_center + crossing * axis|² = radius² gives the exact
	// position where the edge crosses the sphere. An active edge crosses the
	// surface exactly once, so exactly one root lies on the edge.
	const real_t half_b = from_center[p_axis];
	const real_t constant_term = from_center.length_squared() - _radius * _radius;
	const real_t sqrt_discriminant = Math::sqrt(MAX(half_b * half_b - constant_term, (real_t)0.0));
	real_t crossing = -half_b - sqrt_discriminant;
	if (crossing < 0.0 || crossing > 1.0) {
		crossing = -half_b + sqrt_discriminant;
	}
	point[p_axis] += crossing;
	return VoxelEdgeData::encode(point - _center, crossing);
}
