#include "editor_viewport_rotation_4d.h"

#include "editor_main_screen_4d.h"

#if GDEXTENSION
#elif GODOT_MODULE
#include "editor/themes/editor_scale.h"
#endif

// How far apart the axis circles are from the center of the gizmo.
// Godot's 3D uses 80, but for 4D we have more axes, so we need to space them out more.
constexpr float GIZMO_BASE_SIZE = 100.0f;

String _get_axis_letter(int p_axis) {
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
			if (!is_connected(SceneStringName(mouse_exited), callable_mp(this, &EditorViewportRotation4D::_on_mouse_exited))) {
				connect(SceneStringName(mouse_exited), callable_mp(this, &EditorViewportRotation4D::_on_mouse_exited));
			}
			_update_theme();
		} break;
		case NOTIFICATION_DRAW: {
			if (_editor_main_screen != nullptr) {
				_draw();
			}
		} break;
		case NOTIFICATION_THEME_CHANGED: {
			_update_theme();
		} break;
	}
}

void EditorViewportRotation4D::_draw() {
	const real_t radius = GIZMO_BASE_SIZE * _editor_scale / 2.0f;
	const Vector2 center = Vector2(radius, radius);

	if (_focused_axis > -2 || _orbiting_mouse_button_index != -1) {
		draw_circle(center, radius, Color(0.5f, 0.5f, 0.5f, 0.25f), true, -1.0f, true);
	}

	Vector<Axis2D> axis_to_draw;
	_get_sorted_axis(axis_to_draw);
	for (int i = 0; i < axis_to_draw.size(); ++i) {
		_draw_axis(axis_to_draw[i]);
	}
}

void EditorViewportRotation4D::_draw_axis(const Axis2D &p_axis) {
	const bool focused = _focused_axis == p_axis.axis;
	const bool positive = p_axis.axis < 4;
	const int direction = p_axis.axis % 4;

	const Color axis_color = _axis_colors[direction];
	const float alpha = p_axis.z_index + 1.75f;
	const Color c = focused ? Color(axis_color.lightened(0.75f), 1.0f) : Color(axis_color, alpha);
	const real_t axis_circle_radius = 8.0f * _editor_scale;

	if (positive) {
		// Draw axis lines for the positive axes.
		const Vector2 center = get_size() / 2.0f;
		const Vector2 diff = p_axis.screen_point - center;
		const float line_length = MAX(diff.length() - axis_circle_radius - 0.5f * _editor_scale, 0.0f);

		draw_line(center + diff.limit_length(0.5f * _editor_scale), center + diff.limit_length(line_length), c, 1.5f * _editor_scale, true);

		draw_circle(p_axis.screen_point, axis_circle_radius, c, true, -1.0f, true);

		// Draw the axis letter for the positive axes.
		const String axis_letter = _get_axis_letter(direction);
		const Ref<Font> &font = get_theme_font(SNAME("rotation_control"), StringName("EditorFonts"));
		const int font_size = get_theme_font_size(SNAME("rotation_control_size"), StringName("EditorFonts"));
		const Size2 char_size = font->get_char_size(axis_letter[0], font_size);
		const Vector2 char_offset = Vector2(-char_size.width / 2.0f, char_size.height * 0.25f);
		draw_char(font, p_axis.screen_point + char_offset, axis_letter, font_size, Color(0.0f, 0.0f, 0.0f, alpha * 0.6f));
	} else {
		// Draw an outline around the negative axes.
		draw_circle(p_axis.screen_point, axis_circle_radius, c, true, -1.0f, true);
		draw_circle(p_axis.screen_point, axis_circle_radius * 0.8f, c.darkened(0.4f), true, -1.0f, true);
	}
}

