#include "editor_main_screen_4d.h"

#include "editor_input_surface_4d.h"
#include "editor_viewport_rotation_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/editor_selection.hpp>
#include <godot_cpp/classes/editor_settings.hpp>
#include <godot_cpp/classes/v_separator.hpp>
#elif GODOT_MODULE
#include "editor/editor_data.h"
#include "editor/editor_interface.h"
#include "editor/editor_settings.h"
#include "editor/themes/editor_scale.h"
#include "scene/gui/separator.h"
#endif

Vector2 EditorMainScreen4D::_get_warped_mouse_motion(const Ref<InputEventMouseMotion> &p_ev_mouse_motion) const {
	Vector2 relative;
	if (bool(EDITOR_GET("editors/3d/navigation/warped_mouse_panning"))) {
		relative = Input::get_singleton()->warp_mouse_motion(p_ev_mouse_motion, _input_surface_4d->get_global_rect());
	} else {
		relative = p_ev_mouse_motion->get_relative();
	}
	return relative;
}

void EditorMainScreen4D::_on_button_toggled(const bool p_toggled_on, const int p_option) {
	press_menu_item(p_option);
}

void EditorMainScreen4D::_update_theme() {
	// Set the toolbar offsets. Note that these numbers are designed to match Godot's 2D and 3D editor toolbars.
	_toolbar_hbox->set_offset(Side::SIDE_LEFT, 4.0f * EDSCALE);
	_toolbar_hbox->set_offset(Side::SIDE_RIGHT, -4.0f * EDSCALE);
	_toolbar_hbox->set_offset(Side::SIDE_TOP, 0.0f);
	_toolbar_hbox->set_offset(Side::SIDE_BOTTOM, 29.5f * EDSCALE);
	_sub_viewport_container->set_offset(Side::SIDE_TOP, 33.0f * EDSCALE);
	// Set the information label offsets.
	_information_label->set_offset(SIDE_LEFT, 6.0f * EDSCALE);
	_information_label->set_offset(SIDE_RIGHT, -6.0f * EDSCALE);
	_information_label->set_offset(SIDE_TOP, -40.0f * EDSCALE);
	_information_label->set_offset(SIDE_BOTTOM, -6.0f * EDSCALE);
	// Set icons.
	_toolbar_buttons[TOOLBAR_BUTTON_SELECT]->set_icon(get_editor_theme_icon(SNAME("ToolSelect")));
	_toolbar_buttons[TOOLBAR_BUTTON_MOVE]->set_icon(get_editor_theme_icon(SNAME("ToolMove")));
	_toolbar_buttons[TOOLBAR_BUTTON_ROTATE]->set_icon(get_editor_theme_icon(SNAME("ToolRotate")));
	_toolbar_buttons[TOOLBAR_BUTTON_SCALE]->set_icon(get_editor_theme_icon(SNAME("ToolScale")));
	_toolbar_buttons[TOOLBAR_BUTTON_USE_LOCAL_TRANSFORM]->set_icon(get_editor_theme_icon(SNAME("Object")));
	// Set the axis colors.
	_axis_colors.clear();
	_axis_colors.push_back(get_theme_color(SNAME("axis_x_color"), StringName("Editor")));
	_axis_colors.push_back(get_theme_color(SNAME("axis_y_color"), StringName("Editor")));
	_axis_colors.push_back(get_theme_color(SNAME("axis_z_color"), StringName("Editor")));
	_axis_colors.push_back(Color(0.9f, 0.82f, 0.1f)); // W axis color.
}

