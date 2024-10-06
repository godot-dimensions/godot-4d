#pragma once

#include "../rendering_engine_4d.h"

// Trivial CPU-based renderer that draws wireframes to a Control-based canvas.
// Very inefficient, but easy to implement, and even once we have a better
// renderer, this can still be useful for testing and debugging.
class WireframeCanvasRenderingEngine4D : public RenderingEngine4D {
	GDCLASS(WireframeCanvasRenderingEngine4D, RenderingEngine4D);

	Vector2 _project_point_3d_to_2d(const Vector3 &p_point) const;
	Vector3 _project_point_4d_to_3d(const Vector4 &p_point) const;
	Vector2 _project_point_4d_to_2d(const Vector4 &p_vertex) const;

protected:
	static void _bind_methods() {}

public:
	void setup_for_viewport() override;
	void render_frame() override;
};
