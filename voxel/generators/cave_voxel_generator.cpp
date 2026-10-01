#include "cave_voxel_generator.h"

void CaveVoxelGenerator::_update_threshold() {
	// The tunnel boundary lies where the three fields' combined magnitude
	// reaches the threshold, so near a tunnel's axis, the radius along an
	// axis is the threshold over the fields' combined RMS derivative: sqrt(3)
	// times 0.676 (the measured RMS derivative of one unit-scale octave) per
	// octave term, divided by the axis' scale. The octave terms' amplitudes
	// follow the persistence and their scales double, giving the sum below.
	real_t octave_sum = 0.0f;
	real_t term = 1.0f;
	const real_t ratio = 4.0f * _noise.persistence * _noise.persistence;
	for (int i = 0; i < _noise.octaves; i++) {
		octave_sum += term;
		term *= ratio;
	}
	const real_t octave_factor = Math::sqrt(octave_sum) * (real_t)2.0f / (real_t)(1 << _noise.octaves);
	const real_t threshold = (real_t)0.586f * octave_factor * _diameter / _noise.scale.x;
	_threshold_squared = threshold * threshold;
}

real_t CaveVoxelGenerator::_field_squared(const Vector4 &p_point, const bool p_early_out) const {
	LayeredVoxelNoise noise = _noise;
	real_t total = 0.0f;
	for (int i = 0; i < 3; i++) {
		noise.seed = derive_seed(_noise.seed, (uint32_t)i);
		const real_t value = noise.sample(p_point, nullptr);
		total += value * value;
		// Once over the threshold, the point is outside the tunnels no
		// matter what the remaining fields hold.
		if (p_early_out && total >= _threshold_squared) {
			break;
		}
	}
	return total;
}

// The gradient of the squared field magnitude, up to a constant factor.
Vector4 CaveVoxelGenerator::_field_gradient(const Vector4 &p_point) const {
	LayeredVoxelNoise noise = _noise;
	Vector4 total;
	for (int i = 0; i < 3; i++) {
		noise.seed = derive_seed(_noise.seed, (uint32_t)i);
		Vector4 gradient;
		const real_t value = noise.sample(p_point, &gradient);
		total += value * gradient;
	}
	return total;
}

VoxelMaterial CaveVoxelGenerator::get_material(const Vector4i &p_voxel) const {
	return _field_squared(get_voxel_center(p_voxel), true) < _threshold_squared ? _cave_material : _outer_material;
}

VoxelEdgeData CaveVoxelGenerator::get_edge_data(const Vector4i &p_voxel, const int p_axis) const {
	const Vector4 center = get_voxel_center(p_voxel);
	Vector4 neighbor = center;
	neighbor[p_axis] += 1.0f;
	const real_t field_1 = _field_squared(center, false);
	const real_t field_2 = _field_squared(neighbor, false);
	const real_t crossing = (_threshold_squared - field_1) / (field_2 - field_1);
	Vector4 point = center;
	point[p_axis] += crossing;
	return VoxelEdgeData(_field_gradient(point), crossing);
}

void CaveVoxelGenerator::set_seed(const int p_seed) {
	_noise.seed = (uint32_t)p_seed;
	emit_changed();
}

void CaveVoxelGenerator::set_octaves(const int p_octaves) {
	ERR_FAIL_COND_MSG(p_octaves < 1 || p_octaves > 16, "CaveVoxelGenerator octaves must be between 1 and 16. Refusing to set.");
	_noise.octaves = p_octaves;
	_update_threshold();
	emit_changed();
}

void CaveVoxelGenerator::set_persistence(const real_t p_persistence) {
	_noise.persistence = p_persistence;
	_update_threshold();
	emit_changed();
}

void CaveVoxelGenerator::set_vertical_scale(const real_t p_vertical_scale) {
	ERR_FAIL_COND_MSG(p_vertical_scale <= 0.0f, "CaveVoxelGenerator scales must be positive. Refusing to set.");
	_noise.scale.y = p_vertical_scale;
	_update_threshold();
	emit_changed();
}

