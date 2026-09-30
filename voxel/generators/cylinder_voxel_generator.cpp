#include "cylinder_voxel_generator.h"

bool CylinderVoxelGenerator::_contains_point(const Vector4 &p_point) const {
	Vector4 offset = p_point - _center;
	if (Math::abs(offset.y) > _height * 0.5f) {
		return false;
	}
	offset.y = 0.0f;
	return offset.length_squared() <= _radius * _radius;
}

VoxelMaterial CylinderVoxelGenerator::get_material(const Vector4i &p_voxel) const {
	return _contains_point(get_voxel_center(p_voxel)) ? _material_inner : _material_outer;
}

VoxelEdgeData CylinderVoxelGenerator::get_edge_data(const Vector4i &p_voxel, const int p_axis) const {
	const Vector4 center = get_voxel_center(p_voxel);
	if (p_axis == 1) {
		// An active vertical edge lies within the radius and crosses a cap:
		// the top cap if it starts inside, or the bottom cap if it ends
		// inside.
		const real_t cap = _center.y + (_contains_point(center) ? _height : -_height) * 0.5f;
		return VoxelEdgeData(Vector4(0.0f, 1.0f, 0.0f, 0.0f), cap - center.y);
	}
	// An active horizontal edge lies within the caps and crosses the radial
	// surface. Solving |from_center + crossing * axis|² = radius² over the
	// horizontal axes gives the exact position where the edge crosses it; the
	// root on the edge is the crossing.
	Vector4 from_center = center - _center;
	from_center.y = 0.0f;
	const real_t half_b = from_center[p_axis];
	const real_t constant_term = from_center.length_squared() - _radius * _radius;
	const real_t sqrt_discriminant = Math::sqrt(MAX(half_b * half_b - constant_term, (real_t)0.0));
	real_t crossing = -half_b - sqrt_discriminant;
	if (crossing < 0.0f || crossing > 1.0f) {
		crossing = -half_b + sqrt_discriminant;
	}
	Vector4 normal = from_center;
	normal[p_axis] += crossing;
	return VoxelEdgeData(normal.normalized(), crossing);
}

void CylinderVoxelGenerator::set_center(const Vector4 &p_center) {
	_center = p_center;
	emit_changed();
}

void CylinderVoxelGenerator::set_height(const real_t p_height) {
	ERR_FAIL_COND_MSG(p_height < 0.0f, "CylinderVoxelGenerator height must not be negative. Refusing to set.");
	_height = p_height;
	emit_changed();
}

void CylinderVoxelGenerator::set_radius(const real_t p_radius) {
	ERR_FAIL_COND_MSG(p_radius < 0.0f, "CylinderVoxelGenerator radius must not be negative. Refusing to set.");
	_radius = p_radius;
	emit_changed();
}

void CylinderVoxelGenerator::set_material_inner(const int p_material) {
	ERR_FAIL_COND_MSG((int)(VoxelMaterial)p_material != p_material, "CylinderVoxelGenerator materials must fit in the material range. Refusing to set.");
	_material_inner = (VoxelMaterial)p_material;
	emit_changed();
}

void CylinderVoxelGenerator::set_material_outer(const int p_material) {
	ERR_FAIL_COND_MSG((int)(VoxelMaterial)p_material != p_material, "CylinderVoxelGenerator materials must fit in the material range. Refusing to set.");
	_material_outer = (VoxelMaterial)p_material;
	emit_changed();
}

void CylinderVoxelGenerator::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_center"), &CylinderVoxelGenerator::get_center);
	ClassDB::bind_method(D_METHOD("set_center", "center"), &CylinderVoxelGenerator::set_center);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "center"), "set_center", "get_center");

	ClassDB::bind_method(D_METHOD("get_height"), &CylinderVoxelGenerator::get_height);
	ClassDB::bind_method(D_METHOD("set_height", "height"), &CylinderVoxelGenerator::set_height);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "height", PROPERTY_HINT_RANGE, "0,100,0.001,or_greater"), "set_height", "get_height");

	ClassDB::bind_method(D_METHOD("get_radius"), &CylinderVoxelGenerator::get_radius);
	ClassDB::bind_method(D_METHOD("set_radius", "radius"), &CylinderVoxelGenerator::set_radius);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius", PROPERTY_HINT_RANGE, "0,100,0.001,or_greater"), "set_radius", "get_radius");

	ClassDB::bind_method(D_METHOD("get_material_inner"), &CylinderVoxelGenerator::get_material_inner);
	ClassDB::bind_method(D_METHOD("set_material_inner", "material"), &CylinderVoxelGenerator::set_material_inner);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "material_inner", PROPERTY_HINT_RANGE, "0,255,1"), "set_material_inner", "get_material_inner");

	ClassDB::bind_method(D_METHOD("get_material_outer"), &CylinderVoxelGenerator::get_material_outer);
	ClassDB::bind_method(D_METHOD("set_material_outer", "material"), &CylinderVoxelGenerator::set_material_outer);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "material_outer", PROPERTY_HINT_RANGE, "0,255,1"), "set_material_outer", "get_material_outer");
}
