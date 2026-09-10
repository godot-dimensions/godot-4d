#pragma once

#include "../../math/rect4.h"
#include "material_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/array_mesh.hpp>

#include <godot_cpp/core/gdvirtual.gen.inc>
#elif GODOT_MODULE
#include "scene/resources/mesh.h"
#endif

class ArrayWireMesh4D;
class WireMesh4D;

class Mesh4D : public Resource {
	GDCLASS(Mesh4D, Resource);

protected:
	// Rect bounds need to be defined on the base Mesh4D class.
	Rect4 _rect_bounds = Rect4();
	bool _is_rect_bounds_dirty = true;

private:
	Ref<ArrayMesh> _proxy_mesh_3d;
	bool _is_mesh_data_valid = false;
	bool _is_proxy_mesh_3d_dirty = true;

protected:
	// Slightly under the 32-bit integer limit to avoid overflows.
	static constexpr int64_t MAX_VERTICES = 2147483640;

	static void _bind_methods();
	virtual bool validate_mesh_data();

public:
	static PackedInt32Array deduplicate_edge_indices(const PackedInt32Array &p_items);

	virtual const Rect4 &get_rect_bounds() = 0;
	PackedVector4Array get_rect_bounds_bind();

	// Returns a 3D mesh with the 4D vertex data awkwardly packed into various vertex properties.
	Ref<ArrayMesh> get_proxy_mesh_3d();
	// Called when the proxy 3D mesh is requested and has been marked dirty.
	virtual void append_proxy_mesh_surfaces_3d(const Ref<ArrayMesh> &p_proxy_mesh);
	// Call when the mesh is modified to indicate that
	// the proxy 3D mesh used for rendering needs to be updated.
	void mark_proxy_mesh_3d_dirty() { _is_proxy_mesh_3d_dirty = true; }
	void mark_mesh_bounds_and_proxy_mesh_3d_dirty() {
		_is_proxy_mesh_3d_dirty = true;
		_is_rect_bounds_dirty = true;
	}

	bool is_mesh_data_valid();
	void reset_mesh_data_validation();
	virtual void validate_material_for_mesh(const Ref<Material4D> &p_material);

	GDVIRTUAL1(_append_proxy_mesh_surfaces_3d, Ref<ArrayMesh>);
	GDVIRTUAL0R(bool, _validate_mesh_data);
	GDVIRTUAL1(_validate_material_for_mesh, Ref<Material4D>);
};