void EditorMainScreen4D::_notification(int p_what) {
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

void EditorMainScreen4D::focus_selected_nodes() {
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

PackedColorArray EditorMainScreen4D::get_axis_colors() const {
	return _axis_colors;
}

Basis4D EditorMainScreen4D::get_view_camera_basis() const {
	return _editor_camera_4d->get_basis();
}

void EditorMainScreen4D::navigation_freelook(const Ref<InputEventWithModifiers> &p_event) {
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

void EditorMainScreen4D::navigation_orbit(const Ref<InputEventWithModifiers> &p_event) {
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

void EditorMainScreen4D::navigation_pan(const Ref<InputEventWithModifiers> &p_event) {
	Vector2 relative = _get_warped_mouse_motion(p_event);
	Vector4 pan;
	if (p_event->is_ctrl_pressed() || p_event->is_command_or_control_pressed()) {
		pan = Vector4(0.0f, 0.0f, relative.y, relative.x);
	} else {
		pan = Vector4(-relative.x, relative.y, 0.0f, 0.0f);
	}
	_editor_camera_4d->pan_camera(pan / get_size().y);
}

String _format_number(const double p_number) {
	const int decimals = MAX(0, 3 - log10(p_number));
	String number_text = String::num(p_number, decimals);
	if (number_text.length() < 3 && !number_text.contains(".")) {
		number_text += ".0";
	}
	return number_text;
}

void EditorMainScreen4D::navigation_change_speed(const double p_speed_change) {
	const double speed_and_zoom = _editor_camera_4d->change_speed_and_zoom(p_speed_change);
	set_information_text("Speed: " + _format_number(speed_and_zoom) + "m/s");
}

void EditorMainScreen4D::navigation_change_zoom(const double p_zoom_change) {
	const double speed_and_zoom = _editor_camera_4d->change_speed_and_zoom(p_zoom_change);
	set_information_text("Zoom: " + _format_number(speed_and_zoom) + "m");
}

void EditorMainScreen4D::press_menu_item(const int p_option) {
	switch (p_option) {
		case TOOLBAR_BUTTON_SELECT:
		case TOOLBAR_BUTTON_MOVE:
		case TOOLBAR_BUTTON_ROTATE:
		case TOOLBAR_BUTTON_SCALE: {
			for (int i = 0; i < TOOLBAR_BUTTON_MODE_MAX; i++) {
				_toolbar_buttons[i]->set_pressed(i == p_option);
			}
			// TODO: Implement transform gizmo.
		} break;
		case TOOLBAR_BUTTON_USE_LOCAL_TRANSFORM: {
			// TODO: Implement local coords in transform gizmo.
		} break;
	}
}

void EditorMainScreen4D::set_ground_view_axis(const Vector4::Axis p_axis) {
	_editor_camera_4d->set_ground_view_axis(p_axis);
}

void EditorMainScreen4D::set_information_text(const String &p_text, const double p_auto_hide_time) {
	_information_label->set_text(p_text);
	_information_label->show();
	_information_label_auto_hide_time = p_auto_hide_time;
	set_process(true);
}

void EditorMainScreen4D::set_orthogonal_view_plane(const Vector4::Axis p_right, const Vector4::Axis p_up) {
	_editor_camera_4d->set_orthogonal_view_plane(p_right, p_up);
}

EditorMainScreen4D::EditorMainScreen4D() {
	set_name(StringName("EditorMainScreen4D"));
	set_visible(false);
	set_v_size_flags(SIZE_EXPAND_FILL);
	set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT);
	// Setup the toolbar and tool buttons.
	_toolbar_hbox = memnew(HBoxContainer);
	_toolbar_hbox->set_anchors_and_offsets_preset(Control::PRESET_TOP_WIDE);

	add_child(_toolbar_hbox);
	_toolbar_buttons[TOOLBAR_BUTTON_SELECT] = memnew(Button);
	_toolbar_buttons[TOOLBAR_BUTTON_SELECT]->set_toggle_mode(true);
	_toolbar_buttons[TOOLBAR_BUTTON_SELECT]->set_theme_type_variation("FlatButton");
	_toolbar_buttons[TOOLBAR_BUTTON_SELECT]->set_pressed(true);
	_toolbar_buttons[TOOLBAR_BUTTON_SELECT]->set_tooltip_text(TTR("(Q) Select nodes and manipulate their position and scale."));
	_toolbar_buttons[TOOLBAR_BUTTON_SELECT]->connect(StringName("pressed"), callable_mp(this, &EditorMainScreen4D::press_menu_item).bind(TOOLBAR_BUTTON_SELECT));
	_toolbar_hbox->add_child(_toolbar_buttons[TOOLBAR_BUTTON_SELECT]);
	_toolbar_hbox->add_child(memnew(VSeparator));

	_toolbar_buttons[TOOLBAR_BUTTON_MOVE] = memnew(Button);
	_toolbar_buttons[TOOLBAR_BUTTON_MOVE]->set_toggle_mode(true);
	_toolbar_buttons[TOOLBAR_BUTTON_MOVE]->set_theme_type_variation("FlatButton");
	_toolbar_buttons[TOOLBAR_BUTTON_MOVE]->set_tooltip_text(TTR("(W) Move selected node, changing its position."));
	_toolbar_buttons[TOOLBAR_BUTTON_MOVE]->connect(StringName("pressed"), callable_mp(this, &EditorMainScreen4D::press_menu_item).bind(TOOLBAR_BUTTON_MOVE));
	_toolbar_hbox->add_child(_toolbar_buttons[TOOLBAR_BUTTON_MOVE]);

	_toolbar_buttons[TOOLBAR_BUTTON_ROTATE] = memnew(Button);
	_toolbar_buttons[TOOLBAR_BUTTON_ROTATE]->set_toggle_mode(true);
	_toolbar_buttons[TOOLBAR_BUTTON_ROTATE]->set_theme_type_variation("FlatButton");
	_toolbar_buttons[TOOLBAR_BUTTON_ROTATE]->set_tooltip_text(TTR("(E) Rotate selected node."));
	_toolbar_buttons[TOOLBAR_BUTTON_ROTATE]->connect(StringName("pressed"), callable_mp(this, &EditorMainScreen4D::press_menu_item).bind(TOOLBAR_BUTTON_ROTATE));
	_toolbar_hbox->add_child(_toolbar_buttons[TOOLBAR_BUTTON_ROTATE]);

	_toolbar_buttons[TOOLBAR_BUTTON_SCALE] = memnew(Button);
	_toolbar_buttons[TOOLBAR_BUTTON_SCALE]->set_toggle_mode(true);
	_toolbar_buttons[TOOLBAR_BUTTON_SCALE]->set_theme_type_variation("FlatButton");
	_toolbar_buttons[TOOLBAR_BUTTON_SCALE]->set_tooltip_text(TTR("(R) Scale selected node."));
	_toolbar_buttons[TOOLBAR_BUTTON_SCALE]->connect(StringName("pressed"), callable_mp(this, &EditorMainScreen4D::press_menu_item).bind(TOOLBAR_BUTTON_SCALE));
	_toolbar_hbox->add_child(_toolbar_buttons[TOOLBAR_BUTTON_SCALE]);
	_toolbar_hbox->add_child(memnew(VSeparator));

	_toolbar_buttons[TOOLBAR_BUTTON_USE_LOCAL_TRANSFORM] = memnew(Button);
	_toolbar_buttons[TOOLBAR_BUTTON_USE_LOCAL_TRANSFORM]->set_toggle_mode(true);
	_toolbar_buttons[TOOLBAR_BUTTON_USE_LOCAL_TRANSFORM]->set_theme_type_variation("FlatButton");
	_toolbar_buttons[TOOLBAR_BUTTON_USE_LOCAL_TRANSFORM]->set_tooltip_text(TTR("(T) If pressed, use the object's local transform for the gizmo. Else, transform in global space."));
	_toolbar_buttons[TOOLBAR_BUTTON_USE_LOCAL_TRANSFORM]->connect("toggled", callable_mp(this, &EditorMainScreen4D::_on_button_toggled).bind(TOOLBAR_BUTTON_USE_LOCAL_TRANSFORM));
	_toolbar_hbox->add_child(_toolbar_buttons[TOOLBAR_BUTTON_USE_LOCAL_TRANSFORM]);

	// Set up the viewport container and camera.
	_sub_viewport_container = memnew(SubViewportContainer);
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
	_input_surface_4d->set_editor_main_screen(this);
	_sub_viewport_container->add_child(_input_surface_4d);

	_viewport_rotation_4d = memnew(EditorViewportRotation4D);
	_viewport_rotation_4d->set_editor_main_screen(this);
	_input_surface_4d->add_child(_viewport_rotation_4d);
}

EditorMainScreen4D::~EditorMainScreen4D() {
	for (int i = 0; i < TOOLBAR_BUTTON_MAX; i++) {
		_toolbar_buttons[i]->queue_free();
	}
	_editor_camera_4d->queue_free();
	_information_label->queue_free();
	_input_surface_4d->queue_free();
	_toolbar_hbox->queue_free();
	_sub_viewport_container->queue_free();
	_sub_viewport->queue_free();
	_viewport_rotation_4d->queue_free();
}
