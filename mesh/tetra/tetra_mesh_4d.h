#pragma once

#include "../mesh_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/array_mesh.hpp>
#elif GODOT_MODULE
class ArrayMesh;
#endif

class TetraMesh4D : public Mesh4D {
	GDCLASS(TetraMesh4D, Mesh4D);

protected:
	static void _bind_methods();
	PackedInt32Array _edge_indices_cache;
	PackedVector4Array _edge_positions_cache;

public:
	Ref<ArrayMesh> export_uvw_map_mesh();
	void tetra_mesh_clear_cache();

	virtual PackedInt32Array get_cell_indices();
	virtual PackedVector4Array get_cell_positions();
	virtual PackedVector4Array get_cell_normals();
	virtual PackedVector3Array get_cell_uvw_map();

	virtual PackedInt32Array get_edge_indices() override;
	virtual PackedVector4Array get_edge_positions() override;

	GDVIRTUAL0R(PackedInt32Array, _get_cell_indices);
	GDVIRTUAL0R(PackedVector4Array, _get_cell_positions);
	GDVIRTUAL0R(PackedVector4Array, _get_cell_normals);
	GDVIRTUAL0R(PackedVector3Array, _get_cell_uvw_map);
};
