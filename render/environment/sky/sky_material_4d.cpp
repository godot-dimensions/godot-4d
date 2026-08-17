#include "sky_material_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/rendering_server.hpp>
#elif GODOT_MODULE
#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR < 6
#include "servers/rendering_server.h"
#else
#include "servers/rendering/rendering_server.h"
#endif
#endif

void SkyMaterial4D::set_energy_multiplier(real_t p_energy_multiplier) {
	_energy_multiplier = p_energy_multiplier;
	set_shader_parameter("energy_multiplier", _energy_multiplier);
}

void SkyMaterial4D::_validate_property(PropertyInfo &p_property) const {
	const String property_name = p_property.name;
	if (property_name == String("shader_parameter/energy_multiplier") ||
			property_name == String("shader_parameter/world_up_direction_4d") ||
			property_name.begins_with("shader_parameter/light0_") ||
			property_name.begins_with("shader_parameter/light1_") ||
			property_name.begins_with("shader_parameter/light2_") ||
			property_name.begins_with("shader_parameter/light3_")) {
		// These uniforms are supplied by RenderingEngine4D immediately before rendering.
		p_property.usage = PROPERTY_USAGE_NONE;
	}
}

void SkyMaterial4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_energy_multiplier", "multiplier"), &SkyMaterial4D::set_energy_multiplier);
	ClassDB::bind_method(D_METHOD("get_energy_multiplier"), &SkyMaterial4D::get_energy_multiplier);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "energy_multiplier", PROPERTY_HINT_RANGE, "0.1,2,0.001,exp,or_greater,or_less"), "set_energy_multiplier", "get_energy_multiplier");
}
