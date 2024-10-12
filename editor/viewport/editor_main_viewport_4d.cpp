#include "editor_main_viewport_4d.h"

#include "editor_camera_4d.h"
#include "editor_input_surface_4d.h"
#include "editor_viewport_rotation_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/editor_selection.hpp>
#elif GODOT_MODULE
#include "editor/editor_data.h"
#include "editor/editor_interface.h"
#endif

Vector2 EditorMainViewport4D::_get_warped_mouse_motion(const Ref<InputEventMouseMotion> &p_ev_mouse_motion) const {
#if GODOT_MODULE
	if (bool(EDITOR_GET("editors/3d/navigation/warped_mouse_panning"))) {
		return Input::get_singleton()->warp_mouse_motion(p_ev_mouse_motion, _input_surface_4d->get_global_rect());
	}
#endif // GODOT_MODULE
	return p_ev_mouse_motion->get_relative();
}

void EditorMainViewport4D::_update_theme() {
	// Set the information label offsets.
	_information_label->set_offset(SIDE_LEFT, 6.0f * EDSCALE);
	_information_label->set_offset(SIDE_RIGHT, -6.0f * EDSCALE);
	_information_label->set_offset(SIDE_TOP, -40.0f * EDSCALE);
	_information_label->set_offset(SIDE_BOTTOM, -6.0f * EDSCALE);
	// Set the axis colors.
	_axis_colors.clear();
	_axis_colors.push_back(get_theme_color(StringName("axis_x_color"), StringName("Editor")));
	_axis_colors.push_back(get_theme_color(StringName("axis_y_color"), StringName("Editor")));
	_axis_colors.push_back(get_theme_color(StringName("axis_z_color"), StringName("Editor")));
	_axis_colors.push_back(Color(0.9f, 0.82f, 0.1f)); // W axis color.
}

void EditorMainViewport4D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_PROCESS: {
			_information_label_auto_hide_time -= get_process_delta_time();
			if (_information_label_auto_hide_time <= 0.0) {
				_information_label->hide();
				set_process(false);
			}
		} break;
		case NOTIFICATION_ENTER_TREE: {
			_update_theme();
		} break;
		case NOTIFICATION_THEME_CHANGED: {
			_update_theme();
		} break;
	}
}

void EditorMainViewport4D::focus_selected_nodes() {
	TypedArray<Node> selected_nodes = EditorInterface::get_singleton()->get_selection()->get_selected_nodes();
	Vector4 position_sum = Vector4();
	int position_count = 0;
	for (int i = 0; i < selected_nodes.size(); i++) {
		Node4D *node_4d = Object::cast_to<Node4D>(selected_nodes[i]);
		if (node_4d != nullptr) {
			position_sum += node_4d->get_global_position();
			position_count++;
		}
	}
	if (position_count > 0) {
		_editor_camera_4d->set_target_position(position_sum / position_count);
	}
}

PackedColorArray EditorMainViewport4D::get_axis_colors() const {
	return _axis_colors;
}

Basis4D EditorMainViewport4D::get_view_camera_basis() const {
	return _editor_camera_4d->get_basis();
}

void EditorMainViewport4D::navigation_freelook(const Ref<InputEventWithModifiers> &p_event) {
	Vector2 relative = _get_warped_mouse_motion(p_event);
	const real_t degrees_per_pixel = EDITOR_GET("editors/3d/freelook/freelook_sensitivity");
	const real_t radians_per_pixel = Math::deg_to_rad(degrees_per_pixel);
	const bool invert_y_axis = EDITOR_GET("editors/3d/navigation/invert_y_axis");
	Vector2 rotation_radians = relative * radians_per_pixel;
	if (invert_y_axis) {
		rotation_radians.y = -rotation_radians.y;
	}
	if (p_event->is_ctrl_pressed() || p_event->is_command_or_control_pressed()) {
		_editor_camera_4d->freelook_rotate_ground_basis(Basis4D::from_xw(-rotation_radians.x) * Basis4D::from_zw(rotation_radians.y));
	} else {
		_editor_camera_4d->freelook_rotate_ground_basis_and_pitch(Basis4D::from_zx(-rotation_radians.x), -rotation_radians.y);
	}
	_viewport_rotation_4d->queue_redraw();
}

