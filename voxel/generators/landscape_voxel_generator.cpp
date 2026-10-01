#include "landscape_voxel_generator.h"

real_t LandscapeVoxelGenerator::_relative_height(const Vector4 &p_point, Vector4 *r_gradient) const {
	const real_t height = p_point.y + _noise.sample(p_point, r_gradient);
	if (r_gradient != nullptr) {
		r_gradient->y += (real_t)1.0f;
	}
	return height;
}

int LandscapeVoxelGenerator::_band_for_height(const real_t p_height) const {
	const int cutoff_count = _material_cutoffs.size();
	int band = 0;
	for (int i = 0; i < cutoff_count; i++) {
		if (_material_cutoffs[i] <= p_height) {
			band++;
		}
	}
	return band;
}

VoxelMaterial LandscapeVoxelGenerator::_material_for_height(const real_t p_height, real_t *r_cutoff) const {
	const int band = _band_for_height(p_height);
	if (r_cutoff != nullptr && band > 0) {
		*r_cutoff = _material_cutoffs[band - 1];
	}
	return band < _materials.size() ? (VoxelMaterial)_materials[band] : VoxelMaterial::UNDEFINED;
}

VoxelMaterial LandscapeVoxelGenerator::get_material(const Vector4i &p_voxel) const {
	const Vector4 center = get_voxel_center(p_voxel);
	// With no cutoff within the noise's reach of the height, no possible
	// noise value can change the band, so the noise need not be computed.
	const real_t max_noise = _noise.max_value();
	if (_band_for_height(center.y - max_noise) == _band_for_height(center.y + max_noise)) {
		return _material_for_height(center.y - max_noise, nullptr);
	}
	return _material_for_height(_relative_height(center, nullptr), nullptr);
}

VoxelEdgeData LandscapeVoxelGenerator::get_edge_data(const Vector4i &p_voxel, const int p_axis) const {
	const Vector4 center = get_voxel_center(p_voxel);
	Vector4 neighbor = center;
	neighbor[p_axis] += 1.0f;
	const real_t height_1 = _relative_height(center, nullptr);
	const real_t height_2 = _relative_height(neighbor, nullptr);
	// The surface between the two ends' materials lies at the higher end's
	// band-entry cutoff, which always separates the two heights.
	real_t cutoff = 0.0f;
	_material_for_height(MAX(height_1, height_2), &cutoff);
	const real_t crossing = (cutoff - height_1) / (height_2 - height_1);
	Vector4 point = center;
	point[p_axis] += crossing;
	Vector4 gradient;
	_relative_height(point, &gradient);
	return VoxelEdgeData(gradient, crossing);
}

void LandscapeVoxelGenerator::set_seed(const int p_seed) {
	_noise.seed = (uint32_t)p_seed;
	emit_changed();
}

void LandscapeVoxelGenerator::set_octaves(const int p_octaves) {
	ERR_FAIL_COND_MSG(p_octaves < 1 || p_octaves > 16, "LandscapeVoxelGenerator octaves must be between 1 and 16. Refusing to set.");
	_noise.octaves = p_octaves;
	emit_changed();
}

void LandscapeVoxelGenerator::set_persistence(const real_t p_persistence) {
	_noise.persistence = p_persistence;
	emit_changed();
}

void LandscapeVoxelGenerator::set_vertical_scale(const real_t p_vertical_scale) {
	ERR_FAIL_COND_MSG(p_vertical_scale <= 0.0f, "LandscapeVoxelGenerator scales must be positive. Refusing to set.");
	_noise.scale.y = p_vertical_scale;
	emit_changed();
}

void LandscapeVoxelGenerator::set_horizontal_scale(const real_t p_horizontal_scale) {
	ERR_FAIL_COND_MSG(p_horizontal_scale <= 0.0f, "LandscapeVoxelGenerator scales must be positive. Refusing to set.");
	_noise.scale.x = p_horizontal_scale;
	_noise.scale.z = p_horizontal_scale;
	_noise.scale.w = p_horizontal_scale;
	emit_changed();
}

