#include "directional_light_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/rendering_server.hpp>
#elif GODOT_MODULE
#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR < 6
#include "servers/rendering_server.h"
#else
#include "servers/rendering/rendering_server.h"
#endif
#endif

RID DirectionalLight4D::create_3d_cross_section_render_base() const {
	return RenderingServer::get_singleton()->directional_light_create();
}

bool DirectionalLight4D::update_3d_cross_section_render_base(const Projection &p_relative_to_camera_basis, const Vector4 &p_relative_to_camera_position, const RID p_light_3d_render_base) const {
	ERR_FAIL_COND_V_MSG(!p_light_3d_render_base.is_valid(), false, "DirectionalLight4D render base RID for Light3D is invalid.");
	RenderingServer *rendering_server = RenderingServer::get_singleton();
	ERR_FAIL_NULL_V(rendering_server, false);
	if (!is_visible_in_tree()) {
		return false;
	}
	// The angular radius of the projection is the right approximation for the soft shadow logic that
	// consumes it either way, so the cross section only differs in the fallback energy correction.
	update_3d_projected_render_base(p_relative_to_camera_basis, p_relative_to_camera_position, p_light_3d_render_base);
#ifndef GODOT_LIGHT_SLICE_PARAMETERS_ENABLED
	// Fallback for when the base engine doesn't support slice parameters in the 3D lights (missing Godot Dimensions engine modifications).
	// Stock Godot normalizes the projected 3D light direction, losing the length of its XYZ part.
	// With the slice-normal approximation N4 = (N3, 0), that length scales the Lambertian dot product.
	// DirectionalLight3D/4D emit along local -Z, so the slice component is the negated W component of local +Z.
	const Basis4D relative_basis_4d = Basis4D(p_relative_to_camera_basis);
	const real_t slice_direction = CLAMP(-relative_basis_4d.z.w / emission_direction_length_4d, (real_t)-1.0, (real_t)1.0);
	const real_t projected_direction_scale = Math::sqrt(MAX((real_t)0.0, (real_t)1.0 - slice_direction * slice_direction));
	if (projected_direction_scale <= CMP_EPSILON) {
		return false;
	}
	rendering_server->light_set_param(p_light_3d_render_base, RSE::LIGHT_PARAM_ENERGY, get_light_energy() * projected_direction_scale);
#endif
	return true;
}

RID DirectionalLight4D::create_3d_projected_render_base() const {
	return RenderingServer::get_singleton()->directional_light_create();
}

void DirectionalLight4D::update_3d_projected_render_base(const Projection &p_relative_to_camera_basis, const Vector4 &p_relative_to_camera_position, const RID p_light_3d_render_base) const {
	ERR_FAIL_COND_MSG(!p_light_3d_render_base.is_valid(), "DirectionalLight4D render base RID for Light3D is invalid.");
	RenderingServer *rendering_server = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rendering_server);
	const Basis4D relative_basis_4d = Basis4D(p_relative_to_camera_basis);
	// DirectionalLight3D/4D emit along local -Z, so the slice component is the negated W component of local +Z.
	const real_t emission_direction_length_4d = relative_basis_4d.z.length();
	const real_t slice_direction = emission_direction_length_4d <= CMP_EPSILON ? (real_t)0.0 : CLAMP(-relative_basis_4d.z.w / emission_direction_length_4d, (real_t)-1.0, (real_t)1.0);
	const double slice_direction_sq = (double)slice_direction * (double)slice_direction;
	// Project the 4D spherical cap onto the W = 0 unit sphere:
	// cos(angular_radius_3d) = sqrt(cos(angular_radius_4d)^2 - slice_direction^2) / sqrt(1 - slice_direction^2),
	// or the whole sphere if the cap includes a ±W pole.
	const double angular_radius_cos_4d = Math::cos(_angular_radius_radians);
	const double angular_radius_cos_4d_sq = angular_radius_cos_4d * angular_radius_cos_4d;
	double angular_radius_radians_3d = Math_TAU / 2.0;
	if (angular_radius_cos_4d_sq > slice_direction_sq) {
		angular_radius_radians_3d = Math::acos(MIN(Math::sqrt((angular_radius_cos_4d_sq - slice_direction_sq) / (1.0 - slice_direction_sq)), 1.0));
	}
#ifdef GODOT_LIGHT_SLICE_PARAMETERS_ENABLED
	rendering_server->light_set_param(p_light_3d_render_base, RSE::LIGHT_PARAM_SLICE_DIRECTION, slice_direction);
#endif
	rendering_server->light_set_param(p_light_3d_render_base, RSE::LIGHT_PARAM_ENERGY, get_light_energy());
	rendering_server->light_set_param(p_light_3d_render_base, RSE::LIGHT_PARAM_SIZE, Math::rad_to_deg(angular_radius_radians_3d));
	rendering_server->light_set_color(p_light_3d_render_base, get_light_color());
}

void DirectionalLight4D::set_angular_radius_degrees(const double p_angular_radius_degrees) {
	ERR_FAIL_COND_MSG(p_angular_radius_degrees < 0.0 || p_angular_radius_degrees > 90.0, "DirectionalLight4D angular radius must be between 0 and 90 degrees. Refusing to set.");
	_angular_radius_radians = Math::deg_to_rad(p_angular_radius_degrees);
}

void DirectionalLight4D::set_angular_radius_radians(const double p_angular_radius_radians) {
	ERR_FAIL_COND_MSG(p_angular_radius_radians < 0.0 || p_angular_radius_radians > Math_TAU / 4.0, "DirectionalLight4D angular radius must be between 0 and 90 degrees. Refusing to set.");
	_angular_radius_radians = p_angular_radius_radians;
}

void DirectionalLight4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_angular_radius_degrees"), &DirectionalLight4D::get_angular_radius_degrees);
	ClassDB::bind_method(D_METHOD("set_angular_radius_degrees", "angular_radius_degrees"), &DirectionalLight4D::set_angular_radius_degrees);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "angular_radius_degrees", PROPERTY_HINT_RANGE, "0.001,90,0.0001,exp", PROPERTY_USAGE_EDITOR), "set_angular_radius_degrees", "get_angular_radius_degrees");

	ClassDB::bind_method(D_METHOD("get_angular_radius_radians"), &DirectionalLight4D::get_angular_radius_radians);
	ClassDB::bind_method(D_METHOD("set_angular_radius_radians", "angular_radius_radians"), &DirectionalLight4D::set_angular_radius_radians);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "angular_radius_radians", PROPERTY_HINT_RANGE, "0.001,1.57079632679,0.0001,exp", PROPERTY_USAGE_STORAGE), "set_angular_radius_radians", "get_angular_radius_radians");
}
