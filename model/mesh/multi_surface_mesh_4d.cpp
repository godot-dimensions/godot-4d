#include "multi_surface_mesh_4d.h"

#include "poly/array_poly_mesh_4d.h"
#include "single_surface_mesh_4d.h"
#include "tetra/array_tetra_mesh_4d.h"
#include "wire/array_wire_mesh_4d.h"

// The strongest merge condition is that the mesh types must be writable (Array* types) and match, but this
// check is excluded because the types need to be handled separately anyway in `merge_compatible_surfaces`.
bool MultiSurfaceMesh4D::_can_merge_surfaces_except_considering_type(const Ref<SingleSurfaceMesh4D> &p_surface, const Ref<SingleSurfaceMesh4D> &p_merged_surface, const Ref<Material4D> &p_merged_surface_original_material) {
	// First check: Obviously, can't merge if either surface is null.
	if (p_surface.is_null() || p_merged_surface.is_null()) {
		return false;
	}
	// Second check: Only merge if the names match. Mismatching names disqualify the surfaces from being merged.
	if (p_surface->get_name() != p_merged_surface->get_name()) {
		return false;
	}
	// Third check: Only merge if the materials are the same instance. This is intentional: two materials
	// with equal values but separate objects mean the user wants separate surfaces. The array meshes'
	// `merge_with` duplicates the merged surface's material before combining color arrays into it, so
	// compare against the material the merged surface had before any merging, not its current one.
	if (p_surface->get_material() != p_merged_surface_original_material) {
		return false;
	}
	// The array mesh merge functions can reject invalid data without reporting success.
	// Keep both surfaces rather than dropping the source after a rejected merge.
	return p_surface->is_mesh_data_valid() && p_merged_surface->is_mesh_data_valid();
}

void MultiSurfaceMesh4D::_ensure_all_surfaces_are_writable() {
	// Start from the existing surfaces so that every slot is kept, then replace the ones that need converting.
	Vector<Ref<SingleSurfaceMesh4D>> writable_surfaces = _surface_meshes;
	for (int surface_index = 0; surface_index < _surface_meshes.size(); surface_index++) {
		const Ref<SingleSurfaceMesh4D> &surface_mesh = _surface_meshes[surface_index];
		if (surface_mesh.is_valid()) {
			ERR_CONTINUE_MSG(!surface_mesh->is_mesh_data_valid(), "MultiSurfaceMesh4D: Surface " + itos(surface_index) + " has invalid mesh data, so it will be left as-is instead of being made writable.");
			const Ref<PolyMesh4D> poly_mesh = surface_mesh;
			if (poly_mesh.is_valid()) {
				const Ref<ArrayPolyMesh4D> array_poly_mesh = poly_mesh;
				if (array_poly_mesh.is_null()) {
					writable_surfaces.set(surface_index, poly_mesh->to_array_poly_mesh());
				} else {
					writable_surfaces.set(surface_index, array_poly_mesh);
				}
				continue;
			}
			const Ref<TetraMesh4D> tetra_mesh = surface_mesh;
			if (tetra_mesh.is_valid()) {
				const Ref<ArrayTetraMesh4D> array_tetra_mesh = tetra_mesh;
				if (array_tetra_mesh.is_null()) {
					writable_surfaces.set(surface_index, tetra_mesh->to_array_tetra_mesh());
				} else {
					writable_surfaces.set(surface_index, array_tetra_mesh);
				}
				continue;
			}
			const Ref<WireMesh4D> wire_mesh = surface_mesh;
			if (wire_mesh.is_valid()) {
				const Ref<ArrayWireMesh4D> array_wire_mesh = wire_mesh;
				if (array_wire_mesh.is_null()) {
					writable_surfaces.set(surface_index, wire_mesh->to_array_wire_mesh());
				} else {
					writable_surfaces.set(surface_index, array_wire_mesh);
				}
				continue;
			}
			ERR_CONTINUE_MSG(true, "MultiSurfaceMesh4D: Surface " + itos(surface_index) + " has an unsupported mesh type, so it will be left as-is instead of being made writable.");
		}
	}
	// Always set through the function to ensure the validations and signal connections are properly handled.
	set_surface_meshes(writable_surfaces);
}

