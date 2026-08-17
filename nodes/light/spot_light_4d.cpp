#include "spot_light_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/rendering_server.hpp>
#elif GODOT_MODULE
#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR < 6
#include "servers/rendering_server.h"
#else
#include "servers/rendering/rendering_server.h"
#endif
#endif

RID SpotLight4D::create_light_3d_render_base() const {
	return RenderingServer::get_singleton()->spot_light_create();
}

bool SpotLight4D::update_light_3d_render_base(const Projection &p_relative_to_camera_basis, const Vector4 &p_relative_to_camera_position, const RID p_light_3d_render_base) const {
	ERR_FAIL_COND_V_MSG(!p_light_3d_render_base.is_valid(), false, "SpotLight4D render base RID for Light3D is invalid.");
	RenderingServer *rendering_server = RenderingServer::get_singleton();
	ERR_FAIL_NULL_V(rendering_server, false);
	if (!is_visible_in_tree()) {
		return false;
	}
	// Godot's 3D lights do not include the node scale. However, 4D lights should include the node scale.
	const Basis4D relative_basis_4d = Basis4D(p_relative_to_camera_basis);
	const real_t uniform_scale_4d = ABS(relative_basis_4d.get_uniform_scale());
	const real_t scaled_range_4d = _spot_range_meters * uniform_scale_4d;
	if (scaled_range_4d <= 0.0 || _spot_angle_degrees <= 0.0) {
		return false;
	}
	const real_t slice_offset_w = p_relative_to_camera_position.w;
	const real_t slice_offset_sq = slice_offset_w * slice_offset_w;
	const real_t scaled_range_4d_sq = scaled_range_4d * scaled_range_4d;
	// Squared radius of the 3D spherical cross-section: R3² = R4² - h².
	const real_t slice_range_3d_sq = scaled_range_4d_sq - slice_offset_sq;
	if (slice_range_3d_sq <= 0.0) {
		return false;
	}
	const real_t slice_range_3d = Math::sqrt(slice_range_3d_sq);
	// Same thing for size, but `MAX` with zero instead of exit conditions.
	const real_t scaled_size_4d = _spot_light_size_meters * uniform_scale_4d;
	const real_t scaled_size_4d_sq = scaled_size_4d * scaled_size_4d;
	const real_t scaled_size_3d_sq = scaled_size_4d_sq - slice_offset_sq;
	const real_t slice_size_3d = Math::sqrt(MAX(0.0, scaled_size_3d_sq));
	real_t range_attenuation_3d = _spot_range_attenuation;
	real_t energy_3d = get_light_energy();
	real_t spot_angle_3d = _spot_angle_degrees;
	real_t spot_angle_attenuation_3d = _spot_angle_attenuation;
	const real_t emission_direction_length_4d = relative_basis_4d.z.length();
	if (emission_direction_length_4d <= CMP_EPSILON) {
		return false;
	}
	// SpotLight3D/4D emit along local -Z, so the slice component is the negated W component of local +Z.
	const real_t slice_direction = CLAMP(-relative_basis_4d.z.w / emission_direction_length_4d, (real_t)-1.0, (real_t)1.0);
#ifdef GODOT_LIGHT_SLICE_PARAMETERS_ENABLED
	rendering_server->light_set_param(p_light_3d_render_base, RSE::LIGHT_PARAM_SLICE_DIRECTION, slice_direction);
	rendering_server->light_set_param(p_light_3d_render_base, RSE::LIGHT_PARAM_SLICE_OFFSET, slice_offset_w);
#else
	// Fallback for when the base engine doesn't support slice parameters in the 3D lights (missing Godot Dimensions engine modifications).
	const real_t rep_dist = 0.5f * slice_range_3d;
	const real_t rep_dist_sq = 0.25f * slice_range_3d_sq;
	const real_t distance_4d_sq = rep_dist_sq + slice_offset_sq;
	const real_t distance_4d = Math::sqrt(distance_4d_sq);
	range_attenuation_3d *= rep_dist_sq / distance_4d_sq;
	const real_t exact_range_ratio_sq = distance_4d_sq / scaled_range_4d_sq;
	const real_t exact_taper = Math::pow(1.0f - exact_range_ratio_sq * exact_range_ratio_sq, 2.0f);
	constexpr real_t fallback_taper = (15.0f / 16.0f) * (15.0f / 16.0f);
	energy_3d *= exact_taper * Math::pow(distance_4d, -_spot_range_attenuation) / (fallback_taper * Math::pow(rep_dist, -range_attenuation_3d));
	// A 4D cone generally does not slice to a constant-angle 3D cone. Fit a 3D cone at the same
	// representative distance used for the range attenuation fallback.
	const real_t projected_direction_scale = Math::sqrt(MAX((real_t)0.0, (real_t)1.0 - slice_direction * slice_direction));
	const real_t cone_cos_4d = Math::cos(Math::deg_to_rad(_spot_angle_degrees));
	const real_t axis_cos_4d = CLAMP((projected_direction_scale * rep_dist - slice_offset_w * slice_direction) / distance_4d, (real_t)-1.0, (real_t)1.0);
	const real_t cone_rim_denom = 1.0f - cone_cos_4d;
	const real_t exact_axis_rim = MAX((real_t)1e-4, (1.0f - MAX(axis_cos_4d, cone_cos_4d)) / cone_rim_denom);
	const real_t exact_axis_spot_attenuation = _spot_angle_attenuation == 0.0f ? (axis_cos_4d > cone_cos_4d ? 1.0f : 0.0f) : 1.0f - Math::pow(exact_axis_rim, 1.0f / _spot_angle_attenuation);
	if (exact_axis_spot_attenuation <= 0.0f) {
		return false;
	}
	if (projected_direction_scale <= CMP_EPSILON) {
		// A perpendicular cone is angularly uniform within the slice at a fixed radius.
		spot_angle_3d = 180.0;
		spot_angle_attenuation_3d = CMP_EPSILON;
		energy_3d *= exact_axis_spot_attenuation;
	} else {
		const real_t fitted_cone_cos_3d = (cone_cos_4d * distance_4d + slice_offset_w * slice_direction) / (rep_dist * projected_direction_scale);
		if (fitted_cone_cos_3d >= 1.0) {
			// At most an infinitesimally narrow ray intersects the slice at the fitting distance.
			return false;
		}
		spot_angle_3d = fitted_cone_cos_3d <= -1.0 ? 180.0 : Math::rad_to_deg(Math::acos(fitted_cone_cos_3d));
		const real_t fallback_axis_spot_attenuation = _spot_angle_attenuation == 0.0f ? 1.0f : 1.0f - Math::pow((real_t)1e-4, 1.0f / _spot_angle_attenuation);
		energy_3d *= exact_axis_spot_attenuation / fallback_axis_spot_attenuation;
	}
#endif
	rendering_server->light_set_param(p_light_3d_render_base, RSE::LIGHT_PARAM_ENERGY, energy_3d);
	rendering_server->light_set_param(p_light_3d_render_base, RSE::LIGHT_PARAM_RANGE, slice_range_3d);
	rendering_server->light_set_param(p_light_3d_render_base, RSE::LIGHT_PARAM_SIZE, slice_size_3d);
	rendering_server->light_set_param(p_light_3d_render_base, RSE::LIGHT_PARAM_ATTENUATION, range_attenuation_3d);
	rendering_server->light_set_param(p_light_3d_render_base, RSE::LIGHT_PARAM_SPOT_ANGLE, spot_angle_3d);
	rendering_server->light_set_param(p_light_3d_render_base, RSE::LIGHT_PARAM_SPOT_ATTENUATION, spot_angle_attenuation_3d);
	rendering_server->light_set_color(p_light_3d_render_base, get_light_color());
	return true;
}

