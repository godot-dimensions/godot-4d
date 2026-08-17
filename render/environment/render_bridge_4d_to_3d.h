#pragma once

#include "../../render/environment/sky/sky_material_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/environment.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/classes/sky.hpp>
#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#elif GODOT_MODULE
#include "scene/resources/3d/world_3d.h"
#include "scene/resources/environment.h"
#include "scene/resources/material.h"
#include "scene/resources/sky.h"
#endif

class Camera4D;

class EnvironmentRenderBridge4DTo3D : public Object {
	GDCLASS(EnvironmentRenderBridge4DTo3D, Object);

	constexpr static int64_t MAX_SUNS = 4;

	Ref<World3D> _cross_section_world_3d; // Supplied by a RenderingEngine4D.
	Ref<Environment> _cross_section_environment_3d;
	Ref<Sky> _cross_section_sky_3d;
	Ref<SkyMaterial4D> _sky_material_source_4d;
	Ref<ShaderMaterial> _sky_material_4d_to_3d;
	HashMap<StringName, Variant> _sky_shader_parameter_cache;
	bool _background_uses_sky = false;

	static bool _is_renderer_sky_shader_parameter(const StringName &p_name);
	void _set_background_uses_sky(const bool p_enabled, const bool p_force_update = false);
	void _set_sky_shader_parameter(const StringName &p_name, const Variant &p_value);
	bool _sync_sky_material(const Ref<SkyMaterial4D> &p_sky_material_source);

public:
	void setup_environment_resources(const Ref<World3D> &p_cross_section_world_3d);
	void update_environment(Camera4D *p_camera);
	void cleanup_render_resources();
};
