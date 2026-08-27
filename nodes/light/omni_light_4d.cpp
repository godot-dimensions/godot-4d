#include "omni_light_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/rendering_server.hpp>
#elif GODOT_MODULE
#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR < 6
#include "servers/rendering_server.h"
#else
#include "servers/rendering/rendering_server.h"
#endif
#endif

RID OmniLight4D::create_3d_render_base() const {
	return RenderingServer::get_singleton()->omni_light_create();
}

bool OmniLight4D::update_3d_cross_section_render_base(const Projection &p_relative_to_camera_basis, const Vector4 &p_relative_to_camera_position, const RID p_light_3d_render_base) const {
	ERR_FAIL_COND_V_MSG(!p_light_3d_render_base.is_valid(), false, "OmniLight4D render base RID for Light3D is invalid.");
	RenderingServer *rendering_server = RenderingServer::get_singleton();
	ERR_FAIL_NULL_V(rendering_server, false);
	if (!is_visible_in_tree()) {
		return false;
	}
	// Godot's 3D lights do not include the node scale. However, 4D lights should include the node scale.
	const real_t uniform_scale_4d = ABS(Basis4D(p_relative_to_camera_basis).get_uniform_scale());
	const real_t scaled_range_4d = _omni_range_meters * uniform_scale_4d;
	const real_t slice_offset_w = p_relative_to_camera_position.w;
	// The light's 4D sphere only meets the W = 0 slice if its radius reaches that far along W.
	if (scaled_range_4d <= ABS(slice_offset_w)) {
		return false;
	}
#ifdef GODOT_LIGHT_SLICE_PARAMETERS_ENABLED
	// Both projected and cross-section rendering just need the true 4D parameters in this case.
	update_3d_projected_render_base(p_relative_to_camera_basis, p_relative_to_camera_position, p_light_3d_render_base);
#else
	// Fallback for when the base engine doesn't support slice parameters in the 3D lights (missing Godot Dimensions engine modifications).
	const real_t slice_offset_sq = slice_offset_w * slice_offset_w;
	const real_t scaled_range_4d_sq = scaled_range_4d * scaled_range_4d;
	// Squared radius of the 3D spherical cross-section: R3² = R4² - h².
	const real_t slice_range_3d_sq = scaled_range_4d_sq - slice_offset_sq;
	const real_t slice_range_3d = Math::sqrt(slice_range_3d_sq);
	const real_t scaled_size_4d = _omni_light_size_meters * uniform_scale_4d;
	const real_t rep_dist = 0.5f * slice_range_3d;
	const real_t rep_dist_sq = 0.25f * slice_range_3d_sq;
	const real_t distance_4d_sq = rep_dist_sq + slice_offset_sq;
	const real_t distance_4d = Math::sqrt(distance_4d_sq);
	const real_t range_attenuation_3d = _omni_range_attenuation * (rep_dist_sq / distance_4d_sq);
	const real_t exact_range_ratio_sq = distance_4d_sq / scaled_range_4d_sq;
	const real_t exact_taper = Math::pow(1.0f - exact_range_ratio_sq * exact_range_ratio_sq, 2.0f);
	constexpr real_t fallback_taper = (15.0f / 16.0f) * (15.0f / 16.0f);
	real_t energy_3d = get_light_energy();
	energy_3d *= exact_taper * Math::pow(distance_4d, -_omni_range_attenuation) / (fallback_taper * Math::pow(rep_dist, -range_attenuation_3d));
	rendering_server->light_set_param(p_light_3d_render_base, RSE::LIGHT_PARAM_ENERGY, energy_3d);
	rendering_server->light_set_param(p_light_3d_render_base, RSE::LIGHT_PARAM_RANGE, slice_range_3d);
	rendering_server->light_set_param(p_light_3d_render_base, RSE::LIGHT_PARAM_SIZE, scaled_size_4d);
	rendering_server->light_set_param(p_light_3d_render_base, RSE::LIGHT_PARAM_ATTENUATION, range_attenuation_3d);
	rendering_server->light_set_color(p_light_3d_render_base, get_light_color());
#endif
	return true;
}

