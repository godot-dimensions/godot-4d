#include "editor_viewport_rotation_4d.h"

#include "editor_main_viewport_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/input_event_screen_drag.hpp>
#include <godot_cpp/classes/input_event_screen_touch.hpp>
#elif GODOT_MODULE
#include "scene/resources/font.h"
#endif

// How far apart the axis circles are from the center of the gizmo.
// Godot's 3D uses 80, but for 4D we have more axes, so we need to space them out more.
constexpr float GIZMO_4D_BASE_SIZE = 100.0f;

String _get_axis_letter_4d(int p_axis) {
	switch (p_axis) {
		case 0:
			return "X";
		case 1:
			return "Y";
		case 2:
			return "Z";
		case 3:
			return "W";
		default:
			return "?";
	}
}

void EditorViewportRotation4D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			if (!is_connected(StringName("mouse_exited"), callable_mp(this, &EditorViewportRotation4D::_on_mouse_exited))) {
				connect(StringName("mouse_exited"), callable_mp(this, &EditorViewportRotation4D::_on_mouse_exited));
			}
			_update_theme();
		} break;
		case NOTIFICATION_DRAW: {
			if (_editor_main_viewport != nullptr) {
				_draw();
			}
		} break;
		case NOTIFICATION_THEME_CHANGED: {
			_update_theme();
		} break;
	}
}

void EditorViewportRotation4D::_draw() {
	const Vector2 center = get_size() / 2.0f;
	if (_focused_axis.axis_number > -2 || _orbiting_mouse_button_index != -1) {
		draw_circle(center, center.x, Color(0.5f, 0.5f, 0.5f, 0.25f), true, -1.0f, true);
	}
	Vector<Axis2D> axis_to_draw;
	_get_sorted_axis(center, axis_to_draw);
	for (int i = 0; i < axis_to_draw.size(); ++i) {
		Axis2D axis = axis_to_draw[i];
		if (axis.axis_type == AXIS_TYPE_LINE) {
			_draw_axis_line(axis, center);
		} else if (axis.axis_type == AXIS_TYPE_PLANE) {
			_draw_axis_plane(axis);
		} else {
			_draw_axis_circle(axis);
		}
	}
}

void EditorViewportRotation4D::_draw_axis_circle(const Axis2D &p_axis) {
	const bool is_focused = _focused_axis.axis_number == p_axis.axis_number && _focused_axis.axis_type == p_axis.axis_type;
	const Color axis_color = _axis_colors[p_axis.axis_number];
	const float alpha = MIN(2.0f, p_axis.z_index + 2.0f);
	const Color color = is_focused ? Color(axis_color.lightened(0.75f), 1.0f) : Color(axis_color, alpha);
	const real_t axis_circle_radius = (8.0f + p_axis.z_index) * _editor_scale;
	if (p_axis.axis_type == AXIS_TYPE_CIRCLE_POSITIVE) {
		draw_circle(p_axis.screen_point, axis_circle_radius, color, true, -1.0f, true);
		// Draw the axis letter for the positive axes.
		const String axis_letter = _get_axis_letter_4d(p_axis.axis_number);
		const Ref<Font> &font = get_theme_font(StringName("rotation_control"), StringName("EditorFonts"));
		const int font_size = get_theme_font_size(StringName("rotation_control_size"), StringName("EditorFonts"));
		const Size2 char_size = font->get_char_size(axis_letter[0], font_size);
		const Vector2 char_offset = Vector2(-char_size.width / 2.0f, char_size.height * 0.25f);
		draw_char(font, p_axis.screen_point + char_offset, axis_letter, font_size, Color(0.0f, 0.0f, 0.0f, alpha * 0.6f));
	} else {
		// Draw an outline around the negative axes.
		draw_circle(p_axis.screen_point, axis_circle_radius, color, true, -1.0f, true);
		draw_circle(p_axis.screen_point, axis_circle_radius * 0.8f, color.darkened(0.4f), true, -1.0f, true);
	}
}

void EditorViewportRotation4D::_draw_axis_line(const Axis2D &p_axis, const Vector2 &p_center) {
	const bool is_focused = _focused_axis.axis_number == p_axis.axis_number && _focused_axis.axis_type == AXIS_TYPE_CIRCLE_POSITIVE;
	const Color axis_color = _axis_colors[p_axis.axis_number];
	const float alpha = MIN(2.0f, p_axis.z_index + 2.0f);
	const Color color = is_focused ? Color(axis_color.lightened(0.75f), 1.0f) : Color(axis_color, alpha);
	draw_line(p_center, p_axis.screen_point, color, 1.5f * _editor_scale, true);
}

