#pragma once

#include "../rendering_engine_4d.h"

// Trivial CPU-based renderer that draws wireframes to a Control-based canvas.
// Very inefficient, but easy to implement, and even once we have a better
// renderer, this can still be useful for testing and debugging.
class WireframeCanvasRenderingEngine4D : public RenderingEngine4D {
	GDCLASS(WireframeCanvasRenderingEngine4D, RenderingEngine4D);

protected:
	static void _bind_methods() {}

public:
	virtual String get_friendly_name() const override { return "Wireframe Canvas"; }
	virtual bool prefers_wireframe_meshes() override { return true; }
	virtual void setup_for_viewport() override;
	virtual void cleanup_for_viewport() override;
	virtual void render_frame() override;
};
