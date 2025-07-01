#pragma once

#include "editor_viewport_4d_defines.h"

#if GDEXTENSION
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/sub_viewport.hpp>
#include <godot_cpp/classes/sub_viewport_container.hpp>
#elif GODOT_MODULE
#include "scene/gui/label.h"
#include "scene/gui/subviewport_container.h"
#include "scene/main/viewport.h"
#endif

// Class for the 4D editor viewport, which there may be up to 4 of.
// Uses EditorCamera4D, EditorInputSurface4D, EditorViewportRotation4D,
// and lots of other classes to handle the 4D editor viewport.
class EditorMainViewport4D : public Control {
	GDCLASS(EditorMainViewport4D, Control);

public:
	// Keep the first items in sync with EditorTransformGizmo4D.
	enum ToolbarButton {
		TOOLBAR_BUTTON_SELECT, // 0
		TOOLBAR_BUTTON_MOVE, // 1
		TOOLBAR_BUTTON_ROTATE, // 2
		TOOLBAR_BUTTON_SCALE, // 3
		TOOLBAR_BUTTON_MODE_MAX, // 4
		TOOLBAR_BUTTON_USE_LOCAL_TRANSFORM = TOOLBAR_BUTTON_MODE_MAX, // Still 4
		TOOLBAR_BUTTON_MAX
	};

private:
	SubViewportContainer *_sub_viewport_container = nullptr;
	SubViewport *_sub_viewport = nullptr;
	Label *_information_label = nullptr;
	EditorCamera4D *_editor_camera_4d = nullptr;
	EditorInputSurface4D *_input_surface_4d = nullptr;
	EditorMainScreen4D *_editor_main_screen = nullptr;
	EditorTransformGizmo4D *_transform_gizmo_4d = nullptr;
	EditorViewportRotation4D *_viewport_rotation_4d = nullptr;

	PackedColorArray _axis_colors;
	double _information_label_auto_hide_time = 0.0;

	Vector2 _get_warped_mouse_motion(const Ref<InputEventMouseMotion> &p_ev_mouse_motion) const;
	void _on_button_toggled(const bool p_toggled_on, const int p_option);
	void _update_theme();

protected:
	static void _bind_methods() {}
	void _notification(int p_what);

public:
	void focus_selected_nodes();
	PackedColorArray get_axis_colors() const;
	Basis4D get_view_camera_basis() const;
	EditorCamera4D *get_editor_camera_4d() const { return _editor_camera_4d; }
	void navigation_freelook(const Ref<InputEventMouseMotion> &p_input_event);
	void navigation_orbit(const Ref<InputEventMouseMotion> &p_input_event);
	void navigation_pan(const Ref<InputEventMouseMotion> &p_input_event);
	void navigation_change_speed(const double p_speed_change);
	void navigation_change_zoom(const double p_zoom_change);
	void viewport_mouse_input(const Ref<InputEventMouse> &p_mouse_event);

	void set_ground_view_axis(const Vector4::Axis p_axis);
	void set_information_text(const String &p_text, const double p_auto_hide_time = 1.5);
	void set_orthogonal_view_plane(const Vector4::Axis p_right, const Vector4::Axis p_up);

	void setup(EditorMainScreen4D *p_editor_main_screen, EditorTransformGizmo4D *p_transform_gizmo_4d);
};