void EditorViewportRotation4D::_draw_axis_plane(const Axis2D &p_axis) {
	const bool is_focused = _focused_axis.axis_number == p_axis.axis_number && _focused_axis.axis_type == p_axis.axis_type && _focused_axis.secondary_axis_number == p_axis.secondary_axis_number;
	Color primary_color = _axis_colors[p_axis.axis_number];
	Color secondary_color = _axis_colors[p_axis.secondary_axis_number];
	if (is_focused) {
		primary_color = primary_color.lightened(0.75f);
		secondary_color = secondary_color.lightened(0.75f);
	} else {
		const float alpha = MIN(2.0f, p_axis.z_index + 2.0f);
		primary_color.a = alpha;
		secondary_color.a = alpha;
	}
	constexpr float QUARTER_TURN = Math_TAU / 4.0f;
	const float outer_radius = (4.0f + p_axis.z_index) * _editor_scale;
	_draw_filled_arc(p_axis.screen_point, outer_radius, p_axis.angle + QUARTER_TURN, p_axis.angle + QUARTER_TURN * 3.0f, primary_color);
	_draw_filled_arc(p_axis.screen_point, outer_radius, p_axis.angle - QUARTER_TURN, p_axis.angle + QUARTER_TURN, secondary_color);
	if (p_axis.z_index < 4.0f) {
		const float inner_radius = (3.0f + p_axis.z_index) * _editor_scale;
		_draw_filled_arc(p_axis.screen_point, inner_radius, p_axis.angle + QUARTER_TURN, p_axis.angle + QUARTER_TURN * 3.0f, primary_color.darkened(0.4f));
		_draw_filled_arc(p_axis.screen_point, inner_radius, p_axis.angle - QUARTER_TURN, p_axis.angle + QUARTER_TURN, secondary_color.darkened(0.4f));
	} else {
		// If the circle is big enough, draw letters.
		const String primary_letter = _get_axis_letter_4d(p_axis.axis_number);
		const String secondary_letter = _get_axis_letter_4d(p_axis.secondary_axis_number);
		const Ref<Font> &font = get_theme_font(StringName("rotation_control"), StringName("EditorFonts"));
		const int font_size = get_theme_font_size(StringName("rotation_control_size"), StringName("EditorFonts"));
		const Vector2 primary_char_size = font->get_char_size(primary_letter[0], font_size);
		const Vector2 secondary_char_size = font->get_char_size(secondary_letter[0], font_size);
		const Vector2 primary_char_offset = Vector2(Math::ceil(-5.5f * _editor_scale - 0.5f * primary_char_size.width), primary_char_size.height * 0.25f);
		const Vector2 secondary_char_offset = Vector2(Math::floor(5.5f * _editor_scale - 0.5f * secondary_char_size.width), secondary_char_size.height * 0.25f);
		draw_char(font, p_axis.screen_point + primary_char_offset, primary_letter, font_size, Color(0.0f, 0.0f, 0.0f, primary_color.a * 0.6f));
		draw_char(font, p_axis.screen_point + secondary_char_offset, secondary_letter, font_size, Color(0.0f, 0.0f, 0.0f, secondary_color.a * 0.6f));
	}
}

void EditorViewportRotation4D::_draw_filled_arc(const Vector2 &p_center, real_t p_radius, real_t p_start_angle, real_t p_end_angle, const Color &p_color) {
	ERR_THREAD_GUARD;
	constexpr int ARC_POINTS = 16;
	const real_t angle_step = (p_end_angle - p_start_angle) / (ARC_POINTS - 1);
	PackedVector2Array points;
	points.resize(ARC_POINTS);
	for (int i = 0; i < ARC_POINTS; i++) {
		const real_t angle = p_start_angle + i * angle_step;
		points.set(i, p_center + Vector2(Math::cos(angle), Math::sin(angle)) * p_radius);
	}
	PackedColorArray colors = { p_color };
	draw_polygon(points, colors);
}

EditorViewportRotation4D::Axis2D EditorViewportRotation4D::_make_plane_axis(const Basis4D &p_basis, const int p_right, const int p_up, const Vector2 &p_center, const real_t p_radius) {
	Axis2D ret;
	ret.axis_type = AXIS_TYPE_PLANE;
	ret.axis_number = p_right;
	ret.secondary_axis_number = p_up;
	const Vector4 right_vec4 = p_basis[p_right];
	const Vector4 up_vec4 = p_basis[p_up];
	const Vector4 axis_4d = (right_vec4 + up_vec4).normalized();
	ret.screen_point = p_center + Vector2(axis_4d.x, -axis_4d.y) * p_radius;
	ret.angle = Vector2(right_vec4.x, -right_vec4.y).angle_to_point(Vector2(up_vec4.x, -up_vec4.y));
	ret.z_index = axis_4d.z;
	return ret;
}

