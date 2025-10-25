#pragma once

#include "poly_mesh_4d.h"

class BoxTetraMesh4D;
class BoxWireMesh4D;
class WireMesh4D;

class BoxPolyMesh4D : public PolyMesh4D {
	GDCLASS(BoxPolyMesh4D, PolyMesh4D);

public:
	enum BoxPolyTextureMap {
		BOX_POLY_TEXTURE_MAP_CROSS_ISLAND,
		BOX_POLY_TEXTURE_MAP_FILL_EACH_SIDE,
		BOX_POLY_TEXTURE_MAP_COMPACT_2X2X2_GRID,
		BOX_POLY_TEXTURE_MAP_LONG_CROSS,
	};

private:
	PackedVector4Array _vertices_cache;

	Vector4 _size = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	BoxPolyTextureMap _poly_texture_map = BOX_POLY_TEXTURE_MAP_CROSS_ISLAND;

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

	BoxPolyTextureMap get_poly_texture_map() const { return _poly_texture_map; }
	void set_poly_texture_map(const BoxPolyTextureMap p_map);

	virtual Vector<Vector<PackedInt32Array>> get_poly_cell_indices() override;
	virtual PackedVector4Array get_poly_cell_vertices() override;
	virtual PackedVector4Array get_poly_cell_boundary_normals() override;
	virtual Vector<PackedVector4Array> get_poly_cell_vertex_normals() override;
	virtual Vector<PackedVector3Array> get_poly_cell_texture_map() override;

	virtual PackedInt32Array get_edge_indices() override;
	virtual PackedVector4Array get_vertices() override;

	static Ref<BoxPolyMesh4D> from_box_tetra_mesh(const Ref<BoxTetraMesh4D> &p_tetra_mesh);
	static Ref<BoxPolyMesh4D> from_box_wire_mesh(const Ref<BoxWireMesh4D> &p_wire_mesh);
	Ref<BoxTetraMesh4D> to_box_tetra_mesh() const;
	Ref<BoxWireMesh4D> to_box_wire_mesh() const;
	virtual Ref<TetraMesh4D> to_tetra_mesh() override;
	virtual Ref<WireMesh4D> to_wire_mesh() override;
};

VARIANT_ENUM_CAST(BoxPolyMesh4D::BoxPolyTextureMap);