// These trivial functions may look pointless, but they are necessary to properly connect the signals.
void MultiSurfaceMesh4D::_on_surface_mesh_data_validation_reset() {
	// A surface's data changed in a way that may affect its validity, so this mesh's validity is
	// in question too. Resetting the validation also marks the bounds and proxy mesh dirty.
	reset_mesh_data_validation();
}

void MultiSurfaceMesh4D::_on_surface_proxy_mesh_3d_marked_dirty() {
	// A surface was transformed, resized, or otherwise changed without affecting its validity,
	// so only the bounds and proxy mesh of this mesh need to be rebuilt. Every surface validation
	// reset is followed by this signal too, which harmlessly marks dirty a second time.
	mark_mesh_bounds_and_proxy_mesh_3d_dirty();
}

bool MultiSurfaceMesh4D::validate_mesh_data() {
	for (int surface_index = 0; surface_index < _surface_meshes.size(); surface_index++) {
		const Ref<SingleSurfaceMesh4D> &surface_mesh = _surface_meshes[surface_index];
		if (surface_mesh.is_valid()) {
			if (!surface_mesh->is_mesh_data_valid()) {
				return false;
			}
		}
	}
	return true;
}

const Rect4 &MultiSurfaceMesh4D::get_rect_bounds() {
	if (likely(!_is_rect_bounds_dirty)) {
		return _rect_bounds;
	}
	_rect_bounds = Rect4(); // Start by including the mesh's local origin always, even if the mesh does not cover that point.
	for (int i = 0; i < _surface_meshes.size(); i++) {
		const Ref<SingleSurfaceMesh4D> &surface_mesh = _surface_meshes[i];
		if (surface_mesh.is_valid()) {
			_rect_bounds = _rect_bounds.merge(surface_mesh->get_rect_bounds());
		}
	}
	_is_rect_bounds_dirty = false;
	return _rect_bounds;
}

void MultiSurfaceMesh4D::set_surface_meshes(const Vector<Ref<SingleSurfaceMesh4D>> &p_surface_meshes) {
	for (int surface_index = 0; surface_index < p_surface_meshes.size(); surface_index++) {
		const Ref<SingleSurfaceMesh4D> &surface_mesh = p_surface_meshes[surface_index];
		// We generally only want valid meshes, but null entries may be set during
		// intermediate edit states (such as expanding the array in the editor).
		if (surface_mesh.is_valid()) {
			// Refuse to allow the same surface mesh to be added multiple times.
			for (int prev_index = 0; prev_index < surface_index; prev_index++) {
				ERR_FAIL_COND_MSG(p_surface_meshes[prev_index] == surface_mesh, "A MultiSurfaceMesh4D cannot contain the same surface mesh multiple times. Refusing to set surface meshes.");
			}
		}
	}
	// Reconnect the signals that keep this mesh's validity, bounds, and proxy mesh in sync with its surfaces.
	// Transforms and primitive size changes only emit "proxy_mesh_3d_marked_dirty", so both signals are needed.
	for (int surface_index = 0; surface_index < _surface_meshes.size(); surface_index++) {
		const Ref<SingleSurfaceMesh4D> &surface_mesh = _surface_meshes[surface_index];
		if (surface_mesh.is_valid()) {
			surface_mesh->disconnect(StringName("mesh_data_validation_reset"), callable_mp(this, &MultiSurfaceMesh4D::_on_surface_mesh_data_validation_reset));
			surface_mesh->disconnect(StringName("proxy_mesh_3d_marked_dirty"), callable_mp(this, &MultiSurfaceMesh4D::_on_surface_proxy_mesh_3d_marked_dirty));
		}
	}
	for (int surface_index = 0; surface_index < p_surface_meshes.size(); surface_index++) {
		const Ref<SingleSurfaceMesh4D> &surface_mesh = p_surface_meshes[surface_index];
		if (surface_mesh.is_valid()) {
			surface_mesh->connect(StringName("mesh_data_validation_reset"), callable_mp(this, &MultiSurfaceMesh4D::_on_surface_mesh_data_validation_reset));
			surface_mesh->connect(StringName("proxy_mesh_3d_marked_dirty"), callable_mp(this, &MultiSurfaceMesh4D::_on_surface_proxy_mesh_3d_marked_dirty));
		}
	}
	_surface_meshes = p_surface_meshes;
	// This also marks the bounds and proxy mesh dirty.
	reset_mesh_data_validation();
}

