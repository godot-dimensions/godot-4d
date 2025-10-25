#pragma once

#include "poly_mesh_4d.h"

class OrthoplexTetraMesh4D;
class OrthoplexWireMesh4D;
class WireMesh4D;

class OrthoplexPolyMesh4D : public PolyMesh4D {
	GDCLASS(OrthoplexPolyMesh4D, PolyMesh4D);

private:
	PackedVector4Array _vertices_cache;

	Vector4 _size = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

	void _clear_caches();

protected:
	static void _bind_methods();
	virtual bool validate_mesh_data() override { return true; }
	virtual bool _validate_poly_mesh_data_only() override { return true; }

public:
	Vector4 get_half_extents() const;
	void set_half_extents(const Vector4 &p_half_extents);

	Vector4 get_size() const;
	void set_size(const Vector4 &p_size);

	virtual Vector<Vector<PackedInt32Array>> get_poly_cell_indices() override;
	virtual PackedVector4Array get_poly_cell_vertices() override;
	virtual PackedVector4Array get_poly_cell_boundary_normals() override;
	virtual Vector<PackedVector4Array> get_poly_cell_vertex_normals() override;
	virtual Vector<PackedVector3Array> get_poly_cell_texture_map() override;

	virtual PackedInt32Array get_edge_indices() override;
	virtual PackedVector4Array get_vertices() override;

	static Ref<OrthoplexPolyMesh4D> from_orthoplex_tetra_mesh(const Ref<OrthoplexTetraMesh4D> &p_tetra_mesh);
	static Ref<OrthoplexPolyMesh4D> from_orthoplex_wire_mesh(const Ref<OrthoplexWireMesh4D> &p_wire_mesh);
	Ref<OrthoplexTetraMesh4D> to_orthoplex_tetra_mesh() const;
	Ref<OrthoplexWireMesh4D> to_orthoplex_wire_mesh() const;
	virtual Ref<TetraMesh4D> to_tetra_mesh() override;
	virtual Ref<WireMesh4D> to_wire_mesh() override;
};
