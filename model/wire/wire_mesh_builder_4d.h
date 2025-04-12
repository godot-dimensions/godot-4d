#pragma once

#include "array_wire_mesh_4d.h"

// Static helper class for 4D wire mesh building functions.
class WireMeshBuilder4D : public Object {
	GDCLASS(WireMeshBuilder4D, Object);

protected:
	static WireMeshBuilder4D *singleton;
	static void _bind_methods();

public:
	static Ref<ArrayWireMesh4D> create_3d_orthoplex_sphere(const real_t p_radius, const int p_subdivisions, const real_t p_cone_tip_position = 0.0);
	static Ref<ArrayWireMesh4D> create_3d_subdivided_box(const Vector3 &p_size, const Vector3i &p_subdivision_segments, const bool p_fill_cell = false, const bool p_breakup_edges = false);

	static WireMeshBuilder4D *get_singleton() { return singleton; }
	WireMeshBuilder4D() { singleton = this; }
	~WireMeshBuilder4D() { singleton = nullptr; }
};
