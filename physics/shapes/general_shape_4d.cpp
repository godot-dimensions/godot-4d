#include "general_shape_4d.h"

bool GeneralShape4D::_warnings_enabled = true;

#define GENERAL_SHAPE_4D_CURVE_ERR_WARN_TAPER(m_curve, m_ret)                                                      \
	ERR_FAIL_COND_V(m_curve.is_null(), m_ret);                                                                     \
	if (_warnings_enabled && m_curve->get_taper_count() > 0) {                                                     \
		WARN_PRINT("GeneralShape4D: Curve tapering is not currently supported by this class, and has no effect."); \
	}

#define GENERAL_SHAPE_4D_CURVE_ERR_WARN(m_curve, m_ret)                                                                                   \
	GENERAL_SHAPE_4D_CURVE_ERR_WARN_TAPER(m_curve, m_ret)                                                                                 \
	if (_warnings_enabled && m_curve->get_exponent() != 2.0) {                                                                            \
		WARN_PRINT("GeneralShape4D: Custom curve exponents are not fully supported by this function, and the result may be inaccurate."); \
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
		ERR_FAIL_COND_V_MSG(curve_count > 2, surface, "GeneralShape4D.get_surface_volume: Steinmetz solids are not supported.");
		return surface;
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
	// On each target axis, the shape reaches as far as its support point in the direction of that row of the basis.
	// The shape is symmetric around its center, so the bounds are symmetric around the target origin, and only
	// the positive direction is needed. The support point is the sum of the base box's and the curves' support
	// points, so their reaches add up. Like the support point, this is exact for any transform and any exponent.
	const Vector4 base_half_extents = get_base_half_extents();
	Vector4 basis_rows[4];
	Vector4 bounds_half_extents;
	for (int axis = 0; axis < 4; axis++) {
		basis_rows[axis] = p_to_target.basis.get_row(axis);
		// The base box reaches the farthest at the corner with the same signs as the row.
		bounds_half_extents[axis] = basis_rows[axis].abs().dot(base_half_extents);
	}
	// Note: This does not handle Steinmetz solids, or any other case of curves sharing axes.
	for (int64_t curve_index = 0; curve_index < _curves.size(); curve_index++) {
		const Ref<GeneralShapeCurve4D> curve = _curves[curve_index];
		// The bounds are exact for any exponent, so only warn about tapering here.
		GENERAL_SHAPE_4D_CURVE_ERR_WARN_TAPER(curve, Rect4(p_to_target.origin - bounds_half_extents, bounds_half_extents * 2.0f));
		const Vector4 radii = curve->get_radii();
		const double exponent = curve->get_exponent();
		for (int axis = 0; axis < 4; axis++) {
			bounds_half_extents[axis] += basis_rows[axis].dot(_get_curve_support_offset(radii, exponent, basis_rows[axis]));
		}
	}
	return Rect4(p_to_target.origin - bounds_half_extents, bounds_half_extents * 2.0f);
}

Vector4 GeneralShape4D::get_nearest_point(const Vector4 &p_local_point) const {
	const Vector4 half_extents = get_base_half_extents();
	Vector4 local_offset_abs = p_local_point.abs();
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
		const double exponent = curve->get_exponent();
		double ellipsoid_distance_pow = 0.0f;
		if (likely(exponent == 2.0)) {
			for (int64_t used_index = 0; used_index < used_axes.size(); used_index++) {
				const int32_t axis = used_axes[used_index];
				ellipsoid_distance_pow += (local_offset_abs[axis] * local_offset_abs[axis]) / (radii[axis] * radii[axis]);
			}
		} else {
			for (int64_t used_index = 0; used_index < used_axes.size(); used_index++) {
				const int32_t axis = used_axes[used_index];
				ellipsoid_distance_pow += Math::pow((double)local_offset_abs[axis] / (double)radii[axis], exponent);
			}
		}
		if (ellipsoid_distance_pow <= 1.0) {
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
		if (likely(exponent == 2.0)) {
			scaled_offset_abs = scaled_offset_abs.normalized();
		} else {
			// Normalize with the curve's exponent instead of Euclidean length, so that the point lands on the curve's surface.
			// Dividing by the largest value first avoids overflow with large exponents.
			const real_t scaled_offset_max = MAX(MAX(scaled_offset_abs.x, scaled_offset_abs.y), MAX(scaled_offset_abs.z, scaled_offset_abs.w));
			double scaled_offset_length_pow = 0.0;
			for (int64_t used_index = 0; used_index < used_axes.size(); used_index++) {
				const int32_t axis = used_axes[used_index];
				scaled_offset_length_pow += Math::pow(Math::abs((double)scaled_offset_abs[axis] / scaled_offset_max), exponent);
			}
			scaled_offset_abs /= scaled_offset_max * Math::pow(scaled_offset_length_pow, 1.0 / exponent);
		}
		for (int64_t used_index = 0; used_index < used_axes.size(); used_index++) {
			const int32_t axis = used_axes[used_index];
			const real_t rescaled_offset_value = scaled_offset_abs[axis] * radii[axis];
			nearest_point_abs[axis] += rescaled_offset_value;
			local_offset_abs[axis] -= rescaled_offset_value;
		}
	}
	return nearest_point_abs * p_local_point.sign();
}

