#pragma once

#include "../bodies/area_4d.h"
#include "../bodies/physics_body_4d.h"
#include "../bodies/rigid_body_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>
#include <godot_cpp/variant/typed_array.hpp>
#elif GODOT_MODULE
#include "core/object/ref_counted.h"
#include "core/variant/typed_array.h"
#endif

class PhysicsEngine4D : public RefCounted {
	GDCLASS(PhysicsEngine4D, RefCounted);

protected:
	static void _bind_methods();

public:
	virtual void move_and_collide(PhysicsBody4D *p_body, Vector4 p_motion);
	virtual void move_area(Area4D *p_area, Vector4 p_motion);
	virtual void step_dynamic_rigid_body(RigidBody4D *p_body, double p_delta);

	GDVIRTUAL2(_move_and_collide, PhysicsBody4D *, Vector4);
	GDVIRTUAL2(_move_area, Area4D *, Vector4);
	GDVIRTUAL2(_step_dynamic_rigid_body, RigidBody4D *, double);
};