void EditorViewportRotation4D::_get_sorted_axis_screen_aligned(const Basis4D &p_basis, const Vector2 &p_center, const real_t p_radius, const int p_right_index, const int p_up_index, const int p_perp_right, const int p_perp_up, Vector<Axis2D> &r_axis) {
	const Vector2 axis_right_offset = p_radius * Vector2(p_basis[p_right_index].x, -p_basis[p_right_index].y);
	const Vector2 axis_up_offset = p_radius * Vector2(p_basis[p_up_index].x, -p_basis[p_up_index].y);
	// Axis lines.
	Axis2D axis_right;
	axis_right.axis_number = p_right_index;
	axis_right.z_index = -1.0f;
	axis_right.axis_type = AXIS_TYPE_LINE;
	axis_right.screen_point = p_center + axis_right_offset;
	r_axis.push_back(axis_right);
	Axis2D axis_up;
	axis_up.axis_number = p_up_index;
	axis_up.z_index = -1.0f;
	axis_up.axis_type = AXIS_TYPE_LINE;
	axis_up.screen_point = p_center + axis_up_offset;
	r_axis.push_back(axis_up);
	// Axis circles.
	axis_right.axis_type = AXIS_TYPE_CIRCLE_POSITIVE;
	axis_right.z_index = 0.0f;
	r_axis.push_back(axis_right);
	axis_right.axis_type = AXIS_TYPE_CIRCLE_NEGATIVE;
	axis_right.screen_point = p_center - axis_right_offset;
	r_axis.push_back(axis_right);
	axis_up.axis_type = AXIS_TYPE_CIRCLE_POSITIVE;
	axis_up.z_index = 0.0f;
	r_axis.push_back(axis_up);
	axis_up.axis_type = AXIS_TYPE_CIRCLE_NEGATIVE;
	axis_up.screen_point = p_center - axis_up_offset;
	r_axis.push_back(axis_up);
	// Parallel direction.
	Axis2D axis_parallel;
	axis_parallel.axis_type = AXIS_TYPE_PLANE;
	axis_parallel.axis_number = p_right_index;
	axis_parallel.secondary_axis_number = p_up_index;
	axis_parallel.z_index = 1.0f;
	axis_parallel.angle = axis_right_offset.angle_to_point(axis_up_offset);
	const Vector4 axis_parallel_4d = (p_basis[p_right_index] + p_basis[p_up_index]).normalized();
	axis_parallel.screen_point = p_center + p_radius * Vector2(axis_parallel_4d.x, -axis_parallel_4d.y);
	r_axis.push_back(axis_parallel);
	// Perpendicular direction.
	Axis2D axis_perp;
	axis_perp.axis_type = AXIS_TYPE_PLANE;
	axis_perp.axis_number = p_perp_right;
	axis_perp.secondary_axis_number = p_perp_up;
	axis_perp.screen_point = p_center;
	axis_perp.z_index = 8.0f;
	r_axis.push_back(axis_perp);
}

