#pragma once

#include "shape_4d.h"

class BoxShape4D : public Shape4D {
	GDCLASS(BoxShape4D, Shape4D);

	Vector4 _size = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

protected:
	static void _bind_methods();

public:
	Vector4 get_half_extents() const;
	void set_half_extents(const Vector4 &p_half_extents);

	Vector4 get_size() const;
	void set_size(const Vector4 &p_size);

	virtual bool has_point(const Vector4 &p_point) const override;
};
