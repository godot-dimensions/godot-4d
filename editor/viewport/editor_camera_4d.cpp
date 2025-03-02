#include "editor_camera_4d.h"

#include "../../nodes/camera_4d.h"

void EditorCamera4D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_PROCESS: {
			const real_t delta = (real_t)get_process_delta_time();
			// Interpolate camera Z position for the speed and zoom level.
			const real_t zoom_inertia = EDITOR_GET("editors/3d/navigation_feel/zoom_inertia");
			const real_t current_z = _camera->get_position().z;
			const real_t target_z = _target_speed_and_zoom;
			const real_t new_z = Math::lerp(current_z, target_z, MIN((real_t)1.0f, delta / zoom_inertia));
			// For speed, use the same logic as zoom, except we want to preserve the camera's global position during freelook.
			if (Input::get_singleton()->is_mouse_button_pressed(MOUSE_BUTTON_RIGHT)) {
				const Vector4 camera_position = _camera->get_global_position();
				_camera->set_position(Vector4(0.0f, 0.0f, new_z, 0.0f));
				_target_position += camera_position - _camera->get_global_position();
				_process_freelook_movement(delta);
			} else {
				// For the orbit camera, interpolate towards the target position.
				_camera->set_position(Vector4(0.0f, 0.0f, new_z, 0.0f));
				const real_t translation_inertia = EDITOR_GET("editors/3d/navigation_feel/translation_inertia");
				set_position(get_position().lerp(_target_position, delta / translation_inertia));
			}
			_camera->set_orthographic_size(new_z);
		} break;
	}
}

void EditorCamera4D::_process_freelook_movement(const real_t p_delta) {
	Input *input = Input::get_singleton();
	Vector4 local_movement = Vector4(
			input->is_physical_key_pressed(KEY_D) - input->is_physical_key_pressed(KEY_A),
			input->is_physical_key_pressed(KEY_R) - input->is_physical_key_pressed(KEY_F),
			input->is_physical_key_pressed(KEY_S) - input->is_physical_key_pressed(KEY_W),
			input->is_physical_key_pressed(KEY_E) - input->is_physical_key_pressed(KEY_Q));
	if (input->is_key_pressed(KEY_SHIFT)) {
		local_movement *= 2.0f;
	}
	pan_camera(local_movement * p_delta);
}

void EditorCamera4D::_update_camera_auto_orthographicness() {
	if (_is_explicit_orthographic || !_is_auto_orthographic) {
		return;
	}
	constexpr real_t AUTO_ORTHOGRAPHIC_THRESHOLD = CMP_EPSILON;
	const Basis4D basis = get_basis();
	bool should_be_orthographic = false;
	if (Math::abs(basis.x.x) < AUTO_ORTHOGRAPHIC_THRESHOLD && Math::abs(basis.y.x) < AUTO_ORTHOGRAPHIC_THRESHOLD) {
		if (Math::abs(basis.x.y) < AUTO_ORTHOGRAPHIC_THRESHOLD && Math::abs(basis.y.y) < AUTO_ORTHOGRAPHIC_THRESHOLD) {
			should_be_orthographic = true;
		} else if (Math::abs(basis.x.z) < AUTO_ORTHOGRAPHIC_THRESHOLD && Math::abs(basis.y.z) < AUTO_ORTHOGRAPHIC_THRESHOLD) {
			should_be_orthographic = true;
		} else if (Math::abs(basis.x.w) < AUTO_ORTHOGRAPHIC_THRESHOLD && Math::abs(basis.y.w) < AUTO_ORTHOGRAPHIC_THRESHOLD) {
			should_be_orthographic = true;
		}
	} else if (Math::abs(basis.x.y) < AUTO_ORTHOGRAPHIC_THRESHOLD && Math::abs(basis.y.y) < AUTO_ORTHOGRAPHIC_THRESHOLD) {
		if (Math::abs(basis.x.z) < AUTO_ORTHOGRAPHIC_THRESHOLD && Math::abs(basis.y.z) < AUTO_ORTHOGRAPHIC_THRESHOLD) {
			should_be_orthographic = true;
		} else if (Math::abs(basis.x.w) < AUTO_ORTHOGRAPHIC_THRESHOLD && Math::abs(basis.y.w) < AUTO_ORTHOGRAPHIC_THRESHOLD) {
			should_be_orthographic = true;
		}
	} else if (Math::abs(basis.x.z) < AUTO_ORTHOGRAPHIC_THRESHOLD && Math::abs(basis.y.z) < AUTO_ORTHOGRAPHIC_THRESHOLD) {
		if (Math::abs(basis.x.w) < AUTO_ORTHOGRAPHIC_THRESHOLD && Math::abs(basis.y.w) < AUTO_ORTHOGRAPHIC_THRESHOLD) {
			should_be_orthographic = true;
		}
	}
	if (should_be_orthographic) {
		_camera->set_projection_type(Camera4D::PROJECTION4D_ORTHOGRAPHIC);
		_camera->set_near(-_camera->get_far());
	} else {
		_camera->set_projection_type(Camera4D::PROJECTION4D_PERSPECTIVE_4D);
		_camera->set_near(0.05);
	}
}

