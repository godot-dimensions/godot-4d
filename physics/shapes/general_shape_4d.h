#pragma once

#include "shape_4d.h"

#include "general_shape_curve_4d.h"

#if GDEXTENSION
#include <godot_cpp/variant/typed_array.hpp>
#elif GODOT_MODULE
#include "core/variant/typed_array.h"
#endif

class GeneralShape4D : public Shape4D {
	GDCLASS(GeneralShape4D, Shape4D);

	Vector4 _base_size = Vector4();
	TypedArray<GeneralShapeCurve4D> _curves;
	static bool _warnings_enabled;

protected:
	static void _bind_methods();

public:
	Vector4 get_base_size() const { return _base_size; }
	void set_base_size(const Vector4 &p_base_size);

	Vector4 get_base_half_extents() const { return _base_size * 0.5f; }
	void set_base_half_extents(const Vector4 &p_base_half_extents);

	TypedArray<GeneralShapeCurve4D> get_curves() const { return _curves; }
	void set_curves(const TypedArray<GeneralShapeCurve4D> &p_curves) { _curves = p_curves; }

	static void set_warnings_enabled(bool p_warnings_enabled) { _warnings_enabled = p_warnings_enabled; }

	virtual real_t get_hypervolume() const override;
	virtual real_t get_surface_volume() const override;
	virtual Rect4 get_rect_bounds(const Transform4D &p_to_target = Transform4D()) const override;

	virtual Vector4 get_nearest_point(const Vector4 &p_point) const override;
	virtual Vector4 get_support_point(const Vector4 &p_direction) const override;
	virtual bool has_point(const Vector4 &p_point) const override;

	virtual bool is_equal_exact(const Ref<Shape4D> &p_shape) const override;
};
