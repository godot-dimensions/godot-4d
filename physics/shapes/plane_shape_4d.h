#pragma once

#include "shape_4d.h"

#include "../../math/plane_4d.h"

class PlaneShape4D : public Shape4D {
	GDCLASS(PlaneShape4D, Shape4D);

	// The plane is supposed to be infinite, but for some calculations, provide a finite size.
	static constexpr real_t PLANE_WIDTH = 1e18;
	static constexpr real_t PLANE_START = PLANE_WIDTH * -0.5;
	static constexpr real_t PLANE_THICKNESS = 1e3;

protected:
	static void _bind_methods() {}

public:
	static Plane4D get_plane_4d() { return Plane4D(Vector4(0, 1, 0, 0), 0); }

	virtual real_t get_hypervolume() const override { return INFINITY; }
	virtual real_t get_surface_volume() const override { return INFINITY; }
	virtual Rect4 get_rect_bounds(const Transform4D &p_to_target = Transform4D()) const override;

	virtual Vector4 get_nearest_point(const Vector4 &p_point) const override;
	virtual Vector4 get_support_point(const Vector4 &p_direction) const override;
	virtual bool has_point(const Vector4 &p_point) const override;

	virtual bool is_equal_exact(const Ref<Shape4D> &p_shape) const override;

	virtual Ref<WireMesh4D> to_wire_mesh() const override;
};