void SpotLight4D::set_spot_angle_degrees(const double p_spot_angle_degrees) {
	ERR_FAIL_COND_MSG(p_spot_angle_degrees < 0.0 || p_spot_angle_degrees > 180.0, "SpotLight4D spot angle must be between 0 and 180 degrees. Refusing to set.");
	_spot_angle_degrees = p_spot_angle_degrees;
}

void SpotLight4D::set_spot_angle_attenuation(const real_t p_spot_angle_attenuation) {
	_spot_angle_attenuation = p_spot_angle_attenuation;
}

void SpotLight4D::set_spot_range_meters(const real_t p_spot_range_meters) {
	ERR_FAIL_COND_MSG(p_spot_range_meters < 0.0, "SpotLight4D spot range must not be negative. Refusing to set.");
	_spot_range_meters = p_spot_range_meters;
}

void SpotLight4D::set_spot_range_attenuation(const real_t p_spot_range_attenuation) {
	_spot_range_attenuation = p_spot_range_attenuation;
}

void SpotLight4D::set_spot_light_size_meters(const real_t p_spot_light_size_meters) {
	ERR_FAIL_COND_MSG(p_spot_light_size_meters < 0.0, "SpotLight4D spot light size must not be negative. Refusing to set.");
	_spot_light_size_meters = p_spot_light_size_meters;
}

