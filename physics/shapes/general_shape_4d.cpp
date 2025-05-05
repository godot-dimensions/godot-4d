#include "general_shape_4d.h"

bool GeneralShape4D::_warnings_enabled = true;

#define GENERAL_SHAPE_4D_CURVE_ERR_WARN(m_curve, m_ret)                                                                          \
	ERR_FAIL_COND_V(m_curve.is_null(), m_ret);                                                                                   \
	if (_warnings_enabled) {                                                                                                     \
		if (m_curve->get_exponent() != 2.0) {                                                                                    \
			WARN_PRINT("GeneralShape4D: Custom curve exponents are not currently supported by this class, and have no effect."); \
		}                                                                                                                        \
		if (m_curve->get_taper_count() > 0) {                                                                                    \
			WARN_PRINT("GeneralShape4D: Curve tapering is not currently supported by this class, and have no effect.");          \
		}                                                                                                                        \
	}

#define GENERAL_SHAPE_4D_RADII_WARN(m_radii, m_radii_average, m_msg)                                                                                                                                                                                         \
	if (_warnings_enabled && ((m_radii.x != 0.0f && m_radii.x != m_radii_average) || (m_radii.y != 0.0f && m_radii.y != m_radii_average) || (m_radii.z != 0.0f && m_radii.z != m_radii_average) || (m_radii.w != 0.0f && m_radii.w != m_radii_average))) {   \
		WARN_PRINT("GeneralShape4D." m_msg " This approximation works well for ellipsoids that are close to (hyper)spheres, but diverges for highly elongated ellipsoids. You can disable this warning with `GeneralShape4D.set_warnings_enabled(false)`."); \
	}

void GeneralShape4D::set_base_size(const Vector4 &p_base_size) {
	_base_size = p_base_size.abs();
}

void GeneralShape4D::set_base_half_extents(const Vector4 &p_base_half_extents) {
	_base_size = p_base_half_extents.abs() * 2.0f;
}