void OmniLight4D::update_3d_projected_render_base(const Projection &p_relative_to_camera_basis, const Vector4 &p_relative_to_camera_position, const RID p_light_3d_render_base) const {
	ERR_FAIL_COND_MSG(!p_light_3d_render_base.is_valid(), "OmniLight4D render base RID for Light3D is invalid.");
	RenderingServer *rendering_server = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rendering_server);
	// Godot's 3D lights do not include the node scale. However, 4D lights should include the node scale.
	const real_t uniform_scale_4d = ABS(Basis4D(p_relative_to_camera_basis).get_uniform_scale());
	// The projection of a hypersphere is a sphere of the same radius, so the 4D range and size are used as-is
	// whether or not the fallback is in use. The fallback is a very poor approximation in this case whatever values are used.
	const real_t scaled_range_4d = _omni_range_meters * uniform_scale_4d;
	const real_t scaled_size_4d = _omni_light_size_meters * uniform_scale_4d;
#ifdef GODOT_LIGHT_SLICE_PARAMETERS_ENABLED
	rendering_server->light_set_param(p_light_3d_render_base, RSE::LIGHT_PARAM_SLICE_OFFSET, p_relative_to_camera_position.w);
#endif
	rendering_server->light_set_param(p_light_3d_render_base, RSE::LIGHT_PARAM_ENERGY, get_light_energy());
	rendering_server->light_set_param(p_light_3d_render_base, RSE::LIGHT_PARAM_RANGE, scaled_range_4d);
	rendering_server->light_set_param(p_light_3d_render_base, RSE::LIGHT_PARAM_SIZE, scaled_size_4d);
	rendering_server->light_set_param(p_light_3d_render_base, RSE::LIGHT_PARAM_ATTENUATION, _omni_range_attenuation);
	rendering_server->light_set_color(p_light_3d_render_base, get_light_color());
}

void OmniLight4D::set_omni_range_meters(const real_t p_omni_range_meters) {
	ERR_FAIL_COND_MSG(p_omni_range_meters < 0.0, "OmniLight4D omni range must not be negative. Refusing to set.");
	_omni_range_meters = p_omni_range_meters;
}

void OmniLight4D::set_omni_range_attenuation(const real_t p_omni_range_attenuation) {
	_omni_range_attenuation = p_omni_range_attenuation;
}

void OmniLight4D::set_omni_light_size_meters(const real_t p_omni_light_size_meters) {
	ERR_FAIL_COND_MSG(p_omni_light_size_meters < 0.0, "OmniLight4D omni light size must not be negative. Refusing to set.");
	_omni_light_size_meters = p_omni_light_size_meters;
}

void OmniLight4D::_bind_methods() {
	ADD_GROUP("Omni", "omni_");
	ClassDB::bind_method(D_METHOD("get_omni_range_meters"), &OmniLight4D::get_omni_range_meters);
	ClassDB::bind_method(D_METHOD("set_omni_range_meters", "omni_range_meters"), &OmniLight4D::set_omni_range_meters);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "omni_range_meters", PROPERTY_HINT_RANGE, "0,4096,0.001,or_greater,exp,suffix:m"), "set_omni_range_meters", "get_omni_range_meters");

	ClassDB::bind_method(D_METHOD("get_omni_range_attenuation"), &OmniLight4D::get_omni_range_attenuation);
	ClassDB::bind_method(D_METHOD("set_omni_range_attenuation", "omni_range_attenuation"), &OmniLight4D::set_omni_range_attenuation);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "omni_range_attenuation", PROPERTY_HINT_RANGE, "-10,10,0.001,or_greater,or_less"), "set_omni_range_attenuation", "get_omni_range_attenuation");

	ClassDB::bind_method(D_METHOD("get_omni_light_size_meters"), &OmniLight4D::get_omni_light_size_meters);
	ClassDB::bind_method(D_METHOD("set_omni_light_size_meters", "omni_light_size_meters"), &OmniLight4D::set_omni_light_size_meters);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "omni_light_size_meters", PROPERTY_HINT_RANGE, "0,4096,0.001,or_greater,exp,suffix:m"), "set_omni_light_size_meters", "get_omni_light_size_meters");
}
