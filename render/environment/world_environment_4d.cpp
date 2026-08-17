#include "world_environment_4d.h"

#include "../../render/rendering_server_4d.h"

void WorldEnvironment4D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			_is_registered_with_rendering_server = true;
			RenderingServer4D::get_singleton()->register_world_environment(this);
		} break;
		case NOTIFICATION_EXIT_TREE: {
			RenderingServer4D::get_singleton()->unregister_world_environment(this);
			_is_registered_with_rendering_server = false;
		} break;
	}
}

bool WorldEnvironment4D::is_current() const {
	return _is_current;
}

void WorldEnvironment4D::set_current(const bool p_enabled) {
	_is_current = p_enabled;
	if (_is_registered_with_rendering_server) {
		if (p_enabled) {
			RenderingServer4D::get_singleton()->make_world_environment_current(this);
		} else {
			RenderingServer4D::get_singleton()->clear_world_environment_current(this);
		}
	}
}

void WorldEnvironment4D::clear_current(const bool p_enable_next) {
	_is_current = false;
	if (p_enable_next && _is_registered_with_rendering_server) {
		RenderingServer4D::get_singleton()->clear_world_environment_current(this);
	}
}

void WorldEnvironment4D::make_current() {
	_is_current = true;
	if (_is_registered_with_rendering_server) {
		RenderingServer4D::get_singleton()->make_world_environment_current(this);
	}
}

void WorldEnvironment4D::set_ambient_light_sky_contribution(const float p_ambient_light_sky_contribution) {
	_ambient_light_sky_contribution = CLAMP(p_ambient_light_sky_contribution, 0.0f, 1.0f);
}

void WorldEnvironment4D::set_tonemapper(const ToneMapper p_tone_mapper) {
	_tone_mapper = p_tone_mapper;
	notify_property_list_changed();
}

void WorldEnvironment4D::_validate_property(PropertyInfo &p_property) const {
	if (p_property.name == StringName("tonemap_white") && (_tone_mapper == TONE_MAPPER_LINEAR || _tone_mapper == TONE_MAPPER_AGX)) {
		p_property.usage = PROPERTY_USAGE_NO_EDITOR;
	}
}

void WorldEnvironment4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_current"), &WorldEnvironment4D::is_current);
	ClassDB::bind_method(D_METHOD("set_current", "enabled"), &WorldEnvironment4D::set_current);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "current"), "set_current", "is_current");
	ClassDB::bind_method(D_METHOD("clear_current", "enable_next"), &WorldEnvironment4D::clear_current);
	ClassDB::bind_method(D_METHOD("make_current"), &WorldEnvironment4D::make_current);

	ClassDB::bind_method(D_METHOD("get_sky_material"), &WorldEnvironment4D::get_sky_material);
	ClassDB::bind_method(D_METHOD("set_sky_material", "sky_material"), &WorldEnvironment4D::set_sky_material);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "sky_material", PROPERTY_HINT_RESOURCE_TYPE, "SkyMaterial4D"), "set_sky_material", "get_sky_material");

	ADD_GROUP("Ambient Light", "ambient_light_");
	ClassDB::bind_method(D_METHOD("get_ambient_light_color"), &WorldEnvironment4D::get_ambient_light_color);
	ClassDB::bind_method(D_METHOD("set_ambient_light_color", "color"), &WorldEnvironment4D::set_ambient_light_color);
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "ambient_light_color", PROPERTY_HINT_COLOR_NO_ALPHA), "set_ambient_light_color", "get_ambient_light_color");
	ClassDB::bind_method(D_METHOD("get_ambient_light_sky_contribution"), &WorldEnvironment4D::get_ambient_light_sky_contribution);
	ClassDB::bind_method(D_METHOD("set_ambient_light_sky_contribution", "contribution"), &WorldEnvironment4D::set_ambient_light_sky_contribution);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "ambient_light_sky_contribution", PROPERTY_HINT_RANGE, "0,1,0.001"), "set_ambient_light_sky_contribution", "get_ambient_light_sky_contribution");

	ADD_GROUP("Tonemap", "tonemap_");
	ClassDB::bind_method(D_METHOD("get_tonemapper"), &WorldEnvironment4D::get_tonemapper);
	ClassDB::bind_method(D_METHOD("set_tonemapper", "mode"), &WorldEnvironment4D::set_tonemapper);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "tonemap_mode", PROPERTY_HINT_ENUM, "Linear,Reinhard,Filmic,ACES,AgX"), "set_tonemapper", "get_tonemapper");
	ClassDB::bind_method(D_METHOD("get_tonemap_exposure"), &WorldEnvironment4D::get_tonemap_exposure);
	ClassDB::bind_method(D_METHOD("set_tonemap_exposure", "exposure"), &WorldEnvironment4D::set_tonemap_exposure);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "tonemap_exposure", PROPERTY_HINT_RANGE, "0,16,0.01"), "set_tonemap_exposure", "get_tonemap_exposure");
	ClassDB::bind_method(D_METHOD("get_tonemap_white"), &WorldEnvironment4D::get_tonemap_white);
	ClassDB::bind_method(D_METHOD("set_tonemap_white", "white"), &WorldEnvironment4D::set_tonemap_white);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "tonemap_white", PROPERTY_HINT_RANGE, "0,16,0.01"), "set_tonemap_white", "get_tonemap_white");

	BIND_ENUM_CONSTANT(TONE_MAPPER_LINEAR);
	BIND_ENUM_CONSTANT(TONE_MAPPER_REINHARDT);
	BIND_ENUM_CONSTANT(TONE_MAPPER_FILMIC);
	BIND_ENUM_CONSTANT(TONE_MAPPER_ACES);
	BIND_ENUM_CONSTANT(TONE_MAPPER_AGX);
}