/* clang-format off */
real_t GeneralShape4D::get_hypervolume() const {
	// Start with the volume of the base box.
	real_t volume = _base_size.x * _base_size.y * _base_size.z * _base_size.w;
	const int64_t curve_count = _curves.size();
	if (curve_count == 0) {
		return volume;
	}
	// Note: This function does not handle Steinmetz solids, or any other case of curves sharing axes.
	const Ref<GeneralShapeCurve4D> curve = _curves[0];
	GENERAL_SHAPE_4D_CURVE_ERR_WARN(curve, volume);
	const Vector4 radii = curve->get_radii();
	const int curve_dimension = curve->get_radii_dimension();
	// Curve dimension 4 is for 4D hyperspheres, also used in 4D capsules and rounded boxes.
	if (curve_dimension == 4) {
		// 4D part.
		volume += (0.125 * Math_TAU * Math_TAU) * (radii.x * radii.y * radii.z * radii.w);
		// 3D part.
		volume += (Math_TAU * 2.0 / 3.0) * (
			radii.x * radii.y * radii.z * _base_size.w +
			radii.x * radii.y * _base_size.z * radii.w +
			radii.x * _base_size.y * radii.z * radii.w +
			_base_size.x * radii.y * radii.z * radii.w
		);
		// 2D part.
		volume += Math_PI * (
			radii.x * radii.y * _base_size.z * _base_size.w +
			radii.x * _base_size.y * radii.z * _base_size.w +
			radii.x * _base_size.y * _base_size.z * radii.w +
			_base_size.x * radii.y * radii.z * _base_size.w +
			_base_size.x * radii.y * _base_size.z * radii.w +
			_base_size.x * _base_size.y * radii.z * radii.w
		);
		// 1D part.
		volume += 2.0 * (
			radii.x * _base_size.y * _base_size.z * _base_size.w +
			_base_size.x * radii.y * _base_size.z * _base_size.w +
			_base_size.x * _base_size.y * radii.z * _base_size.w +
			_base_size.x * _base_size.y * _base_size.z * radii.w
		);
		ERR_FAIL_COND_V_MSG(curve_count > 1, volume, "GeneralShape4D.get_hypervolume: Steinmetz solids are not supported.");
		return volume;
	}
	const PackedInt32Array zero_axes = curve->get_zero_axes();
	const PackedInt32Array used_axes = curve->get_used_axes();
	// Curve dimension 3 is for 3D spheres, as used in 4D cylinders/spherinders.
	if (curve_dimension == 3) {
		CRASH_COND(zero_axes.size() != 1 || used_axes.size() != 3);
		// 3D part.
		real_t volume_3d = (Math_TAU * 2.0 / 3.0) * (radii[used_axes[0]] * radii[used_axes[1]] * radii[used_axes[2]]);
		// 2D part.
		volume_3d += Math_PI * (
			radii[used_axes[0]] * radii[used_axes[1]] * _base_size[used_axes[2]] +
			radii[used_axes[0]] * _base_size[used_axes[1]] * radii[used_axes[2]] +
			_base_size[used_axes[0]] * radii[used_axes[1]] * radii[used_axes[2]]
		);
		// 1D part.
		volume_3d += 2.0 * (
			radii[used_axes[0]] * _base_size[used_axes[1]] * _base_size[used_axes[2]] +
			_base_size[used_axes[0]] * radii[used_axes[1]] * _base_size[used_axes[2]] +
			_base_size[used_axes[0]] * _base_size[used_axes[1]] * radii[used_axes[2]]
		);
		volume += volume_3d * _base_size[zero_axes[0]];
		ERR_FAIL_COND_V_MSG(curve_count > 1, volume, "GeneralShape4D.get_hypervolume: Steinmetz solids are not supported.");
		return volume;
	}
	// Curve dimension 2 is for 2D circles, as used in 4D cubinders and duocylinders.
	if (curve_dimension == 2) {
		CRASH_COND(zero_axes.size() != 2 || used_axes.size() != 2);
		// 2D part.
		real_t volume0_2d = Math_PI * (radii[used_axes[0]] * radii[used_axes[1]]);
		// 1D part.
		volume0_2d += 2.0 * (
			radii[used_axes[0]] * _base_size[used_axes[1]] +
			_base_size[used_axes[0]] * radii[used_axes[1]]
		);
		volume += volume0_2d * (_base_size[zero_axes[0]] * _base_size[zero_axes[1]]);
		if (curve_count == 1) {
			// If this is the only curve, we can return now.
			return volume;
		}
		// There may be more curves in the case of a duocylinder.
		const Ref<GeneralShapeCurve4D> curve1 = _curves[1];
		ERR_FAIL_COND_V(curve1.is_null(), volume);
		const Vector4 radii1 = curve1->get_radii();
		const PackedInt32Array zero_axes1 = curve1->get_zero_axes();
		const PackedInt32Array used_axes1 = curve1->get_used_axes();
		if (zero_axes1.size() == 2 && used_axes1.size() == 2 && (radii * radii1 == Vector4())) {
			real_t volume1_2d = Math_PI * (radii1[used_axes1[0]] * radii1[used_axes1[1]]);
			volume1_2d += 2.0 * (
				radii1[used_axes1[0]] * _base_size[used_axes1[1]] +
				_base_size[used_axes1[0]] * radii1[used_axes1[1]]
			);
			volume += volume1_2d * (_base_size[zero_axes1[0]] * _base_size[zero_axes1[1]]);
			// Square meters times square meters gives quartic meters.
			volume += volume0_2d * volume1_2d;
			return volume;
		}
		ERR_FAIL_V_MSG(volume, "GeneralShape4D.get_hypervolume: Steinmetz solids are not supported.");
	}
	// Curve dimension 1 is not really a valid curve, but we can handle it anyway.
	if (curve_dimension == 1) {
		CRASH_COND(zero_axes.size() != 3 || used_axes.size() != 1);
		// 1D part.
		real_t volume_1d = 2.0 * (radii[used_axes[0]]);
		volume += volume_1d * (_base_size[zero_axes[0]] * _base_size[zero_axes[1]] * _base_size[zero_axes[2]]);
		return volume;
	}
	ERR_FAIL_V_MSG(volume, "GeneralShape4D.get_hypervolume: Invalid curve dimension.");
}