void LandscapeVoxelGenerator::set_intensity(const real_t p_intensity) {
	_noise.intensity = p_intensity;
	emit_changed();
}

void LandscapeVoxelGenerator::set_materials(const PackedInt32Array &p_materials) {
	for (const int32_t material : p_materials) {
		ERR_FAIL_COND_MSG((int32_t)(VoxelMaterial)material != material, "LandscapeVoxelGenerator materials must fit in the material range. Refusing to set.");
	}
	_materials = p_materials;
	emit_changed();
}

void LandscapeVoxelGenerator::set_material_cutoffs(const PackedFloat32Array &p_material_cutoffs) {
	_material_cutoffs = p_material_cutoffs;
	emit_changed();
}

void LandscapeVoxelGenerator::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_seed"), &LandscapeVoxelGenerator::get_seed);
	ClassDB::bind_method(D_METHOD("set_seed", "seed"), &LandscapeVoxelGenerator::set_seed);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "seed"), "set_seed", "get_seed");

	ClassDB::bind_method(D_METHOD("get_octaves"), &LandscapeVoxelGenerator::get_octaves);
	ClassDB::bind_method(D_METHOD("set_octaves", "octaves"), &LandscapeVoxelGenerator::set_octaves);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "octaves", PROPERTY_HINT_RANGE, "1,16,1"), "set_octaves", "get_octaves");

	ClassDB::bind_method(D_METHOD("get_persistence"), &LandscapeVoxelGenerator::get_persistence);
	ClassDB::bind_method(D_METHOD("set_persistence", "persistence"), &LandscapeVoxelGenerator::set_persistence);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "persistence", PROPERTY_HINT_RANGE, "0,1,0.01,or_greater"), "set_persistence", "get_persistence");

	ClassDB::bind_method(D_METHOD("get_vertical_scale"), &LandscapeVoxelGenerator::get_vertical_scale);
	ClassDB::bind_method(D_METHOD("set_vertical_scale", "vertical_scale"), &LandscapeVoxelGenerator::set_vertical_scale);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "vertical_scale", PROPERTY_HINT_RANGE, "0.1,64,0.01,or_greater"), "set_vertical_scale", "get_vertical_scale");

	ClassDB::bind_method(D_METHOD("get_horizontal_scale"), &LandscapeVoxelGenerator::get_horizontal_scale);
	ClassDB::bind_method(D_METHOD("set_horizontal_scale", "horizontal_scale"), &LandscapeVoxelGenerator::set_horizontal_scale);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "horizontal_scale", PROPERTY_HINT_RANGE, "0.1,64,0.01,or_greater"), "set_horizontal_scale", "get_horizontal_scale");

	ClassDB::bind_method(D_METHOD("get_intensity"), &LandscapeVoxelGenerator::get_intensity);
	ClassDB::bind_method(D_METHOD("set_intensity", "intensity"), &LandscapeVoxelGenerator::set_intensity);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "intensity", PROPERTY_HINT_RANGE, "0,64,0.01,or_greater"), "set_intensity", "get_intensity");

	ClassDB::bind_method(D_METHOD("get_materials"), &LandscapeVoxelGenerator::get_materials);
	ClassDB::bind_method(D_METHOD("set_materials", "materials"), &LandscapeVoxelGenerator::set_materials);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT32_ARRAY, "materials"), "set_materials", "get_materials");

	ClassDB::bind_method(D_METHOD("get_material_cutoffs"), &LandscapeVoxelGenerator::get_material_cutoffs);
	ClassDB::bind_method(D_METHOD("set_material_cutoffs", "material_cutoffs"), &LandscapeVoxelGenerator::set_material_cutoffs);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_FLOAT32_ARRAY, "material_cutoffs"), "set_material_cutoffs", "get_material_cutoffs");
}
