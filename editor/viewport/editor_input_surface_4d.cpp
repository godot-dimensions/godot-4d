#include "editor_input_surface_4d.h"

#include "editor_main_screen_4d.h"

void EditorInputSurface4D::GDEXTMOD_GUI_INPUT(const Ref<InputEvent> &p_event) {
	ERR_FAIL_COND(p_event.is_null());
	Ref<InputEventMouseButton> mouse_button = p_event;
	if (mouse_button.is_valid()) {
		MouseButton mouse_button_index = mouse_button->get_button_index();
		if (Input::get_singleton()->is_mouse_button_pressed(MOUSE_BUTTON_RIGHT)) {
			if (mouse_button_index == MOUSE_BUTTON_WHEEL_UP) {
				_editor_main_screen->navigation_change_speed(1.15);
			} else if (mouse_button_index == MOUSE_BUTTON_WHEEL_DOWN) {
				_editor_main_screen->navigation_change_speed(1.0 / 1.15);
			}
		} else {
			if (mouse_button_index == MOUSE_BUTTON_WHEEL_UP) {
				_editor_main_screen->navigation_change_zoom(1.0 / 1.15);
			} else if (mouse_button_index == MOUSE_BUTTON_WHEEL_DOWN) {
				_editor_main_screen->navigation_change_zoom(1.15);
			}
		}
		grab_focus();
	}
	Ref<InputEventMouseMotion> mouse_motion = p_event;
	if (mouse_motion.is_valid()) {
		BitField<MouseButtonMask> mouse_button_mask = mouse_motion->get_button_mask();
		if (mouse_button_mask.has_flag(MOUSE_BUTTON_MASK_MIDDLE)) {
			if (mouse_motion->is_shift_pressed()) {
				_editor_main_screen->navigation_pan(mouse_motion);
			} else {
				_editor_main_screen->navigation_orbit(mouse_motion);
			}
		} else if (mouse_button_mask.has_flag(MOUSE_BUTTON_MASK_RIGHT)) {
			_editor_main_screen->navigation_freelook(mouse_motion);
		} else {
			return;
		}
		grab_focus();
	}
	Ref<InputEventKey> key = p_event;
	if (key.is_valid()) {
		if (key->is_pressed()) {
			if (!Input::get_singleton()->is_mouse_button_pressed(MOUSE_BUTTON_RIGHT)) {
				if (key->get_keycode() == KEY_Q) {
					_editor_main_screen->press_menu_item(EditorMainScreen4D::TOOLBAR_BUTTON_SELECT);
				} else if (key->get_keycode() == KEY_W) {
					_editor_main_screen->press_menu_item(EditorMainScreen4D::TOOLBAR_BUTTON_MOVE);
				} else if (key->get_keycode() == KEY_E) {
					_editor_main_screen->press_menu_item(EditorMainScreen4D::TOOLBAR_BUTTON_ROTATE);
				} else if (key->get_keycode() == KEY_R) {
					_editor_main_screen->press_menu_item(EditorMainScreen4D::TOOLBAR_BUTTON_SCALE);
				} else if (key->get_keycode() == KEY_T) {
					_editor_main_screen->press_menu_item(EditorMainScreen4D::TOOLBAR_BUTTON_USE_LOCAL_TRANSFORM);
				} else if (key->get_keycode() == KEY_F) {
					_editor_main_screen->focus_selected_nodes();
				}
			}
		}
	}
}

void EditorInputSurface4D::set_editor_main_screen(EditorMainScreen4D *p_editor_main_screen) {
	_editor_main_screen = p_editor_main_screen;
}

EditorInputSurface4D::EditorInputSurface4D() {
	set_name(StringName("EditorInputSurface4D"));
	set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT);
	set_focus_mode(FOCUS_ALL);
}
