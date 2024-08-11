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

	virtual real_t get_hypervolume() const override;
	virtual Vector4 get_nearest_point(const Vector4 &p_point) const override;
	virtual Vector4 get_support_point(const Vector4 &p_direction) const override;
	virtual real_t get_surface_volume() const override;
	virtual bool has_point(const Vector4 &p_point) const override;
	virtual Ref<TetraMesh4D> to_tetra_mesh() const override;
	virtual Ref<WireMesh4D> to_wire_mesh() const override;
};
