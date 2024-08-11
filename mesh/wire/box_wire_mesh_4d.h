#pragma once

#include "wire_mesh_4d.h"

class BoxTetraMesh4D;

class BoxWireMesh4D : public WireMesh4D {
	GDCLASS(BoxWireMesh4D, WireMesh4D);

	PackedVector4Array _vertices_cache;
	Vector4 _size = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

protected:
	static void _bind_methods();

public:
	const PackedInt32Array BOX_EDGE_INDICES = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, // X
		0, 2, 1, 3, 4, 6, 5, 7, 8, 10, 9, 11, 12, 14, 13, 15, // Y
		0, 4, 1, 5, 2, 6, 3, 7, 8, 12, 9, 13, 10, 14, 11, 15, // Z
		0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15, // W
	};

	Vector4 get_half_extents() const;
	void set_half_extents(const Vector4 &p_half_extents);

	Vector4 get_size() const;
	void set_size(const Vector4 &p_size);

	virtual PackedInt32Array get_edge_indices() override;
	virtual PackedVector4Array get_edge_positions() override;
	virtual PackedVector4Array get_vertices() override;

	Ref<BoxTetraMesh4D> to_tetra_mesh() const;
	static Ref<BoxWireMesh4D> from_tetra_mesh(const Ref<BoxTetraMesh4D> &p_tetra_mesh);
};