const Camera4D *EditorCamera4D::get_camera_readonly() const {
	return _camera;
}

double EditorCamera4D::change_speed_and_zoom(const double p_change) {
	real_t min_distance = _camera->get_near() * 4.0f;
	real_t max_distance = _camera->get_far() * 0.25f;
	if (unlikely(min_distance > max_distance)) {
		_target_speed_and_zoom = (min_distance + max_distance) * 0.5f;
	} else {
		_target_speed_and_zoom = CLAMP(_target_speed_and_zoom * p_change, min_distance, max_distance);
	}
	return _target_speed_and_zoom;
}

void EditorCamera4D::pan_camera(const Vector4 &p_pan_amount) {
	const Vector4 local_pan_offset = 2.0f * _camera->get_position().z * p_pan_amount;
	_target_position += get_basis().xform(local_pan_offset);
	set_position(_target_position);
}

void EditorCamera4D::freelook_rotate_ground_basis(const Basis4D &p_ground_basis) {
	const Vector4 camera_position = _camera->get_global_position();
	_ground_basis *= p_ground_basis;
	set_basis(_ground_basis * Basis4D::from_yz(_pitch_angle));
	_target_position += camera_position - _camera->get_global_position();
	set_position(_target_position);
	_update_camera_auto_orthographicness();
}

void EditorCamera4D::freelook_rotate_ground_basis_and_pitch(const Basis4D &p_ground_basis, const real_t p_pitch_angle) {
	const Vector4 camera_position = _camera->get_global_position();
	_ground_basis *= p_ground_basis;
	_pitch_angle = CLAMP(_pitch_angle + p_pitch_angle, -1.57, 1.57);
	set_basis(_ground_basis * Basis4D::from_yz(_pitch_angle));
	_target_position += camera_position - _camera->get_global_position();
	set_position(_target_position);
	_update_camera_auto_orthographicness();
}

void EditorCamera4D::orbit_rotate_ground_basis(const Basis4D &p_ground_basis) {
	_ground_basis *= p_ground_basis;
	set_basis(_ground_basis * Basis4D::from_yz(_pitch_angle));
	_update_camera_auto_orthographicness();
}

void EditorCamera4D::orbit_rotate_ground_basis_and_pitch(const Basis4D &p_ground_basis, const real_t p_pitch_angle) {
	_ground_basis *= p_ground_basis;
	_pitch_angle = CLAMP(_pitch_angle + p_pitch_angle, -1.57, 1.57);
	set_basis(_ground_basis * Basis4D::from_yz(_pitch_angle));
	_update_camera_auto_orthographicness();
}

void EditorCamera4D::set_ground_view_axis(const Vector4::Axis p_axis, const real_t p_yaw_angle, const real_t p_pitch_angle) {
	_pitch_angle = p_pitch_angle;
	const real_t yaw_sin = Math::sin(p_yaw_angle);
	const real_t yaw_cos = Math::cos(p_yaw_angle);
	switch (p_axis) {
		case Vector4::AXIS_X: {
			_ground_basis = Basis4D(Vector4(0, 0, -yaw_sin, yaw_cos), Vector4(0, 1, 0, 0), Vector4(0, 0, yaw_cos, yaw_sin), Vector4(-1, 0, 0, 0));
		} break;
		case Vector4::AXIS_Y: {
			_ground_basis = Basis4D(Vector4(yaw_cos, 0, -yaw_sin, 0), Vector4(0, 0, 0, 1), Vector4(yaw_sin, 0, yaw_cos, 0), Vector4(0, -1, 0, 0));
		} break;
		case Vector4::AXIS_Z: {
			_ground_basis = Basis4D(Vector4(yaw_cos, 0, 0, -yaw_sin), Vector4(0, 1, 0, 0), Vector4(yaw_sin, 0, 0, yaw_cos), Vector4(0, 0, -1, 0));
		} break;
		case Vector4::AXIS_W: {
			_ground_basis = Basis4D(Vector4(yaw_cos, 0, -yaw_sin, 0), Vector4(0, 1, 0, 0), Vector4(yaw_sin, 0, yaw_cos, 0), Vector4(0, 0, 0, 1));
		} break;
	}
	set_basis(_ground_basis * Basis4D::from_yz(_pitch_angle));
	_update_camera_auto_orthographicness();
}

