#include "wireframe_render_canvas.h"

#if GODOT_MODULE
void WireframeRenderCanvas::_notification(int p_what) {
	if (p_what == NOTIFICATION_DRAW) {
		_draw();
	}
}
#endif // GODOT_MODULE

void WireframeRenderCanvas::_draw() {
	draw_rect(Rect2(Vector2(), get_size()), _background_color);
	const Vector2 half_size = get_size() * 0.5f;
	const Vector2 project_scale = _camera_aspect == Camera4D::KEEP_WIDTH ? Vector2(half_size.x, half_size.x) : Vector2(half_size.y, half_size.y);
	for (int i = 0; i < _edge_vertices_to_draw.size(); i++) {
		const PackedVector2Array edge_verts = _edge_vertices_to_draw[i];
		PackedVector2Array verts;
		verts.resize(edge_verts.size());
		for (int vertex_index = 0; vertex_index < edge_verts.size(); vertex_index++) {
			verts.set(vertex_index, edge_verts[vertex_index] * project_scale + half_size);
		}
		draw_multiline_colors(verts, _edge_colors_to_draw[i], _edge_thicknesses_to_draw[i]);
	}
}

void WireframeRenderCanvas::set_background_color(const Color &p_background_color) {
	_background_color = p_background_color;
}

void WireframeRenderCanvas::set_camera_aspect(const Camera4D::KeepAspect p_camera_aspect) {
	_camera_aspect = p_camera_aspect;
}

void WireframeRenderCanvas::set_edge_colors_to_draw(const Vector<PackedColorArray> &p_edge_colors_to_draw) {
	_edge_colors_to_draw = p_edge_colors_to_draw;
}

void WireframeRenderCanvas::set_edge_thicknesses_to_draw(const PackedFloat32Array &p_edge_thicknesses_to_draw) {
	_edge_thicknesses_to_draw = p_edge_thicknesses_to_draw;
}

void WireframeRenderCanvas::set_edge_vertices_to_draw(const Vector<PackedVector2Array> &p_edge_vertices_to_draw) {
	_edge_vertices_to_draw = p_edge_vertices_to_draw;
}

WireframeRenderCanvas::WireframeRenderCanvas() {
	set_z_index(-4000);
	set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT);
}
