#pragma once

#include "tetra_mesh_4d.h"

class BoxWireMesh4D;
class WireMesh4D;

class BoxTetraMesh4D : public TetraMesh4D {
	GDCLASS(BoxTetraMesh4D, TetraMesh4D);

public:
	enum BoxTetraDecomp {
		BOX_TETRA_DECOMP_40_CELL,
		BOX_TETRA_DECOMP_48_CELL,
		BOX_TETRA_DECOMP_48_CELL_POLYTOPE,
	};

	enum BoxCellTextureMap {
		BOX_CELL_TEXTURE_MAP_CROSS_ISLAND,
		BOX_CELL_TEXTURE_MAP_FILL_EACH_SIDE,
		BOX_CELL_TEXTURE_MAP_COMPACT_2X2X2_GRID,
		BOX_CELL_TEXTURE_MAP_LONG_CROSS,
	};

private:
	PackedVector4Array _vertices_cache;

	Vector4 _size = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	BoxTetraDecomp _tetra_decomp = BOX_TETRA_DECOMP_40_CELL;
	BoxCellTextureMap _cell_texture_map = BOX_CELL_TEXTURE_MAP_CROSS_ISLAND;

	void _clear_caches();

protected:
	static void _bind_methods();
	virtual bool validate_mesh_data() override { return true; }

public:
	Vector4 get_half_extents() const;
	void set_half_extents(const Vector4 &p_half_extents);

	Vector4 get_size() const;
	void set_size(const Vector4 &p_size);

	BoxTetraDecomp get_tetra_decomp() const;
	void set_tetra_decomp(const BoxTetraDecomp p_decomp);

	BoxCellTextureMap get_cell_texture_map() const { return _cell_texture_map; }
	void set_cell_texture_map(const BoxCellTextureMap p_map);

	virtual Ref<ArrayMesh> export_uvw_map_mesh() override;
	virtual PackedInt32Array get_cell_indices() override;
	virtual PackedVector4Array get_cell_boundary_normals() override;
	virtual PackedVector4Array get_cell_vertex_normals() override;
	virtual PackedVector3Array get_cell_uvw_map() override;
	virtual PackedInt32Array get_edge_indices() override;
	virtual PackedVector4Array get_vertices() override;

	static Ref<BoxTetraMesh4D> from_box_wire_mesh(const Ref<BoxWireMesh4D> &p_wire_mesh);
	Ref<BoxWireMesh4D> to_box_wire_mesh() const;
	virtual Ref<TetraMesh4D> to_tetra_mesh() override;
	virtual Ref<WireMesh4D> to_wire_mesh() override;
};

VARIANT_ENUM_CAST(BoxTetraMesh4D::BoxTetraDecomp);
VARIANT_ENUM_CAST(BoxTetraMesh4D::BoxCellTextureMap);