TypedArray<SingleSurfaceMesh4D> MultiSurfaceMesh4D::get_surface_meshes_bind() const {
	TypedArray<SingleSurfaceMesh4D> surface_meshes_bind;
	for (int surface_index = 0; surface_index < _surface_meshes.size(); surface_index++) {
		surface_meshes_bind.append(_surface_meshes[surface_index]);
	}
	return surface_meshes_bind;
}

void MultiSurfaceMesh4D::set_surface_meshes_bind(const TypedArray<SingleSurfaceMesh4D> &p_surface_meshes) {
	Vector<Ref<SingleSurfaceMesh4D>> surface_meshes;
	const int64_t surface_mesh_count = p_surface_meshes.size();
	surface_meshes.resize(surface_mesh_count);
	for (int surface_index = 0; surface_index < surface_mesh_count; surface_index++) {
		// We generally only want valid meshes, but null entries may be set during
		// intermediate edit states (such as expanding the array in the editor).
		// This is converted from a Variant, so it is not C++ "&".
		const Ref<SingleSurfaceMesh4D> surface_mesh = p_surface_meshes[surface_index];
		surface_meshes.set(surface_index, surface_mesh);
	}
	set_surface_meshes(surface_meshes);
}

void MultiSurfaceMesh4D::append_proxy_mesh_surfaces_3d(const Ref<ArrayMesh> &p_proxy_mesh) {
	ERR_FAIL_COND(p_proxy_mesh.is_null());
	// Null and empty surfaces add no 3D surface, so the 3D surface indices don't line up
	// with the 4D surface indices in general. Record where each 4D surface's 3D surface ended up
	// by comparing the proxy mesh's surface count before and after appending each surface.
	const int surface_count_4d = _surface_meshes.size();
	_proxy_surface_indices_3d.resize(surface_count_4d);
	for (int surface_index_4d = 0; surface_index_4d < surface_count_4d; surface_index_4d++) {
		const Ref<SingleSurfaceMesh4D> &surface_mesh = _surface_meshes[surface_index_4d];
		const int surface_count_3d_before = p_proxy_mesh->get_surface_count();
		if (surface_mesh.is_valid()) {
			surface_mesh->append_proxy_mesh_surfaces_3d(p_proxy_mesh);
		}
		const bool added_surface_3d = p_proxy_mesh->get_surface_count() > surface_count_3d_before;
		_proxy_surface_indices_3d.set(surface_index_4d, added_surface_3d ? surface_count_3d_before : -1);
	}
}

int MultiSurfaceMesh4D::get_proxy_surface_index_3d(const int p_surface_index_4d) const {
	if (p_surface_index_4d < 0 || p_surface_index_4d >= _proxy_surface_indices_3d.size()) {
		return -1;
	}
	return _proxy_surface_indices_3d[p_surface_index_4d];
}

void MultiSurfaceMesh4D::validate_material_for_mesh(const Ref<Material4D> &p_material) {
	// Always call the virtual method to allow derived classes to provide more material validation.
	GDVIRTUAL_CALL(_validate_material_for_mesh, p_material);
	// For MultiSurfaceMesh4D specifically: Pass along to each individual surface mesh.
	for (int surface_index = 0; surface_index < _surface_meshes.size(); surface_index++) {
		const Ref<SingleSurfaceMesh4D> &surface_mesh = _surface_meshes[surface_index];
		if (surface_mesh.is_valid()) {
			surface_mesh->validate_material_for_mesh(p_material);
		}
	}
}

