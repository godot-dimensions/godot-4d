#include "gradient_sky_material_4d.h"

#include "gradient_sky_shader.glsl.gen.h"

#if GDEXTENSION
#include <godot_cpp/classes/rendering_server.hpp>
#elif GODOT_MODULE
#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR < 6
#include "servers/rendering_server.h"
#else
#include "servers/rendering/rendering_server.h"
#endif
#endif

Ref<Shader> GradientSkyMaterial4D::_shader;

void GradientSkyMaterial4D::set_top_color(const Color &p_top_color) {
	_top_color = p_top_color;
	set_shader_parameter("top_color", _top_color);
}

void GradientSkyMaterial4D::set_horizon_color(const Color &p_horizon_color) {
	_horizon_color = p_horizon_color;
	set_shader_parameter("horizon_color", _horizon_color);
}

void GradientSkyMaterial4D::set_top_curve(const real_t p_top_curve) {
	_top_curve = p_top_curve;
	set_shader_parameter("inv_top_curve", 0.6f / MAX(_top_curve, (real_t)CMP_EPSILON));
}

void GradientSkyMaterial4D::set_bottom_color(const Color &p_bottom_color) {
	_bottom_color = p_bottom_color;
	set_shader_parameter("bottom_color", _bottom_color);
}

void GradientSkyMaterial4D::set_bottom_curve(const real_t p_bottom_curve) {
	_bottom_curve = p_bottom_curve;
	set_shader_parameter("inv_bottom_curve", 0.6f / MAX(_bottom_curve, (real_t)CMP_EPSILON));
}

void GradientSkyMaterial4D::set_sun_angle_max(const real_t p_sun_angle_max) {
	_sun_angle_max = p_sun_angle_max;
	set_shader_parameter("sun_angle_max", Math::cos(Math::deg_to_rad(_sun_angle_max)));
}

void GradientSkyMaterial4D::set_sun_curve(const real_t p_sun_curve) {
	_sun_curve = p_sun_curve;
	set_shader_parameter("inv_sun_curve", 1.6f / Math::pow(MAX(_sun_curve, (real_t)CMP_EPSILON), (real_t)1.4f));
}

void GradientSkyMaterial4D::_validate_property(PropertyInfo &p_property) const {
	SkyMaterial4D::_validate_property(p_property);
	const String property_name = p_property.name;
	if (property_name == String("shader") || property_name.begins_with("shader_parameter/")) {
		p_property.usage = PROPERTY_USAGE_NONE;
	}
}

void GradientSkyMaterial4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_horizon_color"), &GradientSkyMaterial4D::get_horizon_color);
	ClassDB::bind_method(D_METHOD("set_horizon_color", "color"), &GradientSkyMaterial4D::set_horizon_color);
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "horizon_color", PROPERTY_HINT_COLOR_NO_ALPHA), "set_horizon_color", "get_horizon_color");

	ADD_GROUP("Top (Sky)", "top_");
	ClassDB::bind_method(D_METHOD("get_top_color"), &GradientSkyMaterial4D::get_top_color);
	ClassDB::bind_method(D_METHOD("set_top_color", "color"), &GradientSkyMaterial4D::set_top_color);
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "top_color", PROPERTY_HINT_COLOR_NO_ALPHA), "set_top_color", "get_top_color");
	ClassDB::bind_method(D_METHOD("get_top_curve"), &GradientSkyMaterial4D::get_top_curve);
	ClassDB::bind_method(D_METHOD("set_top_curve", "curve"), &GradientSkyMaterial4D::set_top_curve);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "top_curve", PROPERTY_HINT_EXP_EASING), "set_top_curve", "get_top_curve");

	ADD_GROUP("Bottom (Ground)", "bottom_");
	ClassDB::bind_method(D_METHOD("get_bottom_color"), &GradientSkyMaterial4D::get_bottom_color);
	ClassDB::bind_method(D_METHOD("set_bottom_color", "color"), &GradientSkyMaterial4D::set_bottom_color);
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "bottom_color", PROPERTY_HINT_COLOR_NO_ALPHA), "set_bottom_color", "get_bottom_color");
	ClassDB::bind_method(D_METHOD("get_bottom_curve"), &GradientSkyMaterial4D::get_bottom_curve);
	ClassDB::bind_method(D_METHOD("set_bottom_curve", "curve"), &GradientSkyMaterial4D::set_bottom_curve);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "bottom_curve", PROPERTY_HINT_EXP_EASING), "set_bottom_curve", "get_bottom_curve");

	ADD_GROUP("Sun", "sun_");
	ClassDB::bind_method(D_METHOD("get_sun_angle_max"), &GradientSkyMaterial4D::get_sun_angle_max);
	ClassDB::bind_method(D_METHOD("set_sun_angle_max", "degrees"), &GradientSkyMaterial4D::set_sun_angle_max);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "sun_angle_max", PROPERTY_HINT_RANGE, "0.001,360,0.001,degrees,exp,or_less"), "set_sun_angle_max", "get_sun_angle_max");
	ClassDB::bind_method(D_METHOD("get_sun_curve"), &GradientSkyMaterial4D::get_sun_curve);
	ClassDB::bind_method(D_METHOD("set_sun_curve", "curve"), &GradientSkyMaterial4D::set_sun_curve);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "sun_curve", PROPERTY_HINT_EXP_EASING), "set_sun_curve", "get_sun_curve");
}

void GradientSkyMaterial4D::init_shader() {
	if (_shader.is_valid()) {
		return;
	}
	_shader.instantiate();
	_shader->set_name(String("4D Gradient Sky Shader"));
	_shader->set_code(gradient_sky_shader_shader_glsl);
	if (RenderingServer::get_singleton() != nullptr) {
		RenderingServer::get_singleton()->shader_set_path_hint(_shader->get_rid(), String("4D Gradient Sky Shader"));
	}
}

void GradientSkyMaterial4D::cleanup_shader() {
	_shader.unref();
}

GradientSkyMaterial4D::GradientSkyMaterial4D() {
	init_shader();
	set_shader(_shader);
	// Set these in the constructor to initialize the shader parameters.
	set_horizon_color(_horizon_color);
	set_top_color(_top_color);
	set_top_curve(_top_curve);
	set_bottom_color(_bottom_color);
	set_bottom_curve(_bottom_curve);
	set_sun_angle_max(_sun_angle_max);
	set_sun_curve(_sun_curve);
	set_energy_multiplier(1.0f);
}