void CaveVoxelGenerator::set_horizontal_scale(const real_t p_horizontal_scale) {
	ERR_FAIL_COND_MSG(p_horizontal_scale <= 0.0f, "CaveVoxelGenerator scales must be positive. Refusing to set.");
	_noise.scale.x = p_horizontal_scale;
	_noise.scale.z = p_horizontal_scale;
	_noise.scale.w = p_horizontal_scale;
	_update_threshold();
	emit_changed();
}

void CaveVoxelGenerator::set_diameter(const real_t p_diameter) {
	ERR_FAIL_COND_MSG(p_diameter <= 0.0f, "CaveVoxelGenerator diameter must be positive. Refusing to set.");
	_diameter = p_diameter;
	_update_threshold();
	emit_changed();
}

void CaveVoxelGenerator::set_cave_material(const int p_material) {
	ERR_FAIL_COND_MSG((int)(VoxelMaterial)p_material != p_material, "CaveVoxelGenerator materials must fit in the material range. Refusing to set.");
	_cave_material = (VoxelMaterial)p_material;
	emit_changed();
}

void CaveVoxelGenerator::set_outer_material(const int p_material) {
	ERR_FAIL_COND_MSG((int)(VoxelMaterial)p_material != p_material, "CaveVoxelGenerator materials must fit in the material range. Refusing to set.");
	_outer_material = (VoxelMaterial)p_material;
	emit_changed();
}

void CaveVoxelGenerator::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_seed"), &CaveVoxelGenerator::get_seed);
	ClassDB::bind_method(D_METHOD("set_seed", "seed"), &CaveVoxelGenerator::set_seed);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "seed"), "set_seed", "get_seed");

	ClassDB::bind_method(D_METHOD("get_octaves"), &CaveVoxelGenerator::get_octaves);
	ClassDB::bind_method(D_METHOD("set_octaves", "octaves"), &CaveVoxelGenerator::set_octaves);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "octaves", PROPERTY_HINT_RANGE, "1,16,1"), "set_octaves", "get_octaves");

	ClassDB::bind_method(D_METHOD("get_persistence"), &CaveVoxelGenerator::get_persistence);
	ClassDB::bind_method(D_METHOD("set_persistence", "persistence"), &CaveVoxelGenerator::set_persistence);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "persistence", PROPERTY_HINT_RANGE, "0,1,0.01,or_greater"), "set_persistence", "get_persistence");

	ClassDB::bind_method(D_METHOD("get_vertical_scale"), &CaveVoxelGenerator::get_vertical_scale);
	ClassDB::bind_method(D_METHOD("set_vertical_scale", "vertical_scale"), &CaveVoxelGenerator::set_vertical_scale);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "vertical_scale", PROPERTY_HINT_RANGE, "0.1,64,0.01,or_greater"), "set_vertical_scale", "get_vertical_scale");

	ClassDB::bind_method(D_METHOD("get_horizontal_scale"), &CaveVoxelGenerator::get_horizontal_scale);
	ClassDB::bind_method(D_METHOD("set_horizontal_scale", "horizontal_scale"), &CaveVoxelGenerator::set_horizontal_scale);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "horizontal_scale", PROPERTY_HINT_RANGE, "0.1,64,0.01,or_greater"), "set_horizontal_scale", "get_horizontal_scale");

	ClassDB::bind_method(D_METHOD("get_diameter"), &CaveVoxelGenerator::get_diameter);
	ClassDB::bind_method(D_METHOD("set_diameter", "diameter"), &CaveVoxelGenerator::set_diameter);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "diameter", PROPERTY_HINT_RANGE, "0.5,16,0.01,or_greater"), "set_diameter", "get_diameter");

	ClassDB::bind_method(D_METHOD("get_cave_material"), &CaveVoxelGenerator::get_cave_material);
	ClassDB::bind_method(D_METHOD("set_cave_material", "material"), &CaveVoxelGenerator::set_cave_material);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "cave_material", PROPERTY_HINT_RANGE, "0,255,1"), "set_cave_material", "get_cave_material");

	ClassDB::bind_method(D_METHOD("get_outer_material"), &CaveVoxelGenerator::get_outer_material);
	ClassDB::bind_method(D_METHOD("set_outer_material", "material"), &CaveVoxelGenerator::set_outer_material);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "outer_material", PROPERTY_HINT_RANGE, "0,255,1"), "set_outer_material", "get_outer_material");
}
