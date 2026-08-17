#include "physical_sky_material_4d.h"

#include "physical_sky_shader.glsl.gen.h"

#if GDEXTENSION
#include <godot_cpp/classes/rendering_server.hpp>
#elif GODOT_MODULE
#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR < 6
#include "servers/rendering_server.h"
#else
#include "servers/rendering/rendering_server.h"
#endif
#endif

Ref<Shader> PhysicalSkyMaterial4D::_shader;

void PhysicalSkyMaterial4D::set_rayleigh_coefficient(const real_t p_rayleigh_coefficient) {
	_rayleigh_coefficient = p_rayleigh_coefficient;
	set_shader_parameter("rayleigh_coefficient", _rayleigh_coefficient);
}

void PhysicalSkyMaterial4D::set_rayleigh_color(const Color &p_rayleigh_color) {
	_rayleigh_color = p_rayleigh_color;
	set_shader_parameter("rayleigh_color", _rayleigh_color);
}

void PhysicalSkyMaterial4D::set_mie_coefficient(const real_t p_mie_coefficient) {
	_mie_coefficient = p_mie_coefficient;
	set_shader_parameter("mie_coefficient", _mie_coefficient);
}

void PhysicalSkyMaterial4D::set_mie_anisotropy(const real_t p_mie_anisotropy) {
	_mie_anisotropy = p_mie_anisotropy;
	set_shader_parameter("mie_anisotropy", _mie_anisotropy);
}

void PhysicalSkyMaterial4D::set_mie_color(const Color &p_mie_color) {
	_mie_color = p_mie_color;
	set_shader_parameter("mie_color", _mie_color);
}

void PhysicalSkyMaterial4D::set_sun_glow_intensity(const real_t p_sun_glow_intensity) {
	ERR_FAIL_COND_MSG(p_sun_glow_intensity < 0.0, "PhysicalSkyMaterial4D sun glow intensity must not be negative. Refusing to set.");
	_sun_glow_intensity = p_sun_glow_intensity;
	set_shader_parameter("sun_glow_intensity", _sun_glow_intensity);
}

void PhysicalSkyMaterial4D::set_sun_glow_half_width_radians(const real_t p_sun_glow_half_width_radians) {
	ERR_FAIL_COND_MSG(p_sun_glow_half_width_radians < 0.0 || p_sun_glow_half_width_radians > Math_PI, "PhysicalSkyMaterial4D sun glow half width must be between 0 and 180 degrees. Refusing to set.");
	_sun_glow_half_width_radians = p_sun_glow_half_width_radians;
	set_shader_parameter("sun_glow_half_width_radians", _sun_glow_half_width_radians);
}

void PhysicalSkyMaterial4D::set_ground_color(const Color &p_ground_color) {
	_ground_color = p_ground_color;
	set_shader_parameter("ground_color", _ground_color);
}

void PhysicalSkyMaterial4D::_validate_property(PropertyInfo &p_property) const {
	SkyMaterial4D::_validate_property(p_property);
	const String property_name = p_property.name;
	if (property_name == String("shader") || property_name.begins_with("shader_parameter/")) {
		p_property.usage = PROPERTY_USAGE_NONE;
	}
}

