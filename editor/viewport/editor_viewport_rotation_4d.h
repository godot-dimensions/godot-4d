#pragma once

#include "editor_viewport_4d_defines.h"

// Editor viewport rotation navigation gizmo (the thing in the top right corner).
// Shows the current view rotation and allows the user to rotate the view.
// Users can drag to spin like a ball, or click on an axis to make that perpendicular to the view.
class EditorViewportRotation4D : public Control {
	GDCLASS(EditorViewportRotation4D, Control);

	static constexpr int MOUSE_SENTINEL_INDEX = 100; // Distinguish from touch events.

	enum HitType2D {
		HIT_TYPE_NONE,
		HIT_TYPE_BACKGROUND,
		HIT_TYPE_AXIS_CIRCLE_POSITIVE,
		HIT_TYPE_AXIS_CIRCLE_NEGATIVE,
		HIT_TYPE_AXIS_LINE,
		HIT_TYPE_PLANE,
	};

	struct HitTarget2D {
		Vector2 screen_point = Vector2();
		float z_index = 0.0f;
		float angle = 0.0f; // Only used by the PLANE hit type.
		HitType2D hit_type = HIT_TYPE_NONE;
		int8_t primary_axis_number = -1; // 0 to 3 for XYZW, or -1 for none.
		int8_t secondary_axis_number = -1; // 0 to 3 for XYZW. Only used by the PLANE hit type.
	};

	struct HitTarget2DCompare {
		_FORCE_INLINE_ bool operator()(const HitTarget2D &p_left, const HitTarget2D &p_right) const {
			return p_left.z_index < p_right.z_index;
		}
	};

	EditorMainViewport4D *_editor_main_viewport = nullptr;
	PackedColorArray _axis_colors;
	HitTarget2D _focused_target = HitTarget2D();
	Vector2i _orbiting_mouse_start = Vector2i();
	int _orbiting_mouse_button_index = -1;
	real_t _editor_scale = 1.0f;

	void _draw_axis_circle(const HitTarget2D &p_target);
	void _draw_axis_line(const HitTarget2D &p_target, const Vector2 &p_center);
	void _draw_plane_semicircles(const HitTarget2D &p_target);
	void _draw_filled_arc(const Vector2 &p_center, const real_t p_radius, const real_t p_start_angle, const real_t p_end_angle, const Color &p_color);
	void _get_sorted_axis_targets(const Vector2 &p_center, Vector<HitTarget2D> &r_targets);
	void _get_sorted_axis_screen_aligned(const Basis4D &p_basis, const Vector2 &p_center, const real_t p_radius, const int p_right_index, const int p_up_index, const int p_perp_right, const int p_perp_up, Vector<HitTarget2D> &r_targets);
	HitTarget2D _make_plane_target(const Basis4D &p_basis, const int p_a, const int p_b, const Vector2 &p_center, const real_t p_radius);
	void _on_mouse_exited();
	void _process_click(int p_index, Vector2 p_position, bool p_pressed);
	void _process_drag(Ref<InputEvent> p_event, int p_index, Vector2 p_position);
	void _update_focus();
	void _update_theme();

protected:
	static void _bind_methods() {}
	void _notification(int p_what);

public:
#if GDEXTENSION
	void _draw() override;
#elif GODOT_MODULE
	void _draw();
#endif

	virtual void GDEXTMOD_GUI_INPUT(const Ref<InputEvent> &p_event) override;

	void setup(EditorMainViewport4D *p_editor_main_viewport);
};
