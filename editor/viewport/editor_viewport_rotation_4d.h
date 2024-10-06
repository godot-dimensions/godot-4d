#pragma once

#if GDEXTENSION
#include <godot_cpp/classes/control.hpp>

#define GDEXTMOD_GUI_INPUT _gui_input
#elif GODOT_MODULE
#include "scene/gui/control.h"

#define GDEXTMOD_GUI_INPUT gui_input
#endif

class EditorMainScreen4D;

// Editor viewport rotation navigation gizmo (the thing in the top right corner).
// Shows the current view rotation and allows the user to rotate the view.
// Users can drag to spin like a ball, or click on an axis to make that perpendicular to the view.
class EditorViewportRotation4D : public Control {
	GDCLASS(EditorViewportRotation4D, Control);

	struct Axis2D {
		Vector2 screen_point;
		float z_index = 0.0f;
		int axis = -1;
	};

	struct Axis2DCompare {
		_FORCE_INLINE_ bool operator()(const Axis2D &l, const Axis2D &r) const {
			return l.z_index < r.z_index;
		}
	};

	EditorMainScreen4D *_editor_main_screen = nullptr;
	Vector<Color> _axis_colors;
	Vector2i _orbiting_mouse_start;
	int _orbiting_mouse_button_index = -1;
	int _focused_axis = -2;
	real_t _editor_scale = 1.0f;

	void _draw_axis(const Axis2D &p_axis);
	void _get_sorted_axis(Vector<Axis2D> &r_axis);
	void _on_mouse_exited();
	void _process_click(int p_index, Vector2 p_position, bool p_pressed);
	void _process_drag(Ref<InputEventWithModifiers> p_event, int p_index, Vector2 p_position);
	void _update_focus();
	void _update_theme();

protected:
	void _notification(int p_what);

public:
#if GDEXTENSION
	void _draw() override;
#elif GODOT_MODULE
	void _draw();
#endif

	virtual void GDEXTMOD_GUI_INPUT(const Ref<InputEvent> &p_event) override;

	void set_editor_main_screen(EditorMainScreen4D *p_editor_main_screen);

	EditorViewportRotation4D();
};
