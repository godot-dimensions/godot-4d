#pragma once

#include "../single_surface_mesh_4d.h"
#include "wire_material_4d.h"

class WireMesh4D : public SingleSurfaceMesh4D {
	GDCLASS(WireMesh4D, SingleSurfaceMesh4D);

protected:
	static void _bind_methods();

	PackedVector4Array _edge_positions_cache;

public:
	void wire_mesh_clear_cache();
	virtual PackedVector4Array get_edge_positions() override;

	// The proxy 3D mesh encodes each edge as a line of two vertices. See `append_proxy_mesh_surfaces_3d` for the layout.
	static constexpr int64_t PROXY_VERTS_PER_EDGE = 2;
	// Godot splits a surface's vertex data into a vertex buffer and an attribute buffer. For the proxy format:
	// Vertex buffer: position (3 floats). Attribute buffer: one RGBA float custom channel.
	static constexpr int64_t PROXY_VERTEX_BYTES_PER_VERT = 3 * 4;
	static constexpr int64_t PROXY_ATTRIBUTE_BYTES_PER_VERT = 4 * 4;
	static constexpr int64_t PROXY_BYTES_PER_VERT = PROXY_VERTEX_BYTES_PER_VERT + PROXY_ATTRIBUTE_BYTES_PER_VERT;
	// Godot's RenderingServer computes each buffer's byte size as a 32-bit int, so a buffer of 2 GiB or more
	// overflows and crashes. The attribute buffer is the larger of the proxy buffers, so it sets the limit.
	static constexpr int64_t PROXY_MAX_BUFFER_BYTES = INT32_MAX;
	static constexpr int64_t PROXY_MAX_VERTS_PER_SURFACE = PROXY_MAX_BUFFER_BYTES / PROXY_ATTRIBUTE_BYTES_PER_VERT;
	static constexpr int64_t PROXY_MAX_EDGES_PER_SURFACE = PROXY_MAX_VERTS_PER_SURFACE / PROXY_VERTS_PER_EDGE;

	virtual void append_proxy_mesh_surfaces_3d(const Ref<ArrayMesh> &p_proxy_mesh) override;

	Ref<Material4D> get_fallback_material() override;
	static void init_fallback_material();
	static void cleanup_fallback_material();

private:
	static Ref<WireMaterial4D> _fallback_material;
};