real_t GeneralShape4D::get_surface_volume() const {
	// Start with the surface area of the base box.
	real_t surface = 2.0f * (
		_base_size.x * _base_size.y * _base_size.z +
		_base_size.x * _base_size.y * _base_size.w +
		_base_size.x * _base_size.z * _base_size.w +
		_base_size.y * _base_size.z * _base_size.w);
	const int64_t curve_count = _curves.size();
	if (curve_count == 0) {
		return surface;
	}
	// Note: This function does not handle Steinmetz solids, or any other case of curves sharing axes.
	const Ref<GeneralShapeCurve4D> curve = _curves[0];
	GENERAL_SHAPE_4D_CURVE_ERR_WARN(curve, surface);
	const Vector4 radii = curve->get_radii();
	const real_t radii_sum = curve->get_radii_sum();
	const int curve_dimension = curve->get_radii_dimension();
	const real_t radii_average = radii_sum / curve_dimension;
	GENERAL_SHAPE_4D_RADII_WARN(radii, radii_average, "get_surface_volume: There is no closed-form solution for the surface area of ellipsoids. An approximation will be used instead: the surface area of a (hyper)sphere with the average radius of the ellipsoid.");
	// Curve dimension 4 is for 4D hyperspheres, also used in 4D capsules and rounded boxes.
	if (curve_dimension == 4) {
		// 4D curve part.
		surface += (0.5 * Math_TAU * Math_TAU) * (radii_average * radii_average * radii_average);
		// 3D curve part.
		surface += (2.0 * Math_TAU) * (radii_average * radii_average) * (_base_size.x + _base_size.y + _base_size.z + _base_size.w);
		// 2D curve part.
		surface += Math_TAU * (radii_average * radii_average) * (
			_base_size.x * _base_size.y +
			_base_size.x * _base_size.z +
			_base_size.x * _base_size.w +
			_base_size.y * _base_size.z +
			_base_size.y * _base_size.w +
			_base_size.z * _base_size.w
		);
		// No 1D curve part, since the 1D curve part is already included in the base box surface area.
		ERR_FAIL_COND_V_MSG(curve_count > 1, surface, "GeneralShape4D.get_surface_volume: Steinmetz solids are not supported.");
		return surface;
	}
	const PackedInt32Array zero_axes = curve->get_zero_axes();
	const PackedInt32Array used_axes = curve->get_used_axes();
	// Curve dimension 3 is for 3D spheres, as used in 4D cylinders/spherinders.
	if (curve_dimension == 3) {
		CRASH_COND(zero_axes.size() != 1 || used_axes.size() != 3);
		// 3D curve part.
		surface += (2.0 * Math_TAU) * (radii_average * radii_average) * _base_size[zero_axes[0]];
		// 2D curve part.
		surface += Math_TAU * radii_average * _base_size[zero_axes[0]] * (_base_size[used_axes[0]] + _base_size[used_axes[1]] + _base_size[used_axes[2]]);
		// No 1D curve part, since the 1D curve part is already included in the base box surface area.
		// 3D flat part.
		real_t flat_surface = (Math_TAU * 2.0 / 3.0) * (radii_average * radii_average * radii_average);
		// 2D flat part.
		flat_surface += Math_PI * (radii_average * radii_average) * (_base_size[used_axes[0]] + _base_size[used_axes[1]] + _base_size[used_axes[2]]);
		// 1D flat part.
		flat_surface += (2.0 * radii_average) * (
			_base_size[used_axes[0]] * _base_size[used_axes[1]] +
			_base_size[used_axes[0]] * _base_size[used_axes[2]] +
			_base_size[used_axes[1]] * _base_size[used_axes[2]]
		);
		// The flat part exists on both sides of the cylinder, so we need to multiply it by 2.
		surface += flat_surface * 2.0;
		ERR_FAIL_COND_V_MSG(curve_count > 1, surface, "GeneralShape4D.get_surface_volume: Steinmetz solids are not supported.");
		return surface;
	}
	// Curve dimension 2 is for 2D circles, as used in 4D cubinders and duocylinders.
	if (curve_dimension == 2) {
		CRASH_COND(zero_axes.size() != 2 || used_axes.size() != 2);
		// 2D curve part.
		surface += Math_TAU * radii_average * _base_size[zero_axes[0]] * _base_size[zero_axes[1]];
		// No 1D curve part, since the 1D curve part is already included in the base box surface area.
		// 2D flat part.
		real_t flat_surface0 = Math_PI * (radii_average * radii_average);
		// 1D flat part.
		flat_surface0 += (2.0 * radii_average) * (_base_size[used_axes[0]] + _base_size[used_axes[1]]);
		// The flat part exists on multiple sides of the cubinder, so we need to multiply it by 2.
		surface += 2.0 * flat_surface0 * (_base_size[zero_axes[0]] + _base_size[zero_axes[1]]);
		if (curve_count == 1) {
			// If this is the only curve, we can return now.
			return surface;
		}
		// There may be more curves in the case of a duocylinder.
		const Ref<GeneralShapeCurve4D> curve1 = _curves[1];
		GENERAL_SHAPE_4D_CURVE_ERR_WARN(curve1, surface);
		const Vector4 radii1 = curve1->get_radii();
		const PackedInt32Array zero_axes1 = curve1->get_zero_axes();
		const PackedInt32Array used_axes1 = curve1->get_used_axes();
		if (zero_axes1.size() == 2 && used_axes1.size() == 2 && (radii * radii1 == Vector4())) {
			const real_t radii1_sum = curve1->get_radii_sum();
			const int curve1_dimension = curve1->get_radii_dimension();
			const real_t radii1_average = radii1_sum / curve1_dimension;
			GENERAL_SHAPE_4D_RADII_WARN(radii1, radii1_average, "get_surface_volume: There is no closed-form solution for the surface area of ellipse. An approximation will be used instead: the surface area of a circle with the average radius of the ellipse.");
			// Surface provided by the second curve of the duocylinder.
			surface += Math_TAU * radii1_average * _base_size[zero_axes1[0]] * _base_size[zero_axes1[1]];
			real_t flat_surface1 = Math_PI * (radii1_average * radii1_average);
			flat_surface1 += (2.0 * radii1_average) * (_base_size[used_axes1[0]] + _base_size[used_axes1[1]]);
			surface += 2.0 * flat_surface1 * (_base_size[zero_axes1[0]] + _base_size[zero_axes1[1]]);
			// Duocylinder-specific surface volume, see DuocylinderShape4D.
			surface += (2.0 * Math_PI * Math_PI) * (radii_average * radii1_average) * (radii_average + radii1_average);
		}
		ERR_FAIL_V_MSG(surface, "GeneralShape4D.get_surface_volume: Steinmetz solids are not supported.");
	}
	// Curve dimension 1 is not really a valid curve, but we can handle it anyway.
	if (curve_dimension == 1) {
		CRASH_COND(zero_axes.size() != 3 || used_axes.size() != 1);
		// 1D part.
		surface += 2.0 * (radii[used_axes[0]]) * (_base_size[zero_axes[0]] * _base_size[zero_axes[1]] * _base_size[zero_axes[2]]);
		return surface;
	}
	ERR_FAIL_V_MSG(surface, "GeneralShape4D.get_surface_volume: Invalid curve dimension.");
}
/* clang-format on */