void EditorMainViewport4D::navigation_orbit(const Ref<InputEventWithModifiers> &p_event) {
	Vector2 relative = _get_warped_mouse_motion(p_event);
	const real_t degrees_per_pixel = EDITOR_GET("editors/3d/navigation_feel/orbit_sensitivity");
	const real_t radians_per_pixel = Math::deg_to_rad(degrees_per_pixel);
	const bool invert_y_axis = EDITOR_GET("editors/3d/navigation/invert_y_axis");
	const bool invert_x_axis = EDITOR_GET("editors/3d/navigation/invert_x_axis");
	Vector2 rotation_radians = relative * radians_per_pixel;
	if (invert_x_axis) {
		rotation_radians.x = -rotation_radians.x;
	}
	if (invert_y_axis) {
		rotation_radians.y = -rotation_radians.y;
	}
	if (p_event->is_ctrl_pressed() || p_event->is_command_or_control_pressed()) {
		_editor_camera_4d->orbit_rotate_ground_basis(Basis4D::from_xw(-rotation_radians.x) * Basis4D::from_zw(rotation_radians.y));
	} else {
		_editor_camera_4d->orbit_rotate_ground_basis_and_pitch(Basis4D::from_zx(-rotation_radians.x), -rotation_radians.y);
	}
	_viewport_rotation_4d->queue_redraw();
}

void EditorMainViewport4D::navigation_pan(const Ref<InputEventWithModifiers> &p_event) {
	Vector2 relative = _get_warped_mouse_motion(p_event);
	Vector4 pan;
	if (p_event->is_ctrl_pressed() || p_event->is_command_or_control_pressed()) {
		pan = Vector4(0.0f, 0.0f, relative.y, relative.x);
	} else {
		pan = Vector4(-relative.x, relative.y, 0.0f, 0.0f);
	}
	_editor_camera_4d->pan_camera(pan / get_size().y);
}

String _viewport_4d_format_number(const double p_number) {
	const int decimals = MAX(0, 3 - log10(p_number));
	String number_text = String::num(p_number, decimals);
	if (number_text.length() < 3 && !number_text.contains(".")) {
		number_text += ".0";
	}
	return number_text;
}

void EditorMainViewport4D::navigation_change_speed(const double p_speed_change) {
	const double speed_and_zoom = _editor_camera_4d->change_speed_and_zoom(p_speed_change);
	set_information_text("Speed: " + _viewport_4d_format_number(speed_and_zoom) + "m/s");
}

void EditorMainViewport4D::navigation_change_zoom(const double p_zoom_change) {
	const double speed_and_zoom = _editor_camera_4d->change_speed_and_zoom(p_zoom_change);
	set_information_text("Zoom: " + _viewport_4d_format_number(speed_and_zoom) + "m");
}

void EditorMainViewport4D::set_ground_view_axis(const Vector4::Axis p_axis) {
	_editor_camera_4d->set_ground_view_axis(p_axis);
	_viewport_rotation_4d->queue_redraw();
}

void EditorMainViewport4D::set_information_text(const String &p_text, const double p_auto_hide_time) {
	_information_label->set_text(p_text);
	_information_label->show();
	_information_label_auto_hide_time = p_auto_hide_time;
	set_process(true);
}

void EditorMainViewport4D::set_orthogonal_view_plane(const Vector4::Axis p_right, const Vector4::Axis p_up) {
	_editor_camera_4d->set_orthogonal_view_plane(p_right, p_up);
	_viewport_rotation_4d->queue_redraw();
}

void EditorMainViewport4D::set_editor_main_screen(EditorMainScreen4D *p_editor_main_screen) {
	_editor_main_screen = p_editor_main_screen;
}

EditorMainViewport4D::EditorMainViewport4D() {
	set_name(StringName("EditorMainViewport4D"));
	set_clip_children_mode(Control::CLIP_CHILDREN_AND_DRAW);
	set_v_size_flags(SIZE_EXPAND_FILL);
	set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT);
	// Set up the viewport container and camera.
	_sub_viewport_container = memnew(SubViewportContainer);
	_sub_viewport_container->set_clip_children_mode(Control::CLIP_CHILDREN_AND_DRAW);
	_sub_viewport_container->set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT);
	_sub_viewport_container->set_stretch(true);
	add_child(_sub_viewport_container);

	_sub_viewport = memnew(SubViewport);
	_sub_viewport_container->add_child(_sub_viewport);

	_information_label = memnew(Label);
	_information_label->set_anchors_and_offsets_preset(Control::PRESET_BOTTOM_WIDE);
	_information_label->set_vertical_alignment(VerticalAlignment::VERTICAL_ALIGNMENT_BOTTOM);
	_sub_viewport_container->add_child(_information_label);

	_editor_camera_4d = memnew(EditorCamera4D);
	_sub_viewport->add_child(_editor_camera_4d);

	// Set up the input surfaces.
	_input_surface_4d = memnew(EditorInputSurface4D);
	_input_surface_4d->set_editor_main_viewport(this);
	_sub_viewport_container->add_child(_input_surface_4d);

	_viewport_rotation_4d = memnew(EditorViewportRotation4D);
	_viewport_rotation_4d->set_editor_main_viewport(this);
	_input_surface_4d->add_child(_viewport_rotation_4d);
}
