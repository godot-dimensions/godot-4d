#pragma once

#include "../single_surface_mesh_4d.h"
#include "tetra_material_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/array_mesh.hpp>
#elif GODOT_MODULE
class ArrayMesh;
#endif

class ArrayTetraMesh4D;

class TetraMesh4D : public SingleSurfaceMesh4D {
	GDCLASS(TetraMesh4D, SingleSurfaceMesh4D);

protected:
	static void _bind_methods();
	PackedInt32Array _edge_indices_cache;
	PackedVector4Array _edge_positions_cache;
	PackedVector4Array _simplex_positions_cache;
	PackedFloat64Array _nearest_tetra_inverse_metric_cache;
	void _tetra_mesh_clear_cache_internal();

	Ref<ArrayMesh> convert_texture_map_to_mesh(const PackedInt32Array &p_texture_map_indices);

public:
	// Nearest point and signed distance.
	void populate_inverse_metric_cache();
	real_t get_signed_distance_to_mesh(const Vector4 &p_local_point, Vector4 *r_nearest_point_on_tet, int *r_tetrahedron_index);
	real_t get_signed_distance_to_mesh_bind(const Vector4 &p_local_point);

	// Raycast.
	bool raycast_intersects_fast(const Vector4 &p_local_from, const Vector4 &p_local_direction, const real_t p_max_distance = Math_INF);
	Dictionary raycast_intersects(const Vector4 &p_local_from, const Vector4 &p_local_direction, const real_t p_max_distance = Math_INF);

	// Cache and validation.
	void tetra_mesh_clear_cache(const bool p_reset_validation = true);
	virtual bool validate_mesh_data() override;
	virtual void validate_material_for_mesh(const Ref<Material4D> &p_material) override;

	// Conversion.
	Ref<ArrayTetraMesh4D> to_array_tetra_mesh();
	virtual Ref<TetraMesh4D> to_tetra_mesh();
	virtual Ref<ArrayMesh> export_texture_map_mesh();

	// Getters.
	virtual PackedInt32Array get_simplex_cell_vertex_indices();
	virtual PackedInt32Array get_simplex_cell_normal_indices();
	virtual PackedInt32Array get_simplex_cell_texture_map_indices();
	virtual PackedVector4Array get_simplex_cell_boundary_normals();
	PackedVector4Array get_simplex_cell_positions();

	// Edges.
	static PackedInt32Array calculate_edge_indices_from_simplex_cell_vertex_indices(const PackedInt32Array &p_simplex_cell_vertex_indices, const bool p_deduplicate = true);
	virtual PackedInt32Array get_edge_indices() override;
	virtual PackedVector4Array get_edge_positions() override;

	// 3D.
	// The proxy 3D mesh encodes each tetrahedron as four triangles, so twelve vertices, with the tetrahedron's
	// data duplicated into every one of them. See `append_proxy_mesh_surfaces_3d` for the layout.
	static constexpr int64_t PROXY_VERTS_PER_TET = 12;
	// Godot splits a surface's vertex data into a vertex buffer and an attribute buffer. For the proxy format:
	// Vertex buffer: position (3 floats) + normal (4 bytes) + tangent (4 bytes, Godot adds it whenever normals are used).
	static constexpr int64_t PROXY_VERTEX_BYTES_PER_VERT = 3 * 4 + 4 + 4;
	// Attribute buffer: color (4 bytes) + UV (2 floats) + UV2 (2 floats) + four RGBA float custom channels.
	static constexpr int64_t PROXY_ATTRIBUTE_BYTES_PER_VERT = 4 + 2 * 4 + 2 * 4 + 4 * 4 * 4;
	static constexpr int64_t PROXY_BYTES_PER_VERT = PROXY_VERTEX_BYTES_PER_VERT + PROXY_ATTRIBUTE_BYTES_PER_VERT;
	// Godot's RenderingServer computes each buffer's byte size as a 32-bit int, so a buffer of 2 GiB or more
	// overflows and crashes. The attribute buffer is the largest of the proxy buffers, so it sets the limit.
	static constexpr int64_t PROXY_MAX_BUFFER_BYTES = INT32_MAX;
	static constexpr int64_t PROXY_MAX_VERTS_PER_SURFACE = PROXY_MAX_BUFFER_BYTES / PROXY_ATTRIBUTE_BYTES_PER_VERT;
	static constexpr int64_t PROXY_MAX_TETS_PER_SURFACE = PROXY_MAX_VERTS_PER_SURFACE / PROXY_VERTS_PER_TET;

	virtual void append_proxy_mesh_surfaces_3d(const Ref<ArrayMesh> &p_proxy_mesh) override;

	// Fallback material.
	Ref<Material4D> get_fallback_material() override;
	static void init_fallback_material();
	static void cleanup_fallback_material();

	GDVIRTUAL0R(PackedInt32Array, _get_simplex_cell_vertex_indices);
	GDVIRTUAL0R(PackedInt32Array, _get_simplex_cell_normal_indices);
	GDVIRTUAL0R(PackedInt32Array, _get_simplex_cell_texture_map_indices);
	GDVIRTUAL0R(PackedVector4Array, _get_simplex_cell_boundary_normals);

private:
	static Ref<TetraMaterial4D> _fallback_material;
};
