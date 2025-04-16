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

private:
	PackedVector4Array _cell_positions_cache;
	PackedVector4Array _vertices_cache;

	Vector4 _size = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	BoxTetraDecomp _tetra_decomp = BOX_TETRA_DECOMP_40_CELL;

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

	virtual PackedInt32Array get_cell_indices() override;
	virtual PackedVector4Array get_cell_positions() override;
	virtual PackedVector4Array get_cell_normals() override;
	virtual PackedVector3Array get_cell_uvw_map() override;
	virtual PackedInt32Array get_edge_indices() override;
	virtual PackedVector4Array get_vertices() override;

	static Ref<BoxTetraMesh4D> from_box_wire_mesh(const Ref<BoxWireMesh4D> &p_wire_mesh);
	Ref<BoxWireMesh4D> to_box_wire_mesh() const;
	virtual Ref<WireMesh4D> to_wire_mesh() override;
};

VARIANT_ENUM_CAST(BoxTetraMesh4D::BoxTetraDecomp);