Rect4 GeneralShape4D::get_rect_bounds(const Transform4D &p_to_target) const {
	const Vector4 half_extents = get_base_half_extents();
	const int64_t curve_count = _curves.size();
	Rect4 rect_bounds = Rect4(p_to_target.origin, Vector4(0, 0, 0, 0));
	if (curve_count == 0) {
		for (int x = -1; x < 2; x += 2) {
			for (int y = -1; y < 2; y += 2) {
				for (int z = -1; z < 2; z += 2) {
					for (int w = -1; w < 2; w += 2) {
						const Vector4 sign = Vector4(x, y, z, w);
						const Vector4 corner = sign * half_extents;
						rect_bounds.expand_self_to_point(p_to_target.xform(corner));
					}
				}
			}
		}
	}
	// Include the axis-aligned radii of the curves in the bounds.
	// This is not perfect, but it should be good enough for most cases.
	Vector4 farthest_extent_radii;
	for (int curve_index = 0; curve_index < _curves.size(); curve_index++) {
		const Ref<GeneralShapeCurve4D> curve = _curves[curve_index];
		GENERAL_SHAPE_4D_CURVE_ERR_WARN(curve, rect_bounds);
		farthest_extent_radii = farthest_extent_radii.max(curve->get_radii());
	}
	for (int x = -1; x < 2; x += 2) {
		for (int y = -1; y < 2; y += 2) {
			for (int z = -1; z < 2; z += 2) {
				for (int w = -1; w < 2; w += 2) {
					const Vector4 sign = Vector4(x, y, z, w);
					const Vector4 corner = sign * half_extents;
					for (int i = 0; i < 4; i++) {
						Vector4 curve_extent_radius;
						curve_extent_radius[i] = sign[i] * farthest_extent_radii[i];
						const Vector4 local_edge = corner + curve_extent_radius;
						rect_bounds.expand_self_to_point(p_to_target.xform(local_edge));
					}
				}
			}
		}
	}
	return rect_bounds;
}

