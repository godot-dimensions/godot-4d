#pragma once

#include "../../nodes/camera_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/control.hpp>
#elif GODOT_MODULE
#include "scene/gui/control.h"
#endif

class WireframeRenderCanvas : public Control {
	GDCLASS(WireframeRenderCanvas, Control);

	Color _background_color = Color(0.0f, 0.0f, 0.0f);
	Camera4D::KeepAspect _camera_aspect = Camera4D::KEEP_HEIGHT;
	Vector<PackedColorArray> _edge_colors_to_draw;
	PackedFloat32Array _edge_thicknesses_to_draw;
	Vector<PackedVector2Array> _edge_vertices_to_draw;

protected:
	static void _bind_methods() {}
#if GODOT_MODULE
	void _notification(int p_what);
#endif

public:
#if GDEXTENSION
	void _draw() override;
#elif GODOT_MODULE
	void _draw();
#endif

	void set_background_color(const Color &p_background_color);
	void set_camera_aspect(const Camera4D::KeepAspect p_camera_aspect);
	void set_edge_colors_to_draw(const Vector<PackedColorArray> &p_edge_colors_to_draw);
	void set_edge_thicknesses_to_draw(const PackedFloat32Array &p_edge_thicknesses_to_draw);
	void set_edge_vertices_to_draw(const Vector<PackedVector2Array> &p_edge_vertices_to_draw);

	WireframeRenderCanvas();
};
