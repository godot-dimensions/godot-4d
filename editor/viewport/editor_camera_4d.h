#pragma once

#include "editor_viewport_4d_defines.h"

class Camera4D;

// Node that handles navigating around the 4D editor viewport.
// It can pan, zoom, orbit, and freelook just like the 3D editor camera.
// Not actually a camera itself, but has one as a child node.
class EditorCamera4D : public Node4D {
	GDCLASS(EditorCamera4D, Node4D);

	Camera4D *_camera = nullptr;
	Basis4D _ground_basis;
	Vector4 _target_position;
	real_t _pitch_angle;

	double _target_speed_and_zoom = 4.0;
	int _zoom_failed_attempts_count = 0;
	bool _is_auto_orthographic = true;
	bool _is_explicit_orthographic = false;

	void _process_freelook_movement(const real_t p_delta);
	void _update_camera_auto_orthographicness();

protected:
	static void _bind_methods() {}
	void _notification(int p_what);

public:
	const Camera4D *get_camera_readonly() const;
	double change_speed_and_zoom(const double p_change);
	void pan_camera(const Vector4 &p_pan_amount);
	void freelook_rotate_ground_basis(const Basis4D &p_ground_basis);
	void freelook_rotate_ground_basis_and_pitch(const Basis4D &p_ground_basis, const real_t p_pitch_angle);
	void orbit_rotate_ground_basis(const Basis4D &p_ground_basis);
	void orbit_rotate_ground_basis_and_pitch(const Basis4D &p_ground_basis, const real_t p_pitch_angle);
	void set_ground_view_axis(const Vector4::Axis p_axis, const real_t p_yaw_angle = 0.5f, const real_t p_pitch_angle = -0.5f);
	void set_target_position(const Vector4 &p_position);
	void set_orthogonal_view_plane(const Vector4::Axis p_right, const Vector4::Axis p_up);

	EditorCamera4D();
};