Vector4 GeneralShape4D::get_support_point(const Vector4 &p_local_direction) const {
	// First, account for the base box.
	const Vector4 half_extents = get_base_half_extents();
	Vector4 support = Vector4(
			(p_local_direction.x > 0.0f) ? half_extents.x : -half_extents.x,
			(p_local_direction.y > 0.0f) ? half_extents.y : -half_extents.y,
			(p_local_direction.z > 0.0f) ? half_extents.z : -half_extents.z,
			(p_local_direction.w > 0.0f) ? half_extents.w : -half_extents.w);
	// Next, check the curves, if any. Note: This does not handle Steinmetz solids, or any other case of curves sharing axes.
	const int64_t curve_count = _curves.size();
	for (int64_t curve_index = 0; curve_index < curve_count; curve_index++) {
		const Ref<GeneralShapeCurve4D> curve = _curves[curve_index];
		// The support point is exact for any exponent, so only warn about tapering here.
		GENERAL_SHAPE_4D_CURVE_ERR_WARN_TAPER(curve, Vector4());
		support += _get_curve_support_offset(curve->get_radii(), curve->get_exponent(), p_local_direction);
	}
	return support;
}

Vector4 GeneralShape4D::_get_curve_support_offset(const Vector4 &p_radii, const double p_exponent, const Vector4 &p_local_direction) {
	// A curve is the set of points where the sum of |x_i / r_i|^p is at most 1, using its radii r and exponent p.
	// Scaling by the radii turns this into the unit ball of the Lp norm, so the support point is x_i = r_i * y_i,
	// where y is the support point of the unit ball in the scaled direction c_i = r_i * d_i.
	// Unused axes have zero radii, so their values of c are zero, and they do not contribute to the support point.
	const Vector4 scaled_direction = p_radii * p_local_direction;
	const Vector4 scaled_direction_abs = scaled_direction.abs();
	int max_axis = 0;
	for (int axis = 1; axis < 4; axis++) {
		if (scaled_direction_abs[axis] > scaled_direction_abs[max_axis]) {
			max_axis = axis;
		}
	}
	const real_t scaled_direction_max = scaled_direction_abs[max_axis];
	if (!(scaled_direction_max > 0.0f)) {
		// The direction is perpendicular to all of the curve's axes, so every point on the curve is equally far
		// along it. The curve's center is a valid choice, and avoids dividing by zero below.
		return Vector4();
	}
	const Vector4 scaled_direction_sign = scaled_direction.sign();
	if (likely(p_exponent == 2.0)) {
		// Ellipsoid, where y = c / |c|. Dividing by the largest value first avoids overflow and underflow.
		return p_radii * (scaled_direction / scaled_direction_max).normalized();
	}
	if (p_exponent <= 1.0) {
		// An exponent of 1 is a diamond (cross-polytope), and below 1 the curve is not convex, but has a diamond
		// as its convex hull. Either way, the support point is the vertex on the axis with the largest value of c.
		Vector4 vertex = Vector4();
		vertex[max_axis] = scaled_direction_sign[max_axis] * p_radii[max_axis];
		return vertex;
	}
	// In general, the support point uses the dual exponent q = p / (p - 1): y_i = sign(c_i) * |c_i|^(q - 1) / ||c||_q^(q - 1).
	// Note that q - 1 = 1 / (p - 1) and (q - 1) / q = 1 / p. When p is close to 1, q is huge, so dividing c
	// by its largest value first is required to avoid overflow and underflow. This does not change y.
	const double dual_exponent = p_exponent / (p_exponent - 1.0);
	const double dual_exponent_minus_one = 1.0 / (p_exponent - 1.0);
	double dual_length_pow = 0.0;
	Vector4 support_offset = Vector4();
	for (int axis = 0; axis < 4; axis++) {
		const double ratio = (double)scaled_direction_abs[axis] / (double)scaled_direction_max;
		if (ratio > 0.0) {
			dual_length_pow += Math::pow(ratio, dual_exponent);
			support_offset[axis] = Math::pow(ratio, dual_exponent_minus_one);
		}
	}
	// The largest ratio is exactly 1, so dual_length_pow is at least 1, and this never divides by zero.
	support_offset /= Math::pow(dual_length_pow, 1.0 / p_exponent);
	return p_radii * scaled_direction_sign * support_offset;
}

bool GeneralShape4D::has_point(const Vector4 &p_local_point) const {
	const Vector4 half_extents = get_base_half_extents();
	Vector4 local_offset_abs = p_local_point.abs();
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
		// The point check is exact for any exponent, so only warn about tapering here.
		GENERAL_SHAPE_4D_CURVE_ERR_WARN_TAPER(curve, false);
		const Vector4 radii = curve->get_radii();
		const PackedInt32Array used_axes = curve->get_used_axes();
		const double exponent = curve->get_exponent();
		double ellipsoid_distance_pow = 0.0f;
		if (likely(exponent == 2.0)) {
			for (int64_t used_index = 0; used_index < used_axes.size(); used_index++) {
				const int axis = used_axes[used_index];
				ellipsoid_distance_pow += (local_offset_abs[axis] * local_offset_abs[axis]) / (radii[axis] * radii[axis]);
			}
		} else {
			for (int64_t used_index = 0; used_index < used_axes.size(); used_index++) {
				const int axis = used_axes[used_index];
				ellipsoid_distance_pow += Math::pow((double)local_offset_abs[axis] / (double)radii[axis], exponent);
			}
		}
		if (ellipsoid_distance_pow > 1.0) {
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

bool GeneralShape4D::is_equal_exact(const Ref<Shape4D> &p_shape) const {
	const Ref<GeneralShape4D> other = p_shape;
	if (other.is_null()) {
		return false;
	}
	if (_base_size != other->_base_size) {
		return false;
	}
	if (_curves.size() != other->_curves.size()) {
		return false;
	}
	for (int64_t i = 0; i < _curves.size(); i++) {
		const Ref<GeneralShapeCurve4D> this_curve = _curves[i];
		const Ref<GeneralShapeCurve4D> other_curve = other->_curves[i];
		if (this_curve.is_null() || other_curve.is_null()) {
			return false;
		}
		if (!this_curve->is_equal_exact(other_curve)) {
			return false;
		}
	}
	return true;
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
