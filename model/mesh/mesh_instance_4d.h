#pragma once

#include "../../nodes/node_4d.h"
#include "material_4d.h"
#include "mesh_4d.h"

class MeshInstance4D : public Node4D {
	GDCLASS(MeshInstance4D, Node4D);

	Vector<Ref<Material4D>> _material_overrides;
	Ref<Mesh4D> _mesh;

	static Ref<Material4D> _get_valid_active_material_for_surface(const Ref<SingleSurfaceMesh4D> &p_surface_mesh, Ref<Material4D> p_material);

protected:
	static void _bind_methods();
	void _notification(int p_what);
	void _validate_property(PropertyInfo &p_property) const;

public:
	Ref<Material4D> get_active_material(const int p_surface_index = 0) const;

	Ref<Material4D> get_material_override() const;
	void set_material_override(const Ref<Material4D> &p_material_override);

	Vector<Ref<Material4D>> get_material_overrides() const;
	void set_material_overrides(const Vector<Ref<Material4D>> &p_material_overrides);

	TypedArray<Material4D> get_material_overrides_bind() const;
	void set_material_overrides_bind(const TypedArray<Material4D> &p_material_overrides);

	Ref<Mesh4D> get_mesh() const;
	void set_mesh(const Ref<Mesh4D> &p_mesh);

	virtual Rect4 get_rect_bounds_local(const Transform4D &p_to_target = Transform4D()) const override;
	virtual Dictionary raycast_intersects_local(const Vector4 &p_local_from, const Vector4 &p_local_direction, const real_t p_max_distance = Math_INF, const bool p_inside_is_zero = false) const override;
};
