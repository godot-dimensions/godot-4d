#include "mesh_4d.h"

#include "wire/array_wire_mesh_4d.h"

#if GDEXTENSION
#include <godot_cpp/templates/hash_set.hpp>
#elif GODOT_MODULE
#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR < 6
#include "servers/rendering_server.h"
#else
#include "servers/rendering/rendering_server.h"
#endif
#endif

PackedInt32Array Mesh4D::deduplicate_edge_indices(const PackedInt32Array &p_items) {
	HashSet<Vector2i> unique_items;
	PackedInt32Array deduplicated_items;
	for (int i = 0; i < p_items.size() - 1; i += 2) {
		Vector2i edge_indices = Vector2i(p_items[i], p_items[i + 1]);
		if (edge_indices.x > edge_indices.y) {
			SWAP(edge_indices.x, edge_indices.y);
		}
		if (unique_items.has(edge_indices)) {
			continue;
		}
		unique_items.insert(edge_indices);
		deduplicated_items.push_back(edge_indices.x);
		deduplicated_items.push_back(edge_indices.y);
	}
	return deduplicated_items;
}

bool Mesh4D::is_mesh_data_valid() {
	if (likely(_is_mesh_data_valid)) {
		return true;
	}
	_is_mesh_data_valid = validate_mesh_data();
	if (!_is_mesh_data_valid) {
		ERR_PRINT("Mesh4D: Mesh data is invalid on mesh '" + get_name() + "'.");
	}
	return _is_mesh_data_valid;
}

void Mesh4D::reset_mesh_data_validation() {
	_is_mesh_data_valid = false;
	emit_signal("mesh_data_validation_reset");
}

bool Mesh4D::validate_mesh_data() {
	bool ret = false;
	GDVIRTUAL_CALL(_validate_mesh_data, ret);
	return ret;
}

void Mesh4D::append_proxy_mesh_surfaces_3d(const Ref<ArrayMesh> &p_proxy_mesh) {
	GDVIRTUAL_CALL(_append_proxy_mesh_surfaces_3d, p_proxy_mesh);
}

PackedVector4Array Mesh4D::get_rect_bounds_bind() {
	// Wraps the virtual so that C++ overrides are visible to scripts too.
	const Rect4 rect_bounds = get_rect_bounds();
	PackedVector4Array ret;
	ret.push_back(rect_bounds.position);
	ret.push_back(rect_bounds.size);
	return ret;
}

Ref<ArrayMesh> Mesh4D::get_proxy_mesh_3d() {
	if (_proxy_mesh_3d.is_null()) {
		_proxy_mesh_3d.instantiate();
	}
	if (_is_proxy_mesh_3d_dirty) {
		const String mesh_path_or_name = get_path().is_empty() ? get_name() : get_path();
		const String proxy_mesh_hint = mesh_path_or_name + String(" Proxy Mesh 3D");
		_proxy_mesh_3d->set_name(proxy_mesh_hint);
		_proxy_mesh_3d->clear_surfaces();
		append_proxy_mesh_surfaces_3d(_proxy_mesh_3d);
		_is_proxy_mesh_3d_dirty = false;
#if GODOT_MODULE
		if (RenderingServer::get_singleton() != nullptr && _proxy_mesh_3d->get_rid().is_valid()) {
			RenderingServer::get_singleton()->mesh_set_path(_proxy_mesh_3d->get_rid(), proxy_mesh_hint);
		}
#endif
	}
	return _proxy_mesh_3d;
}

void Mesh4D::validate_material_for_mesh(const Ref<Material4D> &p_material) {
	GDVIRTUAL_CALL(_validate_material_for_mesh, p_material);
}

void Mesh4D::_bind_methods() {
	ADD_SIGNAL(MethodInfo("mesh_data_validation_reset"));

	ClassDB::bind_static_method("Mesh4D", D_METHOD("deduplicate_edge_indices", "items"), &Mesh4D::deduplicate_edge_indices);
	ClassDB::bind_method(D_METHOD("get_rect_bounds"), &Mesh4D::get_rect_bounds_bind);

	ClassDB::bind_method(D_METHOD("get_proxy_mesh_3d"), &Mesh4D::get_proxy_mesh_3d);
	ClassDB::bind_method(D_METHOD("append_proxy_mesh_surfaces_3d", "proxy_mesh"), &Mesh4D::append_proxy_mesh_surfaces_3d);
	ClassDB::bind_method(D_METHOD("mark_proxy_mesh_3d_dirty"), &Mesh4D::mark_proxy_mesh_3d_dirty);

	ClassDB::bind_method(D_METHOD("is_mesh_data_valid"), &Mesh4D::is_mesh_data_valid);
	ClassDB::bind_method(D_METHOD("reset_mesh_data_validation"), &Mesh4D::reset_mesh_data_validation);
	ClassDB::bind_method(D_METHOD("validate_material_for_mesh", "material"), &Mesh4D::validate_material_for_mesh);

	GDVIRTUAL_BIND(_append_proxy_mesh_surfaces_3d, "proxy_mesh");
	GDVIRTUAL_BIND(_validate_mesh_data);
	GDVIRTUAL_BIND(_validate_material_for_mesh, "material");
}
