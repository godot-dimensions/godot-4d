#include "plain_sky_material_4d.h"

#include "plain_sky_shader.glsl.gen.h"

#if GDEXTENSION
#include <godot_cpp/classes/rendering_server.hpp>
#elif GODOT_MODULE
#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR < 6
#include "servers/rendering_server.h"
#else
#include "servers/rendering/rendering_server.h"
#endif
#endif

Ref<Shader> PlainSkyMaterial4D::_shader;

void PlainSkyMaterial4D::set_color(const Color &p_color) {
	_color = p_color;
	set_shader_parameter("color", _color);
}

void PlainSkyMaterial4D::_validate_property(PropertyInfo &p_property) const {
	SkyMaterial4D::_validate_property(p_property);
	const String property_name = p_property.name;
	if (property_name == String("shader") || property_name.begins_with("shader_parameter/")) {
		p_property.usage = PROPERTY_USAGE_NONE;
	}
}

void PlainSkyMaterial4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_color", "color"), &PlainSkyMaterial4D::set_color);
	ClassDB::bind_method(D_METHOD("get_color"), &PlainSkyMaterial4D::get_color);
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color", PROPERTY_HINT_COLOR_NO_ALPHA), "set_color", "get_color");
}

void PlainSkyMaterial4D::init_shader() {
	if (_shader.is_valid()) {
		return;
	}
	_shader.instantiate();
	_shader->set_name(String("4D Plain Sky Shader"));
	_shader->set_code(plain_sky_shader_shader_glsl);
	if (RenderingServer::get_singleton() != nullptr) {
		RenderingServer::get_singleton()->shader_set_path_hint(_shader->get_rid(), String("4D Plain Sky Shader"));
	}
}

void PlainSkyMaterial4D::cleanup_shader() {
	_shader.unref();
}

PlainSkyMaterial4D::PlainSkyMaterial4D() {
	init_shader();
	set_shader(_shader);
	set_color(Color(0.0f, 0.0f, 0.0f));
	set_energy_multiplier(1.0f);
}
