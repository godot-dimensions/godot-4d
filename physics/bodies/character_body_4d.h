#pragma once

#include "physics_body_4d.h"

class CharacterBody4D : public PhysicsBody4D {
	GDCLASS(CharacterBody4D, PhysicsBody4D);

protected:
	static void _bind_methods();

public:
};
