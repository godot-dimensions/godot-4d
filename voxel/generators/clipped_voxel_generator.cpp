#include "clipped_voxel_generator.h"

// The result at a point where the base and modifier have these materials.
static VoxelMaterial _clipped_material(const VoxelMaterial p_base, const VoxelMaterial p_modifier) {
	if (p_base == VoxelMaterial::UNDEFINED) {
		return VoxelMaterial::UNDEFINED;
	}
	return p_modifier != VoxelMaterial::UNDEFINED ? p_modifier : p_base;
}

VoxelMaterial ClippedVoxelGenerator::get_material(const Vector4i &p_voxel) const {
	if (_base.is_null()) {
		return VoxelMaterial::UNDEFINED;
	}
	const VoxelMaterial base = _base->get_material(p_voxel);
	if (base == VoxelMaterial::UNDEFINED || _modifier.is_null()) {
		return base;
	}
	return _clipped_material(base, _modifier->get_material(p_voxel));
}

// How much of a surface the boundary between two result materials carries:
// the border of the defined region outranks a face between materials, which
// outranks a seam between two materials needing no face.
static int _transition_significance(const VoxelMaterial p_before, const VoxelMaterial p_after) {
	if (p_before == VoxelMaterial::UNDEFINED || p_after == VoxelMaterial::UNDEFINED) {
		return 2;
	}
	return get_face_between(p_before, p_after) != VoxelFace::NONE ? 1 : 0;
}

VoxelEdgeData ClippedVoxelGenerator::get_edge_data(const Vector4i &p_voxel, const int p_axis) const {
	ERR_FAIL_COND_V(_base.is_null(), VoxelEdgeData());
	Vector4i upper_voxel = p_voxel;
	upper_voxel[p_axis]++;
	const VoxelMaterial base_lower = _base->get_material(p_voxel);
	const VoxelMaterial base_upper = _base->get_material(upper_voxel);
	const VoxelMaterial modifier_lower = _modifier.is_valid() ? _modifier->get_material(p_voxel) : VoxelMaterial::UNDEFINED;
	const VoxelMaterial modifier_upper = _modifier.is_valid() ? _modifier->get_material(upper_voxel) : VoxelMaterial::UNDEFINED;
	const bool base_changes = base_lower != base_upper;
	const bool modifier_changes = modifier_lower != modifier_upper;
	if (!base_changes && !modifier_changes) {
		// Never the case on an active edge.
		return VoxelEdgeData();
	}
	if (!base_changes) {
		// The base is uniformly defined here, so the result changes exactly
		// where the modifier does.
		return _modifier->get_edge_data(p_voxel, p_axis);
	}
	if (!modifier_changes) {
		// A uniform modifier leaves the base's transition as the only one,
		// though a defined modifier hides its materials.
		return _base->get_edge_data(p_voxel, p_axis);
	}
	// Both change, splitting the edge into three segments whose result
	// materials follow from which transition comes first.
	const VoxelEdgeData base_data = _base->get_edge_data(p_voxel, p_axis);
	const VoxelEdgeData modifier_data = _modifier->get_edge_data(p_voxel, p_axis);
	const bool base_first = base_data.position <= modifier_data.position;
	const VoxelMaterial segments[3] = {
		_clipped_material(base_lower, modifier_lower),
		_clipped_material(base_first ? base_upper : base_lower, base_first ? modifier_lower : modifier_upper),
		_clipped_material(base_upper, modifier_upper),
	};
	const bool first_visible = segments[0] != segments[1];
	const bool second_visible = segments[1] != segments[2];
	const VoxelEdgeData &first_data = base_first ? base_data : modifier_data;
	const VoxelEdgeData &second_data = base_first ? modifier_data : base_data;
	if (first_visible != second_visible) {
		return first_visible ? first_data : second_data;
	}
	if (!first_visible) {
		// No visible transition: never the case on an active edge.
		return base_data;
	}
	// Both transitions appear in the result, but only one surface can be
	// stored: keep the more significant one, or the modifier's boundary when
	// they rank equally.
	const int first_significance = _transition_significance(segments[0], segments[1]);
	const int second_significance = _transition_significance(segments[1], segments[2]);
	if (first_significance != second_significance) {
		return first_significance > second_significance ? first_data : second_data;
	}
	return base_first ? second_data : first_data;
}

void ClippedVoxelGenerator::set_base(const Ref<VoxelGenerator> &p_base) {
	_base = p_base;
	emit_changed();
}

void ClippedVoxelGenerator::set_modifier(const Ref<VoxelGenerator> &p_modifier) {
	_modifier = p_modifier;
	emit_changed();
}

void ClippedVoxelGenerator::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_base"), &ClippedVoxelGenerator::get_base);
	ClassDB::bind_method(D_METHOD("set_base", "base"), &ClippedVoxelGenerator::set_base);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "base", PROPERTY_HINT_RESOURCE_TYPE, "VoxelGenerator"), "set_base", "get_base");

	ClassDB::bind_method(D_METHOD("get_modifier"), &ClippedVoxelGenerator::get_modifier);
	ClassDB::bind_method(D_METHOD("set_modifier", "modifier"), &ClippedVoxelGenerator::set_modifier);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "modifier", PROPERTY_HINT_RESOURCE_TYPE, "VoxelGenerator"), "set_modifier", "get_modifier");
}