Vector4 GeneralShape4D::get_nearest_point(const Vector4 &p_point) const {
	const Vector4 half_extents = get_base_half_extents();
	Vector4 local_offset_abs = p_point.abs();
	Vector4 nearest_point_abs = Vector4();
	// First, account for the base box.
	for (int i = 0; i < 4; i++) {
		if (local_offset_abs[i] > half_extents[i]) {
			nearest_point_abs[i] = half_extents[i];
			local_offset_abs[i] -= half_extents[i];
		} else {
			nearest_point_abs[i] = local_offset_abs[i];
			local_offset_abs[i] = 0.0f;
		}
	}
	// Next, check the curves, if any. Note: This does not handle Steinmetz solids, or any other case of curves sharing axes.
	for (int64_t curve_index = 0; curve_index < _curves.size(); curve_index++) {
		const Ref<GeneralShapeCurve4D> curve = _curves[curve_index];
		GENERAL_SHAPE_4D_CURVE_ERR_WARN(curve, Vector4());
		const Vector4 radii = curve->get_radii();
		const PackedInt32Array used_axes = curve->get_used_axes();
		real_t ellipsoid_distance_squared = 0.0f;
		for (int64_t used_index = 0; used_index < used_axes.size(); used_index++) {
			const int32_t axis = used_axes[used_index];
			ellipsoid_distance_squared += (local_offset_abs[axis] * local_offset_abs[axis]) / (radii[axis] * radii[axis]);
		}
		if (ellipsoid_distance_squared <= 1.0f) {
			// The point is inside the ellipsoid on these axes, and we are done with them.
			for (int64_t used_index = 0; used_index < used_axes.size(); used_index++) {
				const int32_t axis = used_axes[used_index];
				nearest_point_abs[axis] += local_offset_abs[axis];
				local_offset_abs[axis] = 0.0f;
			}
			continue;
		}
		// The point is outside the ellipsoid.
		const real_t radii_sum = curve->get_radii_sum();
		const int curve_dimension = curve->get_radii_dimension();
		const real_t radii_average = radii_sum / curve_dimension;
		GENERAL_SHAPE_4D_RADII_WARN(radii, radii_average, "get_nearest_point: There is no closed-form solution for the nearest point on ellipsoids. An approximation will be used instead: the nearest point on a scaled (hyper)sphere with the average radius of the ellipsoid.");
		Vector4 scaled_offset_abs = Vector4();
		for (int64_t used_index = 0; used_index < used_axes.size(); used_index++) {
			const int32_t axis = used_axes[used_index];
			scaled_offset_abs[axis] = local_offset_abs[axis] / radii[axis];
		}
		scaled_offset_abs = scaled_offset_abs.normalized();
		for (int64_t used_index = 0; used_index < used_axes.size(); used_index++) {
			const int32_t axis = used_axes[used_index];
			const real_t rescaled_offset_value = scaled_offset_abs[axis] * radii[axis];
			nearest_point_abs[axis] += rescaled_offset_value;
			local_offset_abs[axis] -= rescaled_offset_value;
		}
	}
	return nearest_point_abs * p_point.sign();
}

