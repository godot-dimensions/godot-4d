#pragma once

#include "../../math/transform_4d.h"
#include "wire_mesh_4d.h"

class ArrayWireMesh4D : public WireMesh4D {
	GDCLASS(ArrayWireMesh4D, WireMesh4D);

	PackedInt32Array _edge_indices;
	PackedVector4Array _vertices;

protected:
	static void _bind_methods();

public:
	void merge_with(const Ref<ArrayWireMesh4D> &p_array_wire_mesh_4d, const Transform4D &p_transform = Transform4D());
	void merge_with_bind(const Ref<ArrayWireMesh4D> &p_array_wire_mesh_4d, const Vector4 &p_offset = Vector4(), const Projection &p_basis = Projection());

	virtual PackedInt32Array get_edge_indices() override;
	void set_edge_indices(const PackedInt32Array &p_edge_indices);

	virtual PackedVector4Array get_vertices() override;
	void set_vertices(const PackedVector4Array &p_vertices);
};
