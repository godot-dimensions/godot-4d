#pragma once

#include "../material_4d.h"
#include "../mesh_4d.h"
#include "wire_material_4d.h"

class WireMesh4D : public Mesh4D {
	GDCLASS(WireMesh4D, Mesh4D);

protected:
	static void _bind_methods();
	void update_cross_section_mesh() override;

	PackedVector4Array _edge_positions_cache;

public:
	void wire_mesh_clear_cache();
	virtual PackedVector4Array get_edge_positions() override;

	Ref<Material4D> get_default_material() override;
	static void init_default_material();
	static void cleanup_default_material();

private:
	static Ref<WireMaterial4D> _default_material;
};