void MultiSurfaceMesh4D::merge_compatible_surfaces() {
	// Out-of-place merge, to avoid mutating the array while iterating over it.
	// Note that the meshes placed into this array are intentionally the same instances as in the original array, and are mutated.
	// Note that `set_surface_meshes` and `merge_with` guarantee that the `_surface_meshes` array does not contain instance-duplicate meshes.
	// Merging into a surface replaces its material with a duplicate holding the combined color arrays, so the
	// material each merged surface started with is remembered to keep matching later surfaces that share it.
	Vector<Ref<SingleSurfaceMesh4D>> merged_surfaces;
	Vector<Ref<Material4D>> merged_surface_original_materials;
	for (int64_t existing_surface_index = 0; existing_surface_index < _surface_meshes.size(); existing_surface_index++) {
		const Ref<SingleSurfaceMesh4D> &existing_surface = _surface_meshes[existing_surface_index];
		bool merged = false;
		// Check if the existing surface can be merged with any of the already merged surfaces.
		for (int64_t merged_surface_index = 0; merged_surface_index < merged_surfaces.size(); merged_surface_index++) {
			const Ref<SingleSurfaceMesh4D> &merged_surface = merged_surfaces[merged_surface_index];
			if (_can_merge_surfaces_except_considering_type(existing_surface, merged_surface, merged_surface_original_materials[merged_surface_index])) {
				const Ref<ArrayPolyMesh4D> array_poly_mesh_existing = existing_surface;
				const Ref<ArrayPolyMesh4D> array_poly_mesh_merged = merged_surface;
				if (array_poly_mesh_existing.is_valid() && array_poly_mesh_merged.is_valid()) {
					array_poly_mesh_merged->merge_with(array_poly_mesh_existing);
					merged = true;
					break;
				}
				const Ref<ArrayTetraMesh4D> array_tetra_mesh_existing = existing_surface;
				const Ref<ArrayTetraMesh4D> array_tetra_mesh_merged = merged_surface;
				if (array_tetra_mesh_existing.is_valid() && array_tetra_mesh_merged.is_valid()) {
					array_tetra_mesh_merged->merge_with(array_tetra_mesh_existing);
					merged = true;
					break;
				}
				const Ref<ArrayWireMesh4D> array_wire_mesh_existing = existing_surface;
				const Ref<ArrayWireMesh4D> array_wire_mesh_merged = merged_surface;
				if (array_wire_mesh_existing.is_valid() && array_wire_mesh_merged.is_valid()) {
					array_wire_mesh_merged->merge_with(array_wire_mesh_existing);
					merged = true;
					break;
				}
			}
		}
		// If we have not merged this existing surface with another, append it to the merged surfaces.
		if (!merged) {
			merged_surfaces.append(existing_surface);
			merged_surface_original_materials.append(existing_surface.is_valid() ? existing_surface->get_material() : Ref<Material4D>());
		}
	}
	// Always set through the function to ensure the validations and signal connections are properly handled.
	set_surface_meshes(merged_surfaces);
}