void PhysicalSkyMaterial4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_ground_color"), &PhysicalSkyMaterial4D::get_ground_color);
	ClassDB::bind_method(D_METHOD("set_ground_color", "color"), &PhysicalSkyMaterial4D::set_ground_color);
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "ground_color", PROPERTY_HINT_COLOR_NO_ALPHA), "set_ground_color", "get_ground_color");
	ClassDB::bind_method(D_METHOD("get_sun_glow_intensity"), &PhysicalSkyMaterial4D::get_sun_glow_intensity);
	ClassDB::bind_method(D_METHOD("set_sun_glow_intensity", "intensity"), &PhysicalSkyMaterial4D::set_sun_glow_intensity);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "sun_glow_intensity", PROPERTY_HINT_RANGE, "0,16,0.001,or_greater"), "set_sun_glow_intensity", "get_sun_glow_intensity");
	ClassDB::bind_method(D_METHOD("get_sun_glow_half_width_radians"), &PhysicalSkyMaterial4D::get_sun_glow_half_width_radians);
	ClassDB::bind_method(D_METHOD("set_sun_glow_half_width_radians", "half_width_radians"), &PhysicalSkyMaterial4D::set_sun_glow_half_width_radians);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "sun_glow_half_width_radians", PROPERTY_HINT_RANGE, "0.001,90,0.0001,exp,or_greater,radians_as_degrees"), "set_sun_glow_half_width_radians", "get_sun_glow_half_width_radians");

	ADD_GROUP("Rayleigh Scattering", "rayleigh_");
	ClassDB::bind_method(D_METHOD("get_rayleigh_color"), &PhysicalSkyMaterial4D::get_rayleigh_color);
	ClassDB::bind_method(D_METHOD("set_rayleigh_color", "color"), &PhysicalSkyMaterial4D::set_rayleigh_color);
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "rayleigh_color", PROPERTY_HINT_COLOR_NO_ALPHA), "set_rayleigh_color", "get_rayleigh_color");
	ClassDB::bind_method(D_METHOD("get_rayleigh_coefficient"), &PhysicalSkyMaterial4D::get_rayleigh_coefficient);
	ClassDB::bind_method(D_METHOD("set_rayleigh_coefficient", "coefficient"), &PhysicalSkyMaterial4D::set_rayleigh_coefficient);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "rayleigh_coefficient", PROPERTY_HINT_RANGE, U"0.00000001,0.02,0.000000001,exp,or_greater,or_less,suffix:m⁻¹"), "set_rayleigh_coefficient", "get_rayleigh_coefficient");

	ADD_GROUP("Mie Scattering", "mie_");
	ClassDB::bind_method(D_METHOD("get_mie_color"), &PhysicalSkyMaterial4D::get_mie_color);
	ClassDB::bind_method(D_METHOD("set_mie_color", "color"), &PhysicalSkyMaterial4D::set_mie_color);
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "mie_color", PROPERTY_HINT_COLOR_NO_ALPHA), "set_mie_color", "get_mie_color");
	ClassDB::bind_method(D_METHOD("get_mie_coefficient"), &PhysicalSkyMaterial4D::get_mie_coefficient);
	ClassDB::bind_method(D_METHOD("set_mie_coefficient", "coefficient"), &PhysicalSkyMaterial4D::set_mie_coefficient);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "mie_coefficient", PROPERTY_HINT_RANGE, U"0.00000001,0.02,0.000000001,exp,or_greater,or_less,suffix:m⁻¹"), "set_mie_coefficient", "get_mie_coefficient");
	ClassDB::bind_method(D_METHOD("get_mie_anisotropy"), &PhysicalSkyMaterial4D::get_mie_anisotropy);
	ClassDB::bind_method(D_METHOD("set_mie_anisotropy", "anisotropy"), &PhysicalSkyMaterial4D::set_mie_anisotropy);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "mie_anisotropy", PROPERTY_HINT_RANGE, "-1,1,0.001"), "set_mie_anisotropy", "get_mie_anisotropy");
}

void PhysicalSkyMaterial4D::init_shader() {
	if (_shader.is_valid()) {
		return;
	}
	_shader.instantiate();
	_shader->set_name(String("4D Physical Sky Shader"));
	_shader->set_code(physical_sky_shader_shader_glsl);
	if (RenderingServer::get_singleton() != nullptr) {
		RenderingServer::get_singleton()->shader_set_path_hint(_shader->get_rid(), String("4D Physical Sky Shader"));
	}
}

void PhysicalSkyMaterial4D::cleanup_shader() {
	_shader.unref();
}

PhysicalSkyMaterial4D::PhysicalSkyMaterial4D() {
	init_shader();
	set_shader(_shader);
	set_rayleigh_coefficient(_rayleigh_coefficient);
	set_rayleigh_color(_rayleigh_color);
	set_mie_coefficient(_mie_coefficient);
	set_mie_anisotropy(_mie_anisotropy);
	set_mie_color(_mie_color);
	set_sun_glow_intensity(_sun_glow_intensity);
	set_sun_glow_half_width_radians(_sun_glow_half_width_radians);
	set_ground_color(_ground_color);
	set_energy_multiplier(1.0f);
}
