#pragma once

#include "../../math/transform_4d.h"
#include "mesh_4d.h"

class SingleSurfaceMesh4D;

class MultiSurfaceMesh4D : public Mesh4D {
	GDCLASS(MultiSurfaceMesh4D, Mesh4D);

	// The array of surface meshes contained within this MultiSurfaceMesh4D.
	// Stored as `Vector<>` to guarantee the only changes to the array happen in `set_surface_meshes`.
	// Using `TypedArray<>` would allow external modifications, so we only use it in the bindings.
	Vector<Ref<SingleSurfaceMesh4D>> _surface_meshes;
	// For each surface, the index of its first surface in the proxy 3D mesh, or -1 if it added none.
	// Rebuilt by `append_proxy_mesh_surfaces_3d`, so it matches the proxy mesh that was last generated.
	PackedInt32Array _proxy_surface_indices_3d;

	static bool _can_merge_surfaces_except_considering_type(const Ref<SingleSurfaceMesh4D> &p_surface, const Ref<SingleSurfaceMesh4D> &p_merged_surface, const Ref<Material4D> &p_merged_surface_original_material);
	void _ensure_all_surfaces_are_writable();
	void _on_surface_mesh_data_validation_reset();
	void _on_surface_proxy_mesh_3d_marked_dirty();

protected:
	static void _bind_methods();
	virtual bool validate_mesh_data() override;

public:
	virtual const Rect4 &get_rect_bounds() override;

	const Vector<Ref<SingleSurfaceMesh4D>> &get_surface_meshes() const { return _surface_meshes; }
	void set_surface_meshes(const Vector<Ref<SingleSurfaceMesh4D>> &p_surface_meshes);

	TypedArray<SingleSurfaceMesh4D> get_surface_meshes_bind() const;
	void set_surface_meshes_bind(const TypedArray<SingleSurfaceMesh4D> &p_surface_meshes);

	virtual void append_proxy_mesh_surfaces_3d(const Ref<ArrayMesh> &p_proxy_mesh) override;
	virtual int get_proxy_surface_index_3d(const int p_surface_index_4d) const override;
	virtual void validate_material_for_mesh(const Ref<Material4D> &p_material) override;

	void merge_compatible_surfaces();
	void merge_with(const Ref<Mesh4D> &p_other, const Transform4D &p_transform = Transform4D());
	void merge_with_bind(const Ref<Mesh4D> &p_other, const Vector4 &p_offset = Vector4(), const Projection &p_basis = Projection());
	void transform_mesh(const Transform4D &p_transform);
	void transform_mesh_bind(const Vector4 &p_offset, const Projection &p_basis = Projection());
};
