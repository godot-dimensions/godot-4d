#pragma once

#include "../rendering_engine_4d.h"

class Material4D;
class Mesh4D;
class WireframeRenderCanvas4D;

// Trivial CPU-based renderer that draws wireframes to a Control-based canvas.
// Very inefficient, but easy to implement, and even once we have a better
// renderer, this can still be useful for testing and debugging.
class WireframeCanvasRenderingEngine4D : public RenderingEngine4D {
	GDCLASS(WireframeCanvasRenderingEngine4D, RenderingEngine4D);

	static Color _get_material_edge_color(const Ref<Material4D> &p_material, const Ref<Mesh4D> &p_mesh, int p_edge_index);
	static WireframeRenderCanvas4D *_get_valid_render_canvas(const Viewport *p_viewport);

protected:
	static void _bind_methods() {}

public:
	virtual String get_friendly_name() const override { return "Wireframe Canvas"; }
	virtual bool prefers_wireframe_meshes() const override { return true; }
	virtual bool requires_transparent_background() const override { return false; }
	virtual bool supports_lighting() const override { return false; }
	virtual bool supports_godot_rendering_method(const String &) const override { return true; }
	virtual void setup_for_viewport() override;
	virtual void cleanup_for_viewport() override;
	virtual void render_frame() override;
};
