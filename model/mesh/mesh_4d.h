#pragma once

#include "../../math/rect4.h"
#include "material_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/array_mesh.hpp>
#elif GODOT_MODULE
#include "scene/resources/mesh.h"
#endif

class ArrayWireMesh4D;
class WireMesh4D;

class Mesh4D : public Resource {
	GDCLASS(Mesh4D, Resource);

	Rect4 _rect_bounds;
	Ref<ArrayMesh> _proxy_mesh_3d;
	Ref<Material4D> _material;
	bool _is_mesh_data_valid = false;
	bool _is_proxy_mesh_3d_dirty = true;
	bool _is_rect_bounds_dirty = true;

protected:
	// Slightly under the 32-bit integer limit to avoid overflows.
	static constexpr int64_t MAX_VERTICES = 2147483640;

	static void _bind_methods();
	virtual bool validate_mesh_data();
	// Call when the mesh is modified to indicate that
	// the proxy 3D mesh used for rendering needs to be updated.
	void mark_proxy_mesh_3d_dirty() { _is_proxy_mesh_3d_dirty = true; }
	void mark_mesh_bounds_and_proxy_mesh_3d_dirty() {
		_is_proxy_mesh_3d_dirty = true;
		_is_rect_bounds_dirty = true;
	}

public:
	static PackedInt32Array deduplicate_edge_indices(const PackedInt32Array &p_items);
	bool has_edge_indices(int p_first, int p_second);

	// Called when the proxy 3D mesh is requested and has been marked dirty.
	virtual void append_proxy_mesh_surfaces_3d(const Ref<ArrayMesh> &p_proxy_mesh);

	bool is_mesh_data_valid();
	void reset_mesh_data_validation();
	virtual void validate_material_for_mesh(const Ref<Material4D> &p_material);

	Ref<ArrayWireMesh4D> to_array_wire_mesh();
	virtual Ref<WireMesh4D> to_wire_mesh();

	Rect4 get_rect_bounds();
	// Returns a 3D mesh with the 4D vertex data awkwardly packed into various vertex properties.
	Ref<ArrayMesh> get_proxy_mesh_3d();

	Ref<Material4D> get_material() const;
	void set_material(const Ref<Material4D> &p_material);
	virtual Ref<Material4D> get_fallback_material();

	virtual PackedInt32Array get_edge_indices();
	virtual PackedVector4Array get_edge_positions();
	virtual PackedVector4Array get_vertex_positions();
	virtual PackedVector4Array get_normal_values();
	virtual PackedVector3Array get_texture_map_values();

	GDVIRTUAL0R(PackedInt32Array, _get_edge_indices);
	GDVIRTUAL0R(PackedVector4Array, _get_edge_positions);
	GDVIRTUAL0R(PackedVector4Array, _get_vertex_positions);
	GDVIRTUAL0R(PackedVector4Array, _get_normal_values);
	GDVIRTUAL0R(PackedVector3Array, _get_texture_map_values);

	GDVIRTUAL0R(bool, _validate_mesh_data);
	GDVIRTUAL0R(Ref<Material4D>, _get_fallback_material);
	GDVIRTUAL1(_append_proxy_mesh_surfaces_3d, Ref<ArrayMesh>);
	GDVIRTUAL1(_validate_material_for_mesh, Ref<Material4D>);
};