void EditorViewportRotation4D::_get_sorted_axis(const Vector2 &p_center, Vector<Axis2D> &r_axis) {
	const Vector2 center = get_size() / 2.0f;
	const real_t radius = get_size().x / 2.0f - 10.0f * _editor_scale;
	const Basis4D camera_basis_transposed = _editor_main_viewport->get_view_camera_basis().transposed();
	// Special cases: Axes are aligned with the screen.
	constexpr real_t SPECIAL_CASE_THRESHOLD = CMP_EPSILON;
	if (Math::abs(camera_basis_transposed.x.x) < SPECIAL_CASE_THRESHOLD && Math::abs(camera_basis_transposed.x.y) < SPECIAL_CASE_THRESHOLD) {
		if (Math::abs(camera_basis_transposed.y.x) < SPECIAL_CASE_THRESHOLD && Math::abs(camera_basis_transposed.y.y) < SPECIAL_CASE_THRESHOLD) {
			_get_sorted_axis_screen_aligned(camera_basis_transposed, center, radius, 2, 3, 0, 1, r_axis);
			return;
		} else if (Math::abs(camera_basis_transposed.z.x) < SPECIAL_CASE_THRESHOLD && Math::abs(camera_basis_transposed.z.y) < SPECIAL_CASE_THRESHOLD) {
			_get_sorted_axis_screen_aligned(camera_basis_transposed, center, radius, 3, 1, 0, 2, r_axis);
			return;
		} else if (Math::abs(camera_basis_transposed.w.x) < SPECIAL_CASE_THRESHOLD && Math::abs(camera_basis_transposed.w.y) < SPECIAL_CASE_THRESHOLD) {
			_get_sorted_axis_screen_aligned(camera_basis_transposed, center, radius, 2, 1, 0, 3, r_axis);
			return;
		}
	} else if (Math::abs(camera_basis_transposed.y.x) < SPECIAL_CASE_THRESHOLD && Math::abs(camera_basis_transposed.y.y) < SPECIAL_CASE_THRESHOLD) {
		if (Math::abs(camera_basis_transposed.z.x) < SPECIAL_CASE_THRESHOLD && Math::abs(camera_basis_transposed.z.y) < SPECIAL_CASE_THRESHOLD) {
			_get_sorted_axis_screen_aligned(camera_basis_transposed, center, radius, 0, 3, 2, 1, r_axis);
			return;
		} else if (Math::abs(camera_basis_transposed.w.x) < SPECIAL_CASE_THRESHOLD && Math::abs(camera_basis_transposed.w.y) < SPECIAL_CASE_THRESHOLD) {
			_get_sorted_axis_screen_aligned(camera_basis_transposed, center, radius, 0, 2, 3, 1, r_axis);
			return;
		}
	} else if (Math::abs(camera_basis_transposed.z.x) < SPECIAL_CASE_THRESHOLD && Math::abs(camera_basis_transposed.z.y) < SPECIAL_CASE_THRESHOLD) {
		if (Math::abs(camera_basis_transposed.w.x) < SPECIAL_CASE_THRESHOLD && Math::abs(camera_basis_transposed.w.y) < SPECIAL_CASE_THRESHOLD) {
			_get_sorted_axis_screen_aligned(camera_basis_transposed, center, radius, 0, 1, 2, 3, r_axis);
			return;
		}
	}
	// Add axes in each direction.
	for (int i = 0; i < 4; i++) {
		const Vector4 axis_4d = camera_basis_transposed[i];
		const Vector2 axis_screen_position = Vector2(axis_4d.x, -axis_4d.y) * radius;

		if (axis_screen_position.is_zero_approx()) {
			// Special case when the axis is aligned with the camera.
			Axis2D axis;
			axis.axis_type = AXIS_TYPE_CIRCLE_POSITIVE;
			axis.axis_number = i;
			axis.screen_point = center;
			r_axis.push_back(axis);
		} else {
			Axis2D pos_axis;
			pos_axis.axis_type = AXIS_TYPE_CIRCLE_POSITIVE;
			pos_axis.axis_number = i;
			pos_axis.screen_point = center + axis_screen_position;
			pos_axis.z_index = axis_4d.z;
			r_axis.push_back(pos_axis);

			Axis2D line_axis;
			line_axis.axis_type = AXIS_TYPE_LINE;
			line_axis.axis_number = i;
			line_axis.screen_point = center + axis_screen_position;
			// Ensure the lines draw behind their connected circles.
			line_axis.z_index = MIN(axis_4d.z, 0.0f) - (float)CMP_EPSILON;
			r_axis.push_back(line_axis);

			Axis2D neg_axis;
			neg_axis.axis_type = AXIS_TYPE_CIRCLE_NEGATIVE;
			neg_axis.axis_number = i;
			neg_axis.screen_point = center - axis_screen_position;
			neg_axis.z_index = -axis_4d.z;
			r_axis.push_back(neg_axis);
		}
	}
	// Add orthogonal planes.
	r_axis.append(_make_plane_axis(camera_basis_transposed, 0, 1, center, radius));
	r_axis.append(_make_plane_axis(camera_basis_transposed, 2, 1, center, radius));
	r_axis.append(_make_plane_axis(camera_basis_transposed, 0, 2, center, radius));
	r_axis.append(_make_plane_axis(camera_basis_transposed, 0, 3, center, radius));
	r_axis.append(_make_plane_axis(camera_basis_transposed, 3, 1, center, radius));
	r_axis.append(_make_plane_axis(camera_basis_transposed, 2, 3, center, radius));
	// Sort the axes by z_index.
	r_axis.sort_custom<Axis2DCompare>();
}

void EditorViewportRotation4D::_on_mouse_exited() {
	_focused_axis.axis_number = -2;
	queue_redraw();
}

