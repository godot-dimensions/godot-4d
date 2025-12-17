#pragma once

#include "wire_mesh_4d.h"

class OrthoplexTetraMesh4D;

class OrthoplexWireMesh4D : public WireMesh4D {
	GDCLASS(OrthoplexWireMesh4D, WireMesh4D);

	PackedVector4Array _vertices_cache;
	Vector4 _size = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

protected:
	static void _bind_methods();
	virtual bool validate_mesh_data() override { return true; }

public:
	Vector4 get_half_extents() const;
	void set_half_extents(const Vector4 &p_half_extents);

	Vector4 get_size() const;
	void set_size(const Vector4 &p_size);

	virtual PackedInt32Array get_edge_indices() override;
	virtual PackedVector4Array get_edge_positions() override;
	virtual PackedVector4Array get_vertices() override;

	static Ref<OrthoplexWireMesh4D> from_tetra_mesh(const Ref<OrthoplexTetraMesh4D> &p_tetra_mesh);
	Ref<OrthoplexTetraMesh4D> to_tetra_mesh() const;
	virtual Ref<WireMesh4D> to_wire_mesh() override;
};
