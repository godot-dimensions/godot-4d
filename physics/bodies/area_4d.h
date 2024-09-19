#pragma once

#include "collision_object_4d.h"

class Area4D : public CollisionObject4D {
	GDCLASS(Area4D, CollisionObject4D);

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void move_area(Vector4 p_motion);
};
