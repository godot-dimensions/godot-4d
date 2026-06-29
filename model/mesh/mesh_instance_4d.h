#pragma once

#include "../../nodes/node_4d.h"
#include "mesh_4d.h"

class MeshInstance4D : public Node4D {
	GDCLASS(MeshInstance4D, Node4D);

	Ref<Material4D> _material_override;
	Ref<Mesh4D> _mesh;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	Ref<Material4D> get_active_material() const;

	Ref<Material4D> get_material_override() const;
	void set_material_override(const Ref<Material4D> &p_material_override);

	Ref<Mesh4D> get_mesh() const;
	void set_mesh(const Ref<Mesh4D> &p_mesh);

	virtual Rect4 get_rect_bounds_local(const Transform4D &p_inv_relative_to = Transform4D()) const override;
	virtual Dictionary raycast_intersects_local(const Vector4 &p_local_from, const Vector4 &p_local_direction, const bool p_inside_is_zero) const override;
};
