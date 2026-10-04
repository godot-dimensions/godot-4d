#include "clipped_voxel_generator_4d.h"

// The result at a point where the base and modifier have these materials.
VoxelMaterial4D ClippedVoxelGenerator4D::_clipped_material(const VoxelMaterial4D p_base, const VoxelMaterial4D p_modifier) {
	if (p_base == VoxelMaterial4D::UNDEFINED) {
		return VoxelMaterial4D::UNDEFINED;
	}
	return p_modifier != VoxelMaterial4D::UNDEFINED ? p_modifier : p_base;
}

VoxelMaterial4D ClippedVoxelGenerator4D::get_material(const Vector4i &p_voxel) const {
	if (_base.is_null()) {
		return VoxelMaterial4D::UNDEFINED;
	}
	const VoxelMaterial4D base = _base->get_material(p_voxel);
	if (base == VoxelMaterial4D::UNDEFINED || _modifier.is_null()) {
		return base;
	}
	return _clipped_material(base, _modifier->get_material(p_voxel));
}

// How much of a surface the boundary between two result materials carries:
// the border of the defined region outranks a face between materials, which
// outranks a seam between two materials needing no face.
int ClippedVoxelGenerator4D::_transition_significance(const VoxelMaterial4D p_before, const VoxelMaterial4D p_after) {
	if (p_before == VoxelMaterial4D::UNDEFINED || p_after == VoxelMaterial4D::UNDEFINED) {
		return 2;
	}
	return VoxelMaterialUtil4D::get_face_between(p_before, p_after) != VoxelFace4D::NONE ? 1 : 0;
}

VoxelEdgeData4D ClippedVoxelGenerator4D::get_edge_data(const Vector4i &p_voxel, const int p_axis) const {
	ERR_FAIL_COND_V(_base.is_null(), VoxelEdgeData4D());
	Vector4i upper_voxel = p_voxel;
	upper_voxel[p_axis]++;
	const VoxelMaterial4D base_lower = _base->get_material(p_voxel);
	const VoxelMaterial4D base_upper = _base->get_material(upper_voxel);
	const VoxelMaterial4D modifier_lower = _modifier.is_valid() ? _modifier->get_material(p_voxel) : VoxelMaterial4D::UNDEFINED;
	const VoxelMaterial4D modifier_upper = _modifier.is_valid() ? _modifier->get_material(upper_voxel) : VoxelMaterial4D::UNDEFINED;
	const bool base_changes = base_lower != base_upper;
	const bool modifier_changes = modifier_lower != modifier_upper;
	if (!base_changes && !modifier_changes) {
		// Never the case on an active edge.
		return VoxelEdgeData4D();
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
	const VoxelEdgeData4D base_data = _base->get_edge_data(p_voxel, p_axis);
	const VoxelEdgeData4D modifier_data = _modifier->get_edge_data(p_voxel, p_axis);
	const bool base_first = base_data.position <= modifier_data.position;
	const VoxelMaterial4D segments[3] = {
		_clipped_material(base_lower, modifier_lower),
		_clipped_material(base_first ? base_upper : base_lower, base_first ? modifier_lower : modifier_upper),
		_clipped_material(base_upper, modifier_upper),
	};
	const bool first_visible = segments[0] != segments[1];
	const bool second_visible = segments[1] != segments[2];
	const VoxelEdgeData4D &first_data = base_first ? base_data : modifier_data;
	const VoxelEdgeData4D &second_data = base_first ? modifier_data : base_data;
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

bool ClippedVoxelGenerator4D::contains_generator(const VoxelGenerator4D *p_generator) const {
	if (VoxelGenerator4D::contains_generator(p_generator)) {
		return true;
	}
	return (_base.is_valid() && _base->contains_generator(p_generator)) || (_modifier.is_valid() && _modifier->contains_generator(p_generator));
}

void ClippedVoxelGenerator4D::set_base(const Ref<VoxelGenerator4D> &p_base) {
	ERR_FAIL_COND_MSG(p_base.is_valid() && p_base->contains_generator(this), "ClippedVoxelGenerator4D cannot use itself as its base, directly or through other generators. Refusing to set.");
	_base = p_base;
	emit_changed();
}

void ClippedVoxelGenerator4D::set_modifier(const Ref<VoxelGenerator4D> &p_modifier) {
	ERR_FAIL_COND_MSG(p_modifier.is_valid() && p_modifier->contains_generator(this), "ClippedVoxelGenerator4D cannot use itself as its modifier, directly or through other generators. Refusing to set.");
	_modifier = p_modifier;
	emit_changed();
}

void ClippedVoxelGenerator4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_base"), &ClippedVoxelGenerator4D::get_base);
	ClassDB::bind_method(D_METHOD("set_base", "base"), &ClippedVoxelGenerator4D::set_base);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "base", PROPERTY_HINT_RESOURCE_TYPE, "VoxelGenerator4D"), "set_base", "get_base");

	ClassDB::bind_method(D_METHOD("get_modifier"), &ClippedVoxelGenerator4D::get_modifier);
	ClassDB::bind_method(D_METHOD("set_modifier", "modifier"), &ClippedVoxelGenerator4D::set_modifier);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "modifier", PROPERTY_HINT_RESOURCE_TYPE, "VoxelGenerator4D"), "set_modifier", "get_modifier");
}
