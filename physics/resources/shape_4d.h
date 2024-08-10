#pragma once

#if GDEXTENSION
#include <godot_cpp/classes/resource.hpp>
#elif GODOT_MODULE
#include "core/io/resource.h"
#endif

class Shape4D : public Resource {
	GDCLASS(Shape4D, Resource);

protected:
	static void _bind_methods();

public:
	virtual bool has_point(const Vector4 &p_point) const;

	GDVIRTUAL1RC(bool, _has_point, Vector4);
};