void EditorViewportRotation4D::_get_sorted_axis(Vector<Axis2D> &r_axis) {
	const Vector2 center = get_size() / 2.0f;
	const real_t radius = get_size().x / 2.0f - 10.0f * _editor_scale;
	const Basis4D camera_basis = _editor_main_screen->get_view_camera_basis().transposed();

	for (int i = 0; i < 4; i++) {
		const Vector4 axis_4d = camera_basis.get_column(i);
		const Vector2 axis_screen_position = Vector2(axis_4d.x, -axis_4d.y) * radius;

		if (axis_screen_position.is_zero_approx()) {
			// Special case when the axis is aligned with the camera.
			Axis2D axis;
			axis.axis = i + (axis_4d.z <= 0 ? 0 : 4);
			axis.screen_point = center;
			r_axis.push_back(axis);
		} else {
			Axis2D pos_axis;
			pos_axis.axis = i;
			pos_axis.screen_point = center + axis_screen_position;
			pos_axis.z_index = axis_4d.z;
			r_axis.push_back(pos_axis);

			Axis2D neg_axis;
			neg_axis.axis = i + 4;
			neg_axis.screen_point = center - axis_screen_position;
			neg_axis.z_index = -axis_4d.z;
			r_axis.push_back(neg_axis);
		}
	}

	r_axis.sort_custom<Axis2DCompare>();
}

void EditorViewportRotation4D::_on_mouse_exited() {
	_focused_axis = -2;
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
		if (_focused_axis > -1) {
			_editor_main_screen->set_ground_view_axis(Vector4::Axis(_focused_axis % 4));
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
		_editor_main_screen->navigation_orbit(p_event);
		_focused_axis = -1;
	} else {
		_update_focus();
	}
	queue_redraw();
}

void EditorViewportRotation4D::_update_focus() {
	const Vector2 mouse_pos = get_local_mouse_position();
	const int original_focus = _focused_axis;
	_focused_axis = -2;

	if (mouse_pos.distance_to(get_size() / 2.0f) < get_size().x / 2.0f) {
		_focused_axis = -1;
	}

	Vector<Axis2D> axes;
	_get_sorted_axis(axes);

	for (int i = 0; i < axes.size(); i++) {
		const Axis2D &axis = axes[i];
		if (mouse_pos.distance_to(axis.screen_point) < 8.0f * _editor_scale) {
			_focused_axis = axis.axis;
		}
	}

	if (_focused_axis != original_focus) {
		queue_redraw();
	}
}

void EditorViewportRotation4D::_update_theme() {
	_editor_scale = EDSCALE;
	const real_t scaled_size = GIZMO_BASE_SIZE * _editor_scale;
	set_custom_minimum_size(Vector2(scaled_size, scaled_size));
	set_offset(SIDE_RIGHT, -0.1f * scaled_size);
	set_offset(SIDE_BOTTOM, 1.1f * scaled_size);
	set_offset(SIDE_LEFT, -1.1f * scaled_size);
	set_offset(SIDE_TOP, 0.1f * scaled_size);
	// Set the colors.
	_axis_colors.clear();
	_axis_colors.push_back(get_theme_color(SNAME("axis_x_color"), StringName("Editor")));
	_axis_colors.push_back(get_theme_color(SNAME("axis_y_color"), StringName("Editor")));
	_axis_colors.push_back(get_theme_color(SNAME("axis_z_color"), StringName("Editor")));
	_axis_colors.push_back(Color(0.9f, 0.82f, 0.1f)); // W axis color.
	queue_redraw();
}

void EditorViewportRotation4D::GDEXTMOD_GUI_INPUT(const Ref<InputEvent> &p_event) {
	ERR_FAIL_COND(p_event.is_null());

	// Mouse events
	const Ref<InputEventMouseButton> mb = p_event;
	if (mb.is_valid() && mb->get_button_index() == MouseButton::LEFT) {
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

void EditorViewportRotation4D::set_editor_main_screen(EditorMainScreen4D *p_editor_main_screen) {
	_editor_main_screen = p_editor_main_screen;
}

EditorViewportRotation4D::EditorViewportRotation4D() {
	set_anchors_and_offsets_preset(Control::PRESET_TOP_RIGHT);
}
