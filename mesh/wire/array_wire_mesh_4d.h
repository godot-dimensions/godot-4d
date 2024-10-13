#pragma once

#include "../../math/transform_4d.h"
#include "wire_mesh_4d.h"

class ArrayWireMesh4D : public WireMesh4D {
	GDCLASS(ArrayWireMesh4D, WireMesh4D);

	PackedInt32Array _edge_indices;
	PackedVector4Array _vertices;

protected:
	static void _bind_methods();
	virtual bool validate_mesh_data() override;

public:
	void append_edge_points(const Vector4 &p_point_a, const Vector4 &p_point_b, const bool p_deduplicate_vertices = true);
	void append_edge_indices(int p_index_a, int p_index_b);
	int append_vertex(const Vector4 &p_vertex, const bool p_deduplicate_vertices = true);
	PackedInt32Array append_vertices(const PackedVector4Array &p_vertices, const bool p_deduplicate_vertices = true);

	void merge_with(const Ref<ArrayWireMesh4D> &p_array_wire_mesh_4d, const Transform4D &p_transform = Transform4D());
	void merge_with_bind(const Ref<ArrayWireMesh4D> &p_array_wire_mesh_4d, const Vector4 &p_offset = Vector4(), const Projection &p_basis = Projection());

	virtual PackedInt32Array get_edge_indices() override;
	void set_edge_indices(const PackedInt32Array &p_edge_indices);

	virtual PackedVector4Array get_vertices() override;
	void set_vertices(const PackedVector4Array &p_vertices);
};
