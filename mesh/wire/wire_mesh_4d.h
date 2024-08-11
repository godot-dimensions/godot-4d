#pragma once

#include "../mesh_4d.h"

class WireMesh4D : public Mesh4D {
	GDCLASS(WireMesh4D, Mesh4D);

protected:
	static void _bind_methods();
	PackedVector4Array _edge_positions_cache;

public:
	void wire_mesh_clear_cache();
	virtual PackedVector4Array get_edge_positions() override;
};