void SpotLight4D::_bind_methods() {
	ADD_GROUP("Spot", "spot_");
	ClassDB::bind_method(D_METHOD("get_spot_angle_degrees"), &SpotLight4D::get_spot_angle_degrees);
	ClassDB::bind_method(D_METHOD("set_spot_angle_degrees", "spot_angle_degrees"), &SpotLight4D::set_spot_angle_degrees);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "spot_angle_degrees", PROPERTY_HINT_RANGE, "0,180,0.01,degrees"), "set_spot_angle_degrees", "get_spot_angle_degrees");

	ClassDB::bind_method(D_METHOD("get_spot_angle_attenuation"), &SpotLight4D::get_spot_angle_attenuation);
	ClassDB::bind_method(D_METHOD("set_spot_angle_attenuation", "spot_angle_attenuation"), &SpotLight4D::set_spot_angle_attenuation);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "spot_angle_attenuation", PROPERTY_HINT_EXP_EASING, "attenuation"), "set_spot_angle_attenuation", "get_spot_angle_attenuation");

	ClassDB::bind_method(D_METHOD("get_spot_range_meters"), &SpotLight4D::get_spot_range_meters);
	ClassDB::bind_method(D_METHOD("set_spot_range_meters", "spot_range_meters"), &SpotLight4D::set_spot_range_meters);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "spot_range_meters", PROPERTY_HINT_RANGE, "0,4096,0.001,or_greater,exp,suffix:m"), "set_spot_range_meters", "get_spot_range_meters");

	ClassDB::bind_method(D_METHOD("get_spot_range_attenuation"), &SpotLight4D::get_spot_range_attenuation);
	ClassDB::bind_method(D_METHOD("set_spot_range_attenuation", "spot_range_attenuation"), &SpotLight4D::set_spot_range_attenuation);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "spot_range_attenuation", PROPERTY_HINT_RANGE, "-10,10,0.01,or_greater,or_less"), "set_spot_range_attenuation", "get_spot_range_attenuation");

	ClassDB::bind_method(D_METHOD("get_spot_light_size_meters"), &SpotLight4D::get_spot_light_size_meters);
	ClassDB::bind_method(D_METHOD("set_spot_light_size_meters", "spot_light_size_meters"), &SpotLight4D::set_spot_light_size_meters);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "spot_light_size_meters", PROPERTY_HINT_RANGE, "0,4096,0.001,or_greater,exp,suffix:m"), "set_spot_light_size_meters", "get_spot_light_size_meters");
}
