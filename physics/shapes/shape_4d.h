#pragma once

#include "../../godot_4d_defines.h"

#if GDEXTENSION
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>
#elif GODOT_MODULE
#include "core/io/resource.h"
#endif

#include "../../mesh/tetra/tetra_mesh_4d.h"
#include "../../mesh/wire/wire_mesh_4d.h"

class Shape4D : public Resource {
	GDCLASS(Shape4D, Resource);

protected:
	static void _bind_methods();

public:
	virtual real_t get_hypervolume() const;
	virtual Vector4 get_nearest_point(const Vector4 &p_point) const;
	virtual Vector4 get_support_point(const Vector4 &p_direction) const;
	virtual real_t get_surface_volume() const;
	virtual bool has_point(const Vector4 &p_point) const;
	virtual Ref<TetraMesh4D> to_tetra_mesh() const;
	virtual Ref<WireMesh4D> to_wire_mesh() const;

	GDVIRTUAL0RC(real_t, _get_hypervolume);
	GDVIRTUAL1RC(Vector4, _get_nearest_point, Vector4);
	GDVIRTUAL1RC(Vector4, _get_support_point, Vector4);
	GDVIRTUAL0RC(real_t, _get_surface_volume);
	GDVIRTUAL1RC(bool, _has_point, Vector4);
	GDVIRTUAL0RC(Ref<TetraMesh4D>, _to_tetra_mesh);
	GDVIRTUAL0RC(Ref<WireMesh4D>, _to_wire_mesh);
};
