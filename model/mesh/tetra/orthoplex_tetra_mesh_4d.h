#pragma once

#include "tetra_mesh_4d.h"

class OrthoplexWireMesh4D;
class WireMesh4D;

class OrthoplexTetraMesh4D : public TetraMesh4D {
	GDCLASS(OrthoplexTetraMesh4D, TetraMesh4D);

private:
	PackedVector4Array _vertices_cache;

	Vector4 _size = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

	void _clear_caches();

protected:
	static void _bind_methods();
	virtual bool validate_mesh_data() override { return true; }

public:
	Vector4 get_half_extents() const;
	void set_half_extents(const Vector4 &p_half_extents);

	Vector4 get_size() const;
	void set_size(const Vector4 &p_size);

	virtual PackedInt32Array get_cell_indices() override;
	virtual PackedVector4Array get_cell_boundary_normals() override;
	virtual PackedVector4Array get_cell_vertex_normals() override;
	virtual PackedVector3Array get_cell_uvw_map() override;
	virtual PackedVector4Array get_vertices() override;

	static Ref<OrthoplexTetraMesh4D> from_orthoplex_wire_mesh(const Ref<OrthoplexWireMesh4D> &p_wire_mesh);
	Ref<OrthoplexWireMesh4D> to_orthoplex_wire_mesh() const;
	virtual Ref<WireMesh4D> to_wire_mesh() override;
};
