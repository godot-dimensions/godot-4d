#pragma once

#include "physics_body_4d.h"

class StaticBody4D : public PhysicsBody4D {
	GDCLASS(StaticBody4D, PhysicsBody4D);

protected:
	static void _bind_methods();

public:
};