void EditorCamera4D::set_orthogonal_view_plane(const Vector4::Axis p_right, const Vector4::Axis p_up) {
	ERR_FAIL_COND(p_right == p_up);
	if (p_right == Vector4::AXIS_X) {
		if (p_up == Vector4::AXIS_Y) {
			_ground_basis = Basis4D(Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0), Vector4(0, 0, 1, 0), Vector4(0, 0, 0, 1));
		} else if (p_up == Vector4::AXIS_Z) {
			_ground_basis = Basis4D(Vector4(1, 0, 0, 0), Vector4(0, 0, 1, 0), Vector4(0, -1, 0, 0), Vector4(0, 0, 0, 1));
		} else if (p_up == Vector4::AXIS_W) {
			_ground_basis = Basis4D(Vector4(1, 0, 0, 0), Vector4(0, 0, 0, 1), Vector4(0, 0, 1, 0), Vector4(0, -1, 0, 0));
		}
	} else if (p_right == Vector4::AXIS_Y) {
		if (p_up == Vector4::AXIS_X) {
			_ground_basis = Basis4D(Vector4(0, 1, 0, 0), Vector4(1, 0, 0, 0), Vector4(0, 0, -1, 0), Vector4(0, 0, 0, 1));
		} else if (p_up == Vector4::AXIS_Z) {
			_ground_basis = Basis4D(Vector4(0, 1, 0, 0), Vector4(0, 0, 1, 0), Vector4(1, 0, 0, 0), Vector4(0, 0, 0, 1));
		} else if (p_up == Vector4::AXIS_W) {
			_ground_basis = Basis4D(Vector4(0, 1, 0, 0), Vector4(0, 0, 0, 1), Vector4(0, 0, -1, 0), Vector4(-1, 0, 0, 0));
		}
	} else if (p_right == Vector4::AXIS_Z) {
		if (p_up == Vector4::AXIS_X) {
			_ground_basis = Basis4D(Vector4(0, 0, 1, 0), Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0), Vector4(0, 0, 0, 1));
		} else if (p_up == Vector4::AXIS_Y) {
			_ground_basis = Basis4D(Vector4(0, 0, 1, 0), Vector4(0, 1, 0, 0), Vector4(-1, 0, 0, 0), Vector4(0, 0, 0, 1));
		} else if (p_up == Vector4::AXIS_W) {
			_ground_basis = Basis4D(Vector4(0, 0, 1, 0), Vector4(0, 0, 0, 1), Vector4(-1, 0, 0, 0), Vector4(0, -1, 0, 0));
		}
	} else if (p_right == Vector4::AXIS_W) {
		if (p_up == Vector4::AXIS_X) {
			_ground_basis = Basis4D(Vector4(0, 0, 0, 1), Vector4(1, 0, 0, 0), Vector4(0, 0, -1, 0), Vector4(0, -1, 0, 0));
		} else if (p_up == Vector4::AXIS_Y) {
			_ground_basis = Basis4D(Vector4(0, 0, 0, 1), Vector4(0, 1, 0, 0), Vector4(0, 0, 1, 0), Vector4(-1, 0, 0, 0));
		} else if (p_up == Vector4::AXIS_Z) {
			_ground_basis = Basis4D(Vector4(0, 0, 0, 1), Vector4(0, 0, 1, 0), Vector4(1, 0, 0, 0), Vector4(0, -1, 0, 0));
		}
	}
	if (_is_auto_orthographic) {
		_camera->set_projection_type(Camera4D::PROJECTION4D_ORTHOGRAPHIC);
		_camera->set_near(-_camera->get_far());
	}
	_pitch_angle = 0.0f;
	set_basis(_ground_basis);
}

void EditorCamera4D::set_target_position(const Vector4 &p_position) {
	_target_position = p_position;
}

void EditorCamera4D::setup() {
	set_name(StringName("EditorCamera4D"));
	_camera = memnew(Camera4D);
	_camera->set_name(StringName("Camera4D"));
	_camera->set_position(Vector4(0.0f, 0.0f, 4.0f, 0.0f));
	add_child(_camera);

	orbit_rotate_ground_basis_and_pitch(Basis4D::from_zx(0.5f), -0.5f);
	set_process(true);
}
