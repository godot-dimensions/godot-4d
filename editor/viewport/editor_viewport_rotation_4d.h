#pragma once

#include "../../math/basis_4d.h"

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

	enum AxisType2D {
		AXIS_TYPE_CIRCLE_POSITIVE,
		AXIS_TYPE_CIRCLE_NEGATIVE,
		AXIS_TYPE_LINE,
		AXIS_TYPE_PLANE,
	};

	struct Axis2D {
		Vector2 screen_point;
		float z_index = 0.0f;
		float angle = 0.0f; // Only used by the PLANE axis type.
		AxisType2D axis_type = AXIS_TYPE_CIRCLE_POSITIVE;
		int axis_number = -1; // 0 to 3 for XYZW.
		int secondary_axis_number = -1; // 0 to 3 for XYZW. Only used by the PLANE axis type.
	};

	struct Axis2DCompare {
		_FORCE_INLINE_ bool operator()(const Axis2D &p_left, const Axis2D &p_right) const {
			return p_left.z_index < p_right.z_index;
		}
	};

	EditorMainScreen4D *_editor_main_screen = nullptr;
	PackedColorArray _axis_colors;
	Axis2D _focused_axis;
	Vector2i _orbiting_mouse_start;
	int _orbiting_mouse_button_index = -1;
	real_t _editor_scale = 1.0f;

	void _draw_axis_circle(const Axis2D &p_axis);
	void _draw_axis_line(const Axis2D &p_axis, const Vector2 &p_center);
	void _draw_axis_plane(const Axis2D &p_axis);
	void _draw_filled_arc(const Vector2 &p_center, const real_t p_radius, const real_t p_start_angle, const real_t p_end_angle, const Color &p_color);
	void _get_sorted_axis(const Vector2 &p_center, Vector<Axis2D> &r_axis);
	void _get_sorted_axis_screen_aligned(const Basis4D &p_basis, const Vector2 &p_center, const real_t p_radius, const int p_right_index, const int p_up_index, const int p_perp_right, const int p_perp_up, Vector<Axis2D> &r_axis);
	Axis2D _make_plane_axis(const Basis4D &p_basis, const int p_a, const int p_b, const Vector2 &p_center, const real_t p_radius);
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
