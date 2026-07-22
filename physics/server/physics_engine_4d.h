#pragma once

#include "../bodies/area_4d.h"
#include "../bodies/physics_body_4d.h"
#include "../bodies/rigid_body_4d.h"
#include "../kinematic_collision_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/typed_array.hpp>

#include <godot_cpp/core/gdvirtual.gen.inc>
#elif GODOT_MODULE
#include "core/object/ref_counted.h"
#include "core/variant/typed_array.h"
#endif

class PhysicsEngine4D : public RefCounted {
	GDCLASS(PhysicsEngine4D, RefCounted);

protected:
	static void _bind_methods();

public:
	virtual Ref<KinematicCollision4D> move_and_collide(PhysicsBody4D *p_body, const Vector4 &p_motion, const bool p_test_only, const double p_delta_time);
	virtual void move_area(Area4D *p_area, const Vector4 &p_motion);
	virtual void physics_process(const double p_delta_time);

	GDVIRTUAL4R(Ref<KinematicCollision4D>, _move_and_collide, PhysicsBody4D *, const Vector4 &, bool, double);
	GDVIRTUAL2(_move_area, Area4D *, const Vector4 &);
	GDVIRTUAL1(_physics_process, double);
};
