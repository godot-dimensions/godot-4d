#pragma once

#include "../../../godot_4d_defines.h"

#if GDEXTENSION
#include <godot_cpp/classes/shader.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#elif GODOT_MODULE
#include "scene/resources/material.h"
#endif

class SkyMaterial4D : public ShaderMaterial {
	GDCLASS(SkyMaterial4D, ShaderMaterial);

	real_t _energy_multiplier = 1.0f;

protected:
	static void _bind_methods();
	void _validate_property(PropertyInfo &p_property) const;

public:
	real_t get_energy_multiplier() const { return _energy_multiplier; }
	void set_energy_multiplier(real_t p_energy_multiplier);
};