Vector4 GeneralShape4D::get_support_point(const Vector4 &p_direction) const {
	// First, account for the base box.
	const Vector4 half_extents = get_base_half_extents();
	Vector4 support = Vector4(
			(p_direction.x > 0.0f) ? half_extents.x : -half_extents.x,
			(p_direction.y > 0.0f) ? half_extents.y : -half_extents.y,
			(p_direction.z > 0.0f) ? half_extents.z : -half_extents.z,
			(p_direction.w > 0.0f) ? half_extents.w : -half_extents.w);
	// Next, check the curves, if any. Note: This does not handle Steinmetz solids, or any other case of curves sharing axes.
	const int64_t curve_count = _curves.size();
	for (int64_t curve_index = 0; curve_index < curve_count; curve_index++) {
		const Ref<GeneralShapeCurve4D> curve = _curves[curve_index];
		GENERAL_SHAPE_4D_CURVE_ERR_WARN(curve, Vector4());
		const Vector4 radii = curve->get_radii();
		const PackedInt32Array used_axes = curve->get_used_axes();
		// Based on this: https://math.stackexchange.com/questions/2267688/locating-a-line-perpendicular-to-an-ellipse
		Vector4 ellipsoid_support = Vector4();
		real_t ellipsoid_support_len_squared = 0.0f;
		for (int64_t used_index = 0; used_index < used_axes.size(); used_index++) {
			const int axis = used_axes[used_index];
			const real_t radius_squared = radii[axis] * radii[axis];
			ellipsoid_support[axis] = radius_squared;
			ellipsoid_support_len_squared += (p_direction[axis] * p_direction[axis]) * radius_squared;
		}
		support += p_direction * ellipsoid_support / Math::sqrt(ellipsoid_support_len_squared);
	}
	return support;
}

bool GeneralShape4D::has_point(const Vector4 &p_point) const {
	const Vector4 half_extents = get_base_half_extents();
	Vector4 local_offset_abs = p_point.abs();
	// First, account for the base box.
	for (int i = 0; i < 4; i++) {
		if (local_offset_abs[i] > half_extents[i]) {
			local_offset_abs[i] -= half_extents[i];
		} else {
			local_offset_abs[i] = 0.0f;
		}
	}
	// Next, check the curves, if any. Note: This does not handle Steinmetz solids, or any other case of curves sharing axes.
	for (int64_t curve_index = 0; curve_index < _curves.size(); curve_index++) {
		const Ref<GeneralShapeCurve4D> curve = _curves[curve_index];
		GENERAL_SHAPE_4D_CURVE_ERR_WARN(curve, false);
		const Vector4 radii = curve->get_radii();
		const PackedInt32Array used_axes = curve->get_used_axes();
		real_t ellipsoid_distance_squared = 0.0f;
		for (int64_t used_index = 0; used_index < used_axes.size(); used_index++) {
			const int axis = used_axes[used_index];
			ellipsoid_distance_squared += (local_offset_abs[axis] * local_offset_abs[axis]) / (radii[axis] * radii[axis]);
		}
		if (ellipsoid_distance_squared > 1.0f) {
			// The point is outside the ellipsoid.
			return false;
		}
		// Since we know the point is inside the ellipsoid on these axes, we are done with them.
		for (int64_t used_index = 0; used_index < used_axes.size(); used_index++) {
			local_offset_abs[used_axes[used_index]] = 0.0f;
		}
	}
	return local_offset_abs == Vector4();
}

void GeneralShape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_base_size"), &GeneralShape4D::get_base_size);
	ClassDB::bind_method(D_METHOD("set_base_size", "base_size"), &GeneralShape4D::set_base_size);
	ClassDB::bind_method(D_METHOD("get_base_half_extents"), &GeneralShape4D::get_base_half_extents);
	ClassDB::bind_method(D_METHOD("set_base_half_extents", "base_half_extents"), &GeneralShape4D::set_base_half_extents);
	ClassDB::bind_method(D_METHOD("get_curves"), &GeneralShape4D::get_curves);
	ClassDB::bind_method(D_METHOD("set_curves", "curves"), &GeneralShape4D::set_curves);

	ClassDB::bind_static_method("GeneralShape4D", D_METHOD("set_warnings_enabled", "warnings_enabled"), &GeneralShape4D::set_warnings_enabled);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "base_size"), "set_base_size", "get_base_size");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "base_half_extents", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NONE), "set_base_half_extents", "get_base_half_extents");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "curves", PROPERTY_HINT_ARRAY_TYPE, "GeneralShapeCurve4D"), "set_curves", "get_curves");
}