void EditorViewportRotation4D::_process_click(int p_index, Vector2 p_position, bool p_pressed) {
	if (_orbiting_mouse_button_index != -1 && _orbiting_mouse_button_index != p_index) {
		return;
	}
	if (p_pressed) {
		if (p_position.distance_to(get_size() / 2.0f) < get_size().x / 2.0f) {
			_orbiting_mouse_button_index = p_index;
		}
	} else {
		if (_focused_axis.axis_number > -1) {
			if (_focused_axis.secondary_axis_number > -1) {
				_editor_main_viewport->set_orthogonal_view_plane(Vector4::Axis(_focused_axis.axis_number), Vector4::Axis(_focused_axis.secondary_axis_number));
			} else {
				_editor_main_viewport->set_ground_view_axis(Vector4::Axis(_focused_axis.axis_number % 4));
			}
			_update_focus();
		}
		_orbiting_mouse_button_index = -1;
		if (Input::get_singleton()->get_mouse_mode() == Input::MOUSE_MODE_CAPTURED) {
			Input::get_singleton()->set_mouse_mode(Input::MOUSE_MODE_VISIBLE);
			Input::get_singleton()->warp_mouse(_orbiting_mouse_start);
		}
	}
	queue_redraw();
}

void EditorViewportRotation4D::_process_drag(Ref<InputEventWithModifiers> p_event, int p_index, Vector2 p_position) {
	if (_orbiting_mouse_button_index == p_index) {
		if (Input::get_singleton()->get_mouse_mode() == Input::MOUSE_MODE_VISIBLE) {
			Input::get_singleton()->set_mouse_mode(Input::MOUSE_MODE_CAPTURED);
			_orbiting_mouse_start = p_position;
		}
		_editor_main_viewport->navigation_orbit(p_event);
		_focused_axis.axis_number = -1;
	} else {
		_update_focus();
	}
	queue_redraw();
}

void EditorViewportRotation4D::_update_focus() {
	const Vector2 center = get_size() / 2.0f;
	const Vector2 mouse_pos = get_local_mouse_position();
	const int original_focus = _focused_axis.axis_number;
	_focused_axis = Axis2D();
	_focused_axis.z_index = -10.0f;
	if (mouse_pos.distance_to(center) < center.x) {
		_focused_axis.axis_number = -1;
	}
	Vector<Axis2D> axes;
	_get_sorted_axis(center, axes);
	for (int i = 0; i < axes.size(); i++) {
		const Axis2D &axis = axes[i];
		if (axis.z_index > _focused_axis.z_index && mouse_pos.distance_to(axis.screen_point) < 8.0f * _editor_scale) {
			_focused_axis = axis;
		}
	}

	if (_focused_axis.axis_number != original_focus) {
		queue_redraw();
	}
}

void EditorViewportRotation4D::_update_theme() {
	_editor_scale = EDSCALE;
	const real_t scaled_size = GIZMO_4D_BASE_SIZE * _editor_scale;
	set_custom_minimum_size(Vector2(scaled_size, scaled_size));
	set_offset(SIDE_RIGHT, -0.1f * scaled_size);
	set_offset(SIDE_BOTTOM, 1.1f * scaled_size);
	set_offset(SIDE_LEFT, -1.1f * scaled_size);
	set_offset(SIDE_TOP, 0.1f * scaled_size);
	_axis_colors = _editor_main_viewport->get_axis_colors();
	queue_redraw();
}

void EditorViewportRotation4D::GDEXTMOD_GUI_INPUT(const Ref<InputEvent> &p_event) {
	ERR_FAIL_COND(p_event.is_null());

	// Mouse events
	const Ref<InputEventMouseButton> mb = p_event;
	if (mb.is_valid() && mb->get_button_index() == MOUSE_BUTTON_LEFT) {
		_process_click(100, mb->get_position(), mb->is_pressed());
	}

	const Ref<InputEventMouseMotion> mm = p_event;
	if (mm.is_valid()) {
		_process_drag(mm, 100, mm->get_global_position());
	}

	// Touch events
	const Ref<InputEventScreenTouch> screen_touch = p_event;
	if (screen_touch.is_valid()) {
		_process_click(screen_touch->get_index(), screen_touch->get_position(), screen_touch->is_pressed());
	}

	const Ref<InputEventScreenDrag> screen_drag = p_event;
	if (screen_drag.is_valid()) {
		_process_drag(screen_drag, screen_drag->get_index(), screen_drag->get_position());
	}
}

void EditorViewportRotation4D::setup(EditorMainViewport4D *p_editor_main_viewport) {
	// Things that we should do in the constructor but can't in GDExtension
	// due to how GDExtension runs the constructor for each registered class.
	set_name(StringName("EditorViewportRotation4D"));
	set_anchors_and_offsets_preset(Control::PRESET_TOP_RIGHT);

	// Set up things with the arguments (not constructor things).
	_editor_main_viewport = p_editor_main_viewport;
}
