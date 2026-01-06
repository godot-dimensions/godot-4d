#include "base_face_material_4d.h"

#include "../../../render/cross_section/face/base_face_shader.glsl.gen.h"

#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR < 4
#if GDEXTENSION
#include <godot_cpp/classes/project_settings.hpp>
#elif GODOT_MODULE
#include "core/config/project_settings.h"
#endif
#endif

/*BaseFaceMaterial4D::BaseFaceMaterial4D()  {
	_w_negative_fade_rgbu = Vector4(1.0, 0.5, 0.0, 0.0);
	_w_positive_fade_rgbu = Vector4(0.0, 0.5, 1.0, 0.0);
	_w_fade_distance = 5.0;
	_base_color_rgba = Vector4(1.0, 1.0, 1.0, 1.0);
	_base_color_u = 1.0;
}*/

Color BaseFaceMaterial4D::get_w_negative_fade_rgbu() {
	return _w_negative_fade_rgbu;
}
	
void BaseFaceMaterial4D::set_w_negative_fade_rgbu(Color value) {
	_w_negative_fade_rgbu = value;
	update_cross_section_material();
}

Color BaseFaceMaterial4D::get_w_positive_fade_rgbu() {
	return _w_positive_fade_rgbu;
}

void BaseFaceMaterial4D::set_w_positive_fade_rgbu(Color value) {
	_w_positive_fade_rgbu = value;
	update_cross_section_material();
}

float BaseFaceMaterial4D::get_w_fade_distance() {
	return _w_fade_distance;
}

void BaseFaceMaterial4D::set_w_fade_distance(float value) {
	_w_fade_distance = value;
	update_cross_section_material();
}

Color BaseFaceMaterial4D::get_base_color_rgba() {
	return _base_color_rgba;
}

void BaseFaceMaterial4D::set_base_color_rgba(Color value) {
	_base_color_rgba = value;
	update_cross_section_material();
}

float BaseFaceMaterial4D::get_base_color_u() {
	return _base_color_u;
}

void BaseFaceMaterial4D::set_base_color_u(float value) {
	_base_color_u = value;
	update_cross_section_material();
}

float BaseFaceMaterial4D::get_tetrachromacy_amount() {
	return _tetrachromacy_amount;
}

void BaseFaceMaterial4D::set_tetrachromacy_amount(float value) {
	_tetrachromacy_amount = value;
	update_cross_section_material();
}

void BaseFaceMaterial4D::update_cross_section_material() {
	if (_cross_section_material.is_null()) {
		return;
	}
	if (_cross_section_material->get_shader().is_null()) {
#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR < 4
		// In Godot 4.4+, preload the cross-section shaders. In Godot 4.3, lazy-load them when needed.
		if (_cross_section_shader.is_null()) {
			const String rendering_method = ProjectSettings::get_singleton()->get_setting("rendering/renderer/rendering_method");
			if (rendering_method == "gl_compatibility") {
				ERR_FAIL_MSG("4D cross-section rendering is not supported in Godot 4.3 in the compatibility renderer. Please upgrade to Godot 4.4 or later, or switch to a Vulkan-based rendering method to use 4D cross-section rendering.");
			}
			init_shaders();
		}
#endif
		_cross_section_material->set_shader(_cross_section_shader);
	}
	_cross_section_material->set_shader_parameter("w_negative_fade_rgbu",_w_negative_fade_rgbu);
	_cross_section_material->set_shader_parameter("w_positive_fade_rgbu", _w_positive_fade_rgbu);
	_cross_section_material->set_shader_parameter("w_fade_distance", _w_fade_distance);
	_cross_section_material->set_shader_parameter("base_color_rgba", _base_color_rgba);
	_cross_section_material->set_shader_parameter("base_color_u", _base_color_u);
	_cross_section_material->set_shader_parameter("tetrachromacy_amount", _tetrachromacy_amount);
}

/*void FaceMaterial4D::_validate_property(PropertyInfo &p_property) const {
}

FaceMaterial4D::FaceMaterial4D() {
}*/

Ref<Shader> BaseFaceMaterial4D::_cross_section_shader;

void BaseFaceMaterial4D::init_shaders() {
	_cross_section_shader.instantiate();
	_cross_section_shader->set_code(base_face_shader_shader_glsl);
}

void BaseFaceMaterial4D::cleanup_shaders() {
	_cross_section_shader.unref();
}

void BaseFaceMaterial4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_w_negative_fade_rgbu"), &
		BaseFaceMaterial4D::get_w_negative_fade_rgbu);
	ClassDB::bind_method(
		D_METHOD("set_w_negative_fade_rgbu", "value"), &BaseFaceMaterial4D::set_w_negative_fade_rgbu);
	ClassDB::bind_method(D_METHOD("get_w_positive_fade_rgbu"), &
		BaseFaceMaterial4D::get_w_positive_fade_rgbu);
	ClassDB::bind_method(
		D_METHOD("set_w_positive_fade_rgbu", "value"), &BaseFaceMaterial4D::set_w_positive_fade_rgbu);
	ClassDB::bind_method(D_METHOD("get_w_fade_distance"), &
		BaseFaceMaterial4D::get_w_fade_distance);
	ClassDB::bind_method(
		D_METHOD("set_w_fade_distance", "value"), &BaseFaceMaterial4D::set_w_fade_distance);
	ClassDB::bind_method(D_METHOD("get_base_color_rgba"), &
		BaseFaceMaterial4D::get_base_color_rgba);
	ClassDB::bind_method(
		D_METHOD("set_base_color_rgba", "value"), &BaseFaceMaterial4D::set_base_color_rgba);
	ClassDB::bind_method(D_METHOD("get_base_color_u"), &
		BaseFaceMaterial4D::get_base_color_u);
	ClassDB::bind_method(
		D_METHOD("set_base_color_u", "value"), &BaseFaceMaterial4D::set_base_color_u);
	ClassDB::bind_method(D_METHOD("get_tetrachromacy_amount"), &
		BaseFaceMaterial4D::get_tetrachromacy_amount);
	ClassDB::bind_method(
		D_METHOD("set_tetrachromacy_amount", "value"), &BaseFaceMaterial4D::set_tetrachromacy_amount);
	

	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "w_negative_fade_rgbu"), 
		"set_w_negative_fade_rgbu","get_w_negative_fade_rgbu");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "w_positive_fade_rgbu"), 
		"set_w_positive_fade_rgbu","get_w_positive_fade_rgbu");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "w_fade_distance"), 
		"set_w_fade_distance","get_w_fade_distance");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "base_color_rgba"), 
		"set_base_color_rgba","get_base_color_rgba");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "base_color_u"), 
		"set_base_color_u","get_base_color_u");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "tetrachromacy_amount"), 
		"set_tetrachromacy_amount","get_tetrachromacy_amount");
}
