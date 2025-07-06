#pragma once

#include "../model/mesh_instance_4d.h"

#if GDEXTENSION
#define GDEXTMOD_GET_CONFIGURATION_WARNINGS _get_configuration_warnings
#elif GODOT_MODULE
#define GDEXTMOD_GET_CONFIGURATION_WARNINGS get_configuration_warnings
#endif

class Marker4D : public MeshInstance4D {
	GDCLASS(Marker4D, MeshInstance4D);

public:
	enum RuntimeBehavior {
		RUNTIME_BEHAVIOR_SHOW,
		RUNTIME_BEHAVIOR_HIDE,
		RUNTIME_BEHAVIOR_DELETE,
	};

private:
	real_t _marker_extents = 0.25;
	float _darken_negative_amount = 0.75;
	RuntimeBehavior _runtime_behavior = RUNTIME_BEHAVIOR_DELETE;
	int _subdivisions = 4;

protected:
	static void _bind_methods();
	void _notification(int p_what);
	void _validate_property(PropertyInfo &p_property) const;

public:
	virtual PackedStringArray GDEXTMOD_GET_CONFIGURATION_WARNINGS() const override;
	real_t get_marker_extents() const { return _marker_extents; }
	void set_marker_extents(const real_t p_marker_extents);

	float get_darken_negative_amount() const { return _darken_negative_amount; }
	void set_darken_negative_amount(const float p_darken_negative_amount);

	RuntimeBehavior get_runtime_behavior() const { return _runtime_behavior; }
	void set_runtime_behavior(const RuntimeBehavior p_runtime_behavior) { _runtime_behavior = p_runtime_behavior; }

	int get_subdivisions() const { return _subdivisions; }
	void set_subdivisions(const int p_subdivisions);

	void generate_marker_mesh();
};

VARIANT_ENUM_CAST(Marker4D::RuntimeBehavior);