void MultiSurfaceMesh4D::merge_with(const Ref<Mesh4D> &p_other, const Transform4D &p_transform) {
	// Merge multi-surface meshes using recursive calls to merge_with for each individual surface mesh.
	const Ref<MultiSurfaceMesh4D> multi_surface_other = p_other;
	if (multi_surface_other.is_valid()) {
		// Snapshot the array so that merging this mesh with itself only visits the original surfaces.
		const Vector<Ref<SingleSurfaceMesh4D>> other_surface_meshes = multi_surface_other->get_surface_meshes();
		for (int i = 0; i < other_surface_meshes.size(); i++) {
			const Ref<SingleSurfaceMesh4D> &other_surface_mesh = other_surface_meshes[i];
			if (other_surface_mesh.is_valid()) {
				merge_with(other_surface_mesh, p_transform);
			}
		}
		return;
	}
	// Merge single-surface meshes here.
	const Ref<SingleSurfaceMesh4D> single_surface_other = p_other;
	ERR_FAIL_COND(single_surface_other.is_null());
	Ref<SingleSurfaceMesh4D> new_surface;
	// Ensure that the mesh is writable, and apply the transform.
	const Ref<PolyMesh4D> poly_mesh_other = single_surface_other;
	if (poly_mesh_other.is_valid()) {
		const Ref<ArrayPolyMesh4D> array_poly_mesh = poly_mesh_other->to_array_poly_mesh();
		array_poly_mesh->transform_mesh(p_transform);
		new_surface = array_poly_mesh;
	} else {
		const Ref<TetraMesh4D> tetra_mesh_other = single_surface_other;
		if (tetra_mesh_other.is_valid()) {
			const Ref<ArrayTetraMesh4D> array_tetra_mesh = tetra_mesh_other->to_array_tetra_mesh();
			array_tetra_mesh->transform_mesh(p_transform);
			new_surface = array_tetra_mesh;
		} else {
			const Ref<WireMesh4D> wire_mesh_other = single_surface_other;
			if (wire_mesh_other.is_valid()) {
				const Ref<ArrayWireMesh4D> array_wire_mesh = wire_mesh_other->to_array_wire_mesh();
				array_wire_mesh->transform_mesh(p_transform);
				new_surface = array_wire_mesh;
			} else {
				// If an unsupported mesh is encountered, we still want to try and append it, but we have to ignore the transform.
				if (!p_transform.is_equal_approx(Transform4D())) {
					ERR_PRINT("MultiSurfaceMesh4D::merge_with: Unsupported mesh type encountered. Transform will not be applied, but continuing merge anyway.");
				}
				new_surface = single_surface_other->duplicate();
			}
		}
	}
	// Always set through the function to ensure the validations and signal connections are properly handled.
	Vector<Ref<SingleSurfaceMesh4D>> surface_meshes = _surface_meshes;
	surface_meshes.append(new_surface);
	set_surface_meshes(surface_meshes);
}

void MultiSurfaceMesh4D::merge_with_bind(const Ref<Mesh4D> &p_other, const Vector4 &p_offset, const Projection &p_basis) {
	merge_with(p_other, Transform4D(p_basis, p_offset));
}

void MultiSurfaceMesh4D::transform_mesh(const Transform4D &p_transform) {
	_ensure_all_surfaces_are_writable();
	for (int surface_index = 0; surface_index < _surface_meshes.size(); surface_index++) {
		const Ref<SingleSurfaceMesh4D> &surface_mesh = _surface_meshes[surface_index];
		if (surface_mesh.is_valid()) {
			const Ref<ArrayPolyMesh4D> array_poly_mesh = surface_mesh;
			if (array_poly_mesh.is_valid()) {
				array_poly_mesh->transform_mesh(p_transform);
				continue;
			}
			const Ref<ArrayTetraMesh4D> array_tetra_mesh = surface_mesh;
			if (array_tetra_mesh.is_valid()) {
				array_tetra_mesh->transform_mesh(p_transform);
				continue;
			}
			const Ref<ArrayWireMesh4D> array_wire_mesh = surface_mesh;
			if (array_wire_mesh.is_valid()) {
				array_wire_mesh->transform_mesh(p_transform);
				continue;
			}
		}
	}
}

void MultiSurfaceMesh4D::transform_mesh_bind(const Vector4 &p_offset, const Projection &p_basis) {
	transform_mesh(Transform4D(p_basis, p_offset));
}

void MultiSurfaceMesh4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_surface_meshes"), &MultiSurfaceMesh4D::get_surface_meshes_bind);
	ClassDB::bind_method(D_METHOD("set_surface_meshes", "surface_meshes"), &MultiSurfaceMesh4D::set_surface_meshes_bind);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "surface_meshes", PROPERTY_HINT_ARRAY_TYPE, "SingleSurfaceMesh4D"), "set_surface_meshes", "get_surface_meshes");

	ClassDB::bind_method(D_METHOD("merge_compatible_surfaces"), &MultiSurfaceMesh4D::merge_compatible_surfaces);
	ClassDB::bind_method(D_METHOD("merge_with", "mesh", "offset", "basis"), &MultiSurfaceMesh4D::merge_with_bind, DEFVAL(Vector4()), DEFVAL(Projection()));
	ClassDB::bind_method(D_METHOD("transform_mesh", "offset", "basis"), &MultiSurfaceMesh4D::transform_mesh_bind, DEFVAL(Projection()));
}
