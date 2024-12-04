#pragma once

#include "../../godot_4d_defines.h"

#if GDEXTENSION
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>
#elif GODOT_MODULE
#include "core/io/resource.h"
#endif

#include "../../math/rect4.h"
#include "../../math/transform_4d.h"
#include "../../mesh/tetra/tetra_mesh_4d.h"
#include "../../mesh/wire/wire_mesh_4d.h"

class Shape4D : public Resource {
	GDCLASS(Shape4D, Resource);

protected:
	static void _bind_methods();

public:
	virtual real_t get_hypervolume() const;
	virtual real_t get_surface_volume() const;
	virtual Rect4 get_rect_bounds(const Transform4D &p_to_target = Transform4D()) const;
	PackedVector4Array get_rect_bounds_bind(const Projection &p_to_target_basis = Projection(), const Vector4 &p_to_target_offset = Vector4()) const;

	virtual Vector4 get_nearest_point(const Vector4 &p_point) const;
	virtual Vector4 get_support_point(const Vector4 &p_direction) const;
	virtual bool has_point(const Vector4 &p_point) const;

	virtual Ref<TetraMesh4D> to_tetra_mesh() const;
	virtual Ref<WireMesh4D> to_wire_mesh() const;

	GDVIRTUAL0RC(real_t, _get_hypervolume);
	GDVIRTUAL0RC(real_t, _get_surface_volume);
	GDVIRTUAL2RC(PackedVector4Array, _get_rect_bounds, const Projection &, const Vector4 &);

	GDVIRTUAL1RC(Vector4, _get_nearest_point, Vector4);
	GDVIRTUAL1RC(Vector4, _get_support_point, Vector4);
	GDVIRTUAL1RC(bool, _has_point, Vector4);

	GDVIRTUAL0RC(Ref<TetraMesh4D>, _to_tetra_mesh);
	GDVIRTUAL0RC(Ref<WireMesh4D>, _to_wire_mesh);
};
