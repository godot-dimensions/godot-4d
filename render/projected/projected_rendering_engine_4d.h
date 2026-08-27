#pragma once

#include "../tetra_rendering_engine_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/render_data.hpp>
#elif GODOT_MODULE
#include "servers/rendering/storage/render_data.h"
#endif

class ProjectedRenderingEngine4D : public TetraRenderingEngine4D {
	GDCLASS(ProjectedRenderingEngine4D, TetraRenderingEngine4D);

private:
	RID normalize_shader;
	RID normalize_pipeline;
	RID normalize_compositor;
	RID normalize_compositor_effect;

	// Sent from the combined rendering engine, or nil if projected rendering is used alone.
	Variant _cross_section_depth_texture;

	void _free_normalize_resources();

protected:
	static void _bind_methods();

	virtual bool _update_light_3d_render_base(Light4D *p_light_4d, const Projection &p_relative_basis, const Vector4 &p_relative_position, const RID p_render_base) const override;
	virtual Ref<Material> _get_material_3d(const Ref<Material4D> &p_material_4d) override;
	// The shader picks which of the tetrahedron's vertices to emit per output vertex, and the extra
	// vertex in the four-triangle case is solved for in screen space, so the emitted positions are
	// not reliably inside the proxy Mesh3D's custom AABB.
	virtual bool _ignore_mesh_culling() const override { return true; }
	virtual void _update_extra_instance_shader_parameters(const RID p_instance) override;
	virtual void _setup_world_3d() override;

public:
	ProjectedRenderingEngine4D();
	virtual String get_friendly_name() const override { return "Projected"; }
	// The tetrahedra are accumulated with additive blending and then normalized by dividing out the
	// accumulated alpha, which only works if the buffer starts out fully transparent.
	virtual bool requires_transparent_background() const override { return true; }
	// The transparency normalization pass is a compositor effect writing to a storage image, neither
	// of which the Mobile or Compatibility rendering methods provide. Also Mobile's alpha bit depth is insufficient.
	virtual bool supports_godot_rendering_method(const String &p_godot_rendering_method) const override { return p_godot_rendering_method == "forward_plus"; }
	void normalize_image_callback(int64_t p_effect_callback_type, RenderData *p_render_data);
	void set_cross_section_depth_texture(const Variant &p_texture);

	virtual ~ProjectedRenderingEngine4D();
};
